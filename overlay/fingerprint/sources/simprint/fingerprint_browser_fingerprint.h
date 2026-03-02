// Copyright 2024 Simprint. All rights reserved.
// 浏览器指纹修改

#ifndef SIMPRINT_FINGERPRINT_BROWSER_FINGERPRINT_H_
#define SIMPRINT_FINGERPRINT_BROWSER_FINGERPRINT_H_

#include <string>

namespace simprint {
namespace fingerprint {

struct FingerprintConfig;

// 浏览器指纹管理器
class BrowserFingerprint {
 public:
  // 应用 Canvas 指纹配置
  static void ApplyCanvas(const FingerprintConfig& config);

  // 应用 WebGL 图像指纹配置
  static void ApplyWebGLImage(const FingerprintConfig& config);

  // 应用 WebGL 信息指纹配置
  static void ApplyWebGLInfo(const FingerprintConfig& config);

  // 应用 WebGL Vendor/Renderer 配置
  static void ApplyWebGLVendorRenderer(const FingerprintConfig& config);

  // 应用 WebGPU 配置
  static void ApplyWebGPU(const FingerprintConfig& config);

  // 应用字体指纹配置
  static void ApplyFontFingerprint(const FingerprintConfig& config);

  // 应用字体列表配置
  static void ApplyFontList(const FingerprintConfig& config);

  // 应用音频上下文指纹配置
  static void ApplyAudioContext(const FingerprintConfig& config);

  // 应用语音合成指纹配置
  static void ApplySpeechVoices(const FingerprintConfig& config);

  // 应用 ClientRects 指纹配置
  static void ApplyClientRects(const FingerprintConfig& config);

  // 应用媒体设备指纹配置
  static void ApplyMediaDevices(const FingerprintConfig& config);

  // 获取配置的 Canvas 模式
  static std::string GetConfiguredCanvasMode();

  // 获取配置的 WebGL Vendor
  static std::string GetConfiguredWebGLVendor();

  // 获取配置的 WebGL Renderer
  static std::string GetConfiguredWebGLRenderer();

  // 应用 Canvas 噪声到图像数据
  // data: 图像数据指针
  // length: 数据长度（字节数）
  static void ApplyCanvasNoise(unsigned char* data, size_t length);

  // 获取配置的 AudioContext 模式
  static std::string GetConfiguredAudioContextMode();

  // 应用 Audio 噪声到音频数据
  // data: 音频数据指针（float32 数组）
  // length: 数据长度（样本数）
  static void ApplyAudioNoise(float* data, size_t length);

  // 获取配置的 ClientRects 模式
  static std::string GetConfiguredClientRectsMode();

  // 应用 ClientRects 噪声到矩形数据
  // rect: 矩形对象的引用
  static void ApplyClientRectsNoise(float& x, float& y, float& width, float& height);

  // 获取配置的 MediaDevices 模式
  static std::string GetConfiguredMediaDevicesMode();

  // 判断是否应该过滤媒体设备
  // 返回 true 表示应该隐藏该设备
  static bool ShouldFilterMediaDevice(const std::string& device_type);

  // 获取配置的 Speech Voices 模式
  static std::string GetConfiguredSpeechVoicesMode();

  // 应用 WebGL Image 噪声到像素数据
  // data: 像素数据指针
  // length: 数据长度（字节数）
  static void ApplyWebGLImageNoise(unsigned char* data, size_t length);

  // 获取配置的 WebGL Image 模式
  static std::string GetConfiguredWebGLImageMode();

  // 获取配置的 WebGL Info 模式
  static std::string GetConfiguredWebGLInfoMode();

  // 获取配置的 WebGPU 模式
  static std::string GetConfiguredWebGPUMode();

  // 获取配置的 Font Fingerprint 模式
  static std::string GetConfiguredFontFingerprintMode();

  // 应用字体测量噪声到 TextMetrics
  // width, left, right, ascent, descent: 测量值的引用
  static void ApplyFontMeasurementNoise(double& width,
                                        double& left,
                                        double& right,
                                        double& ascent,
                                        double& descent);

  // 获取当前 Profile 的 Canvas seed
  static uint64_t GetCanvasSeed();

  // 获取当前 Profile 的 WebGL seed
  static uint64_t GetWebGLSeed();

  // 获取当前 Profile 的 Audio seed
  static uint64_t GetAudioSeed();

  // 设置当前 Profile ID(在 Profile 创建/切换时调用)
  static void SetCurrentProfileId(const std::string& profile_id);

 private:
  static uint64_t canvas_seed_;
  static uint64_t webgl_seed_;
  static uint64_t audio_seed_;
};

}  // namespace fingerprint
}  // namespace simprint

#endif  // SIMPRINT_FINGERPRINT_BROWSER_FINGERPRINT_H_
