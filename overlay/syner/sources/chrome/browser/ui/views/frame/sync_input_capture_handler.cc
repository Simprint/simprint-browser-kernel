// Copyright 2024 Simprint. All rights reserved.
#include "base/logging.h"

#include "chrome/browser/ui/views/frame/sync_input_capture_handler.h"

#include <algorithm>
#include <bit>
#include <vector>

#include "base/no_destructor.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "simprint/eventbus/eventbus.h"
#include "simprint/eventbus/topics.h"
#include "ui/aura/window.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/clipboard/clipboard_buffer.h"
#include "ui/events/event.h"
#include "ui/events/event_constants.h"
#include "ui/events/event_target.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/events/types/event_type.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

// Simprint: 全局回调指针声明（在全局命名空间中）
using SyncTextCommitCallback = void (*)(uint8_t, const std::u16string&);
extern SyncTextCommitCallback g_sync_text_commit_callback;

// Simprint: 获取 Widget 的 root_index 函数指针（在 content/ 和 ui/ 层使用）
using GetRootIndexForWidgetFunc = uint8_t (*)(void*);
GetRootIndexForWidgetFunc g_get_root_index_for_widget_func = nullptr;

// 前向声明（在匿名命名空间外）
uint8_t GetRootIndexForWidgetPtr(void* ptr);

namespace {

// 主控开启同步后，通过 kSyncInputEvent 上报给 Tauri，从控重放。
// - 鼠标移动 (type=1)
// - 左/右/中键按下 (type=2)、释放 (type=3)，含 click_count 以支持双击
// - 按下后移动为拖拽 (type=5)
// - 滚轮 (type=4)，含 delta_x/delta_y
// Payload 格式：
// - type=1,5：1 byte type + 1 byte root_index + 4 byte norm_x + 4 byte norm_y + 1 byte button = 11 bytes
// - type=2,3：11 bytes + 1 byte click_count = 12 bytes
// - type=4：1 byte type + 1 byte root_index + 4 byte norm_x + 4 byte norm_y + 4 byte delta_x + 4 byte delta_y = 18 bytes
// - type=6,7：1 byte type + 1 byte root_index + 2 byte key_code + 4 byte flags = 8 bytes（KeyDown/KeyUp）
constexpr uint8_t kTypeMove = 1;
constexpr uint8_t kTypeDown = 2;
constexpr uint8_t kTypeUp = 3;
constexpr uint8_t kTypeWheel = 4;
constexpr uint8_t kTypeDragged = 5;
constexpr uint8_t kTypeKeyDown = 6;
constexpr uint8_t kTypeKeyUp = 7;
constexpr uint8_t kTypeTextCommit = 8;  // IME 文本提交（用于中文等输入法）

uint8_t ButtonFlagsToIndex(int flags) {
  if (flags & ui::EF_LEFT_MOUSE_BUTTON) return 0;
  if (flags & ui::EF_RIGHT_MOUSE_BUTTON) return 1;
  if (flags & ui::EF_MIDDLE_MOUSE_BUTTON) return 2;
  return 0;
}

void WriteFloatToPayload(std::vector<uint8_t>& payload, float value) {
  uint32_t bits = std::bit_cast<uint32_t>(value);
  for (int i = 0; i < 4; ++i) {
    payload.push_back(static_cast<uint8_t>((bits >> (8 * i)) & 0xFF));
  }
}

void SendSyncMouseEvent(uint8_t ev_type, uint8_t root_index, float norm_x,
                        float norm_y, uint8_t button, int click_count) {
  std::vector<uint8_t> payload;
  payload.reserve(1 + 1 + sizeof(float) * 2 + 1 +
                  (ev_type == kTypeDown || ev_type == kTypeUp ? 1 : 0));
  payload.push_back(ev_type);
  payload.push_back(static_cast<uint8_t>(root_index));
  WriteFloatToPayload(payload, norm_x);
  WriteFloatToPayload(payload, norm_y);
  payload.push_back(button);
  if (ev_type == kTypeDown || ev_type == kTypeUp) {
    payload.push_back(static_cast<uint8_t>(std::clamp(click_count, 1, 3)));
  }
  simprint::eventbus::EventBus::GetInstance().SendEvent(
      simprint::eventbus::Topic::kSyncInputEvent, std::move(payload));
}

void SendSyncWheelEvent(uint8_t root_index, float norm_x, float norm_y,
                        float delta_x, float delta_y) {
  std::vector<uint8_t> payload;
  payload.reserve(1 + 1 + sizeof(float) * 4);
  payload.push_back(kTypeWheel);
  payload.push_back(root_index);
  WriteFloatToPayload(payload, norm_x);
  WriteFloatToPayload(payload, norm_y);
  WriteFloatToPayload(payload, delta_x);
  WriteFloatToPayload(payload, delta_y);
  simprint::eventbus::EventBus::GetInstance().SendEvent(
      simprint::eventbus::Topic::kSyncInputEvent, std::move(payload));
}

void WriteU16ToPayload(std::vector<uint8_t>& payload, uint16_t value) {
  payload.push_back(static_cast<uint8_t>(value & 0xFF));
  payload.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

void WriteU32ToPayload(std::vector<uint8_t>& payload, uint32_t value) {
  for (int i = 0; i < 4; ++i) {
    payload.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xFF));
  }
}

void SendSyncKeyEvent(uint8_t ev_type, uint8_t root_index,
                      ui::KeyboardCode key_code, int flags) {
  std::vector<uint8_t> payload;
  payload.reserve(8);
  payload.push_back(ev_type);
  payload.push_back(root_index);
  WriteU16ToPayload(payload, static_cast<uint16_t>(key_code));
  WriteU32ToPayload(payload, static_cast<uint32_t>(flags));
  simprint::eventbus::EventBus::GetInstance().SendEvent(
      simprint::eventbus::Topic::kSyncInputEvent, std::move(payload));
}

[[maybe_unused]] void SendSyncTextCommit(uint8_t root_index, const std::u16string& text) {
  if (text.empty()) return;
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() != 1) {
    return;  // 只有主控才发送
  }
  std::string utf8 = base::UTF16ToUTF8(text);
  std::vector<uint8_t> payload;
  payload.reserve(2 + utf8.size());
  payload.push_back(kTypeTextCommit);
  payload.push_back(root_index);
  payload.insert(payload.end(), utf8.begin(), utf8.end());
  simprint::eventbus::EventBus::GetInstance().SendEvent(
      simprint::eventbus::Topic::kSyncInputEvent, std::move(payload));
}

// 初始化全局回调指针
struct SyncTextCommitCallbackInitializer {
  SyncTextCommitCallbackInitializer() {
    g_sync_text_commit_callback = &SendSyncTextCommit;
    g_get_root_index_for_widget_func = &GetRootIndexForWidgetPtr;
  }
};
static SyncTextCommitCallbackInitializer g_initializer;

void SendSyncPaste() {
  static base::TimeTicks s_last_send;
  base::TimeTicks now = base::TimeTicks::Now();
  if ((now - s_last_send).InMilliseconds() < 150) {
    return;  // 防抖：OnKeyEvent 与 CutCopyPaste 可能同时触发
  }
  s_last_send = now;
  ui::Clipboard* clipboard = ui::Clipboard::GetForCurrentThread();
  if (!clipboard) return;
  std::u16string text;
  clipboard->ReadText(ui::ClipboardBuffer::kCopyPaste, /*data_dst=*/nullptr,
                     &text);
  if (text.empty()) return;
  std::string utf8 = base::UTF16ToUTF8(text);
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 1) {
  }
  std::vector<uint8_t> payload(utf8.begin(), utf8.end());
  simprint::eventbus::EventBus::GetInstance().SendEvent(
      simprint::eventbus::Topic::kSyncPaste, std::move(payload));
}

bool GetNormalizedPosition(views::View* root_view,
                           const gfx::Point& root_location,
                           float* norm_x,
                           float* norm_y) {
  if (!root_view || !root_view->GetWidget()) {
    return false;
  }
  const gfx::Rect client_bounds = root_view->GetLocalBounds();
  const int w = client_bounds.width();
  const int h = client_bounds.height();
  if (w <= 0 || h <= 0) {
    return false;
  }
  gfx::Point point_in_client = root_location;
  aura::Window* content_window = static_cast<aura::Window*>(
      root_view->GetWidget()->GetNativeView());
  if (content_window) {
    aura::Window* root_window = content_window->GetRootWindow();
    if (root_window) {
      gfx::Point screen_point = root_window->GetBoundsInScreen().origin();
      screen_point.Offset(root_location.x(), root_location.y());
      gfx::Rect client_in_screen = root_view->GetBoundsInScreen();
      point_in_client =
          gfx::Point(screen_point.x() - client_in_screen.x(),
                     screen_point.y() - client_in_screen.y());
    }
  }
  float nx = static_cast<float>(point_in_client.x()) / w;
  float ny = static_cast<float>(point_in_client.y()) / h;
  *norm_x = std::clamp(nx, 0.0f, 1.0f);
  *norm_y = std::clamp(ny, 0.0f, 1.0f);
  return true;
}

bool GetNormalizedPositionFromScreen(views::View* root_view,
                                     const gfx::Point& screen_point,
                                     float* norm_x,
                                     float* norm_y) {
  if (!root_view || !root_view->GetWidget()) {
    return false;
  }
  const gfx::Rect client_bounds = root_view->GetLocalBounds();
  const int w = client_bounds.width();
  const int h = client_bounds.height();
  if (w <= 0 || h <= 0) {
    return false;
  }
  gfx::Rect client_in_screen = root_view->GetBoundsInScreen();
  gfx::Point point_in_client(screen_point.x() - client_in_screen.x(),
                             screen_point.y() - client_in_screen.y());
  float nx = static_cast<float>(point_in_client.x()) / w;
  float ny = static_cast<float>(point_in_client.y()) / h;
  *norm_x = std::clamp(nx, 0.0f, 1.0f);
  *norm_y = std::clamp(ny, 0.0f, 1.0f);
  return true;
}

bool GetNormalizedPositionFromEvent(views::View* root_view,
                                    const ui::LocatedEvent* event,
                                    float* norm_x,
                                    float* norm_y) {
  if (!root_view || !root_view->GetWidget() || !event) {
    return false;
  }
  aura::Window* content_window = static_cast<aura::Window*>(
      root_view->GetWidget()->GetNativeView());
  aura::Window* main_root =
      content_window ? content_window->GetRootWindow() : nullptr;
  if (main_root && event->target() == main_root) {
    return GetNormalizedPosition(root_view, event->root_location(), norm_x,
                                 norm_y);
  }
  if (event->target() && main_root) {
    aura::Window* event_root = static_cast<aura::Window*>(event->target());
    if (event_root) {
      aura::Window* root = event_root->GetRootWindow();
      if (root) {
        gfx::Point screen_point = root->GetBoundsInScreen().origin();
        screen_point.Offset(event->root_location().x(),
                           event->root_location().y());
        return GetNormalizedPositionFromScreen(root_view, screen_point, norm_x,
                                               norm_y);
      }
    }
  }
  return GetNormalizedPosition(root_view, event->root_location(), norm_x,
                               norm_y);
}

}  // namespace

void SyncInputCaptureHandler::SendSyncPasteIfMain() {
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() != 1) {
    return;
  }
  SendSyncPaste();
}

SyncInputCaptureHandler::SyncInputCaptureHandler(views::View* root_view)
    : root_view_(root_view) {}

SyncInputCaptureHandler::~SyncInputCaptureHandler() = default;

void SyncInputCaptureHandler::SetGetRootViewAndIndexCallback(
    GetRootViewAndIndexCallback callback) {
  get_root_view_and_index_ = std::move(callback);
}

void SyncInputCaptureHandler::SetGetRootViewAndIndexForKeyEventCallback(
    GetRootViewAndIndexForKeyEventCallback callback) {
  get_root_view_and_index_for_key_ = std::move(callback);
}

void SyncInputCaptureHandler::OnKeyEvent(ui::KeyEvent* event) {
  ui::KeyboardCode key_code = event->key_code();
  int flags = event->flags();
  const bool is_ctrl_v = (key_code == ui::VKEY_V) &&
                         (flags & ui::EF_CONTROL_DOWN);
  if (is_ctrl_v) {
    // 网页渲染区、地址栏等处的 Ctrl+V 可能不经过 CutCopyPaste，需在此发送 SyncPaste。
    if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 1 &&
        event->type() == ui::EventType::kKeyPressed) {
      SendSyncPaste();
    }
    return;  // 不发送 KeyDown/KeyUp
  }
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() != 1) {
    return;
  }
  size_t root_index = 0;
  if (get_root_view_and_index_for_key_) {
    auto [view, index] = get_root_view_and_index_for_key_.Run(event);
    if (!view) return;
    root_index = (index > 255) ? 255u : static_cast<size_t>(index);
  }
  uint8_t ri = static_cast<uint8_t>(root_index);
  if (event->type() == ui::EventType::kKeyPressed) {
    SendSyncKeyEvent(kTypeKeyDown, ri, key_code, flags);
  } else if (event->type() == ui::EventType::kKeyReleased) {
    SendSyncKeyEvent(kTypeKeyUp, ri, key_code, flags);
  }
}

void SyncInputCaptureHandler::OnMouseEvent(ui::MouseEvent* event) {
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() != 1) {
    return;
  }
  views::View* root_view = root_view_.get();
  size_t root_index = 0;
  if (get_root_view_and_index_) {
    auto [view, index] = get_root_view_and_index_.Run(event);
    if (!view) return;
    root_view = view;
    root_index = (index > 255) ? 255u : static_cast<size_t>(index);
  }
  float norm_x = 0.0f;
  float norm_y = 0.0f;
  if (!GetNormalizedPositionFromEvent(root_view, event, &norm_x, &norm_y)) {
    return;
  }
  uint8_t ri = static_cast<uint8_t>(root_index);
  if (event->type() == ui::EventType::kMouseMoved) {
    SendSyncMouseEvent(kTypeMove, ri, norm_x, norm_y, 0, 1);
  } else if (event->type() == ui::EventType::kMousePressed) {
    uint8_t button = ButtonFlagsToIndex(event->changed_button_flags());
    int click_count = std::clamp(event->GetClickCount(), 1, 3);
    SendSyncMouseEvent(kTypeDown, ri, norm_x, norm_y, button, click_count);
  } else if (event->type() == ui::EventType::kMouseReleased) {
    uint8_t button = ButtonFlagsToIndex(event->changed_button_flags());
    int click_count = std::clamp(event->GetClickCount(), 1, 3);
    SendSyncMouseEvent(kTypeUp, ri, norm_x, norm_y, button, click_count);
  } else if (event->type() == ui::EventType::kMouseDragged) {
    uint8_t dragged_button = ButtonFlagsToIndex(event->flags());
    SendSyncMouseEvent(kTypeDragged, ri, norm_x, norm_y, dragged_button, 1);
  } else if (event->type() == ui::EventType::kMousewheel) {
    ui::MouseWheelEvent* wheel = static_cast<ui::MouseWheelEvent*>(event);
    SendSyncWheelEvent(ri, norm_x, norm_y,
                      static_cast<float>(wheel->x_offset()),
                      static_cast<float>(wheel->y_offset()));
  }
}

void SyncInputCaptureHandler::OnScrollEvent(ui::ScrollEvent* event) {
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() != 1) {
    return;
  }
  views::View* root_view = root_view_.get();
  size_t root_index = 0;
  if (get_root_view_and_index_) {
    auto [view, index] = get_root_view_and_index_.Run(event);
    if (!view) return;
    root_view = view;
    root_index = (index > 255) ? 255u : static_cast<size_t>(index);
  }
  float norm_x = 0.0f;
  float norm_y = 0.0f;
  if (!GetNormalizedPositionFromEvent(root_view, event, &norm_x, &norm_y)) {
    return;
  }
  float delta_x = event->x_offset();
  float delta_y = event->y_offset();
  SendSyncWheelEvent(static_cast<uint8_t>(root_index), norm_x, norm_y,
                    delta_x, delta_y);
}

uint8_t GetRootIndexForWidget(views::Widget* widget) {
  if (!widget) {
    return 0;
  }

#if defined(USE_AURA)
  // 获取当前活动的 BrowserView
  Browser* browser = chrome::FindLastActive();
  if (!browser) {
    return 0;
  }
  BrowserView* browser_view = BrowserView::GetBrowserViewForBrowser(browser);
  if (!browser_view) {
    return 0;
  }

  // 获取 widget 的 root window
  aura::Window* content = static_cast<aura::Window*>(widget->GetNativeView());
  if (!content) {
    return 0;
  }
  aura::Window* root = content->GetRootWindow();
  if (!root) {
    return 0;
  }

  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
    simprint::LogToConsole(base::StringPrintf(
        "[GetRootIndexForWidget] widget=%p, root=%p", widget, root));
  }

  // 检查是否是主窗口
  aura::Window* main_content = static_cast<aura::Window*>(
      browser_view->GetWidget()->GetNativeView());
  if (main_content) {
    aura::Window* main_root = main_content->GetRootWindow();
    if (root == main_root) {
      return 0;
    }
  }

  // 查找弹出层的 root_index
  std::vector<views::View*> root_views = browser_view->GetSyncReplayRootViews();
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
    simprint::LogToConsole(base::StringPrintf(
        "[GetRootIndexForWidget] checking %zu root_views", root_views.size()));
  }

  for (size_t i = 1; i < root_views.size(); ++i) {
    views::View* root_view = root_views[i];
    if (root_view && root_view->GetWidget()) {
      aura::Window* popup_content =
          static_cast<aura::Window*>(root_view->GetWidget()->GetNativeView());
      if (popup_content) {
        aura::Window* popup_root = popup_content->GetRootWindow();
        if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
          simprint::LogToConsole(base::StringPrintf(
              "[GetRootIndexForWidget] root_views[%zu]: popup_root=%p", i, popup_root));
        }
        if (popup_root == root) {
          if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
            simprint::LogToConsole(base::StringPrintf(
                "[GetRootIndexForWidget] matched popup at index %zu", i));
          }
          return static_cast<uint8_t>(i);
        }
      }
    }
  }
#endif

  return 0;
}

// 辅助函数：从 void* 转换并调用 GetRootIndexForWidget
// 用于 content/ 和 ui/ 层，它们传递 aura::Window* 或 views::Widget*
uint8_t GetRootIndexForWidgetPtr(void* ptr) {
  if (!ptr) {
    return 0;
  }

#if defined(USE_AURA)
  // 尝试将 ptr 作为 aura::Window* 处理
  aura::Window* window = static_cast<aura::Window*>(ptr);

  // 先尝试通过 Widget 查找（适用于 Views 控件）
  views::Widget* widget = views::Widget::GetWidgetForNativeView(window);
  if (widget) {
    if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
      simprint::LogToConsole(base::StringPrintf(
          "[GetRootIndexForWidgetPtr] window=%p, widget=%p (found via GetWidgetForNativeView)",
          window, widget));
    }
    uint8_t root_index = GetRootIndexForWidget(widget);
    if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
      simprint::LogToConsole(base::StringPrintf(
          "[GetRootIndexForWidgetPtr] root_index=%d", (int)root_index));
    }
    return root_index;
  }

  // Widget 为空，说明是 WebUI 的子窗口，通过 root window 匹配
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
    simprint::LogToConsole(base::StringPrintf(
        "[GetRootIndexForWidgetPtr] window=%p, widget=(nil), trying root window match",
        window));
  }

  aura::Window* root = window->GetRootWindow();
  if (!root) {
    return 0;
  }

  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
    simprint::LogToConsole(base::StringPrintf(
        "[GetRootIndexForWidgetPtr] root=%p", root));
  }

  // 获取当前活动的 BrowserView
  Browser* browser = chrome::FindLastActive();
  if (!browser) {
    return 0;
  }
  BrowserView* browser_view = BrowserView::GetBrowserViewForBrowser(browser);
  if (!browser_view) {
    return 0;
  }

  // 检查是否是主窗口
  aura::Window* main_content = static_cast<aura::Window*>(
      browser_view->GetWidget()->GetNativeView());
  if (main_content) {
    aura::Window* main_root = main_content->GetRootWindow();
    if (root == main_root) {
      return 0;
    }
  }

  // 查找弹出层的 root_index
  std::vector<views::View*> root_views = browser_view->GetSyncReplayRootViews();
  if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
    simprint::LogToConsole(base::StringPrintf(
        "[GetRootIndexForWidgetPtr] checking %zu root_views", root_views.size()));
  }

  for (size_t i = 1; i < root_views.size(); ++i) {
    views::View* root_view = root_views[i];
    if (root_view && root_view->GetWidget()) {
      aura::Window* popup_content =
          static_cast<aura::Window*>(root_view->GetWidget()->GetNativeView());
      if (popup_content) {
        aura::Window* popup_root = popup_content->GetRootWindow();
        if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
          simprint::LogToConsole(base::StringPrintf(
              "[GetRootIndexForWidgetPtr] root_views[%zu]: popup_root=%p", i, popup_root));
        }
        if (popup_root == root) {
          if (simprint::eventbus::EventBus::GetInstance().GetSyncRole() == 2) {
            simprint::LogToConsole(base::StringPrintf(
                "[GetRootIndexForWidgetPtr] matched popup at index %zu", i));
          }
          return static_cast<uint8_t>(i);
        }
      }
    }
  }
#endif

  return 0;
}
