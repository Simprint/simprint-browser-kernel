// Copyright 2024 Simprint. All rights reserved.
// 网络与定位指纹修改实现

#include "simprint/fingerprint/network_location_fingerprint.h"

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
void NetworkLocationFingerprint::ApplyLanguage(const FingerprintConfig& config) {
  LOG(INFO) << "[Simprint Language] ApplyLanguage called, has_value: "
            << config.language.has_value();

  if (config.language.has_value()) {
    LOG(INFO) << "[Simprint Language] ApplyLanguage setting language to: "
              << config.language.value();
    simprint::config::SetLanguage(config.language.value());
    SIMPRINT_LOG("Language 配置已应用: " << config.language.value());
  } else {
    LOG(INFO) << "[Simprint Language] ApplyLanguage - language not set in config, skipping";
  }

  // 注意: navigator.language 的拦截已在 navigator_language.cc 中实现
  // interface_language 主要影响浏览器 UI 语言，可能需要在启动时通过命令行参数设置
}

// static
void NetworkLocationFingerprint::ApplyTimezone(const FingerprintConfig& config) {
  if (!config.timezone.has_value()) {
    SIMPRINT_LOG("Timezone 配置未设置，跳过");
    return;
  }

  simprint::config::SetTimezone(config.timezone.value());
  SIMPRINT_LOG("Timezone 配置已应用: " << config.timezone.value());

  // TODO: 需要设置 ICU 时区
  // 实现位置: base/i18n/icu_util.cc 或通过环境变量 TZ
}

// static
void NetworkLocationFingerprint::ApplyGeolocation(const FingerprintConfig& config) {
  if (!config.geolocation.has_value()) {
    SIMPRINT_LOG("Geolocation 配置未设置，跳过");
    return;
  }

  const std::string& geo_config = config.geolocation.value();
  simprint::config::SetGeolocation(geo_config);
  SIMPRINT_LOG("Geolocation 配置已应用: " << geo_config);

  // 应用 geolocationPrompt 配置
  if (config.geolocation_prompt.has_value()) {
    simprint::config::SetGeolocationPrompt(config.geolocation_prompt.value());
    SIMPRINT_LOG("Geolocation Prompt 配置已应用: " << config.geolocation_prompt.value());
  }
}

// static
void NetworkLocationFingerprint::ApplyWebRTC(const FingerprintConfig& config) {
  if (!config.webrtc.has_value()) {
    SIMPRINT_LOG("WebRTC 配置未设置，跳过");
    return;
  }

  simprint::config::SetWebRTCMode(config.webrtc.value());
  SIMPRINT_LOG("WebRTC 配置已应用: " << config.webrtc.value());

  // WebRTC IP 泄露防护已在 rtc_peer_connection_handler.cc 中实现
  // 可能的模式: "disable_non_proxied" (只允许relay), "disable" (完全禁用), "real" (真实)
}

// static
void NetworkLocationFingerprint::ApplyPlatform(const FingerprintConfig& config) {
  if (!config.platform.has_value()) {
    SIMPRINT_LOG("Platform 配置未设置，跳过");
    return;
  }

  simprint::config::SetPlatform(config.platform.value());
  SIMPRINT_LOG("Platform 配置已应用: " << config.platform.value());
}

// static
void NetworkLocationFingerprint::ApplyUserAgent(const FingerprintConfig& config) {
  if (!config.user_agent.has_value()) {
    SIMPRINT_LOG("UserAgent 配置未设置，跳过");
    return;
  }

  simprint::config::SetUserAgent(config.user_agent.value());
  SIMPRINT_LOG("UserAgent 配置已应用: " << config.user_agent.value());
}

// static
std::string NetworkLocationFingerprint::GetConfiguredLanguage() {
  return simprint::config::GetLanguage();
}

// static
std::string NetworkLocationFingerprint::GetConfiguredTimezone() {
  return simprint::config::GetTimezone();
}

// static
std::string NetworkLocationFingerprint::GetConfiguredPlatform() {
  return simprint::config::GetPlatform();
}

// static
std::string NetworkLocationFingerprint::GetConfiguredUserAgent() {
  std::string ua = simprint::config::GetUserAgent();
  LOG(INFO) << "GetConfiguredUserAgent() called, returning: '" << ua << "' (length: " << ua.length() << ")";
  return ua;
}

// static
bool NetworkLocationFingerprint::GetConfiguredGeolocation(double* latitude, double* longitude) {
  if (!latitude || !longitude) {
    return false;
  }

  std::string geo_config = simprint::config::GetGeolocation();
  if (geo_config.empty()) {
    return false;
  }

  // 解析格式: "latitude,longitude"
  size_t comma_pos = geo_config.find(',');
  if (comma_pos != std::string::npos) {
    std::string lat_str = geo_config.substr(0, comma_pos);
    std::string lon_str = geo_config.substr(comma_pos + 1);

    double lat, lon;
    if (base::StringToDouble(lat_str, &lat) && base::StringToDouble(lon_str, &lon)) {
      *latitude = lat;
      *longitude = lon;
      return true;
    }
  }

  return false;
}

// static
std::string NetworkLocationFingerprint::GetConfiguredWebRTCMode() {
  return simprint::config::GetWebRTCMode();
}

// static
std::string NetworkLocationFingerprint::GetConfiguredGeolocationPrompt() {
  return simprint::config::GetGeolocationPrompt();
}

}  // namespace fingerprint
}  // namespace simprint
