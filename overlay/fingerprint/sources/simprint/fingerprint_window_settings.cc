// Copyright 2024 Simprint. All rights reserved.
// 窗口设置实现

#include "simprint/fingerprint/window_settings.h"

#include "simprint/fingerprint/fingerprint_config.h"
#include "simprint/config/fingerprint_config_storage.h"
#include "base/logging.h"

#include <sstream>

#define SIMPRINT_LOG(msg) LOG(INFO) << msg

namespace simprint {
namespace fingerprint {

// static
void WindowSettings::ApplyWindowSize(const FingerprintConfig& config) {
  if (config.window_size.has_value()) {
    simprint::config::SetWindowSizeMode(config.window_size.value());
    SIMPRINT_LOG("Window Size Mode 配置已应用: " << config.window_size.value());
  }

  if (config.window_width.has_value() && config.window_height.has_value()) {
    simprint::config::SetWindowSize(config.window_width.value(), config.window_height.value());
    SIMPRINT_LOG("Window Size 配置已应用: " << config.window_width.value() << "x" << config.window_height.value());
  }
}

// static
void WindowSettings::ApplyWindowPosition(const FingerprintConfig& config) {
  if (!config.window_position.has_value()) {
    SIMPRINT_LOG("Window Position 配置未设置，跳过");
    return;
  }

  simprint::config::SetWindowPositionMode(config.window_position.value());
  SIMPRINT_LOG("Window Position Mode 配置已应用: " << config.window_position.value());

  // 如果是 custom 模式，保存 x, y 坐标
  if (config.window_position.value() == "custom") {
    if (config.window_x.has_value() && config.window_y.has_value()) {
      simprint::config::SetWindowPosition(config.window_x.value(), config.window_y.value());
      SIMPRINT_LOG("Window Position (custom) 配置已应用: x=" << config.window_x.value() << ", y=" << config.window_y.value());
    }
  }
}

// static
int WindowSettings::GetConfiguredWindowWidth() {
  int width = 0, height = 0;
  simprint::config::GetWindowSize(&width, &height);
  return width;
}

// static
int WindowSettings::GetConfiguredWindowHeight() {
  int width = 0, height = 0;
  simprint::config::GetWindowSize(&width, &height);
  return height;
}

// static
std::string WindowSettings::GetConfiguredWindowSizeMode() {
  return simprint::config::GetWindowSizeMode();
}

// static
std::string WindowSettings::GetConfiguredWindowPositionMode() {
  return simprint::config::GetWindowPositionMode();
}

// static
bool WindowSettings::GetConfiguredWindowSize(int* width, int* height) {
  simprint::config::GetWindowSize(width, height);
  return (width && *width > 0 && height && *height > 0);
}

// static
bool WindowSettings::GetConfiguredWindowPosition(int* x, int* y) {
  std::string mode = simprint::config::GetWindowPositionMode();

  if (mode.empty()) {
    return false;
  }

  if (mode == "custom") {
    simprint::config::GetWindowPosition(x, y);
    return true;
  } else if (mode == "center" || mode == "top-left" || mode == "top-right" ||
             mode == "bottom-left" || mode == "bottom-right") {
    // 这些模式返回 true，但不设置 x, y（由调用者根据模式计算位置）
    return true;
  }

  return false;
}

}  // namespace fingerprint
}  // namespace simprint
