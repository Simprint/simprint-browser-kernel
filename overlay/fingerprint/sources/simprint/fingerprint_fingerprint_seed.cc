// Copyright 2024 Simprint. All rights reserved.

#include "simprint/fingerprint/fingerprint_seed.h"
#include <cstring>
#include "base/containers/span.h"

namespace simprint {
namespace fingerprint {

// 从 Profile ID 生成 master seed
std::array<uint8_t, 16> FingerprintSeed::GenerateMasterSeed(
    const std::string& profile_id) {
  std::array<uint8_t, 16> seed = {0};

  // 使用简单的哈希算法
  uint64_t hash = 0x123456789ABCDEF0ULL;

  for (char c : profile_id) {
    hash = hash * 31 + static_cast<uint8_t>(c);
    hash ^= (hash >> 33);
    hash *= 0xff51afd7ed558ccdULL;
  }

  // 填充 seed
  for (size_t i = 0; i < 16; i++) {
    seed[i] = static_cast<uint8_t>((hash >> (i * 4)) & 0xFF);
  }

  return seed;
}

// 派生 Canvas seed
uint64_t FingerprintSeed::DeriveCanvasSeed(
    const std::array<uint8_t, 16>& master_seed) {
  return HashToUint64(master_seed, "canvas");
}

// 派生 WebGL seed
uint64_t FingerprintSeed::DeriveWebGLSeed(
    const std::array<uint8_t, 16>& master_seed) {
  return HashToUint64(master_seed, "webgl");
}

// 派生 Audio seed
uint64_t FingerprintSeed::DeriveAudioSeed(
    const std::array<uint8_t, 16>& master_seed) {
  return HashToUint64(master_seed, "audio");
}

// 内部哈希函数
uint64_t FingerprintSeed::HashToUint64(
    const std::array<uint8_t, 16>& data,
    const char* salt) {
  uint64_t hash = 0xcbf29ce484222325ULL;  // FNV offset basis

  // 混入 salt - 使用 base::span 安全访问
  size_t salt_len = std::strlen(salt);
  UNSAFE_BUFFERS(base::span<const char> salt_span(salt, salt_len));
  for (char c : salt_span) {
    hash ^= static_cast<uint8_t>(c);
    hash *= 0x100000001b3ULL;  // FNV prime
  }

  // 混入 data
  for (uint8_t byte : data) {
    hash ^= byte;
    hash *= 0x100000001b3ULL;
  }

  return hash;
}

}  // namespace fingerprint
}  // namespace simprint
