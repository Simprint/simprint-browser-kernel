// Copyright 2024 Simprint. All rights reserved.
// 屏幕与硬件指纹修改实现

#include "simprint/fingerprint/screen_hardware_fingerprint.h"

#include "simprint/fingerprint/fingerprint_config.h"
#include "simprint/config/fingerprint_config_storage.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/strings/string_number_conversions.h"

#include <sstream>

#define SIMPRINT_LOG(msg) LOG(INFO) << msg

namespace simprint {
namespace fingerprint {

// static
void ScreenHardwareFingerprint::ApplyResolution(const FingerprintConfig& config) {
  if (!config.resolution.has_value()) {
    SIMPRINT_LOG("Resolution 配置未设置，跳过");
    return;
  }

  simprint::config::SetResolution(config.resolution.value());
  SIMPRINT_LOG("Resolution 配置已应用: " << config.resolution.value());
}

// static
void ScreenHardwareFingerprint::ApplyColorDepth(const FingerprintConfig& config) {
  if (!config.color_depth.has_value()) {
    SIMPRINT_LOG("ColorDepth 配置未设置，跳过");
    return;
  }

  simprint::config::SetColorDepth(config.color_depth.value());
  SIMPRINT_LOG("ColorDepth 配置已应用: " << config.color_depth.value());
}

// static
void ScreenHardwareFingerprint::ApplyDevicePixelRatio(const FingerprintConfig& config) {
  if (!config.device_pixel_ratio.has_value()) {
    SIMPRINT_LOG("DevicePixelRatio 配置未设置，跳过");
    return;
  }

  simprint::config::SetDevicePixelRatio(config.device_pixel_ratio.value());
  SIMPRINT_LOG("DevicePixelRatio 配置已应用: " << config.device_pixel_ratio.value());
}

// static
void ScreenHardwareFingerprint::ApplyMaxTouchPoints(const FingerprintConfig& config) {
  if (!config.max_touch_points.has_value()) {
    SIMPRINT_LOG("MaxTouchPoints 配置未设置，跳过");
    return;
  }

  simprint::config::SetMaxTouchPoints(config.max_touch_points.value());
  SIMPRINT_LOG("MaxTouchPoints 配置已应用: " << config.max_touch_points.value());
}

// static
void ScreenHardwareFingerprint::ApplyHardwareConcurrency(const FingerprintConfig& config) {
  if (!config.hardware_concurrency.has_value()) {
    SIMPRINT_LOG("HardwareConcurrency 配置未设置，跳过");
    return;
  }

  simprint::config::SetHardwareConcurrency(config.hardware_concurrency.value());
  SIMPRINT_LOG("HardwareConcurrency 配置已应用: " << config.hardware_concurrency.value());
}

// static
void ScreenHardwareFingerprint::ApplyDeviceMemory(const FingerprintConfig& config) {
  if (!config.device_memory.has_value()) {
    SIMPRINT_LOG("DeviceMemory 配置未设置，跳过");
    return;
  }

  simprint::config::SetDeviceMemory(config.device_memory.value());
  SIMPRINT_LOG("DeviceMemory 配置已应用: " << config.device_memory.value());
}

// static
std::string ScreenHardwareFingerprint::GetConfiguredResolution() {
  return simprint::config::GetResolution();
}

// static
int ScreenHardwareFingerprint::GetConfiguredScreenWidth() {
  int width, height;
  simprint::config::GetResolutionWidthHeight(&width, &height);
  return width;
}

// static
int ScreenHardwareFingerprint::GetConfiguredScreenHeight() {
  int width, height;
  simprint::config::GetResolutionWidthHeight(&width, &height);
  return height;
}

// static
int ScreenHardwareFingerprint::GetConfiguredColorDepth() {
  return simprint::config::GetColorDepth();
}

// static
double ScreenHardwareFingerprint::GetConfiguredDevicePixelRatio() {
  return simprint::config::GetDevicePixelRatio();
}

// static
int ScreenHardwareFingerprint::GetConfiguredMaxTouchPoints() {
  return simprint::config::GetMaxTouchPoints();
}

// static
int ScreenHardwareFingerprint::GetConfiguredHardwareConcurrency() {
  return simprint::config::GetHardwareConcurrency();
}

// static
double ScreenHardwareFingerprint::GetConfiguredDeviceMemory() {
  return simprint::config::GetDeviceMemory();
}

}  // namespace fingerprint
}  // namespace simprint
