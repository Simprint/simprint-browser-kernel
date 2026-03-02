// Copyright 2024 Simprint. All rights reserved.
// 指纹配置存储实现 - 纯配置存储，不包含任何逻辑或日志

#include "simprint/config/fingerprint_config_storage.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include <regex>
#include <sstream>
#include <set>

namespace simprint {
namespace config {

namespace {

// ============================================================================
// 网络与定位存储
// ============================================================================

std::string& GetLanguageStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetTimezoneStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetGeolocationStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetGeolocationPromptStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetPlatformStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetUserAgentStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetWebRTCModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetWebRTCIPStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

// ============================================================================
// 屏幕与硬件存储
// ============================================================================

std::string& GetResolutionStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

int g_color_depth = 0;
double g_device_pixel_ratio = 0.0;
int g_max_touch_points = 0;
int g_hardware_concurrency = 0;
double g_device_memory = 0.0;

// ============================================================================
// 浏览器指纹存储
// ============================================================================

std::string& GetCanvasModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetWebGLVendorStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetWebGLRendererStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetWebGLInfoModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetWebGLImageModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetWebGPUModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetFontModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetFontListModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::vector<std::string>& GetFontListStorage() {
  static base::NoDestructor<std::vector<std::string>> storage;
  return *storage;
}

std::string& GetAudioContextModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetSpeechVoicesModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetClientRectsModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetMediaDevicesModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

// ============================================================================
// 浏览器行为存储
// ============================================================================

std::string& GetDoNotTrackStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::optional<bool> g_sound_permission;
std::optional<bool> g_images_permission;
std::optional<bool> g_video_permission;

bool g_port_scan_protection = false;

std::string& GetPortScanWhitelistStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

bool g_ssl_fingerprint = false;
bool g_hardware_acceleration = true;
bool g_disable_sandbox = false;

std::string& GetDeviceNameStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

bool g_device_name_random = false;

std::string& GetMacAddressStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetStartupParametersStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

std::string& GetMacAddressModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

// ============================================================================
// 窗口设置存储
// ============================================================================

std::string& GetWindowSizeModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

int g_window_width = 0;
int g_window_height = 0;

std::string& GetWindowPositionModeStorage() {
  static base::NoDestructor<std::string> storage;
  return *storage;
}

int g_window_x = 0;
int g_window_y = 0;

}  // namespace

// ============================================================================
// 网络与定位配置实现
// ============================================================================

void SetLanguage(const std::string& language) {
  LOG(INFO) << "[Simprint Language] SetLanguage called with: " << language;
  GetLanguageStorage() = language;
}

std::string GetLanguage() {
  std::string result = GetLanguageStorage();
  LOG(INFO) << "[Simprint Language] GetLanguage returning: " << result;
  return result;
}

std::string GetAcceptLanguageHeader() {
  std::string language = GetLanguageStorage();
  LOG(INFO) << "[Simprint Language] GetAcceptLanguageHeader - storage value: " << language;

  if (language.empty()) {
    LOG(INFO) << "[Simprint Language] GetAcceptLanguageHeader - language is empty, returning empty";
    return "";
  }

  // 从 "en-AU" 生成 "en-AU,en;q=0.9"
  // 从 "zh-CN" 生成 "zh-CN,zh;q=0.9"
  size_t dash_pos = language.find('-');
  if (dash_pos != std::string::npos) {
    std::string base_lang = language.substr(0, dash_pos);
    std::string result = language + "," + base_lang + ";q=0.9";
    LOG(INFO) << "[Simprint Language] GetAcceptLanguageHeader returning: " << result;
    return result;
  }

  // 如果没有 "-"，直接返回原语言（如 "en" -> "en"）
  LOG(INFO) << "[Simprint Language] GetAcceptLanguageHeader returning (no dash): " << language;
  return language;
}

void SetTimezone(const std::string& timezone) {
  GetTimezoneStorage() = timezone;
}

std::string GetTimezone() {
  return GetTimezoneStorage();
}

void SetGeolocation(const std::string& geolocation) {
  GetGeolocationStorage() = geolocation;
}

std::string GetGeolocation() {
  return GetGeolocationStorage();
}

void SetGeolocationPrompt(const std::string& prompt) {
  GetGeolocationPromptStorage() = prompt;
}

std::string GetGeolocationPrompt() {
  return GetGeolocationPromptStorage();
}

void SetPlatform(const std::string& platform) {
  GetPlatformStorage() = platform;
}

std::string GetPlatform() {
  return GetPlatformStorage();
}

// 存储原始完整版本号（用于 Client Hints）
static std::string& GetOriginalChromeVersionStorage() {
  static base::NoDestructor<std::string> original_version;
  return *original_version;
}

void SetUserAgent(const std::string& user_agent) {
  // Simprint: 简化 User-Agent 版本号为 major.0.0.0 格式
  // 使用正则表达式匹配任何浏览器的版本号格式（如 Chrome/144.0.6477.96, Firefox/120.0.1.2 等）
  std::string simplified_ua = user_agent;

  // 匹配格式：浏览器名/主版本.次版本.build.patch
  // 例如：Chrome/144.0.6477.96, Edge/120.0.2210.91, Firefox/121.0.1.0
  std::regex version_regex(R"(([A-Za-z]+)/(\d+)\.(\d+)\.(\d+)\.(\d+))");
  std::smatch match;

  if (std::regex_search(user_agent, match, version_regex)) {
    std::string browser_name = match[1].str();  // 例如 "Chrome"
    std::string major_version = match[2].str(); // 例如 "144"
    std::string minor_version = match[3].str(); // 例如 "0"
    std::string build_version = match[4].str(); // 例如 "6477"
    std::string patch_version = match[5].str(); // 例如 "96"

    // 保存原始完整版本号（用于 Client Hints）
    std::string original_full_version = major_version + "." + minor_version + "." +
                                       build_version + "." + patch_version;
    GetOriginalChromeVersionStorage() = original_full_version;

    // 简化 UA 字符串中的版本号
    std::string simplified_version = browser_name + "/" + major_version + ".0.0.0";
    simplified_ua = std::regex_replace(simplified_ua, version_regex, simplified_version);

    LOG(INFO) << "SetUserAgent() - Original version: " << original_full_version
              << ", Simplified UA: " << simplified_ua;
  }

  GetUserAgentStorage() = simplified_ua;
}

std::string GetUserAgent() {
  std::string ua = GetUserAgentStorage();
  LOG(INFO) << "GetUserAgent() called, returning: '" << ua << "' (length: " << ua.length() << ")";
  return ua;
}

std::string GetOriginalChromeVersion() {
  return GetOriginalChromeVersionStorage();
}

std::string GetChromeVersionFromUserAgent() {
  std::string ua = GetUserAgentStorage();
  if (ua.empty()) {
    return "";
  }

  // 从 User-Agent 中提取 Chrome 版本号
  // 例如: "Mozilla/5.0 ... Chrome/124.0.6367.60 ..." -> "124.0.6367.60"
  size_t chrome_pos = ua.find("Chrome/");
  if (chrome_pos == std::string::npos) {
    return "";
  }

  size_t version_start = chrome_pos + 7;  // "Chrome/" 的长度
  size_t version_end = ua.find_first_of(" /", version_start);

  if (version_end == std::string::npos) {
    return ua.substr(version_start);
  }

  return ua.substr(version_start, version_end - version_start);
}

int GetChromeMajorVersionFromUserAgent() {
  std::string full_version = GetChromeVersionFromUserAgent();
  if (full_version.empty()) {
    return 0;
  }

  // 提取主版本号（第一个点之前的部分）
  size_t dot_pos = full_version.find('.');
  std::string major_version_str = (dot_pos == std::string::npos)
      ? full_version
      : full_version.substr(0, dot_pos);

  int major_version = 0;
  if (base::StringToInt(major_version_str, &major_version)) {
    return major_version;
  }

  return 0;
}

void SetWebRTCMode(const std::string& mode) {
  GetWebRTCModeStorage() = mode;
}

std::string GetWebRTCMode() {
  return GetWebRTCModeStorage();
}

void SetWebRTCIP(const std::string& ip) {
  GetWebRTCIPStorage() = ip;
}

std::string GetWebRTCIP() {
  return GetWebRTCIPStorage();
}

// ============================================================================
// 屏幕与硬件配置实现
// ============================================================================

void SetResolution(const std::string& resolution) {
  GetResolutionStorage() = resolution;
}

std::string GetResolution() {
  return GetResolutionStorage();
}

void GetResolutionWidthHeight(int* width, int* height) {
  const std::string& resolution = GetResolutionStorage();
  if (resolution.empty()) {
    *width = 0;
    *height = 0;
    return;
  }

  size_t pos = resolution.find('x');
  if (pos != std::string::npos) {
    std::string width_str = resolution.substr(0, pos);
    std::string height_str = resolution.substr(pos + 1);

    int w, h;
    if (base::StringToInt(width_str, &w) && base::StringToInt(height_str, &h)) {
      *width = w;
      *height = h;
      return;
    }
  }

  *width = 0;
  *height = 0;
}

void SetColorDepth(int depth) {
  g_color_depth = depth;
}

int GetColorDepth() {
  return g_color_depth;
}

void SetDevicePixelRatio(double ratio) {
  g_device_pixel_ratio = ratio;
}

double GetDevicePixelRatio() {
  return g_device_pixel_ratio;
}

void SetMaxTouchPoints(int points) {
  g_max_touch_points = points;
}

int GetMaxTouchPoints() {
  return g_max_touch_points;
}

void SetHardwareConcurrency(int concurrency) {
  g_hardware_concurrency = concurrency;
}

int GetHardwareConcurrency() {
  return g_hardware_concurrency;
}

void SetDeviceMemory(double memory) {
  g_device_memory = memory;
}

double GetDeviceMemory() {
  return g_device_memory;
}

// ============================================================================
// 浏览器指纹配置实现
// ============================================================================

void SetCanvasMode(const std::string& mode) {
  GetCanvasModeStorage() = mode;
}

std::string GetCanvasMode() {
  return GetCanvasModeStorage();
}

void SetWebGLVendor(const std::string& vendor) {
  GetWebGLVendorStorage() = vendor;
}

std::string GetWebGLVendor() {
  return GetWebGLVendorStorage();
}

void SetWebGLRenderer(const std::string& renderer) {
  GetWebGLRendererStorage() = renderer;
}

std::string GetWebGLRenderer() {
  return GetWebGLRendererStorage();
}

void SetWebGLInfoMode(const std::string& mode) {
  GetWebGLInfoModeStorage() = mode;
}

std::string GetWebGLInfoMode() {
  return GetWebGLInfoModeStorage();
}

void SetWebGLImageMode(const std::string& mode) {
  GetWebGLImageModeStorage() = mode;
}

std::string GetWebGLImageMode() {
  return GetWebGLImageModeStorage();
}

void SetWebGPUMode(const std::string& mode) {
  GetWebGPUModeStorage() = mode;
}

std::string GetWebGPUMode() {
  return GetWebGPUModeStorage();
}

void SetFontMode(const std::string& mode) {
  GetFontModeStorage() = mode;
}

std::string GetFontMode() {
  return GetFontModeStorage();
}

void SetFontListMode(const std::string& mode) {
  GetFontListModeStorage() = mode;
}

std::string GetFontListMode() {
  return GetFontListModeStorage();
}

void SetFontList(const std::vector<std::string>& fonts) {
  GetFontListStorage() = fonts;
}

std::vector<std::string> GetFontList() {
  return GetFontListStorage();
}

void SetAudioContextMode(const std::string& mode) {
  GetAudioContextModeStorage() = mode;
}

std::string GetAudioContextMode() {
  return GetAudioContextModeStorage();
}

void SetSpeechVoicesMode(const std::string& mode) {
  GetSpeechVoicesModeStorage() = mode;
}

std::string GetSpeechVoicesMode() {
  return GetSpeechVoicesModeStorage();
}

void SetClientRectsMode(const std::string& mode) {
  GetClientRectsModeStorage() = mode;
}

std::string GetClientRectsMode() {
  return GetClientRectsModeStorage();
}

void SetMediaDevicesMode(const std::string& mode) {
  GetMediaDevicesModeStorage() = mode;
}

std::string GetMediaDevicesMode() {
  return GetMediaDevicesModeStorage();
}

// ============================================================================
// 浏览器行为配置实现
// ============================================================================

void SetDoNotTrack(const std::string& value) {
  GetDoNotTrackStorage() = value;
}

std::string GetDoNotTrack() {
  return GetDoNotTrackStorage();
}

void SetSoundPermission(bool allowed) {
  g_sound_permission = allowed;
}

std::optional<bool> GetSoundPermission() {
  return g_sound_permission;
}

void SetImagesPermission(bool allowed) {
  g_images_permission = allowed;
}

std::optional<bool> GetImagesPermission() {
  return g_images_permission;
}

void SetVideoPermission(bool allowed) {
  g_video_permission = allowed;
}

std::optional<bool> GetVideoPermission() {
  return g_video_permission;
}

void SetPortScanProtection(bool enabled) {
  g_port_scan_protection = enabled;
}

bool GetPortScanProtection() {
  return g_port_scan_protection;
}

void SetPortScanWhitelist(const std::string& whitelist) {
  GetPortScanWhitelistStorage() = whitelist;
}

std::string GetPortScanWhitelist() {
  return GetPortScanWhitelistStorage();
}

bool ShouldBlockPort(int port) {
  if (!g_port_scan_protection) {
    return false;
  }

  // 检查白名单
  const std::string& whitelist = GetPortScanWhitelistStorage();
  if (!whitelist.empty()) {
    std::istringstream ss(whitelist);
    std::string port_str;
    while (std::getline(ss, port_str, ',')) {
      int whitelisted_port;
      if (base::StringToInt(port_str, &whitelisted_port) && whitelisted_port == port) {
        return false;  // 在白名单中，不阻止
      }
    }
  }

  // 阻止敏感端口
  static const int kBlockedPorts[] = {
    3306, 5432, 27017, 6379, 9200, 5984, 7000, 7001, 8529, 9042,
    1433, 3050, 50000, 1521, 1830,
    22, 23, 3389, 5900, 5901, 5902, 5903,
    445, 139, 135, 137, 138,
    8080, 8081, 8888, 9090, 3000, 5000, 8000, 8001,
    21, 69, 873, 2049
  };

  for (int blocked_port : kBlockedPorts) {
    if (port == blocked_port) {
      return true;
    }
  }

  return false;
}

void SetSSLFingerprint(bool enabled) {
  g_ssl_fingerprint = enabled;
}

bool GetSSLFingerprint() {
  return g_ssl_fingerprint;
}

void SetHardwareAcceleration(bool enabled) {
  g_hardware_acceleration = enabled;
}

bool GetHardwareAcceleration() {
  return g_hardware_acceleration;
}

void SetDeviceName(const std::string& name) {
  GetDeviceNameStorage() = name;
}

std::string GetDeviceName() {
  return GetDeviceNameStorage();
}

void SetDeviceNameRandom(bool random) {
  g_device_name_random = random;
}

bool GetDeviceNameRandom() {
  return g_device_name_random;
}

void SetMacAddress(const std::string& mac) {
  GetMacAddressStorage() = mac;
}

std::string GetMacAddress() {
  return GetMacAddressStorage();
}

void SetMacAddressMode(const std::string& mode) {
  GetMacAddressModeStorage() = mode;
}

std::string GetMacAddressMode() {
  return GetMacAddressModeStorage();
}

void SetStartupParameters(const std::string& params) {
  GetStartupParametersStorage() = params;
}

std::string GetStartupParameters() {
  return GetStartupParametersStorage();
}

void SetDisableSandbox(bool disable) {
  g_disable_sandbox = disable;
}

bool GetDisableSandbox() {
  return g_disable_sandbox;
}

// ============================================================================
// 窗口设置配置实现
// ============================================================================

void SetWindowSizeMode(const std::string& mode) {
  GetWindowSizeModeStorage() = mode;
}

std::string GetWindowSizeMode() {
  return GetWindowSizeModeStorage();
}

void SetWindowSize(int width, int height) {
  g_window_width = width;
  g_window_height = height;
}

void GetWindowSize(int* width, int* height) {
  *width = g_window_width;
  *height = g_window_height;
}

void SetWindowPositionMode(const std::string& mode) {
  GetWindowPositionModeStorage() = mode;
}

std::string GetWindowPositionMode() {
  return GetWindowPositionModeStorage();
}

void SetWindowPosition(int x, int y) {
  g_window_x = x;
  g_window_y = y;
}

void GetWindowPosition(int* x, int* y) {
  *x = g_window_x;
  *y = g_window_y;
}

// ============================================================================
// 配置导出/导入实现
// ============================================================================

std::string ExportAllConfigAsJson() {
  LOG(INFO) << "[Simprint Config Export] ExportAllConfigAsJson called";

  std::string current_language = GetLanguage();
  LOG(INFO) << "[Simprint Config Export] Current language value: " << current_language;

  std::ostringstream json;
  json << "{";

  // 网络与定位
  json << "\"language\":\"" << GetLanguage() << "\",";
  json << "\"timezone\":\"" << GetTimezone() << "\",";
  json << "\"geolocation\":\"" << GetGeolocation() << "\",";
  json << "\"geolocation_prompt\":\"" << GetGeolocationPrompt() << "\",";
  json << "\"platform\":\"" << GetPlatform() << "\",";
  json << "\"user_agent\":\"" << GetUserAgent() << "\",";
  json << "\"original_chrome_version\":\"" << GetOriginalChromeVersion() << "\",";
  json << "\"webrtc_mode\":\"" << GetWebRTCMode() << "\",";
  json << "\"webrtc_ip\":\"" << GetWebRTCIP() << "\",";

  // 屏幕与硬件
  json << "\"resolution\":\"" << GetResolution() << "\",";
  json << "\"color_depth\":" << GetColorDepth() << ",";
  json << "\"device_pixel_ratio\":" << GetDevicePixelRatio() << ",";
  json << "\"max_touch_points\":" << GetMaxTouchPoints() << ",";
  json << "\"hardware_concurrency\":" << GetHardwareConcurrency() << ",";
  json << "\"device_memory\":" << GetDeviceMemory() << ",";

  // 浏览器指纹
  json << "\"canvas_mode\":\"" << GetCanvasMode() << "\",";
  json << "\"webgl_image_mode\":\"" << GetWebGLImageMode() << "\",";
  json << "\"webgl_info_mode\":\"" << GetWebGLInfoMode() << "\",";
  json << "\"webgl_vendor\":\"" << GetWebGLVendor() << "\",";
  json << "\"webgl_renderer\":\"" << GetWebGLRenderer() << "\",";
  json << "\"webgpu_mode\":\"" << GetWebGPUMode() << "\",";
  json << "\"font_mode\":\"" << GetFontMode() << "\",";

  // 导出 font_list 对象
  std::string font_list_mode = GetFontListMode();
  std::vector<std::string> font_list = GetFontList();
  json << "\"font_list\":{";
  json << "\"mode\":\"" << font_list_mode << "\",";
  json << "\"fonts\":[";
  for (size_t i = 0; i < font_list.size(); ++i) {
    json << "\"" << font_list[i] << "\"";
    if (i < font_list.size() - 1) {
      json << ",";
    }
  }
  json << "]},";

  json << "\"audio_mode\":\"" << GetAudioContextMode() << "\",";
  json << "\"speech_voices_mode\":\"" << GetSpeechVoicesMode() << "\",";
  json << "\"client_rects_mode\":\"" << GetClientRectsMode() << "\",";
  json << "\"media_devices_mode\":\"" << GetMediaDevicesMode() << "\",";

  // 浏览器行为
  json << "\"do_not_track\":\"" << GetDoNotTrack() << "\",";
  json << "\"ssl_fingerprint_enabled\":" << (GetSSLFingerprint() ? "true" : "false") << ",";
  json << "\"port_scan_protection_enabled\":" << (GetPortScanProtection() ? "true" : "false") << ",";
  json << "\"scan_whitelist\":\"" << GetPortScanWhitelist() << "\",";

  // 设备设置
  json << "\"device_name\":\"" << GetDeviceName() << "\",";
  json << "\"device_name_random\":" << (GetDeviceNameRandom() ? "true" : "false") << ",";
  json << "\"mac_address\":\"" << GetMacAddress() << "\",";
  json << "\"mac_address_mode\":\"" << GetMacAddressMode() << "\",";

  // 窗口设置
  json << "\"window_size_mode\":\"" << GetWindowSizeMode() << "\",";
  int w, h;
  GetWindowSize(&w, &h);
  json << "\"window_width\":" << w << ",";
  json << "\"window_height\":" << h << ",";
  json << "\"window_position_mode\":\"" << GetWindowPositionMode() << "\",";
  int x, y;
  GetWindowPosition(&x, &y);
  json << "\"window_x\":" << x << ",";
  json << "\"window_y\":" << y;

  json << "}";
  std::string result = json.str();
  LOG(INFO) << "[Simprint Config Export] Exported JSON length: " << result.length() << " bytes";
  LOG(INFO) << "[Simprint Config Export] Exported JSON content: " << result;
  return result;
}

void ImportAllConfigFromJson(const std::string& json) {
  if (json.empty()) {
    LOG(INFO) << "[Simprint Config Import] ImportAllConfigFromJson: empty JSON, skipping";
    return;
  }

  LOG(INFO) << "[Simprint Config Import] ImportAllConfigFromJson: parsing JSON (" << json.length() << " bytes)";
  LOG(INFO) << "[Simprint Config Import] JSON content: " << json;

  auto parsed = base::JSONReader::Read(json, base::JSON_PARSE_RFC);
  if (!parsed || !parsed->is_dict()) {
    LOG(ERROR) << "[Simprint Config Import] ImportAllConfigFromJson: failed to parse JSON";
    return;
  }

  const base::Value::Dict& dict = parsed->GetDict();
  LOG(INFO) << "[Simprint Config Import] JSON parsed successfully, dict has " << dict.size() << " entries";

  // 网络与定位
  if (const std::string* val = dict.FindString("language")) {
    LOG(INFO) << "[Simprint Config Import] Found language in JSON: " << *val;
    SetLanguage(*val);
    LOG(INFO) << "[Simprint Config Import] SetLanguage called with: " << *val;
    std::string verify = GetLanguage();
    LOG(INFO) << "[Simprint Config Import] Verify GetLanguage returns: " << verify;
  } else {
    LOG(WARNING) << "[Simprint Config Import] No language field found in JSON";
  }
  if (const std::string* val = dict.FindString("timezone")) {
    SetTimezone(*val);
    LOG(INFO) << "Imported timezone: " << *val;
  }
  if (const std::string* val = dict.FindString("geolocation")) {
    SetGeolocation(*val);
    LOG(INFO) << "Imported geolocation: " << *val;
  }
  if (const std::string* val = dict.FindString("geolocation_prompt")) {
    SetGeolocationPrompt(*val);
    LOG(INFO) << "Imported geolocation_prompt: " << *val;
  }
  if (const std::string* val = dict.FindString("platform")) {
    SetPlatform(*val);
    LOG(INFO) << "Imported platform: " << *val;
  }
  // Simprint: 先导入原始版本号，再导入 user_agent
  // 这样可以避免 SetUserAgent() 从简化后的 UA 中提取错误的版本号
  if (const std::string* val = dict.FindString("original_chrome_version")) {
    GetOriginalChromeVersionStorage() = *val;
    LOG(INFO) << "Imported original_chrome_version: " << *val;
  }
  if (const std::string* val = dict.FindString("user_agent")) {
    // 注意：这里传入的是简化后的 UA，但原始版本号已经在上面设置了
    GetUserAgentStorage() = *val;
    LOG(INFO) << "Imported user_agent: " << *val;
  }
  if (const std::string* val = dict.FindString("webrtc_mode")) {
    SetWebRTCMode(*val);
    LOG(INFO) << "Imported webrtc_mode: " << *val;
  }
  if (const std::string* val = dict.FindString("webrtc_ip")) {
    SetWebRTCIP(*val);
    LOG(INFO) << "Imported webrtc_ip: " << *val;
  }

  // 屏幕与硬件
  if (const std::string* val = dict.FindString("resolution")) {
    SetResolution(*val);
    LOG(INFO) << "Imported resolution: " << *val;
  }
  if (auto val = dict.FindInt("color_depth")) {
    SetColorDepth(*val);
    LOG(INFO) << "Imported color_depth: " << *val;
  }
  if (auto val = dict.FindDouble("device_pixel_ratio")) {
    SetDevicePixelRatio(*val);
    LOG(INFO) << "Imported device_pixel_ratio: " << *val;
  }
  if (auto val = dict.FindInt("max_touch_points")) {
    SetMaxTouchPoints(*val);
    LOG(INFO) << "Imported max_touch_points: " << *val;
  }
  if (auto val = dict.FindInt("hardware_concurrency")) {
    SetHardwareConcurrency(*val);
    LOG(INFO) << "Imported hardware_concurrency: " << *val;
  }
  if (auto val = dict.FindDouble("device_memory")) {
    SetDeviceMemory(*val);
    LOG(INFO) << "Imported device_memory: " << *val;
  }

  // 浏览器指纹
  if (const std::string* val = dict.FindString("canvas_mode")) {
    SetCanvasMode(*val);
    LOG(INFO) << "Imported canvas_mode: " << *val;
  }
  if (const std::string* val = dict.FindString("webgl_image_mode")) {
    SetWebGLImageMode(*val);
    LOG(INFO) << "Imported webgl_image_mode: " << *val;
  }
  if (const std::string* val = dict.FindString("webgl_info_mode")) {
    SetWebGLInfoMode(*val);
    LOG(INFO) << "Imported webgl_info_mode: " << *val;
  }
  if (const std::string* val = dict.FindString("webgl_vendor")) {
    SetWebGLVendor(*val);
    LOG(INFO) << "Imported webgl_vendor: " << *val;
  }
  if (const std::string* val = dict.FindString("webgl_renderer")) {
    SetWebGLRenderer(*val);
    LOG(INFO) << "Imported webgl_renderer: " << *val;
  }
  if (const std::string* val = dict.FindString("webgpu_mode")) {
    SetWebGPUMode(*val);
    LOG(INFO) << "Imported webgpu_mode: " << *val;
  }
  if (const std::string* val = dict.FindString("font_mode")) {
    SetFontMode(*val);
    LOG(INFO) << "Imported font_mode: " << *val;
  }

  // 导入字体列表对象 {"mode": "random", "fonts": ["Arial", ...]}
  if (const base::Value* font_list_obj = dict.Find("font_list")) {
    if (font_list_obj->is_dict()) {
      const base::Value::Dict& font_dict = font_list_obj->GetDict();

      // 导入 mode
      if (const std::string* mode = font_dict.FindString("mode")) {
        SetFontListMode(*mode);
        LOG(INFO) << "Imported font_list mode: " << *mode;
      }

      // 导入 fonts 数组
      if (const base::Value::List* fonts_list = font_dict.FindList("fonts")) {
        std::vector<std::string> fonts;
        for (const auto& font_value : *fonts_list) {
          if (font_value.is_string()) {
            fonts.push_back(font_value.GetString());
          }
        }
        SetFontList(fonts);
        LOG(INFO) << "Imported font_list: " << fonts.size() << " fonts";
      }
    }
  }

  if (const std::string* val = dict.FindString("audio_mode")) {
    SetAudioContextMode(*val);
    LOG(INFO) << "Imported audio_mode: " << *val;
  }
  if (const std::string* val = dict.FindString("speech_voices_mode")) {
    SetSpeechVoicesMode(*val);
    LOG(INFO) << "Imported speech_voices_mode: " << *val;
  }
  if (const std::string* val = dict.FindString("client_rects_mode")) {
    SetClientRectsMode(*val);
    LOG(INFO) << "Imported client_rects_mode: " << *val;
  }
  if (const std::string* val = dict.FindString("media_devices_mode")) {
    SetMediaDevicesMode(*val);
    LOG(INFO) << "Imported media_devices_mode: " << *val;
  }

  // 浏览器行为
  if (const std::string* val = dict.FindString("do_not_track")) {
    SetDoNotTrack(*val);
    LOG(INFO) << "Imported do_not_track: " << *val;
  }
  if (auto val = dict.FindBool("ssl_fingerprint_enabled")) {
    SetSSLFingerprint(*val);
    LOG(INFO) << "Imported ssl_fingerprint_enabled: " << *val;
  }
  if (auto val = dict.FindBool("port_scan_protection_enabled")) {
    SetPortScanProtection(*val);
    LOG(INFO) << "Imported port_scan_protection_enabled: " << *val;
  }
  if (const std::string* val = dict.FindString("scan_whitelist")) {
    SetPortScanWhitelist(*val);
    LOG(INFO) << "Imported scan_whitelist: " << *val;
  }

  // 设备设置
  if (const std::string* val = dict.FindString("device_name")) {
    SetDeviceName(*val);
    LOG(INFO) << "Imported device_name: " << *val;
  }
  if (auto val = dict.FindBool("device_name_random")) {
    SetDeviceNameRandom(*val);
    LOG(INFO) << "Imported device_name_random: " << *val;
  }
  if (const std::string* val = dict.FindString("mac_address")) {
    SetMacAddress(*val);
    LOG(INFO) << "Imported mac_address: " << *val;
  }
  if (const std::string* val = dict.FindString("mac_address_mode")) {
    SetMacAddressMode(*val);
    LOG(INFO) << "Imported mac_address_mode: " << *val;
  }

  // 窗口设置
  if (const std::string* val = dict.FindString("window_size_mode")) {
    SetWindowSizeMode(*val);
    LOG(INFO) << "Imported window_size_mode: " << *val;
  }
  if (auto w = dict.FindInt("window_width")) {
    if (auto h = dict.FindInt("window_height")) {
      SetWindowSize(*w, *h);
      LOG(INFO) << "Imported window_size: " << *w << "x" << *h;
    }
  }
  if (const std::string* val = dict.FindString("window_position_mode")) {
    SetWindowPositionMode(*val);
    LOG(INFO) << "Imported window_position_mode: " << *val;
  }
  if (auto x = dict.FindInt("window_x")) {
    if (auto y = dict.FindInt("window_y")) {
      SetWindowPosition(*x, *y);
      LOG(INFO) << "Imported window_position: " << *x << "," << *y;
    }
  }

  LOG(INFO) << "ImportAllConfigFromJson: completed successfully";
}

}  // namespace config
}  // namespace simprint
