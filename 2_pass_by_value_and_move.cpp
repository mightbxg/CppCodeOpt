#include <iostream>
#include <string>
#include <vector>

class MyClass {
 public:
  // MyClass(const std::string& name) : name_(name) {}
  MyClass(std::string name) : name_(std::move(name)) {}  // better

 private:
  std::string name_;
};

struct A {
  A(int n) : data(n) {
    std::cout << "A constructed with " << data.size() << " elems" << std::endl;
  }
  A(const A& rhs) : data(rhs.data) {
    std::cout << "A copy constructed" << std::endl;
  }
  A(A&& rhs) : data(std::move(rhs.data)) {
    std::cout << "A move constructed" << std::endl;
  }
  ~A() {
    if (!data.empty())
      std::cout << "A dtor freed " << data.size() << " elems" << std::endl;
  }
  A& operator=(const A& rhs) {
    data = rhs.data;
    std::cout << "A copy assigned" << std::endl;
    return *this;
  }
  A& operator=(A&& rhs) {
    data = std::move(rhs.data);
    std::cout << "A move assigned" << std::endl;
    return *this;
  }

  std::vector<int> data;
};

struct B {
  B(const A& a) : a_(a) {}
  A a_;
};

struct C {
  C(A a) : a_(std::move(a)) {}  // 少一次拷贝和释放内存
  A a_;
};

int main() {
  A a(2);
  std::cout << "\n已有对象作为构造函数参数:\n";
  B b1(a);
  std::cout << "-------------------------------\n";
  C c1(a);

  std::cout << "\n原地构造对象作为函数构造函数参数:\n";
  B b2(3);
  std::cout << "-------------------------------\n";
  C c2(3);

  std::cout << "\n\33[33mend\33[0m\n";
}