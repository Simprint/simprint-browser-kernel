// Copyright 2024 Simprint. All rights reserved.
// 指纹配置管理

#ifndef SIMPRINT_FINGERPRINT_FINGERPRINT_CONFIG_H_
#define SIMPRINT_FINGERPRINT_FINGERPRINT_CONFIG_H_

#include <optional>
#include <string>
#include <vector>

namespace base {
template <typename T>
class NoDestructor;
}

namespace simprint {
namespace fingerprint {

// 指纹模式
enum class FingerprintMode {
  kRandom,   // 随机
  kReal,     // 真实
  kCustom,   // 自定义
  kSystem    // 系统
};

// WebRTC 模式
enum class WebRTCMode {
  kReplace,  // 替换
  kReal,     // 真实
  kDisable   // 禁用
};

// WebGPU 模式
enum class WebGPUMode {
  kWebGLMatch,  // 匹配 WebGL
  kReal,        // 真实
  kDisable      // 禁用
};

// 完整的指纹配置（扁平化结构）
struct FingerprintConfig {
  FingerprintConfig();
  ~FingerprintConfig();

  // === 网络与定位 ===
  std::optional<std::string> language;
  std::optional<std::string> interface_language;
  std::optional<std::string> timezone;
  std::optional<std::string> geolocation_prompt;
  std::optional<std::string> geolocation;
  std::optional<std::string> platform;
  std::optional<std::string> user_agent;

  // === 浏览器行为 ===
  std::optional<bool> sound;
  std::optional<bool> images;
  std::optional<bool> video;
  std::optional<bool> do_not_track;

  // === 窗口设置 ===
  std::optional<std::string> window_size;
  std::optional<int> window_width;
  std::optional<int> window_height;
  std::optional<std::string> window_position;
  std::optional<int> window_x;
  std::optional<int> window_y;

  // === 屏幕与硬件 ===
  std::optional<std::string> resolution;
  std::optional<int> color_depth;
  std::optional<double> device_pixel_ratio;
  std::optional<int> max_touch_points;
  std::optional<int> hardware_concurrency;
  std::optional<int> device_memory;

  // === 浏览器指纹 ===
  std::optional<std::string> canvas;
  std::optional<std::string> webgl_image;
  std::optional<std::string> webgl_info;
  std::optional<std::string> webgl_vendor;
  std::optional<std::string> webgl_renderer;
  std::optional<std::string> webgpu;
  std::optional<std::string> font_fingerprint;
  std::optional<std::string> font_list_mode;
  std::optional<std::vector<std::string>> font_list;
  std::optional<std::string> audio_context;
  std::optional<std::string> speech_voices;
  std::optional<std::string> client_rects;
  std::optional<std::string> media_devices;
  std::optional<std::string> webrtc;

  // === 设备设置 ===
  std::optional<std::string> device_name;
  std::optional<bool> device_name_random;
  std::optional<std::string> mac_address;
  std::optional<std::string> mac_address_mode;
  std::optional<bool> ssl_fingerprint;
  std::optional<bool> port_scan_protection;
  std::optional<std::string> scan_whitelist;
  std::optional<bool> hardware_acceleration;
  std::optional<bool> disable_sandbox;
  std::optional<std::string> startup_parameters;

  // === 偏好设置 ===
  std::optional<bool> random_fingerprint_on_launch;
};

// 指纹配置管理器（单例）
class FingerprintConfigManager {
 public:
  static FingerprintConfigManager* GetInstance();

  // 从 JSON 解析并应用配置
  bool ApplyConfigFromJson(const std::string& json_str);

  // 获取当前配置
  const FingerprintConfig& GetConfig() const { return config_; }

  // 检查配置是否已加载
  bool IsConfigLoaded() const { return config_loaded_; }

 private:
  friend class base::NoDestructor<FingerprintConfigManager>;

  FingerprintConfigManager();
  ~FingerprintConfigManager();

  FingerprintConfigManager(const FingerprintConfigManager&) = delete;
  FingerprintConfigManager& operator=(const FingerprintConfigManager&) = delete;

  // 解析字符串为 FingerprintMode
  static FingerprintMode ParseFingerprintMode(const std::string& mode_str);

  // 解析字符串为 WebRTCMode
  static WebRTCMode ParseWebRTCMode(const std::string& mode_str);

  // 解析字符串为 WebGPUMode
  static WebGPUMode ParseWebGPUMode(const std::string& mode_str);

  FingerprintConfig config_;
  bool config_loaded_ = false;
};

}  // namespace fingerprint
}  // namespace simprint

#endif  // SIMPRINT_FINGERPRINT_FINGERPRINT_CONFIG_H_
