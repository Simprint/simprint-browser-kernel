// Copyright 2024 Simprint. All rights reserved.
// 项目内通用：将日志仅输出到当前标签页 DevTools Console（不写 Chromium 日志，便于通过其他软件启动浏览器时在 DevTools 中调试）

#ifndef SIMPRINT_CONSOLE_LOG_CONSOLE_LOG_H_
#define SIMPRINT_CONSOLE_LOG_CONSOLE_LOG_H_

#include <string>

namespace simprint {

/// 仅输出到当前活动标签页的 DevTools Console，可从任意线程调用。
void LogToConsole(const std::string& message);

}  // namespace simprint

#endif  // SIMPRINT_CONSOLE_LOG_CONSOLE_LOG_H_
