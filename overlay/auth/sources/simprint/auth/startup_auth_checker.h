// Copyright 2024 Simprint. All rights reserved.
// 启动认证检查器 - 确保只有已登录用户才能使用浏览器

#ifndef SIMPRINT_AUTH_STARTUP_AUTH_CHECKER_H_
#define SIMPRINT_AUTH_STARTUP_AUTH_CHECKER_H_

#include "base/memory/weak_ptr.h"
#include "simprint/eventbus/eventbus.h"

namespace simprint {
namespace auth {

// 启动认证检查器
// 用途：在浏览器启动时强制检查用户认证状态
// 安全策略：
//   - 通过 Tauri 启动 + 已登录 → 浏览器正常运行
//   - 通过 Tauri 启动 + 未登录 → 静默退出浏览器
//   - 通过 Tauri 启动 + IPC 异常/超时 → 静默退出浏览器（fail-safe）
//   - 直接启动（未通过 Tauri）→ 静默退出浏览器
class StartupAuthChecker {
 public:
  // 执行启动认证检查（通过 IPC 向 Tauri 查询认证状态）
  // timeout_ms: 超时时间（毫秒），默认 5000ms
  static void Check(int timeout_ms = 5000);

  // 静默退出浏览器（用于未通过 Tauri 启动的情况）
  static void ExitBrowserSilently();

 private:
  // 处理认证响应
  static void OnAuthResponse(const eventbus::EventBus::AuthInfo& auth_info);

  // 处理认证超时或失败
  static void OnAuthTimeout();
};

}  // namespace auth
}  // namespace simprint

#endif  // SIMPRINT_AUTH_STARTUP_AUTH_CHECKER_H_
