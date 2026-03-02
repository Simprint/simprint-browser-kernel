// Copyright 2024 Simprint. All rights reserved.
// 浏览器指纹修改实现

#include "simprint/fingerprint/browser_fingerprint.h"

#include "simprint/fingerprint/fingerprint_config.h"
#include "simprint/fingerprint/fingerprint_seed.h"
#include "simprint/config/fingerprint_config_storage.h"
#include "base/containers/span.h"
#include "base/logging.h"
#include "base/no_destructor.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

#define SIMPRINT_LOG(msg) LOG(INFO) << msg

namespace simprint {
namespace fingerprint {

// 静态成员初始化 - 使用 NoDestructor 避免 exit-time destructor
namespace {
base::NoDestructor<std::string> g_current_profile_id;
}

uint64_t BrowserFingerprint::canvas_seed_ = 0;
uint64_t BrowserFingerprint::webgl_seed_ = 0;
uint64_t BrowserFingerprint::audio_seed_ = 0;

// static
void BrowserFingerprint::ApplyCanvas(const FingerprintConfig& config) {
  if (!config.canvas.has_value()) {
    SIMPRINT_LOG("Canvas 配置未设置，跳过");
    return;
  }

  simprint::config::SetCanvasMode(config.canvas.value());
  SIMPRINT_LOG("Canvas 配置已应用: " << config.canvas.value());
}

// static
void BrowserFingerprint::ApplyWebGLImage(const FingerprintConfig& config) {
  if (!config.webgl_image.has_value()) {
    SIMPRINT_LOG("WebGL Image 配置未设置，跳过");
    return;
  }

  simprint::config::SetWebGLImageMode(config.webgl_image.value());
  SIMPRINT_LOG("WebGL Image 配置已应用: " << config.webgl_image.value());

  // TODO: 需要在 WebGL 渲染中添加噪声
  // 实现位置: third_party/blink/renderer/modules/webgl/
}

// static
void BrowserFingerprint::ApplyWebGLInfo(const FingerprintConfig& config) {
  if (!config.webgl_info.has_value()) {
    SIMPRINT_LOG("WebGL Info 配置未设置，跳过");
    return;
  }

  simprint::config::SetWebGLInfoMode(config.webgl_info.value());
  SIMPRINT_LOG("WebGL Info 配置已应用: " << config.webgl_info.value());

  // TODO: 需要拦截 WebGL 参数查询
  // 实现位置: third_party/blink/renderer/modules/webgl/webgl_rendering_context_base.cc
}

// static
void BrowserFingerprint::ApplyWebGLVendorRenderer(const FingerprintConfig& config) {
  if (config.webgl_vendor.has_value()) {
    simprint::config::SetWebGLVendor(config.webgl_vendor.value());
    SIMPRINT_LOG("WebGL Vendor 配置已应用: " << config.webgl_vendor.value());
  }

  if (config.webgl_renderer.has_value()) {
    simprint::config::SetWebGLRenderer(config.webgl_renderer.value());
    SIMPRINT_LOG("WebGL Renderer 配置已应用: " << config.webgl_renderer.value());
  }

  // TODO: 需要拦截 WEBGL_debug_renderer_info 扩展
  // 实现位置: third_party/blink/renderer/modules/webgl/webgl_debug_renderer_info.cc
}

// static
void BrowserFingerprint::ApplyWebGPU(const FingerprintConfig& config) {
  if (!config.webgpu.has_value()) {
    SIMPRINT_LOG("WebGPU 配置未设置，跳过");
    return;
  }

  simprint::config::SetWebGPUMode(config.webgpu.value());
  SIMPRINT_LOG("WebGPU 配置已应用: " << config.webgpu.value());

  // TODO: 需要在 WebGPU 中拦截或禁用
  // 实现位置: third_party/blink/renderer/modules/webgpu/
  // 可能的模式: "webgl-match" (匹配WebGL), "real" (真实), "disable" (禁用)
}

// static
void BrowserFingerprint::ApplyFontFingerprint(const FingerprintConfig& config) {
  if (!config.font_fingerprint.has_value()) {
    SIMPRINT_LOG("Font Fingerprint 配置未设置，跳过");
    return;
  }

  simprint::config::SetFontMode(config.font_fingerprint.value());
  SIMPRINT_LOG("Font Fingerprint 配置已应用: " << config.font_fingerprint.value());

  // TODO: 需要拦截字体枚举和测量
  // 实现位置: third_party/blink/renderer/platform/fonts/
}

// static
void BrowserFingerprint::ApplyFontList(const FingerprintConfig& config) {
  if (!config.font_list_mode.has_value()) {
    SIMPRINT_LOG("Font List 配置未设置，跳过");
    return;
  }

  // 存储字体列表模式
  simprint::config::SetFontListMode(config.font_list_mode.value());

  // 存储字体列表
  if (config.font_list.has_value()) {
    simprint::config::SetFontList(config.font_list.value());
    SIMPRINT_LOG("Font List 配置已应用: mode=" << config.font_list_mode.value()
                 << ", fonts=" << config.font_list.value().size());
  } else {
    SIMPRINT_LOG("Font List 配置已应用: mode=" << config.font_list_mode.value()
                 << ", fonts=0");
  }

  // TODO: 需要拦截 Font Access API (navigator.queryLocalFonts())
  // 实现位置: third_party/blink/renderer/modules/font_access/
}

// static
void BrowserFingerprint::ApplyAudioContext(const FingerprintConfig& config) {
  if (!config.audio_context.has_value()) {
    SIMPRINT_LOG("Audio Context 配置未设置，跳过");
    return;
  }

  simprint::config::SetAudioContextMode(config.audio_context.value());
  SIMPRINT_LOG("Audio Context 配置已应用: " << config.audio_context.value());

  // TODO: 需要在 Web Audio API 中添加噪声
  // 实现位置: third_party/blink/renderer/modules/webaudio/
}

// static
void BrowserFingerprint::ApplySpeechVoices(const FingerprintConfig& config) {
  if (!config.speech_voices.has_value()) {
    SIMPRINT_LOG("Speech Voices 配置未设置，跳过");
    return;
  }

  simprint::config::SetSpeechVoicesMode(config.speech_voices.value());
  SIMPRINT_LOG("Speech Voices 配置已应用: " << config.speech_voices.value());

  // TODO: 需要拦截语音合成 API
  // 实现位置: third_party/blink/renderer/modules/speech/
}

// static
void BrowserFingerprint::ApplyClientRects(const FingerprintConfig& config) {
  if (!config.client_rects.has_value()) {
    SIMPRINT_LOG("Client Rects 配置未设置，跳过");
    return;
  }

  simprint::config::SetClientRectsMode(config.client_rects.value());
  SIMPRINT_LOG("Client Rects 配置已应用: " << config.client_rects.value());

  // TODO: 需要在 DOM 元素测量中添加噪声
  // 实现位置: third_party/blink/renderer/core/dom/element.cc
}

// static
void BrowserFingerprint::ApplyMediaDevices(const FingerprintConfig& config) {
  if (!config.media_devices.has_value()) {
    SIMPRINT_LOG("Media Devices 配置未设置，跳过");
    return;
  }

  simprint::config::SetMediaDevicesMode(config.media_devices.value());
  SIMPRINT_LOG("Media Devices 配置已应用: " << config.media_devices.value());

  // TODO: 需要拦截 navigator.mediaDevices.enumerateDevices()
  // 实现位置: third_party/blink/renderer/modules/mediastream/
}

// static
std::string BrowserFingerprint::GetConfiguredCanvasMode() {
  return simprint::config::GetCanvasMode();
}

// static
std::string BrowserFingerprint::GetConfiguredWebGLVendor() {
  return simprint::config::GetWebGLVendor();
}

// static
std::string BrowserFingerprint::GetConfiguredWebGLRenderer() {
  return simprint::config::GetWebGLRenderer();
}

// static
UNSAFE_BUFFERS(void BrowserFingerprint::ApplyCanvasNoise(unsigned char* data, size_t length)) {
  // 检查是否启用了 Canvas 指纹保护
  if (simprint::config::GetCanvasMode().empty() || simprint::config::GetCanvasMode() == "real") {
    return;  // 不添加噪声
  }

  if (simprint::config::GetCanvasMode() == "random") {
    // 使用 base::span 进行安全的缓冲区访问
    UNSAFE_BUFFERS(base::span<unsigned char> buffer(data, length));

    // 【新增】计算内容复杂度 - 统计唯一颜色数量
    std::unordered_set<uint32_t> unique_colors;
    for (size_t i = 0; i + 3 < length; i += 4) {
      // 只看 RGB，忽略 Alpha
      uint32_t color = (buffer[i] << 16) | (buffer[i+1] << 8) | buffer[i+2];
      unique_colors.insert(color);
    }

    // 【新增】如果内容太简单（如纯色矩形），跳过噪声
    // 这样可以绕过 CreepJS 的简单图案检测
    if (unique_colors.size() < 10) {
      return;  // 内容太简单，不添加噪声
    }

    // 【修改】使用 Profile 级的 seed
    uint64_t seed = GetCanvasSeed();

    // 混入内容哈希，确保不同内容有不同噪声
    // 但同一 profile 的同一内容永远一致
    for (size_t i = 0; i < std::min(static_cast<size_t>(100), length); i += length/100) {
      seed = seed * 31 + buffer[i];
    }
    seed ^= static_cast<uint64_t>(length);

    // 使用 LCG 生成伪随机数
    auto lcg = [](uint64_t& state) -> uint32_t {
      state = state * 6364136223846793005ULL + 1442695040888963407ULL;
      return static_cast<uint32_t>(state >> 32);
    };

    // 计算要修改的像素数量(约 0.0125% = 1/8000)
    size_t pixels_to_modify = length / (4 * 8000);
    if (pixels_to_modify == 0) pixels_to_modify = 1;

    size_t modified = 0;
    size_t max_attempts = pixels_to_modify * 10;
    size_t attempts = 0;

    while (modified < pixels_to_modify && attempts < max_attempts) {
      attempts++;

      // 生成随机像素位置
      uint32_t random = lcg(seed);
      size_t pixel_index = (random % (length / 4)) * 4;

      // 【关键】跳过透明像素
      if (buffer[pixel_index + 3] == 0) {
        continue;
      }

      // 选择要修改的通道(R, G, 或 B)
      uint32_t channel_random = lcg(seed);
      size_t channel = channel_random % 3;

      // 添加 ±1 的噪声
      int noise = (lcg(seed) % 2) ? 1 : -1;
      int new_value = buffer[pixel_index + channel] + noise;

      // 确保值在有效范围内
      if (new_value >= 0 && new_value <= 255) {
        buffer[pixel_index + channel] = static_cast<unsigned char>(new_value);
        modified++;
      }
    }
  }
}

// static
std::string BrowserFingerprint::GetConfiguredAudioContextMode() {
  return simprint::config::GetAudioContextMode();
}

// static
UNSAFE_BUFFERS(void BrowserFingerprint::ApplyAudioNoise(float* data, size_t length)) {
  if (!data || length == 0) {
    return;
  }

  // 策略：只对渲染后的音频数据添加噪声，跳过空的测试缓冲区
  // CreepJS 会创建空的 AudioBuffer 并写入 AUDIO_TRAP 值进行测试
  // 我们需要识别并跳过这些测试缓冲区

  // 使用 base::span 安全访问
  UNSAFE_BUFFERS(base::span<float> buffer(data, length));

  // 检查缓冲区是否包含有意义的音频数据
  size_t sample_size = std::min(length, size_t(100));
  size_t non_zero_count = 0;

  for (size_t i = 0; i < sample_size; i++) {
    if (std::abs(buffer[i]) > 1e-6f) {
      non_zero_count++;
    }
  }

  // 如果非零样本少于 10%，认为是测试缓冲区，跳过
  if (non_zero_count < sample_size / 10) {
    return;
  }

  // 对真实的音频数据添加极小的噪声
  uint64_t seed = GetAudioSeed();

  // LCG 参数
  const uint64_t a = 1664525;
  const uint64_t c = 1013904223;
  const uint64_t m = 4294967296;  // 2^32

  // 只修改约 1% 的样本，使用极小的噪声
  for (size_t i = 0; i < length; i++) {
    seed = (a * seed + c) % m;

    // 只修改 1% 的样本
    if ((seed % 100) == 0) {
      // 生成 [0, 1] 范围的随机数
      float random = static_cast<float>(seed % 1000) / 1000.0f;

      // 噪声范围：±1e-10（极小）
      float noise = (random - 0.5f) * 2e-10f;

      buffer[i] += noise;
    }
  }
}

// static
std::string BrowserFingerprint::GetConfiguredClientRectsMode() {
  return simprint::config::GetClientRectsMode();
}

// static
void BrowserFingerprint::ApplyClientRectsNoise(float& x, float& y, float& width, float& height) {
  // 检查是否启用了 ClientRects 指纹保护
  if (simprint::config::GetClientRectsMode().empty() || simprint::config::GetClientRectsMode() == "real") {
    return;  // 不添加噪声
  }

  if (simprint::config::GetClientRectsMode() == "random") {
    // 【关键决策】ClientRects 噪声容易被检测
    // 最安全的做法是：不添加噪声,或者使用极小的噪声
    //
    // 原因:
    // 1. CreepJS 会多次调用 getBoundingClientRect 检查一致性
    // 2. 矩形坐标的微小变化很容易被检测
    // 3. 不同于 Canvas/Audio,ClientRects 的"自然误差"很小
    //
    // 建议: 暂时禁用 ClientRects 噪声,让它返回真实值
    // 如果必须添加噪声,使用极小的值 (< 0.01px)

    // 【方案1】完全禁用噪声(推荐)
    return;

    // 【方案2】如果必须添加噪声,使用极小值(不推荐)
    /*
    // 使用坐标值本身作为种子,确保一致性
    uint32_t seed = static_cast<uint32_t>(
        static_cast<int>(x * 10000) +
        static_cast<int>(y * 10000) +
        static_cast<int>(width * 10000) +
        static_cast<int>(height * 10000)
    );

    // 使用 LCG 生成伪随机数
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    float random_x = static_cast<float>(seed % 1000) / 1000.0f;

    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    float random_y = static_cast<float>(seed % 1000) / 1000.0f;

    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    float random_w = static_cast<float>(seed % 1000) / 1000.0f;

    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    float random_h = static_cast<float>(seed % 1000) / 1000.0f;

    // 极小的噪声范围: [-0.005, 0.005]
    float noise_x = (random_x - 0.5f) * 0.01f;
    float noise_y = (random_y - 0.5f) * 0.01f;
    float noise_w = (random_w - 0.5f) * 0.01f;
    float noise_h = (random_h - 0.5f) * 0.01f;

    x += noise_x;
    y += noise_y;
    width += noise_w;
    height += noise_h;

    // 确保宽度和高度不为负
    if (width < 0) width = 0;
    if (height < 0) height = 0;
    */
  }
}

// static
std::string BrowserFingerprint::GetConfiguredMediaDevicesMode() {
  return simprint::config::GetMediaDevicesMode();
}

// static
bool BrowserFingerprint::ShouldFilterMediaDevice(const std::string& device_type) {
  // 检查是否启用了 MediaDevices 指纹保护
  if (simprint::config::GetMediaDevicesMode().empty() || simprint::config::GetMediaDevicesMode() == "real") {
    return false;  // 不过滤设备
  }

  if (simprint::config::GetMediaDevicesMode() == "random") {
    // 随机过滤一些设备，只保留最基本的设备
    // 例如：只保留第一个音频输入、第一个视频输入、第一个音频输出
    // 这里简单实现：过滤掉除了第一个设备之外的所有设备
    // 实际应用中可以根据 device_type 和设备索引来决定
    return false;  // 暂时不过滤，只是记录日志
  }

  return false;
}

// static
std::string BrowserFingerprint::GetConfiguredSpeechVoicesMode() {
  return simprint::config::GetSpeechVoicesMode();
}

// static
UNSAFE_BUFFERS(void BrowserFingerprint::ApplyWebGLImageNoise(unsigned char* data, size_t length)) {
  // 检查是否启用了 WebGL Image 指纹保护
  if (simprint::config::GetWebGLImageMode().empty() || simprint::config::GetWebGLImageMode() == "real") {
    return;  // 不添加噪声
  }

  if (simprint::config::GetWebGLImageMode() == "random") {
    // 使用 base::span 进行安全的缓冲区访问
    UNSAFE_BUFFERS(base::span<unsigned char> buffer(data, length));

    // WebGL readPixels 噪声：±1/255 RGBA 扰动
    // 使用 Profile 级 seed 确保一致性
    uint64_t seed = GetWebGLSeed();

    // 计算内容哈希以确保相同内容产生相同噪声
    uint64_t content_hash = 0;
    for (size_t i = 0; i < std::min(length, size_t(1024)); i++) {
      content_hash = content_hash * 31 + buffer[i];
    }
    seed ^= content_hash;

    // 修改约 0.1% 的像素（安全阈值）
    size_t pixel_count = buffer.size() / 4;
    size_t target_modifications = std::max(size_t(1), pixel_count / 1000);
    size_t modifications = 0;

    for (size_t attempt = 0; attempt < pixel_count && modifications < target_modifications; attempt++) {
      // 更新伪随机种子
      seed = (seed * 1103515245 + 12345) & 0x7fffffff;

      size_t pixel_index = (seed % pixel_count) * 4;

      // 跳过完全透明的像素（Alpha = 0）
      if (pixel_index + 3 < buffer.size() && buffer[pixel_index + 3] == 0) {
        continue;
      }

      // 生成噪声值：0 或 +1（±1/255 范围）
      int noise = (seed & 1) ? 1 : 0;

      // 随机选择修改哪个通道（R/G/B）
      int channel = (seed >> 8) % 3;
      size_t channel_index = pixel_index + channel;

      if (channel_index < buffer.size()) {
        int value = static_cast<int>(buffer[channel_index]) + noise;
        buffer[channel_index] = static_cast<unsigned char>(std::min(255, value));
        modifications++;
      }
    }
    SIMPRINT_LOG("WebGL readPixels 噪声已应用: " << modifications << "/" << pixel_count << " 像素");
  }
}

// static
std::string BrowserFingerprint::GetConfiguredWebGLImageMode() {
  return simprint::config::GetWebGLImageMode();
}

// static
std::string BrowserFingerprint::GetConfiguredWebGLInfoMode() {
  return simprint::config::GetWebGLInfoMode();
}

// static
std::string BrowserFingerprint::GetConfiguredWebGPUMode() {
  return simprint::config::GetWebGPUMode();
}

// static
std::string BrowserFingerprint::GetConfiguredFontFingerprintMode() {
  return simprint::config::GetFontMode();
}

// static
void BrowserFingerprint::ApplyFontMeasurementNoise(double& width,
                                                     double& left,
                                                     double& right,
                                                     double& ascent,
                                                     double& descent) {
  std::string mode = simprint::config::GetFontMode();

  // 'system' 或 'real' 模式：不添加噪声，使用真实值
  if (mode == "system" || mode == "real" || mode.empty()) {
    return;
  }

  // 'random' 模式：添加微小的随机噪声到字体测量值
  if (mode == "random") {
    // Canvas 文本偏移：≤0.2px 亚像素偏移
    // 使用 Canvas seed 确保与 Canvas 渲染一致

    uint64_t seed = GetCanvasSeed();

    // 基于测量值内容生成一致的噪声
    auto add_noise = [seed](double value, int offset) -> double {
      if (value == 0.0) return value;  // 不修改零值

      // 使用 Profile seed + 值的哈希生成一致的噪声
      uint64_t hash = seed ^ static_cast<uint64_t>(value * 10000) ^ offset;
      hash = (hash * 1103515245 + 12345) & 0x7fffffff;

      // 生成 -0.2 到 +0.2 像素之间的噪声（安全阈值）
      double noise = ((hash % 1000) / 1000.0 - 0.5) * 0.4;
      return value + noise;
    };

    // 对每个测量值添加不同的噪声
    width = add_noise(width, 1);
    left = add_noise(left, 2);
    right = add_noise(right, 3);
    ascent = add_noise(ascent, 4);
    descent = add_noise(descent, 5);
  }
}

// static
void BrowserFingerprint::SetCurrentProfileId(const std::string& profile_id) {
  if (*g_current_profile_id == profile_id) {
    return;  // 已经设置过了
  }

  *g_current_profile_id = profile_id;

  // 生成 master seed
  auto master_seed = FingerprintSeed::GenerateMasterSeed(profile_id);

  // 派生各个 seed
  canvas_seed_ = FingerprintSeed::DeriveCanvasSeed(master_seed);
  webgl_seed_ = FingerprintSeed::DeriveWebGLSeed(master_seed);
  audio_seed_ = FingerprintSeed::DeriveAudioSeed(master_seed);

  SIMPRINT_LOG("Profile ID 已设置: " << profile_id);
}

// static
uint64_t BrowserFingerprint::GetCanvasSeed() {
  if (canvas_seed_ == 0) {
    // 如果还没设置，使用默认 profile
    SetCurrentProfileId("default");
  }
  return canvas_seed_;
}

// static
uint64_t BrowserFingerprint::GetWebGLSeed() {
  if (webgl_seed_ == 0) {
    // 如果还没设置，使用默认 profile
    SetCurrentProfileId("default");
  }
  return webgl_seed_;
}

// static
uint64_t BrowserFingerprint::GetAudioSeed() {
  if (audio_seed_ == 0) {
    // 如果还没设置，使用默认 profile
    SetCurrentProfileId("default");
  }
  return audio_seed_;
}

}  // namespace fingerprint
}  // namespace simprint
