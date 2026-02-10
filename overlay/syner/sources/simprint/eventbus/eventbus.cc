// Copyright 2024 Simprint. All rights reserved.
// EventBus 核心实现

#include "simprint/eventbus/eventbus.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <sstream>

#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/lifetime/application_lifetime_desktop.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "simprint/console_log/console_log.h"
#include "ui/gfx/geometry/rect.h"

// 统一输出到 Chromium 日志 + 当前标签页 DevTools Console
#define SIMPRINT_LOG(msg) do { \
  std::ostringstream _oss; \
  _oss << "[Simprint] " << msg; \
  simprint::LogToConsole(_oss.str()); \
} while (0)

namespace simprint {
namespace eventbus {

namespace {
// 同步角色：0=关闭 1=主控 2=从控，由 Tauri 下发 SyncRole 消息设置；供后续视图层捕获/重放使用
std::atomic<int> g_sync_role{0};
}  // namespace

// static
int EventBus::GetSyncRole() {
  return g_sync_role.load(std::memory_order_relaxed);
}

void EventBus::SetSyncReplayCallback(SyncReplayCallback callback) {
  // 允许从 UI 线程注册（由 BrowserView::InitViews 调用），与消息接收的 pipe 线程不同
  sync_replay_callback_ = std::move(callback);
}

void EventBus::SetSyncPasteReplayCallback(SyncPasteReplayCallback callback) {
  sync_paste_replay_callback_ = std::move(callback);
}

// static
EventBus& EventBus::GetInstance() {
  static base::NoDestructor<EventBus> instance;
  return *instance;
}

EventBus::EventBus() {
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

EventBus::~EventBus() {
  Shutdown();
}

void EventBus::Initialize(const std::string& env_id) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  SIMPRINT_LOG("Initialize called with env_id=" << env_id);

  if (state_ != EventBusState::kUninitialized) {
    SIMPRINT_LOG("Already initialized");
    return;
  }

  if (env_id.empty()) {
    SIMPRINT_LOG("No env_id provided, skipping");
    return;
  }

  env_id_ = env_id;
  SIMPRINT_LOG("Creating transport...");

  SetState(EventBusState::kConnecting);

  // 创建传输层
  transport_ = CreateTransport(env_id_, this);

  SIMPRINT_LOG("Calling Connect()...");

  // 连接
  transport_->Connect();

  SIMPRINT_LOG("Connect() returned");
}

void EventBus::Shutdown() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (state_ == EventBusState::kUninitialized) {
    return;
  }

  LOG(INFO) << "EventBus: Shutting down";

  // 发送断开消息
  if (transport_ && transport_->IsConnected()) {
    SendEvent(Topic::kDisconnect);
  }

  // 断开连接
  if (transport_) {
    transport_->Disconnect();
    transport_.reset();
  }

  // 清理
  handlers_.clear();
  owned_handlers_.clear();
  handshake_complete_ = false;

  SetState(EventBusState::kUninitialized);
}

bool EventBus::IsInitialized() const {
  return state_ != EventBusState::kUninitialized;
}

bool EventBus::IsConnected() const {
  return state_ == EventBusState::kConnected && handshake_complete_;
}

bool EventBus::SendRequest(Topic topic, std::vector<uint8_t> data) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!transport_ || !transport_->IsConnected()) {
    LOG(WARNING) << "EventBus: Cannot send request - not connected";
    return false;
  }

  auto message = Message::Request(topic, std::move(data));
  LOG(INFO) << "EventBus: Sending request, topic=" << static_cast<int>(topic)
            << ", msg_id=" << message.msg_id;

  return transport_->Send(message);
}

bool EventBus::SendEvent(Topic topic, std::vector<uint8_t> data) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!transport_ || !transport_->IsConnected()) {
    LOG(WARNING) << "EventBus: Cannot send event - not connected";
    return false;
  }

  auto message = Message::Event(topic, std::move(data));
  LOG(INFO) << "EventBus: Sending event, topic=" << static_cast<int>(topic)
            << ", msg_id=" << message.msg_id;

  return transport_->Send(message);
}

void EventBus::RegisterHandler(std::unique_ptr<MessageHandler> handler) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  auto* raw_handler = handler.get();
  owned_handlers_.push_back(std::move(handler));

  for (Topic topic : raw_handler->GetTopics()) {
    handlers_[topic].push_back(raw_handler);
    LOG(INFO) << "EventBus: Registered handler for topic "
              << static_cast<int>(topic);
  }
}

void EventBus::RegisterHandler(Topic topic, HandlerFunc func) {
  RegisterHandler(std::vector<Topic>{topic}, std::move(func));
}

void EventBus::RegisterHandler(std::vector<Topic> topics, HandlerFunc func) {
  RegisterHandler(
      std::make_unique<FunctionHandler>(std::move(topics), std::move(func)));
}

void EventBus::AddListener(EventBusListener* listener) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  listeners_.push_back(listener);
}

void EventBus::RemoveListener(EventBusListener* listener) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  listeners_.erase(
      std::remove(listeners_.begin(), listeners_.end(), listener),
      listeners_.end());
}

// TransportDelegate 实现

void EventBus::OnConnected() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  SIMPRINT_LOG("OnConnected called, starting handshake");

  SetState(EventBusState::kConnected);

  // 开始握手
  DoHandshake();
}

void EventBus::OnDisconnected() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LOG(INFO) << "EventBus: Transport disconnected";

  handshake_complete_ = false;
  SetState(EventBusState::kDisconnected);
}

void EventBus::OnMessageReceived(Message message) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  LOG(INFO) << "EventBus: Received message, topic=" << static_cast<int>(message.topic)
            << ", type=" << static_cast<int>(message.msg_type)
            << ", msg_id=" << message.msg_id;

  // 检查是否是握手响应
  if (!handshake_complete_ &&
      message.topic == Topic::kHandshake &&
      message.msg_type == MessageType::kResponse &&
      message.msg_id == handshake_request_id_) {
    HandleHandshakeResponse(message);
    return;
  }

  // 分发消息
  DispatchMessage(message);
}

void EventBus::OnError(const std::string& error) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LOG(ERROR) << "EventBus: Transport error - " << error;

  handshake_complete_ = false;
  SetState(EventBusState::kError);
}

// 内部方法

void EventBus::DoHandshake() {
  SIMPRINT_LOG("DoHandshake called");

  auto handshake_data = HandshakeData::Browser(env_id_);
  auto data = handshake_data.ToBytes();
  size_t data_size = data.size();

  auto message = Message::Request(Topic::kHandshake, std::move(data));
  handshake_request_id_ = message.msg_id;

  SIMPRINT_LOG("Sending handshake, msg_id=" << handshake_request_id_ << ", data_size=" << data_size);

  bool sent = transport_->Send(message);
  SIMPRINT_LOG("Handshake send result: " << (sent ? "success" : "failed"));
}

void EventBus::HandleHandshakeResponse(const Message& message) {
  if (message.error_code != 0) {
    LOG(ERROR) << "EventBus: Handshake failed, error_code=" << message.error_code;
    SetState(EventBusState::kError);
    return;
  }

  LOG(INFO) << "EventBus: Handshake successful";
  handshake_complete_ = true;

  // 注册 Disconnect handler，收到关闭请求时关闭浏览器
  RegisterHandler(Topic::kDisconnect, [](const Message& msg) -> std::optional<HandlerResponse> {
    SIMPRINT_LOG("Received Disconnect request from Tauri, closing browser");
    // 关闭所有浏览器窗口并退出
    chrome::CloseAllBrowsersAndQuit();
    return std::nullopt;
  });

  // 注册 WindowSetBounds handler，收到窗口布局请求时调整主窗口位置和大小
  RegisterHandler(Topic::kWindowSetBounds, [](const Message& msg) -> std::optional<HandlerResponse> {
    SIMPRINT_LOG("Received WindowSetBounds request from Tauri");

    // 期望数据格式：按小端序编码的 4 个 int32_t：x, y, width, height
    if (msg.data.size() < sizeof(int32_t) * 4) {
      LOG(WARNING) << "WindowSetBounds: invalid payload size=" << msg.data.size();
      return std::nullopt;
    }

    auto read_int32_le = [&msg](size_t index) -> int32_t {
      const size_t offset = index * sizeof(int32_t);
      // 已在调用处保证总长度 >= 4 * sizeof(int32_t)，这里再做一次防御性检查
      if (offset + sizeof(int32_t) > msg.data.size()) {
        return 0;
      }
      const uint8_t b0 = msg.data[offset];
      const uint8_t b1 = msg.data[offset + 1];
      const uint8_t b2 = msg.data[offset + 2];
      const uint8_t b3 = msg.data[offset + 3];
      uint32_t value = static_cast<uint32_t>(b0) |
                       (static_cast<uint32_t>(b1) << 8) |
                       (static_cast<uint32_t>(b2) << 16) |
                       (static_cast<uint32_t>(b3) << 24);
      return static_cast<int32_t>(value);
    };

    int32_t x = read_int32_le(0);
    int32_t y = read_int32_le(1);
    int32_t width = read_int32_le(2);
    int32_t height = read_int32_le(3);

    // 简单校验，避免传入非法尺寸
    if (width <= 0 || height <= 0) {
      LOG(WARNING) << "WindowSetBounds: invalid size, width=" << width
                   << ", height=" << height;
      return std::nullopt;
    }

    // 找到当前活动浏览器窗口
    Browser* browser = chrome::FindLastActive();
    if (!browser || !browser->window()) {
      LOG(WARNING) << "WindowSetBounds: no active browser window found";
      return std::nullopt;
    }

    gfx::Rect bounds(x, y, width, height);
    SIMPRINT_LOG("WindowSetBounds: applying bounds x=" << x
                                                       << ", y=" << y
                                                       << ", w=" << width
                                                       << ", h=" << height);
    browser->window()->SetBounds(bounds);

    return std::nullopt;
  });

  // 注册 SyncRole handler：设置本机为主控(1)/从控(2)/关闭(0)；捕获与重放由视图层实现
  RegisterHandler(Topic::kSyncRole, [](const Message& msg) -> std::optional<HandlerResponse> {
    if (!msg.data.empty()) {
      int role = static_cast<int>(msg.data[0]);
      if (role >= 0 && role <= 2) {
        g_sync_role.store(role, std::memory_order_relaxed);
        SIMPRINT_LOG("SyncRole set to " << role);
      }
    }
    return std::nullopt;
  });

  // 注册 SyncInputEvent handler：从控时调用已注册的 sync_replay_callback_，在视图层重放
  RegisterHandler(Topic::kSyncInputEvent, [this](const Message& msg) -> std::optional<HandlerResponse> {
    if (GetSyncRole() != 2) {
      VLOG(1) << "EventBus: SyncInputEvent ignored (role=" << GetSyncRole() << ", need 2=slave)";
      return std::nullopt;
    }
    if (!sync_replay_callback_) {
      LOG(WARNING) << "EventBus: SyncInputEvent received but sync_replay_callback_ is null";
      return std::nullopt;
    }
    sync_replay_callback_(msg.data);
    return std::nullopt;
  });

  // 注册 SyncPaste handler：从控时通过 sync_paste_replay_callback_ 插入文本
  RegisterHandler(Topic::kSyncPaste, [this](const Message& msg) -> std::optional<HandlerResponse> {
    if (GetSyncRole() != 2) return std::nullopt;
    if (!sync_paste_replay_callback_) return std::nullopt;
    std::string utf8(msg.data.begin(), msg.data.end());
    std::u16string text;
    if (!base::UTF8ToUTF16(utf8.data(), utf8.size(), &text)) return std::nullopt;
    sync_paste_replay_callback_(std::move(text));
    return std::nullopt;
  });

  NotifyHandshakeComplete();
}

void EventBus::DispatchMessage(const Message& message) {
  auto it = handlers_.find(message.topic);
  if (it == handlers_.end()) {
    LOG(WARNING) << "EventBus: No handler for topic "
                 << static_cast<int>(message.topic);
    return;
  }

  for (auto* handler : it->second) {
    auto response = handler->HandleMessage(message);

    // 如果是 Request 且处理器返回了响应，发送响应
    if (message.msg_type == MessageType::kRequest && response && transport_) {
      auto resp_msg = Message::Response(
          message.msg_id,
          message.topic,
          response->error_code,
          std::move(response->data));

      transport_->Send(resp_msg);
    }
  }
}

void EventBus::SetState(EventBusState state) {
  if (state_ == state) {
    return;
  }

  state_ = state;
  NotifyListeners(state);
}

void EventBus::NotifyListeners(EventBusState state) {
  for (auto* listener : listeners_) {
    listener->OnStateChanged(state);
  }
}

void EventBus::NotifyHandshakeComplete() {
  for (auto* listener : listeners_) {
    listener->OnHandshakeComplete();
  }
}

}  // namespace eventbus
}  // namespace simprint
