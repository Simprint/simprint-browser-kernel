// Copyright 2024 Simprint. All rights reserved.
// 屏幕与硬件指纹修改

#ifndef SIMPRINT_FINGERPRINT_SCREEN_HARDWARE_FINGERPRINT_H_
#define SIMPRINT_FINGERPRINT_SCREEN_HARDWARE_FINGERPRINT_H_

#include <string>

namespace simprint {
namespace fingerprint {

struct FingerprintConfig;

// 屏幕与硬件指纹管理器
class ScreenHardwareFingerprint {
 public:
  // 应用屏幕分辨率配置
  static void ApplyResolution(const FingerprintConfig& config);

  // 应用色深配置
  static void ApplyColorDepth(const FingerprintConfig& config);

  // 应用设备像素比配置
  static void ApplyDevicePixelRatio(const FingerprintConfig& config);

  // 应用最大触摸点数配置
  static void ApplyMaxTouchPoints(const FingerprintConfig& config);

  // 应用硬件并发数配置
  static void ApplyHardwareConcurrency(const FingerprintConfig& config);

  // 应用设备内存配置
  static void ApplyDeviceMemory(const FingerprintConfig& config);

  // 获取配置的分辨率
  static std::string GetConfiguredResolution();

  // 获取配置的屏幕宽度（从分辨率字符串解析）
  static int GetConfiguredScreenWidth();

  // 获取配置的屏幕高度（从分辨率字符串解析）
  static int GetConfiguredScreenHeight();

  // 获取配置的色深
  static int GetConfiguredColorDepth();

  // 获取配置的设备像素比
  static double GetConfiguredDevicePixelRatio();

  // 获取配置的最大触摸点数
  static int GetConfiguredMaxTouchPoints();

  // 获取配置的硬件并发数
  static int GetConfiguredHardwareConcurrency();

  // 获取配置的设备内存
  static double GetConfiguredDeviceMemory();
};

}  // namespace fingerprint
}  // namespace simprint

#endif  // SIMPRINT_FINGERPRINT_SCREEN_HARDWARE_FINGERPRINT_H_
