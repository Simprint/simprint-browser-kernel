// Copyright 2024 Simprint. All rights reserved.

#include "chrome/browser/ui/views/frame/sync_input_replay.h"

#include <algorithm>
#include <bit>

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "ui/base/ime/input_method.h"
#include "ui/base/ime/text_input_client.h"
#include "ui/base/accelerators/accelerator.h"
#include "ui/events/event.h"
#include "ui/events/event_constants.h"
#include "ui/events/event_source.h"
#include "ui/events/event_sink.h"
#include "ui/events/keycodes/dom/dom_code.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/events/types/event_type.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/point_f.h"
#include "simprint/console_log/console_log.h"
#include "simprint/eventbus/eventbus.h"
#include "ui/views/focus/focus_manager.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

#if defined(USE_AURA)
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host.h"
#endif

namespace {

// 新格式：type(1) + root_index(1) + norm_x(4) + norm_y(4) + button(1) = 11; type=2,3: 12; type=4: 18
constexpr size_t kMinPayloadSize = 1 + 1 + sizeof(float) * 2 + 1;  // 11
constexpr size_t kPressReleasePayloadSize = 12;
constexpr size_t kWheelPayloadSize = 1 + 1 + sizeof(float) * 4;    // 18
constexpr size_t kPayloadRootIndexOffset = 1;
constexpr size_t kPayloadNormXOffset = 2;
constexpr size_t kPayloadNormYOffset = 6;
constexpr size_t kPayloadButtonOffset = 10;
constexpr size_t kPayloadClickCountOffset = 11;
constexpr size_t kWheelDeltaXOffset = 10;
constexpr size_t kWheelDeltaYOffset = 14;
// 旧格式（无 root_index）：type(1) + norm_x(4) + norm_y(4) + button(1) = 10; type=2,3: 11; type=4: 17
constexpr size_t kOldMinPayloadSize = 10;
constexpr size_t kOldPressReleasePayloadSize = 11;
constexpr size_t kOldWheelPayloadSize = 17;
constexpr size_t kOldNormXOffset = 1;
constexpr size_t kOldNormYOffset = 5;
constexpr size_t kOldButtonOffset = 9;
constexpr size_t kOldClickCountOffset = 10;
constexpr size_t kOldWheelDeltaXOffset = 10;
constexpr size_t kOldWheelDeltaYOffset = 14;
constexpr uint8_t kTypeMove = 1;
constexpr uint8_t kTypeDown = 2;
constexpr uint8_t kTypeUp = 3;
constexpr uint8_t kTypeWheel = 4;
constexpr uint8_t kTypeDragged = 5;
constexpr uint8_t kTypeKeyDown = 6;
constexpr uint8_t kTypeKeyUp = 7;
constexpr uint8_t kTypeTextCommit = 8;  // IME 文本提交（用于中文等输入法）
constexpr size_t kKeyEventPayloadSize = 8;  // type(1) + root_index(1) + key_code(2) + flags(4)

// 从 data[offset] 起读取 4 字节为 float
float ReadFloat(const std::vector<uint8_t>& data, size_t offset) {
  if (offset + sizeof(float) > data.size()) return 0.f;
  uint32_t bits = static_cast<uint32_t>(data[offset]) |
                  (static_cast<uint32_t>(data[offset + 1]) << 8) |
                  (static_cast<uint32_t>(data[offset + 2]) << 16) |
                  (static_cast<uint32_t>(data[offset + 3]) << 24);
  return std::bit_cast<float>(bits);
}

int ButtonIndexToFlags(uint8_t button) {
  switch (button) {
    case 0: return ui::EF_LEFT_MOUSE_BUTTON;
    case 1: return ui::EF_RIGHT_MOUSE_BUTTON;
    case 2: return ui::EF_MIDDLE_MOUSE_BUTTON;
    default: return ui::EF_LEFT_MOUSE_BUTTON;
  }
}

bool GetRootViewAndPoint(const std::vector<uint8_t>& data,
                         views::View** out_root_view,
                         gfx::Point* out_root_local,
                         gfx::PointF* out_screen_point,
                         size_t* out_root_index) {
  const bool new_fmt = (data.size() >= kMinPayloadSize);
  const size_t min_size = new_fmt ? kMinPayloadSize : kOldMinPayloadSize;
  if (data.size() < min_size) return false;
  uint8_t root_index = 0;
  float norm_x;
  float norm_y;
  if (new_fmt) {
    root_index = data[kPayloadRootIndexOffset];
    norm_x = ReadFloat(data, kPayloadNormXOffset);
    norm_y = ReadFloat(data, kPayloadNormYOffset);
  } else {
    norm_x = ReadFloat(data, kOldNormXOffset);
    norm_y = ReadFloat(data, kOldNormYOffset);
  }
  Browser* browser = chrome::FindLastActive();
  if (!browser) return false;
  BrowserView* browser_view = BrowserView::GetBrowserViewForBrowser(browser);
  if (!browser_view) return false;
  std::vector<views::View*> root_views = browser_view->GetSyncReplayRootViews();
  if (root_views.empty()) return false;
  size_t idx = static_cast<size_t>(root_index);
  if (idx >= root_views.size()) idx = 0;
  views::View* root_view = root_views[idx];
  if (!root_view) return false;
  gfx::Rect bounds = root_view->GetLocalBounds();
  int w = bounds.width();
  int h = bounds.height();
  if (w <= 0 || h <= 0) return false;
  gfx::Point root_local(static_cast<int>(norm_x * w),
                        static_cast<int>(norm_y * h));
  gfx::Point screen_point = root_local;
  views::View::ConvertPointToScreen(root_view, &screen_point);
  *out_root_view = root_view;
  *out_root_local = root_local;
  *out_screen_point = gfx::PointF(screen_point);
  *out_root_index = idx;
  return true;
}

#if defined(USE_AURA)
// 获取 root_view 所属 WindowTreeHost 的 EventSink 及屏幕坐标在 root 内的位置。
// Windows 上菜单使用独立原生窗口，必须通过其 host 派发事件才能到达菜单。
std::pair<ui::EventSink*, gfx::PointF> GetEventSinkAndRootLocation(
    views::View* root_view,
    const gfx::PointF& screen_point) {
  if (!root_view || !root_view->GetWidget()) return {nullptr, {}};
  aura::Window* content =
      static_cast<aura::Window*>(root_view->GetWidget()->GetNativeView());
  if (!content) return {nullptr, {}};
  aura::Window* root = content->GetRootWindow();
  if (!root) return {nullptr, {}};
  aura::WindowTreeHost* host = root->GetHost();
  if (!host) return {nullptr, {}};
  ui::EventSink* sink = host->GetEventSink();
  if (!sink) return {nullptr, {}};
  gfx::Rect root_bounds = root->GetBoundsInScreen();
  gfx::PointF root_location(screen_point.x() - root_bounds.x(),
                            screen_point.y() - root_bounds.y());
  return {sink, root_location};
}

bool DispatchMouseEventViaEventSink(views::View* root_view,
                                    const gfx::PointF& screen_point,
                                    ui::EventType type,
                                    int flags,
                                    int changed_button_flags,
                                    int click_count) {
  auto [sink, root_location] = GetEventSinkAndRootLocation(root_view, screen_point);
  if (!sink) return false;
  base::TimeTicks now = base::TimeTicks::Now();
  ui::MouseEvent event(type, root_location, root_location, now, flags,
                       changed_button_flags);
  if (click_count > 0)
    event.SetClickCount(std::clamp(click_count, 1, 3));
  (void)sink->OnEventFromSource(&event);
  return true;
}

bool DispatchWheelEventViaEventSink(views::View* root_view,
                                    const gfx::PointF& screen_point,
                                    float delta_x,
                                    float delta_y) {
  auto [sink, root_location] = GetEventSinkAndRootLocation(root_view, screen_point);
  if (!sink) return false;
  base::TimeTicks now = base::TimeTicks::Now();
  gfx::Vector2d offset(static_cast<int>(delta_x), static_cast<int>(delta_y));
  ui::MouseWheelEvent event(offset, root_location, root_location, now, 0, 0);
  (void)sink->OnEventFromSource(&event);
  return true;
}

// 键盘事件经 EventSink 派发，使 Enter 等按键能送达网页渲染区（表单 submit、搜索框回车等）
bool DispatchKeyEventViaEventSink(views::View* root_view,
                                  ui::EventType type,
                                  ui::KeyboardCode key_code,
                                  int flags) {
  if (!root_view || !root_view->GetWidget()) return false;
  aura::Window* content =
      static_cast<aura::Window*>(root_view->GetWidget()->GetNativeView());
  if (!content) return false;
  aura::Window* root = content->GetRootWindow();
  if (!root) return false;
  aura::WindowTreeHost* host = root->GetHost();
  if (!host) return false;
  ui::EventSink* sink = host->GetEventSink();
  if (!sink) return false;
  base::TimeTicks now = base::TimeTicks::Now();
  ui::KeyEvent event(type, key_code, flags, now);
  (void)sink->OnEventFromSource(&event);
  return true;
}

#endif  // USE_AURA

// root_index: 0=主窗，1+=弹出层。主窗和弹出层均经 EventSink 派发，使事件送达正确目标（含网页渲染区），悬停和聚焦才能生效。
void ReplayMouseMove(views::View* root_view,
                     const gfx::Point& root_local,
                     const gfx::PointF& screen_point,
                     size_t root_index) {
#if defined(USE_AURA)
  if (DispatchMouseEventViaEventSink(root_view, screen_point,
                                     ui::EventType::kMouseMoved, 0, 0, 0))
    return;
#endif
  base::TimeTicks now = base::TimeTicks::Now();
  ui::MouseEvent event(ui::EventType::kMouseMoved,
                       gfx::PointF(root_local), screen_point, now, 0, 0);
  root_view->OnMouseMoved(event);
}

void ReplayMouseDown(views::View* root_view,
                     const gfx::Point& root_local,
                     const gfx::PointF& screen_point,
                     int button_flags,
                     int click_count,
                     size_t root_index) {
  int flags = button_flags | (click_count >= 2 ? ui::EF_IS_DOUBLE_CLICK : 0);
#if defined(USE_AURA)
  // 主窗和弹出层均经 EventSink，拖拽时 MouseDown 需正确建立 capture
  if (DispatchMouseEventViaEventSink(root_view, screen_point,
                                     ui::EventType::kMousePressed, flags,
                                     button_flags, click_count))
    return;
#endif
  base::TimeTicks now = base::TimeTicks::Now();
  ui::MouseEvent event(ui::EventType::kMousePressed,
                       gfx::PointF(root_local), screen_point, now,
                       flags, button_flags);
  event.SetClickCount(std::clamp(click_count, 1, 3));
  root_view->OnMousePressed(event);
}

void ReplayMouseUp(views::View* root_view,
                   const gfx::Point& root_local,
                   const gfx::PointF& screen_point,
                   int button_flags,
                   int click_count,
                   size_t root_index) {
  int flags = button_flags | (click_count >= 2 ? ui::EF_IS_DOUBLE_CLICK : 0);
#if defined(USE_AURA)
  // 主窗和弹出层均经 EventSink，拖拽时 MouseUp 需送达 capture 目标完成 drop
  if (DispatchMouseEventViaEventSink(root_view, screen_point,
                                     ui::EventType::kMouseReleased, flags,
                                     button_flags, click_count))
    return;
#endif
  base::TimeTicks now = base::TimeTicks::Now();
  ui::MouseEvent event(ui::EventType::kMouseReleased,
                       gfx::PointF(root_local), screen_point, now,
                       flags, button_flags);
  event.SetClickCount(std::clamp(click_count, 1, 3));
  root_view->OnMouseReleased(event);
}

void ReplayMouseDragged(views::View* root_view,
                        const gfx::Point& root_local,
                        const gfx::PointF& screen_point,
                        int button_flags,
                        size_t root_index) {
  // 拖拽必须经 EventSink 派发，使事件送达 capture 目标（如标签拖拽时的 tabstrip）
#if defined(USE_AURA)
  if (DispatchMouseEventViaEventSink(root_view, screen_point,
                                     ui::EventType::kMouseDragged,
                                     button_flags, button_flags, 0))
    return;
#endif
  base::TimeTicks now = base::TimeTicks::Now();
  ui::MouseEvent event(ui::EventType::kMouseDragged,
                       gfx::PointF(root_local), screen_point, now,
                       button_flags, button_flags);
  root_view->OnMouseDragged(event);
}

uint16_t ReadU16(const std::vector<uint8_t>& data, size_t offset) {
  if (offset + 2 > data.size()) return 0;
  return static_cast<uint16_t>(data[offset]) |
         (static_cast<uint16_t>(data[offset + 1]) << 8);
}

uint32_t ReadU32(const std::vector<uint8_t>& data, size_t offset) {
  if (offset + 4 > data.size()) return 0;
  return static_cast<uint32_t>(data[offset]) |
         (static_cast<uint32_t>(data[offset + 1]) << 8) |
         (static_cast<uint32_t>(data[offset + 2]) << 16) |
         (static_cast<uint32_t>(data[offset + 3]) << 24);
}

// 从 key_code 和 flags 推导可打印字符，供 Textfield 等通过 GetCharacter() 插入。
// 不修改主控 payload，仅在从控重放时正确构造 KeyEvent。
char16_t DeriveCharacterFromKeyEvent(ui::KeyboardCode key_code, int flags) {
  if (flags & (ui::EF_CONTROL_DOWN | ui::EF_ALT_DOWN))
    return 0;
  if (key_code >= 0x30 && key_code <= 0x39)  // 0-9
    return static_cast<char16_t>(key_code);
  if (key_code == 0x20)  // space
    return ' ';
  if (key_code >= 0x41 && key_code <= 0x5A) {  // A-Z
    return static_cast<char16_t>((flags & ui::EF_SHIFT_DOWN) ? key_code
                                                             : key_code + 0x20);
  }
  return 0;
}

bool GetRootViewForKeyEvent(const std::vector<uint8_t>& data,
                            views::View** out_root_view,
                            size_t* out_root_index) {
  if (data.size() < kKeyEventPayloadSize) return false;
  uint8_t root_index = data[1];
  Browser* browser = chrome::FindLastActive();
  if (!browser) return false;
  BrowserView* browser_view = BrowserView::GetBrowserViewForBrowser(browser);
  if (!browser_view) return false;
  std::vector<views::View*> root_views = browser_view->GetSyncReplayRootViews();
  if (root_views.empty()) return false;
  size_t idx = static_cast<size_t>(root_index);
  if (idx >= root_views.size()) idx = 0;
  *out_root_view = root_views[idx];
  *out_root_index = idx;
  return true;
}

void ReplayKeyDown(views::View* root_view,
                   ui::KeyboardCode key_code,
                   int flags,
                   size_t root_index) {
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
    simprint::LogToConsole(base::StringPrintf(
        "[SyncReplay] KeyDown key_code=%d flags=0x%x root_index=%zu", key_code,
        flags, root_index));
  }
  views::Widget* widget = root_view ? root_view->GetWidget() : nullptr;
  if (!widget) {
    if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
      simprint::LogToConsole("[SyncReplay] KeyDown: widget is null");
    }
    return;
  }
  // CanHandleAccelerators() 要求 widget->IsActive()，从控未获焦点时快捷键不处理。先激活窗口。
  views::Widget* toplevel = widget->GetTopLevelWidget();
  if (toplevel && toplevel->CanActivate() && !toplevel->IsActive()) {
    toplevel->Activate();
  }
  base::TimeTicks now = base::TimeTicks::Now();
  ui::KeyEvent event = [&]() {
    char16_t ch = DeriveCharacterFromKeyEvent(key_code, flags);
    if (ch) {
      return ui::KeyEvent::FromCharacter(ch, key_code, ui::DomCode::NONE, flags,
                                        now);
    }
    return ui::KeyEvent(ui::EventType::kKeyPressed, key_code, flags, now);
  }();
  // 直接调用 ProcessAccelerator，绕过 SkipDefaultKeyEventProcessing（网页内容区会返回 true 导致快捷键被跳过）
  // 过滤 EF_IS_REPEAT：长按会产生重复 KeyDown，每个都会触发快捷键导致多次执行
  views::FocusManager* fm = widget->GetFocusManager();
  bool accelerator_handled = false;
  if (fm && !fm->shortcut_handling_suspended() &&
      !(flags & ui::EF_IS_REPEAT)) {
    ui::Accelerator accelerator(event);
    accelerator_handled = fm->ProcessAccelerator(accelerator);
  }
  if (!accelerator_handled) {
    if (fm && fm->OnKeyEvent(event) == false) {
      accelerator_handled = true;  // Tab/Arrow 等
    } else {
      char16_t ch = DeriveCharacterFromKeyEvent(key_code, flags);
      ui::InputMethod* im = toplevel ? toplevel->GetInputMethod() : nullptr;
      if (ch && im && im->GetTextInputClient()) {
        std::ignore = im->DispatchKeyEvent(&event);
      } else {
#if defined(USE_AURA)
        // 经 EventSink 派发，使 Enter 等按键能送达网页渲染区（表单 submit、搜索框回车等）
        if (DispatchKeyEventViaEventSink(root_view, ui::EventType::kKeyPressed,
                                         key_code, flags))
          ;  // done
        else
#endif
          static_cast<ui::EventSource*>(widget)->SendEventToSink(&event);
      }
    }
  }
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
    simprint::LogToConsole(base::StringPrintf(
        "[SyncReplay] KeyDown done, accelerator_handled=%d, handled=%d",
        accelerator_handled, event.handled()));
  }
}

void ReplayKeyUp(views::View* root_view,
                 ui::KeyboardCode key_code,
                 int flags,
                 size_t root_index) {
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
    simprint::LogToConsole(base::StringPrintf(
        "[SyncReplay] KeyUp key_code=%d flags=0x%x root_index=%zu", key_code,
        flags, root_index));
  }
  views::Widget* widget = root_view ? root_view->GetWidget() : nullptr;
  if (!widget) {
    if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
      simprint::LogToConsole("[SyncReplay] KeyUp: widget is null");
    }
    return;
  }
  views::Widget* toplevel = widget->GetTopLevelWidget();
  if (toplevel && toplevel->CanActivate() && !toplevel->IsActive()) {
    toplevel->Activate();
  }
  base::TimeTicks now = base::TimeTicks::Now();
  ui::KeyEvent event(ui::EventType::kKeyReleased, key_code, flags, now);
  views::FocusManager* fm = widget->GetFocusManager();
  // KeyUp 不处理快捷键（Ctrl+T/W 等只在 KeyDown 触发），避免与 KeyDown 双重执行
  bool accelerator_handled = false;
  if (fm && fm->OnKeyEvent(event) == false) {
    accelerator_handled = true;
  } else if (fm) {
#if defined(USE_AURA)
    if (!DispatchKeyEventViaEventSink(root_view, ui::EventType::kKeyReleased,
                                     key_code, flags))
#endif
      static_cast<ui::EventSource*>(widget)->SendEventToSink(&event);
  }
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
    simprint::LogToConsole(base::StringPrintf(
        "[SyncReplay] KeyUp done, accelerator_handled=%d, handled=%d",
        accelerator_handled, event.handled()));
  }
}

void ReplayWheel(views::View* root_view,
                 const gfx::Point& root_local,
                 const gfx::PointF& screen_point,
                 float delta_x,
                 float delta_y,
                 size_t root_index) {
#if defined(USE_AURA)
  if (DispatchWheelEventViaEventSink(root_view, screen_point, delta_x,
                                     delta_y))
    return;
#endif
  base::TimeTicks now = base::TimeTicks::Now();
  gfx::Vector2d offset(static_cast<int>(delta_x), static_cast<int>(delta_y));
  ui::MouseWheelEvent event(offset, gfx::PointF(root_local), screen_point,
                            now, 0, 0);
  root_view->OnMouseWheel(event);
}

}  // namespace

void ReplaySyncPaste(const std::u16string& text) {
  if (text.empty()) return;
  // 从控收到粘贴时输出到 DevTools Console，便于调试和确认内容已送达
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
  }
  Browser* browser = chrome::FindLastActive();
  if (!browser) return;
  BrowserView* bv = BrowserView::GetBrowserViewForBrowser(browser);
  if (!bv) return;
  views::Widget* widget = bv->GetWidget();
  if (!widget) return;
  views::Widget* toplevel = widget->GetTopLevelWidget();
  if (!toplevel) return;
  ui::InputMethod* im = toplevel->GetInputMethod();
  if (!im) return;
  ui::TextInputClient* client = im->GetTextInputClient();
  if (!client) return;
  client->InsertText(
      text,
      ui::TextInputClient::InsertTextCursorBehavior::kMoveCursorAfterText);
}

void ReplaySyncInputEvent(std::vector<uint8_t> data) {
  if (data.empty()) return;
  uint8_t ev_type = data[0];

  if (ev_type == kTypeKeyDown || ev_type == kTypeKeyUp) {
    if (data.size() < kKeyEventPayloadSize) {
      if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
      }
      return;
    }
    views::View* root_view = nullptr;
    size_t root_index = 0;
    if (!GetRootViewForKeyEvent(data, &root_view, &root_index)) {
      if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
        simprint::LogToConsole(
            "[SyncReplay] Key: GetRootViewForKeyEvent failed (no browser?)");
      }
      return;
    }
    uint16_t key_code_raw = ReadU16(data, 2);
    uint32_t flags_raw = ReadU32(data, 4);
    ui::KeyboardCode key_code =
        static_cast<ui::KeyboardCode>(key_code_raw);
    int flags = static_cast<int>(flags_raw);
    if (ev_type == kTypeKeyDown) {
      ReplayKeyDown(root_view, key_code, flags, root_index);
    } else {
      ReplayKeyUp(root_view, key_code, flags, root_index);
    }
    return;
  }

  // 处理 IME 文本提交事件
  if (ev_type == kTypeTextCommit) {
    if (data.size() < 2) {
      if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
      }
      return;
    }
    uint8_t root_index = data[1];
    std::string utf8(data.begin() + 2, data.end());
    std::u16string text = base::UTF8ToUTF16(utf8);

    // 获取当前活动的 BrowserView
    Browser* browser = chrome::FindLastActive();
    if (!browser) {
      return;
    }
    BrowserView* browser_view = BrowserView::GetBrowserViewForBrowser(browser);
    if (!browser_view) {
      return;
    }

    // 根据 root_index 获取对应的 root_view 和 Widget
    std::vector<views::View*> root_views = browser_view->GetSyncReplayRootViews();
    if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
      simprint::LogToConsole(base::StringPrintf(
          "[SyncReplay] TextCommit: root_index=%d, root_views.size()=%zu",
          (int)root_index, root_views.size()));
    }

    if (root_views.empty()) {
      return;
    }
    size_t idx = static_cast<size_t>(root_index);
    if (idx >= root_views.size()) {
      if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
        simprint::LogToConsole(base::StringPrintf(
            "[SyncReplay] TextCommit: idx %zu >= root_views.size() %zu, using 0",
            idx, root_views.size()));
      }
      idx = 0;
    }
    views::View* root_view = root_views[idx];
    if (!root_view) {
      return;
    }

    // 获取该 root_view 对应的 Widget 的 InputMethod
    views::Widget* widget = root_view->GetWidget();
    if (!widget) {
      return;
    }

    if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
      simprint::LogToConsole(base::StringPrintf(
          "[SyncReplay] TextCommit: widget=%p", widget));
    }

    ui::InputMethod* input_method = widget->GetInputMethod();
    if (input_method) {
      ui::TextInputClient* client = input_method->GetTextInputClient();
      if (client && client->GetTextInputType() != ui::TEXT_INPUT_TYPE_NONE) {
        if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
          simprint::LogToConsole(base::StringPrintf(
              "[SyncReplay] TextCommit: inserting text to client=%p", client));
        }
        // 直接插入文本
        client->InsertText(
            text,
            ui::TextInputClient::InsertTextCursorBehavior::kMoveCursorAfterText);
      } else {
      }
    } else {
    }
    return;
  }

  const bool new_fmt = (data.size() >= kMinPayloadSize);
  size_t required = (ev_type == kTypeWheel)
                        ? (new_fmt ? kWheelPayloadSize : kOldWheelPayloadSize)
                        : (new_fmt ? kMinPayloadSize : kOldMinPayloadSize);
  if (ev_type == kTypeDown || ev_type == kTypeUp)
    required = new_fmt ? kPressReleasePayloadSize : kOldPressReleasePayloadSize;
  if (data.size() < required) return;

  views::View* root_view = nullptr;
  gfx::Point root_local;
  gfx::PointF screen_point;
  size_t root_index = 0;
  if (!GetRootViewAndPoint(data, &root_view, &root_local, &screen_point,
                           &root_index))
    return;

  if (ev_type == kTypeWheel) {
    size_t dx_off = new_fmt ? kWheelDeltaXOffset : kOldWheelDeltaXOffset;
    size_t dy_off = new_fmt ? kWheelDeltaYOffset : kOldWheelDeltaYOffset;
    float delta_x = ReadFloat(data, dx_off);
    float delta_y = ReadFloat(data, dy_off);
    ReplayWheel(root_view, root_local, screen_point, delta_x, delta_y,
                root_index);
    return;
  }

  size_t btn_off = new_fmt ? kPayloadButtonOffset : kOldButtonOffset;
  size_t cc_off = new_fmt ? kPayloadClickCountOffset : kOldClickCountOffset;
  size_t min_cc_size = new_fmt ? kPressReleasePayloadSize : kOldPressReleasePayloadSize;
  uint8_t button = data[btn_off];
  int button_flags = ButtonIndexToFlags(button);
  int click_count = 1;
  if ((ev_type == kTypeDown || ev_type == kTypeUp) && data.size() >= min_cc_size)
    click_count = std::clamp(static_cast<int>(data[cc_off]), 1, 3);

  switch (ev_type) {
    case kTypeMove:
      ReplayMouseMove(root_view, root_local, screen_point, root_index);
      break;
    case kTypeDown:
      ReplayMouseDown(root_view, root_local, screen_point, button_flags,
                     click_count, root_index);
      break;
    case kTypeUp:
      ReplayMouseUp(root_view, root_local, screen_point, button_flags,
                   click_count, root_index);
      break;
    case kTypeDragged:
      ReplayMouseDragged(root_view, root_local, screen_point, button_flags,
                        root_index);
      break;
    default:
      break;
  }
}
