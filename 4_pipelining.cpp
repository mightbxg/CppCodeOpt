#include <chrono>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
using namespace chrono;

// clang-format off
// 这段代码是 OpenCV 中大津法 (OTSU) 求二值化阈值算法的一部分，
// 执行的操作是先计算图像的灰度直方图，然后把像素值乘频数相加，得到图像像素值总和。
// 由于图像中相邻像素很可能有相同的像素值，即 h[src[j + ?]] 很可能指向同一个内存单元,
// 这种情况下循环中前后两条指令就无法利用流水线，只能串行执行。
// 但如果使用 4 个 buffer，则可以确保循环中 4 条语句使用不同的内存单元,
// 前一条语句进行加法操作时，后一条语句可以同时进行数据读取操作。
// clang-format on

template <bool ENABLE_UNROLL>
static double pixCount(const Mat& image) {
  constexpr int N = 256;  // bin size
  constexpr int buff_size = ENABLE_UNROLL ? N * 4 : N;
  int i, j;
  std::array<int, buff_size> hBuf = {};
  memset(hBuf.data(), 0, hBuf.size() * sizeof(int));
  int* h = hBuf.data();
  int* h_unrolled[3] = {h + N, h + 2 * N, h + 3 * N};
  for (i = 0; i < image.rows; i++) {
    const auto* src = image.ptr<uchar>(i, 0);
    j = 0;
    for (; j <= image.cols - 4; j += 4) {
      h[src[j]]++;
      if constexpr (ENABLE_UNROLL) {
        h_unrolled[0][src[j + 1]]++;
        h_unrolled[1][src[j + 2]]++;
        h_unrolled[2][src[j + 3]]++;
      } else {
        h[src[j + 1]]++;
        h[src[j + 2]]++;
        h[src[j + 3]]++;
      }
    }
    for (; j < image.cols; j++) h[src[j]]++;
  }

  double mu = 0;
  for (i = 0; i < N; i++) {
    if constexpr (ENABLE_UNROLL) {
      h[i] += h_unrolled[0][i] + h_unrolled[1][i] + h_unrolled[2][i];
    }
    mu += i * (double)h[i];
  }
  return mu;
}

int main(int argc, char** argv) {
  cout << std::fixed << std::setprecision(4);
  Mat image(2000, 2000, CV_8UC1, cv::Scalar::all(100));
  auto TimeTest = [&] {
    auto time_start = system_clock::now();
    auto res = pixCount<false>(image);
    auto time_cost = duration<double>(system_clock::now() - time_start).count();
    cout << "  no pipelining: " << time_cost * 1e3 << " ms  result: " << res
         << "\n";
    time_start = system_clock::now();
    res = pixCount<true>(image);
    time_cost = duration<double>(system_clock::now() - time_start).count();
    cout << "with pipelining: " << time_cost * 1e3 << " ms  result: " << res
         << "\n";
  };

  std::cout << "unified value image ------------------------------------\n";
  TimeTest();

  cv::randu(image, cv::Scalar::all(0), cv::Scalar::all(200));
  std::cout << " random value image ------------------------------------\n";
  TimeTest();
}
