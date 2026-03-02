// Copyright 2024 Simprint. All rights reserved.
// 窗口设置

#ifndef SIMPRINT_FINGERPRINT_WINDOW_SETTINGS_H_
#define SIMPRINT_FINGERPRINT_WINDOW_SETTINGS_H_

#include <string>

namespace simprint {
namespace fingerprint {

struct FingerprintConfig;

// 窗口设置管理器
class WindowSettings {
 public:
  // 应用窗口大小配置
  static void ApplyWindowSize(const FingerprintConfig& config);

  // 应用窗口位置配置
  static void ApplyWindowPosition(const FingerprintConfig& config);

  // 获取配置的窗口宽度
  static int GetConfiguredWindowWidth();

  // 获取配置的窗口高度
  static int GetConfiguredWindowHeight();

  // 获取配置的窗口大小模式
  static std::string GetConfiguredWindowSizeMode();

  // 获取配置的窗口位置模式
  static std::string GetConfiguredWindowPositionMode();

  // 获取配置的窗口大小（返回 true 如果有配置）
  static bool GetConfiguredWindowSize(int* width, int* height);

  // 获取配置的窗口位置（返回 true 如果有配置）
  static bool GetConfiguredWindowPosition(int* x, int* y);
};

}  // namespace fingerprint
}  // namespace simprint

#endif  // SIMPRINT_FINGERPRINT_WINDOW_SETTINGS_H_
