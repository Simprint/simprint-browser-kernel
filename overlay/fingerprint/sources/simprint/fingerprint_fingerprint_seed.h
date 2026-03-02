// Copyright 2024 Simprint. All rights reserved.

#ifndef SIMPRINT_FINGERPRINT_FINGERPRINT_SEED_H_
#define SIMPRINT_FINGERPRINT_FINGERPRINT_SEED_H_

#include <array>
#include <string>
#include <cstdint>

namespace simprint {
namespace fingerprint {

// 统一指纹种子管理
// 用于生成 Canvas / WebGL / Audio 的确定性噪声
class FingerprintSeed {
 public:
  // 从 Profile ID 生成 master seed
  static std::array<uint8_t, 16> GenerateMasterSeed(
      const std::string& profile_id);

  // 派生 Canvas seed
  static uint64_t DeriveCanvasSeed(
      const std::array<uint8_t, 16>& master_seed);

  // 派生 WebGL seed
  static uint64_t DeriveWebGLSeed(
      const std::array<uint8_t, 16>& master_seed);

  // 派生 Audio seed
  static uint64_t DeriveAudioSeed(
      const std::array<uint8_t, 16>& master_seed);

 private:
  // 简单的哈希函数(用于派生 seed)
  static uint64_t HashToUint64(
      const std::array<uint8_t, 16>& data,
      const char* salt);
};

}  // namespace fingerprint
}  // namespace simprint

#endif  // SIMPRINT_FINGERPRINT_FINGERPRINT_SEED_H_
