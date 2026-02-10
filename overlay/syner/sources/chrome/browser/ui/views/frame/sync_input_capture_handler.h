// Copyright 2024 Simprint. All rights reserved.
// 同步输入捕获：主控时在视图层捕获全窗口鼠标事件并发送 SyncInputEvent。

#ifndef CHROME_BROWSER_UI_VIEWS_FRAME_SYNC_INPUT_CAPTURE_HANDLER_H_
#define CHROME_BROWSER_UI_VIEWS_FRAME_SYNC_INPUT_CAPTURE_HANDLER_H_

#include <utility>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "ui/events/event_handler.h"

namespace ui {
class Event;
class LocatedEvent;
}

namespace views {
class View;
class Widget;
}

/// 主控执行粘贴时调用，将剪贴板内容通过 kSyncPaste 发送。用于 CutCopyPaste 等命令路径，弥补 key 事件有时收不到的遗漏。
void SendSyncPasteIfMain();

/// 发送文本提交事件（用于 IME 输入同步）
void SendSyncTextCommit(uint8_t root_index, const std::u16string& text);

/// 根据 Widget 获取 root_index（用于 IME 输入同步）
/// 返回值：root_index，如果无法确定则返回 0
uint8_t GetRootIndexForWidget(views::Widget* widget);

// 挂接在 RootView 上的 PreTargetHandler：当本机为同步主控时，
// 将全窗口鼠标事件（按键、移动、滚轮）以归一化坐标序列化为 SyncInputEvent payload 通过 EventBus 发送。
// 不消费事件，正常 UI 行为不受影响。
// 通过 SetGetRootViewAndIndexCallback 设置回调：根据事件所在窗口返回 (RootView, root_index)，用于弹出层重放。
class SyncInputCaptureHandler : public ui::EventHandler {
 public:
  // 根据事件返回 (用于归一化的 RootView, root_index)。root_index: 0=主窗，1,2,…=弹出层。
  // 若 View* 为 null 则不发送该事件。
  using GetRootViewAndIndexCallback =
      base::RepeatingCallback<std::pair<views::View*, size_t>(ui::LocatedEvent*)>;
  using GetRootViewAndIndexForKeyEventCallback =
      base::RepeatingCallback<std::pair<views::View*, size_t>(ui::Event*)>;

  explicit SyncInputCaptureHandler(views::View* root_view);
  SyncInputCaptureHandler(const SyncInputCaptureHandler&) = delete;
  SyncInputCaptureHandler& operator=(const SyncInputCaptureHandler&) = delete;
  ~SyncInputCaptureHandler() override;

  void SetGetRootViewAndIndexCallback(GetRootViewAndIndexCallback callback);
  void SetGetRootViewAndIndexForKeyEventCallback(
      GetRootViewAndIndexForKeyEventCallback callback);

  void OnKeyEvent(ui::KeyEvent* event) override;
  void OnMouseEvent(ui::MouseEvent* event) override;
  void OnScrollEvent(ui::ScrollEvent* event) override;

  // 主控执行粘贴时调用，将剪贴板内容通过 EventBus 发送给从控。
  // 在 CutCopyPaste(IDC_PASTE) 中调用可覆盖所有粘贴触发路径（快捷键、菜单等）。
  static void SendSyncPasteIfMain();

 private:
  raw_ptr<views::View> root_view_;
  GetRootViewAndIndexCallback get_root_view_and_index_;
  GetRootViewAndIndexForKeyEventCallback get_root_view_and_index_for_key_;
};

#endif  // CHROME_BROWSER_UI_VIEWS_FRAME_SYNC_INPUT_CAPTURE_HANDLER_H_
