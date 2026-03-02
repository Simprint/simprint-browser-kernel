// Copyright 2024 Simprint. All rights reserved.
// 指纹配置管理实现

#include "simprint/fingerprint/fingerprint_config.h"
#include "simprint/fingerprint/network_location_fingerprint.h"
#include "simprint/fingerprint/screen_hardware_fingerprint.h"
#include "simprint/fingerprint/browser_fingerprint.h"
#include "simprint/fingerprint/behavior_device_settings.h"
#include "simprint/fingerprint/window_settings.h"
#include "simprint/config/fingerprint_config_storage.h"

#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/values.h"

#include <sstream>

// 统一输出到 Chromium 日志 + 当前标签页 DevTools Console
#define SIMPRINT_LOG(msg) LOG(INFO) << msg

namespace simprint {
namespace fingerprint {

// FingerprintConfig 构造函数和析构函数
FingerprintConfig::FingerprintConfig() = default;
FingerprintConfig::~FingerprintConfig() = default;

// FingerprintConfigManager 构造函数和析构函数
FingerprintConfigManager::FingerprintConfigManager() = default;
FingerprintConfigManager::~FingerprintConfigManager() = default;

// static
FingerprintConfigManager* FingerprintConfigManager::GetInstance() {
  static base::NoDestructor<FingerprintConfigManager> instance;
  return instance.get();
}

// static
FingerprintMode FingerprintConfigManager::ParseFingerprintMode(
    const std::string& mode_str) {
  if (mode_str == "random") return FingerprintMode::kRandom;
  if (mode_str == "real") return FingerprintMode::kReal;
  if (mode_str == "custom") return FingerprintMode::kCustom;
  if (mode_str == "system") return FingerprintMode::kSystem;
  return FingerprintMode::kSystem;  // 默认
}

// static
WebRTCMode FingerprintConfigManager::ParseWebRTCMode(
    const std::string& mode_str) {
  if (mode_str == "replace") return WebRTCMode::kReplace;
  if (mode_str == "real") return WebRTCMode::kReal;
  if (mode_str == "disable") return WebRTCMode::kDisable;
  return WebRTCMode::kDisable;  // 默认
}

// static
WebGPUMode FingerprintConfigManager::ParseWebGPUMode(
    const std::string& mode_str) {
  if (mode_str == "webgl-match") return WebGPUMode::kWebGLMatch;
  if (mode_str == "real") return WebGPUMode::kReal;
  if (mode_str == "disable") return WebGPUMode::kDisable;
  return WebGPUMode::kWebGLMatch;  // 默认
}

bool FingerprintConfigManager::ApplyConfigFromJson(const std::string& json_str) {
  SIMPRINT_LOG("开始解析指纹配置...");

  // 首先导入配置到全局存储（供 Font Access API 等使用）
  simprint::config::ImportAllConfigFromJson(json_str);

  auto value = base::JSONReader::Read(json_str, base::JSON_PARSE_RFC);
  if (!value || !value->is_dict()) {
    SIMPRINT_LOG("错误: 无法解析 JSON 配置");
    return false;
  }

  const base::Value::Dict& dict = value->GetDict();

  // 解析所有字段
  if (const std::string* val = dict.FindString("language"))
    config_.language = *val;
  if (const std::string* val = dict.FindString("interface_language"))
    config_.interface_language = *val;
  if (const std::string* val = dict.FindString("timezone"))
    config_.timezone = *val;
  if (const std::string* val = dict.FindString("geolocation_prompt"))
    config_.geolocation_prompt = *val;
  if (const std::string* val = dict.FindString("geolocation"))
    config_.geolocation = *val;
  if (const std::string* val = dict.FindString("platform"))
    config_.platform = *val;
  if (const std::string* val = dict.FindString("user_agent"))
    config_.user_agent = *val;

  if (std::optional<bool> val = dict.FindBool("sound"))
    config_.sound = *val;
  if (std::optional<bool> val = dict.FindBool("images"))
    config_.images = *val;
  if (std::optional<bool> val = dict.FindBool("video"))
    config_.video = *val;
  if (std::optional<bool> val = dict.FindBool("do_not_track"))
    config_.do_not_track = *val;

  if (const std::string* val = dict.FindString("window_size"))
    config_.window_size = *val;
  if (std::optional<int> val = dict.FindInt("window_width"))
    config_.window_width = *val;
  if (std::optional<int> val = dict.FindInt("window_height"))
    config_.window_height = *val;
  if (const std::string* val = dict.FindString("window_position"))
    config_.window_position = *val;
  if (std::optional<int> val = dict.FindInt("window_x"))
    config_.window_x = *val;
  if (std::optional<int> val = dict.FindInt("window_y"))
    config_.window_y = *val;

  // 解析 resolution 对象格式 {"width": 1920, "height": 1080}
  if (const base::Value* resolution_obj = dict.Find("resolution")) {
    if (resolution_obj->is_dict()) {
      const base::Value::Dict& res_dict = resolution_obj->GetDict();
      if (auto width = res_dict.FindInt("width")) {
        if (auto height = res_dict.FindInt("height")) {
          // 转换为 "widthxheight" 格式
          config_.resolution = std::to_string(*width) + "x" + std::to_string(*height);
          SIMPRINT_LOG("Resolution 解析: " << *config_.resolution);
        }
      }
    }
  }

  if (std::optional<int> val = dict.FindInt("color_depth"))
    config_.color_depth = *val;
  if (std::optional<double> val = dict.FindDouble("device_pixel_ratio"))
    config_.device_pixel_ratio = *val;
  if (std::optional<int> val = dict.FindInt("max_touch_points"))
    config_.max_touch_points = *val;
  if (std::optional<int> val = dict.FindInt("hardware_concurrency"))
    config_.hardware_concurrency = *val;
  if (std::optional<int> val = dict.FindInt("device_memory"))
    config_.device_memory = *val;

  if (const std::string* val = dict.FindString("canvas"))
    config_.canvas = *val;
  if (const std::string* val = dict.FindString("webgl_image"))
    config_.webgl_image = *val;
  if (const std::string* val = dict.FindString("webgl_info"))
    config_.webgl_info = *val;
  if (const std::string* val = dict.FindString("webgl_vendor"))
    config_.webgl_vendor = *val;
  if (const std::string* val = dict.FindString("webgl_renderer"))
    config_.webgl_renderer = *val;
  if (const std::string* val = dict.FindString("webgpu"))
    config_.webgpu = *val;
  if (const std::string* val = dict.FindString("font_fingerprint"))
    config_.font_fingerprint = *val;

  // 解析 font_list 对象格式 {"mode": "random", "fonts": ["Arial", "Calibri", ...]}
  if (const base::Value* font_list_obj = dict.Find("font_list")) {
    if (font_list_obj->is_dict()) {
      const base::Value::Dict& font_dict = font_list_obj->GetDict();

      // 解析 mode
      if (const std::string* mode = font_dict.FindString("mode")) {
        config_.font_list_mode = *mode;
      }

      // 解析 fonts 数组
      if (const base::Value::List* fonts_list = font_dict.FindList("fonts")) {
        std::vector<std::string> fonts;
        for (const auto& font_value : *fonts_list) {
          if (font_value.is_string()) {
            fonts.push_back(font_value.GetString());
          }
        }
        config_.font_list = fonts;
        SIMPRINT_LOG("Font list 解析: mode=" << (config_.font_list_mode ? *config_.font_list_mode : "未设置")
                     << ", fonts=" << fonts.size());
      }
    }
  }

  if (const std::string* val = dict.FindString("audio_context"))
    config_.audio_context = *val;
  if (const std::string* val = dict.FindString("speech_voices"))
    config_.speech_voices = *val;
  if (const std::string* val = dict.FindString("client_rects"))
    config_.client_rects = *val;
  if (const std::string* val = dict.FindString("media_devices"))
    config_.media_devices = *val;
  if (const std::string* val = dict.FindString("webrtc"))
    config_.webrtc = *val;

  if (const std::string* val = dict.FindString("device_name"))
    config_.device_name = *val;
  if (std::optional<bool> val = dict.FindBool("device_name_random"))
    config_.device_name_random = *val;
  if (const std::string* val = dict.FindString("mac_address"))
    config_.mac_address = *val;
  if (const std::string* val = dict.FindString("mac_address_mode"))
    config_.mac_address_mode = *val;
  if (std::optional<bool> val = dict.FindBool("ssl_fingerprint"))
    config_.ssl_fingerprint = *val;
  if (std::optional<bool> val = dict.FindBool("port_scan_protection"))
    config_.port_scan_protection = *val;
  if (const std::string* val = dict.FindString("scan_whitelist"))
    config_.scan_whitelist = *val;
  if (std::optional<bool> val = dict.FindBool("hardware_acceleration"))
    config_.hardware_acceleration = *val;
  if (std::optional<bool> val = dict.FindBool("disable_sandbox"))
    config_.disable_sandbox = *val;
  if (const std::string* val = dict.FindString("startup_parameters"))
    config_.startup_parameters = *val;

  if (std::optional<bool> val = dict.FindBool("random_fingerprint_on_launch"))
    config_.random_fingerprint_on_launch = *val;

  config_loaded_ = true;

  SIMPRINT_LOG("指纹配置解析完成");
  SIMPRINT_LOG("  - Canvas: " << (config_.canvas ? *config_.canvas : "未设置"));
  SIMPRINT_LOG("  - WebGL Vendor: " << (config_.webgl_vendor ? *config_.webgl_vendor : "未设置"));
  SIMPRINT_LOG("  - Hardware Concurrency: " << (config_.hardware_concurrency ? std::to_string(*config_.hardware_concurrency) : "未设置"));

  // 应用所有指纹配置
  SIMPRINT_LOG("开始应用指纹配置...");

  // 2.2 网络与定位指纹
  NetworkLocationFingerprint::ApplyLanguage(config_);
  NetworkLocationFingerprint::ApplyTimezone(config_);
  NetworkLocationFingerprint::ApplyGeolocation(config_);
  NetworkLocationFingerprint::ApplyWebRTC(config_);
  NetworkLocationFingerprint::ApplyPlatform(config_);
  NetworkLocationFingerprint::ApplyUserAgent(config_);

  // 2.3 屏幕与硬件指纹
  ScreenHardwareFingerprint::ApplyResolution(config_);
  ScreenHardwareFingerprint::ApplyColorDepth(config_);
  ScreenHardwareFingerprint::ApplyDevicePixelRatio(config_);
  ScreenHardwareFingerprint::ApplyMaxTouchPoints(config_);
  ScreenHardwareFingerprint::ApplyHardwareConcurrency(config_);
  ScreenHardwareFingerprint::ApplyDeviceMemory(config_);

  // 2.4 浏览器指纹
  BrowserFingerprint::ApplyCanvas(config_);
  BrowserFingerprint::ApplyWebGLImage(config_);
  BrowserFingerprint::ApplyWebGLInfo(config_);
  BrowserFingerprint::ApplyWebGLVendorRenderer(config_);
  BrowserFingerprint::ApplyWebGPU(config_);
  BrowserFingerprint::ApplyFontFingerprint(config_);
  BrowserFingerprint::ApplyFontList(config_);
  BrowserFingerprint::ApplyAudioContext(config_);
  BrowserFingerprint::ApplySpeechVoices(config_);
  BrowserFingerprint::ApplyClientRects(config_);
  BrowserFingerprint::ApplyMediaDevices(config_);

  // 2.5 浏览器行为
  BehaviorDeviceSettings::ApplyDoNotTrack(config_);
  BehaviorDeviceSettings::StoreMediaPermissions(config_);  // 存储配置，稍后在 HostContentSettingsMap 创建时应用
  BehaviorDeviceSettings::ApplySSLFingerprint(config_);
  BehaviorDeviceSettings::ApplyPortScanProtection(config_);
  BehaviorDeviceSettings::ApplyHardwareAcceleration(config_);

  // 2.6 设备设置
  BehaviorDeviceSettings::ApplyDeviceName(config_);
  BehaviorDeviceSettings::ApplyMacAddress(config_);
  BehaviorDeviceSettings::ApplyStartupParameters(config_);
  BehaviorDeviceSettings::ApplySandboxSettings(config_);

  // 2.7 窗口设置
  WindowSettings::ApplyWindowSize(config_);
  WindowSettings::ApplyWindowPosition(config_);

  SIMPRINT_LOG("所有指纹配置应用完成");

  return true;
}

}  // namespace fingerprint
}  // namespace simprint
