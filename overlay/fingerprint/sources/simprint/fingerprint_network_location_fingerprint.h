// Copyright 2024 Simprint. All rights reserved.
// 网络与定位指纹修改

#ifndef SIMPRINT_FINGERPRINT_NETWORK_LOCATION_FINGERPRINT_H_
#define SIMPRINT_FINGERPRINT_NETWORK_LOCATION_FINGERPRINT_H_

#include <string>

namespace simprint {
namespace fingerprint {

struct FingerprintConfig;

// 网络与定位指纹管理器
class NetworkLocationFingerprint {
 public:
  // 应用语言配置
  static void ApplyLanguage(const FingerprintConfig& config);

  // 应用时区配置
  static void ApplyTimezone(const FingerprintConfig& config);

  // 应用地理位置配置
  static void ApplyGeolocation(const FingerprintConfig& config);

  // 应用 WebRTC 配置
  static void ApplyWebRTC(const FingerprintConfig& config);

  // 应用 Platform 配置
  static void ApplyPlatform(const FingerprintConfig& config);

  // 应用 UserAgent 配置
  static void ApplyUserAgent(const FingerprintConfig& config);

  // 获取当前配置的语言
  static std::string GetConfiguredLanguage();

  // 获取当前配置的时区
  static std::string GetConfiguredTimezone();

  // 获取当前配置的 platform（用于 navigator.platform）
  static std::string GetConfiguredPlatform();

  // 获取当前配置的 UserAgent（用于 navigator.userAgent）
  static std::string GetConfiguredUserAgent();

  // 获取当前配置的地理位置（返回 true 表示有配置，false 表示无配置）
  static bool GetConfiguredGeolocation(double* latitude, double* longitude);

  // 获取当前配置的 WebRTC 模式
  static std::string GetConfiguredWebRTCMode();

  // 获取当前配置的 Geolocation Prompt 模式（ask/allow/forbid）
  static std::string GetConfiguredGeolocationPrompt();
};

}  // namespace fingerprint
}  // namespace simprint

#endif  // SIMPRINT_FINGERPRINT_NETWORK_LOCATION_FINGERPRINT_H_
