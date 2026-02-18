// Copyright 2024 Simprint. All rights reserved.
// 启动认证检查器实现

#include "simprint/auth/startup_auth_checker.h"

#include "base/process/process.h"
#include "simprint/console_log/console_log.h"

namespace simprint {
namespace auth {

// static
void StartupAuthChecker::Check(int timeout_ms) {
  LogToConsole("[StartupAuthChecker] Starting auth check");

  eventbus::EventBus::GetInstance().SetAuthStatusCallback(
      [](const eventbus::EventBus::AuthInfo& auth_info) {
        OnAuthResponse(auth_info);
      });

  eventbus::EventBus::GetInstance().RequestAuthInfoWithTimeout(
      timeout_ms,
      base::BindOnce(&StartupAuthChecker::OnAuthTimeout));
}

// static
void StartupAuthChecker::OnAuthResponse(
    const eventbus::EventBus::AuthInfo& auth_info) {
  if (auth_info.is_authenticated) {
    LogToConsole("[StartupAuthChecker] User authenticated");
  } else {
    LogToConsole("[StartupAuthChecker] User not authenticated, terminating");
    ExitBrowserSilently();
  }
}

// static
void StartupAuthChecker::OnAuthTimeout() {
  LogToConsole("[StartupAuthChecker] Auth check timeout, terminating");
  ExitBrowserSilently();
}

// static
void StartupAuthChecker::ExitBrowserSilently() {
  base::Process::TerminateCurrentProcessImmediately(1);
}

}  // namespace auth
}  // namespace simprint
