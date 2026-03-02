// Copyright 2024 Simprint. All rights reserved.
// 指纹配置存储 - 纯配置存储，不包含任何逻辑或日志

#ifndef SIMPRINT_CONFIG_FINGERPRINT_CONFIG_STORAGE_H_
#define SIMPRINT_CONFIG_FINGERPRINT_CONFIG_STORAGE_H_

#include <string>
#include <optional>
#include <vector>

namespace simprint {
namespace config {

// ============================================================================
// 网络与定位配置
// ============================================================================

void SetLanguage(const std::string& language);
std::string GetLanguage();

// 获取完整的 Accept-Language 格式（例如 "en-AU,en;q=0.9"）
std::string GetAcceptLanguageHeader();

void SetTimezone(const std::string& timezone);
std::string GetTimezone();

void SetGeolocation(const std::string& geolocation);
std::string GetGeolocation();

void SetGeolocationPrompt(const std::string& prompt);
std::string GetGeolocationPrompt();

void SetPlatform(const std::string& platform);
std::string GetPlatform();

void SetUserAgent(const std::string& user_agent);
std::string GetUserAgent();

// 获取原始完整版本号（用于 Client Hints）
std::string GetOriginalChromeVersion();

// 从 User-Agent 中提取 Chrome 版本号（例如从 "Chrome/124.0.6367.60" 提取 "124.0.6367.60"）
std::string GetChromeVersionFromUserAgent();

// 从 User-Agent 中提取主版本号（例如从 "Chrome/124.0.6367.60" 提取 "124"）
int GetChromeMajorVersionFromUserAgent();

void SetWebRTCMode(const std::string& mode);
std::string GetWebRTCMode();

void SetWebRTCIP(const std::string& ip);
std::string GetWebRTCIP();

// ============================================================================
// 屏幕与硬件配置
// ============================================================================

void SetResolution(const std::string& resolution);
std::string GetResolution();
void GetResolutionWidthHeight(int* width, int* height);

void SetColorDepth(int depth);
int GetColorDepth();

void SetDevicePixelRatio(double ratio);
double GetDevicePixelRatio();

void SetMaxTouchPoints(int points);
int GetMaxTouchPoints();

void SetHardwareConcurrency(int concurrency);
int GetHardwareConcurrency();

void SetDeviceMemory(double memory);
double GetDeviceMemory();

// ============================================================================
// 浏览器指纹配置
// ============================================================================

void SetCanvasMode(const std::string& mode);
std::string GetCanvasMode();

void SetWebGLVendor(const std::string& vendor);
std::string GetWebGLVendor();

void SetWebGLRenderer(const std::string& renderer);
std::string GetWebGLRenderer();

void SetWebGLInfoMode(const std::string& mode);
std::string GetWebGLInfoMode();

void SetWebGLImageMode(const std::string& mode);
std::string GetWebGLImageMode();

void SetWebGPUMode(const std::string& mode);
std::string GetWebGPUMode();

void SetFontMode(const std::string& mode);
std::string GetFontMode();

void SetFontListMode(const std::string& mode);
std::string GetFontListMode();

void SetFontList(const std::vector<std::string>& fonts);
std::vector<std::string> GetFontList();

void SetAudioContextMode(const std::string& mode);
std::string GetAudioContextMode();

void SetSpeechVoicesMode(const std::string& mode);
std::string GetSpeechVoicesMode();

void SetClientRectsMode(const std::string& mode);
std::string GetClientRectsMode();

void SetMediaDevicesMode(const std::string& mode);
std::string GetMediaDevicesMode();

// ============================================================================
// 浏览器行为配置
// ============================================================================

void SetDoNotTrack(const std::string& value);
std::string GetDoNotTrack();

void SetSoundPermission(bool allowed);
std::optional<bool> GetSoundPermission();

void SetImagesPermission(bool allowed);
std::optional<bool> GetImagesPermission();

void SetVideoPermission(bool allowed);
std::optional<bool> GetVideoPermission();

void SetPortScanProtection(bool enabled);
bool GetPortScanProtection();

void SetPortScanWhitelist(const std::string& whitelist);
std::string GetPortScanWhitelist();
bool ShouldBlockPort(int port);

void SetSSLFingerprint(bool enabled);
bool GetSSLFingerprint();

void SetHardwareAcceleration(bool enabled);
bool GetHardwareAcceleration();

void SetDeviceName(const std::string& name);
std::string GetDeviceName();

void SetDeviceNameRandom(bool random);
bool GetDeviceNameRandom();

void SetMacAddress(const std::string& mac);
std::string GetMacAddress();

void SetMacAddressMode(const std::string& mode);
std::string GetMacAddressMode();

void SetStartupParameters(const std::string& params);
std::string GetStartupParameters();

void SetDisableSandbox(bool disable);
bool GetDisableSandbox();

// ============================================================================
// 窗口设置配置
// ============================================================================

void SetWindowSizeMode(const std::string& mode);
std::string GetWindowSizeMode();

void SetWindowSize(int width, int height);
void GetWindowSize(int* width, int* height);

void SetWindowPositionMode(const std::string& mode);
std::string GetWindowPositionMode();

void SetWindowPosition(int x, int y);
void GetWindowPosition(int* x, int* y);

// ============================================================================
// 配置导出
// ============================================================================

// 将所有配置导出为 JSON 字符串，用于传递给 Renderer 进程
std::string ExportAllConfigAsJson();

// 从 JSON 字符串导入所有配置，用于 Renderer 进程接收配置
void ImportAllConfigFromJson(const std::string& json);

}  // namespace config
}  // namespace simprint

#endif  // SIMPRINT_CONFIG_FINGERPRINT_CONFIG_STORAGE_H_
