#include <iostream>

// 枚举类型中每个枚举变量的值都可以人为指定，但并不意味着它们可以随意定，
// 良好的枚举值设计能极大简化相关逻辑判断。

namespace bad {

// 假设有一个枚举类型用来表示定位模块的融合状态，即用一个枚举变量描述当前定位滤波器是否融合了
// 轮速、GNSS 或 MapMatch，比较粗暴的设计为：
enum FusionStatus {
  kNothingFused = 0,
  kOnlyWheelFused = 1,
  kOnlyGnssFused = 2,
  kOnlyMatchFused = 3,
  kWheelGnssFused = 4,
  kWheelMatchFused = 5,
  kGnssMatchFused = 6,
  kWheelGnssMatchFused = 7,
};

// 显然，随着传感器的增多，需要枚举的组合方式急剧增多，而这同时还让下游使用非常繁琐：
void DoSomething(FusionStatus fusion_status) {
  // 用法1：遍历每一种组合方式
  switch (fusion_status) {
    case kNothingFused:
      // ...
      break;
    case kOnlyWheelFused:
      // ...
      break;
      // ...
    default:
      // ...
      break;
  }

  // 用法2：手动挑选出融合了指定传感器的枚举类型
  if (fusion_status == kOnlyWheelFused || fusion_status == kWheelGnssFused ||
      fusion_status == kWheelMatchFused) {
    // 如果融了轮速，则...
  } else if (fusion_status == kOnlyGnssFused ||
             fusion_status == kWheelGnssFused ||
             fusion_status == kGnssMatchFused) {
    // 如果融了 GNSS，则...
  }  // else if ...
}

}  // namespace bad

namespace good {

// 这种明显有组合关系的枚举值一般采用比特位的设计思路，即：
enum FusionSource {
  kWheel = 1,  // 0b0001
  kGnss = 2,   // 0b0010
  kMatch = 4,  // 0b0100
};

void DoSomething(uint8_t fusion_status) {
  if (fusion_status & FusionSource::kWheel) {
    // 如果融了轮速，则...
  } else if (fusion_status & FusionSource::kGnss) {
    // 如果融了 GNSS，则...
  } else if (fusion_status & FusionSource::kMatch) {
    // 如果融了 MapMatch，则...
  }
}

}  // namespace good

int main() {
  bad::DoSomething(bad::FusionStatus::kWheelGnssFused);
  good::DoSomething(good::kWheel | good::kGnss);
}