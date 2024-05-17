#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// CRTP (Curiously Recurring Template Pattern) 奇异递归模板模式，静态多态
//   1. 函数调用不需要查询虚表，能提高执行效率
//   2. 基类中可以以派生类的身份调用派生类的函数
//   3. 基类可以给派生类提供与派生类型相关的公用函数

// https://en.cppreference.com/w/cpp/language/crtp
// 基本格式：
template <class Derived>
struct Base {
  void name() { (static_cast<Derived*>(this))->impl(); }
};
struct D1 : public Base<D1> {
  void impl() { std::puts("D1::impl()"); }
};
struct D2 : public Base<D2> {
  void impl() { std::puts("D2::impl()"); }
};

void test_name() {
  std::cout << "test_name -------------------------\n";
  D1 d1;
  d1.name();
  D2 d2;
  d2.name();
}

/*****************************************************************************/

// 基类中可以以派生类的身份调用派生类的函数
template <typename Derived>
class EnableOperators {
 public:
  bool operator>(const EnableOperators& rhs) const {
    return rhs.Impl() < this->Impl();
  }
  bool operator==(const EnableOperators& rhs) const {
    auto& d1 = this->Impl();
    auto& d2 = rhs.Impl();
    return !(d1 < d2) && !(d2 < d1);
  }

 private:
  const auto& Impl() const { return *static_cast<const Derived*>(this); }
};

class MyPoint : public EnableOperators<MyPoint> {
 public:
  MyPoint() = default;
  MyPoint(int _x, int _y) : x(_x), y(_y) {}
  bool operator<(const MyPoint& rhs) const {
    return x < rhs.x || (x == rhs.x && y < rhs.y);
  }
  int x{0}, y{0};
};

void test_operator() {
  std::cout << "test_operator ---------------------\n";
  MyPoint pt1(1, 1), pt2(1, 2);
  std::cout << std::boolalpha << (pt1 < pt2) << " " << (pt1 > pt2) << " "
            << (pt1 == pt2) << "\n";
}

/*****************************************************************************/
// 基类可以给派生类提供与派生类型相关的公用函数
// https://stackoverflow.com/a/47514417/8263383

struct Node;
void ProcessNode(const std::shared_ptr<Node>&) {}

struct Node : std::enable_shared_from_this<Node> {
  std::weak_ptr<Node> parent;
  std::vector<std::shared_ptr<Node>> children;

  void add_child(std::shared_ptr<Node> child) {
    ProcessNode(shared_from_this());   // 这里不能直接使用 this
    child->parent = weak_from_this();  // 这里不能直接使用 this
    children.push_back(std::move(child));
  }
};

/*****************************************************************************/
// clang-format off
// 一个实际的例子：异步任务处理模块
//   1. 有很多种不同类型的数据，对每一种数据单开一个线程进行处理
//   2. 不同数据的处理方式不同
//   3. 同一种数据可能由多个不同线程提供，按提供的时间顺序先后在同一个异步线程中处理 (单例)
// clang-format on

template <typename Derived, typename DataType_>
class TaskBase {
 public:
  using DataType = DataType_;
  TaskBase() : stopped_(false), worker_(&TaskBase::Work, this) {}
  ~TaskBase() { Stop(); }
  void Stop() {
    stopped_ = true;
    cv_.notify_all();
    if (worker_.joinable()) worker_.join();
  }

  static auto& Instance() {  // 基类提供与派生类型相关的公用函数
    static TaskBase instance;
    return instance;
  }

  template <typename... Args>
  void Feed(Args&&... args) {  // 基类提供与派生类型相关的公用函数
    std::scoped_lock lock(data_buf_mutex_);
    while (data_buf_.size() > buf_capacity) data_buf_.pop_front();
    data_buf_.emplace_back(std::forward<Args>(args)...);
    cv_.notify_one();
  }

 private:
  void Work() {
    while (!stopped_) {
      std::unique_lock lock(data_buf_mutex_);
      cv_.wait(lock, [&] { return !data_buf_.empty() || stopped_; });
      decltype(data_buf_) data_lst;
      std::swap(data_lst, data_buf_);
      lock.unlock();
      while (!data_lst.empty()) {
        // 基类中以派生类的身份调用派生类的函数
        static_cast<Derived*>(this)->WorkOnce(data_lst.front());
        data_lst.pop_front();
      }
    }
  }

 private:
  std::atomic_bool stopped_;
  std::thread worker_;
  std::condition_variable cv_;

  const std::size_t buf_capacity = 10;
  std::deque<DataType> data_buf_;
  mutable std::mutex data_buf_mutex_;
};

struct DataA {
  DataA() = default;
  explicit DataA(int v) : val(v) {}
  int val{0};
};

class DataATask : public TaskBase<DataATask, DataA> {
 public:
  using Base = TaskBase<DataATask, DataA>;
  using Base::Base;
  friend Base;

 private:
  static void WorkOnce(const DataA& data) {
    std::cout << "DataA: " << data.val << std::endl;
  }
};

struct DataB {
  DataB() = default;
  DataB(int _x, int _y) : x(_x), y(_y) {}
  int x{0}, y{0};
};

class DataBTask : public TaskBase<DataBTask, DataB> {
 public:
  using Base = TaskBase<DataBTask, DataB>;
  using Base::Base;
  friend Base;

 private:
  static void WorkOnce(const DataB& data) {
    std::cout << "DataB: " << data.x << ", " << data.y << std::endl;
    DataATask::Instance().Feed(data.x + data.y);
  }
};

void test_task() {
  using namespace std::chrono_literals;
  std::cout << "test_task -------------------------\n";
  DataATask::Instance().Feed(2);
  DataBTask::Instance().Feed(4, 5);
  std::this_thread::sleep_for(0.1s);  // wait for task finish
}

int main() {
  test_name();
  test_operator();
  test_task();
}