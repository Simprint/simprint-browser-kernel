// Copyright 2024 Simprint. All rights reserved.
// 浏览器行为和设备设置实现

#include "simprint/fingerprint/behavior_device_settings.h"

#include "simprint/fingerprint/fingerprint_config.h"
#include "simprint/config/fingerprint_config_storage.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/containers/span.h"

#include <sstream>

#define SIMPRINT_LOG(msg) LOG(INFO) << msg

namespace simprint {
namespace fingerprint {

namespace {

// 生成随机设备名称
std::string GenerateRandomDeviceName() {
  // 常见的设备品牌和型号
  static constexpr const char* brands[] = {
    "Samsung", "Apple", "Huawei", "Xiaomi", "OPPO", "vivo",
    "OnePlus", "Google", "Motorola", "LG", "Sony", "Nokia"
  };

  static constexpr const char* models[] = {
    "Galaxy S21", "Galaxy S22", "Galaxy S23", "Galaxy A52",
    "iPhone 12", "iPhone 13", "iPhone 14", "iPhone 15",
    "P40", "P50", "Mate 40", "Nova 9",
    "Mi 11", "Mi 12", "Redmi Note 11", "Redmi Note 12",
    "Find X5", "Reno 8", "A96", "A78",
    "X90", "Y76s", "S16", "T2",
    "9 Pro", "10 Pro", "Nord 2", "Nord CE",
    "Pixel 6", "Pixel 7", "Pixel 8",
    "Moto G", "Edge 30", "Razr",
    "Xperia 5", "Xperia 10"
  };

  // 使用简单的伪随机选择（基于时间）
  static int counter = 0;
  counter++;

  constexpr size_t brands_count = sizeof(brands) / sizeof(brands[0]);
  constexpr size_t models_count = sizeof(models) / sizeof(models[0]);

  int brand_idx = (counter * 7) % brands_count;
  int model_idx = (counter * 13) % models_count;

  return std::string(UNSAFE_BUFFERS(brands[brand_idx])) + " " +
         std::string(UNSAFE_BUFFERS(models[model_idx]));
}

}  // namespace

// static
void BehaviorDeviceSettings::ApplyDoNotTrack(const FingerprintConfig& config) {
  if (!config.do_not_track.has_value()) {
    SIMPRINT_LOG("Do Not Track 配置未设置，跳过");
    return;
  }

  std::string dnt_value = config.do_not_track.value() ? "1" : "0";
  simprint::config::SetDoNotTrack(dnt_value);
  SIMPRINT_LOG("Do Not Track 配置已应用: " << dnt_value);
}

// static
void BehaviorDeviceSettings::StoreMediaPermissions(const FingerprintConfig& config) {
  if (config.sound.has_value()) {
    simprint::config::SetSoundPermission(config.sound.value());
    SIMPRINT_LOG("Sound 配置已存储: " << (config.sound.value() ? "enabled" : "disabled"));
  }

  if (config.images.has_value()) {
    simprint::config::SetImagesPermission(config.images.value());
    SIMPRINT_LOG("Images 配置已存储: " << (config.images.value() ? "enabled" : "disabled"));
  }

  if (config.video.has_value()) {
    simprint::config::SetVideoPermission(config.video.value());
    SIMPRINT_LOG("Video 配置已存储: " << (config.video.value() ? "enabled" : "disabled"));
  }
}

// static
bool BehaviorDeviceSettings::GetConfiguredSoundPermission() {
  auto perm = simprint::config::GetSoundPermission();
  return perm.has_value() ? perm.value() : true;
}

// static
bool BehaviorDeviceSettings::GetConfiguredImagesPermission() {
  auto perm = simprint::config::GetImagesPermission();
  return perm.has_value() ? perm.value() : true;
}

// static
bool BehaviorDeviceSettings::GetConfiguredVideoPermission() {
  auto perm = simprint::config::GetVideoPermission();
  return perm.has_value() ? perm.value() : true;
}

// static
void BehaviorDeviceSettings::ApplySSLFingerprint(const FingerprintConfig& config) {
  if (!config.ssl_fingerprint.has_value()) {
    SIMPRINT_LOG("SSL Fingerprint 配置未设置，跳过");
    return;
  }

  simprint::config::SetSSLFingerprint(config.ssl_fingerprint.value());
  SIMPRINT_LOG("SSL Fingerprint 配置已应用: " << (config.ssl_fingerprint.value() ? "enabled" : "disabled"));
}

// static
void BehaviorDeviceSettings::ApplyPortScanProtection(const FingerprintConfig& config) {
  if (!config.port_scan_protection.has_value()) {
    SIMPRINT_LOG("Port Scan Protection 配置未设置，跳过");
    return;
  }

  simprint::config::SetPortScanProtection(config.port_scan_protection.value());
  SIMPRINT_LOG("Port Scan Protection 配置已应用: " << (config.port_scan_protection.value() ? "enabled" : "disabled"));

  // 应用白名单配置
  if (config.scan_whitelist.has_value()) {
    simprint::config::SetPortScanWhitelist(config.scan_whitelist.value());
    SIMPRINT_LOG("Scan Whitelist 配置已应用: " << config.scan_whitelist.value());
  }

  // TODO: 需要在网络请求中拦截端口扫描
  // 实现位置: content/browser/renderer_host/
}

// static
void BehaviorDeviceSettings::ApplyHardwareAcceleration(const FingerprintConfig& config) {
  if (!config.hardware_acceleration.has_value()) {
    SIMPRINT_LOG("Hardware Acceleration 配置未设置，跳过");
    return;
  }

  simprint::config::SetHardwareAcceleration(config.hardware_acceleration.value());
  SIMPRINT_LOG("Hardware Acceleration 配置已应用: " << (config.hardware_acceleration.value() ? "enabled" : "disabled"));

  // 注意：硬件加速的实际禁用需要在 GpuDataManager 初始化时进行
  // 调用位置：chrome/browser/gpu/gpu_mode_manager.cc 或启动时通过命令行参数
}

// static
void BehaviorDeviceSettings::ApplyDeviceName(const FingerprintConfig& config) {
  // 检查是否启用随机设备名称
  if (config.device_name_random.has_value() && config.device_name_random.value()) {
    simprint::config::SetDeviceNameRandom(true);
    // 生成随机设备名称
    std::string random_name = GenerateRandomDeviceName();
    simprint::config::SetDeviceName(random_name);
    SIMPRINT_LOG("Device Name 随机生成: " << random_name);
    return;
  }

  simprint::config::SetDeviceNameRandom(false);

  // 使用自定义设备名称
  if (!config.device_name.has_value()) {
    SIMPRINT_LOG("Device Name 配置未设置，跳过");
    return;
  }

  simprint::config::SetDeviceName(config.device_name.value());
  SIMPRINT_LOG("Device Name 配置已应用: " << config.device_name.value());
}

// static
void BehaviorDeviceSettings::ApplyMacAddress(const FingerprintConfig& config) {
  // 检查 MAC 地址模式
  std::string mode = "custom";  // 默认为 custom
  if (config.mac_address_mode.has_value()) {
    mode = config.mac_address_mode.value();
  }

  simprint::config::SetMacAddressMode(mode);

  if (mode == "real") {
    // 使用真实 MAC 地址，清空配置
    simprint::config::SetMacAddress("");
    SIMPRINT_LOG("MAC Address 模式: real (使用真实 MAC 地址)");
    return;
  }

  // custom 模式：使用自定义 MAC 地址
  if (!config.mac_address.has_value()) {
    SIMPRINT_LOG("MAC Address 配置未设置，跳过");
    return;
  }

  simprint::config::SetMacAddress(config.mac_address.value());
  SIMPRINT_LOG("MAC Address 配置已应用: " << config.mac_address.value());

  // TODO: 需要在网络接口查询中返回自定义 MAC 地址
  // 实现位置: net/base/network_interfaces.cc
}

// static
void BehaviorDeviceSettings::ApplyStartupParameters(const FingerprintConfig& config) {
  if (!config.startup_parameters.has_value()) {
    SIMPRINT_LOG("Startup Parameters 配置未设置，跳过");
    return;
  }

  simprint::config::SetStartupParameters(config.startup_parameters.value());
  SIMPRINT_LOG("Startup Parameters 配置已应用: " << config.startup_parameters.value());

  // 注意: 启动参数通常在进程启动前设置，这里只是记录
}

// static
void BehaviorDeviceSettings::ApplySandboxSettings(const FingerprintConfig& config) {
  if (!config.disable_sandbox.has_value()) {
    SIMPRINT_LOG("Sandbox 配置未设置，跳过");
    return;
  }

  simprint::config::SetDisableSandbox(config.disable_sandbox.value());
  SIMPRINT_LOG("Sandbox 配置已应用: " << (config.disable_sandbox.value() ? "disabled" : "enabled"));
}

// static
std::string BehaviorDeviceSettings::GetConfiguredDeviceName() {
  return simprint::config::GetDeviceName();
}

// static
std::string BehaviorDeviceSettings::GetConfiguredMacAddress() {
  return simprint::config::GetMacAddress();
}

// static
std::optional<bool> BehaviorDeviceSettings::GetConfiguredDoNotTrack() {
  std::string dnt = simprint::config::GetDoNotTrack();
  if (dnt.empty()) {
    return std::nullopt;
  }
  // "1" 表示启用 DNT，其他值表示不启用
  return (dnt == "1");
}

// static
bool BehaviorDeviceSettings::ShouldBlockPort(int port) {
  return simprint::config::ShouldBlockPort(port);
}

// static
bool BehaviorDeviceSettings::GetSSLFingerprintEnabled() {
  return simprint::config::GetSSLFingerprint();
}

// static
bool BehaviorDeviceSettings::GetHardwareAccelerationEnabled() {
  return simprint::config::GetHardwareAcceleration();
}

}  // namespace fingerprint
}  // namespace simprint
