#include <chrono>
#include <iostream>
#include <random>

// clang-format off
// float 代替 double 主要有两个好处：
//   1. float 字节数只有 double 的一般，因此可以节省内存并降低网络传输的带宽需求
//   2. 编译器会自动进行指令集加速，同一时间能进行的 float 计算数量是 double 的两倍
// 但 float 有效精度远低于 double，使用时需注意数值归一化
// clang-format on

template <typename Func>
double benchmark(unsigned samples, unsigned iterations, Func func) {
  using namespace std::chrono;
  samples = std::max(samples, 1u);
  iterations = std::max(iterations, 1u);
  double best_time = std::numeric_limits<double>::infinity();
  for (unsigned s = 0; s < samples; ++s) {
    auto t1 = high_resolution_clock::now();
    for (unsigned i = 0; i < iterations; ++i) func();
    auto t2 = high_resolution_clock::now();
    best_time = std::min(best_time, duration<double>(t2 - t1).count());
  }
  best_time = best_time / iterations;
  return best_time;
}

template <typename T>
std::vector<T> GenRandNums(size_t n) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<T> dist(0.0, 100.0);
  std::vector<T> ret;
  ret.reserve(n);
  for (size_t i = 0; i < n; ++i) ret.emplace_back(dist(gen));
  return ret;
}

template <typename T>
void VecSquare(const std::vector<T>& input, std::vector<T>& output) {
  output.resize(input.size());
  for (size_t i = 0; i < input.size(); ++i) output[i] = input[i] * input[i];
}

int main() {
  const size_t N = (1 << 14);  // 16384
  const unsigned samples = 5;
  const unsigned iter = 10000;

  auto data_float = GenRandNums<float>(N);
  decltype(data_float) res_float(data_float.size());
  auto time_float =
      benchmark(samples, iter, [&] { VecSquare(data_float, res_float); });
  std::cout << "time cost of  float: " << time_float * 1e6 << " us\n";

  auto data_double = GenRandNums<double>(N);
  decltype(data_double) res_double(data_double.size());
  auto time_double =
      benchmark(samples, iter, [&] { VecSquare(data_double, res_double); });
  std::cout << "time cost of double: " << time_double * 1e6 << " us\n";
}