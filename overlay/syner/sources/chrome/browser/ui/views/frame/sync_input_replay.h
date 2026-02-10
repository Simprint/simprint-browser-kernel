// Copyright 2024 Simprint. All rights reserved.
// 同步输入（从控）：收到 SyncInputEvent 时解析、打日志并派发到 RootView 重放（含双击 click_count）。

#ifndef CHROME_BROWSER_UI_VIEWS_FRAME_SYNC_INPUT_REPLAY_H_
#define CHROME_BROWSER_UI_VIEWS_FRAME_SYNC_INPUT_REPLAY_H_

#include <string>
#include <vector>

// 在 UI 线程调用：解析 payload 并重放鼠标/滚轮事件到当前窗口 RootView。
void ReplaySyncInputEvent(std::vector<uint8_t> data);

// 在 UI 线程调用：将文本插入到当前聚焦的 TextInputClient（地址栏等）。
void ReplaySyncPaste(const std::u16string& text);

#endif  // CHROME_BROWSER_UI_VIEWS_FRAME_SYNC_INPUT_REPLAY_H_
