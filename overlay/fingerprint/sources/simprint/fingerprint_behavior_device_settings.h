// Copyright 2024 Simprint. All rights reserved.
// 浏览器行为和设备设置

#ifndef SIMPRINT_FINGERPRINT_BEHAVIOR_DEVICE_SETTINGS_H_
#define SIMPRINT_FINGERPRINT_BEHAVIOR_DEVICE_SETTINGS_H_

#include <optional>
#include <string>

namespace simprint {
namespace fingerprint {

struct FingerprintConfig;

// 浏览器行为和设备设置管理器
class BehaviorDeviceSettings {
 public:
  // 应用 Do Not Track 配置
  static void ApplyDoNotTrack(const FingerprintConfig& config);

  // 存储媒体权限配置（在 FingerprintConfig 解析时调用）
  static void StoreMediaPermissions(const FingerprintConfig& config);

  // 应用 SSL 指纹配置
  static void ApplySSLFingerprint(const FingerprintConfig& config);

  // 应用端口扫描保护配置
  static void ApplyPortScanProtection(const FingerprintConfig& config);

  // 应用硬件加速配置
  static void ApplyHardwareAcceleration(const FingerprintConfig& config);

  // 应用设备名称配置
  static void ApplyDeviceName(const FingerprintConfig& config);

  // 应用 MAC 地址配置
  static void ApplyMacAddress(const FingerprintConfig& config);

  // 应用启动参数配置
  static void ApplyStartupParameters(const FingerprintConfig& config);

  // 应用沙箱配置
  static void ApplySandboxSettings(const FingerprintConfig& config);

  // 获取配置的设备名称
  static std::string GetConfiguredDeviceName();

  // 获取配置的 MAC 地址
  static std::string GetConfiguredMacAddress();

  // 获取配置的 Do Not Track
  static std::optional<bool> GetConfiguredDoNotTrack();

  // 获取媒体权限配置（供外部使用）
  static bool GetConfiguredSoundPermission();
  static bool GetConfiguredImagesPermission();
  static bool GetConfiguredVideoPermission();

  // 获取 SSL 指纹配置（是否启用扩展排列）
  static bool GetSSLFingerprintEnabled();

  // 获取硬件加速配置
  static bool GetHardwareAccelerationEnabled();

  // 检查是否应该阻止指定端口（端口扫描防护）
  static bool ShouldBlockPort(int port);
};

}  // namespace fingerprint
}  // namespace simprint

#endif  // SIMPRINT_FINGERPRINT_BEHAVIOR_DEVICE_SETTINGS_H_
