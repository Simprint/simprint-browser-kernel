// Copyright 2024 Simprint. All rights reserved.
// EventBus 核心 - 单例管理器

#ifndef SIMPRINT_EVENTBUS_EVENTBUS_H_
#define SIMPRINT_EVENTBUS_EVENTBUS_H_

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>


#include "base/memory/weak_ptr.h"
#include "base/no_destructor.h"
#include "base/sequence_checker.h"
#include "simprint/eventbus/handler.h"
#include "simprint/eventbus/message.h"
#include "simprint/eventbus/topics.h"
#include "simprint/eventbus/transport.h"

namespace simprint {
namespace eventbus {

/// 通用日志：同时输出到 Chromium LOG 与当前活动标签页的 DevTools Console（便于调试）
void LogToConsole(const std::string& message);

// EventBus 状态
enum class EventBusState {
  kUninitialized,
  kConnecting,
  kConnected,
  kDisconnected,
  kError
};

// EventBus 事件监听器
class EventBusListener {
 public:
  virtual ~EventBusListener() = default;

  // 状态变化
  virtual void OnStateChanged(EventBusState state) {}

  // 握手完成
  virtual void OnHandshakeComplete() {}
};

// EventBus 单例
class EventBus : public TransportDelegate {
 public:
  static EventBus& GetInstance();

  EventBus(const EventBus&) = delete;
  EventBus& operator=(const EventBus&) = delete;

  // 初始化 EventBus
  // env_id: 环境标识，从命令行参数 --simprint-env-id 获取
  void Initialize(const std::string& env_id);

  // 关闭 EventBus
  void Shutdown();

  // 是否已初始化
  bool IsInitialized() const;

  // 是否已连接
  bool IsConnected() const;

  // 获取当前状态
  EventBusState GetState() const { return state_; }

  // 同步角色：0=关闭 1=主控 2=从控（由 Tauri 下发 SyncRole 设置）
  static int GetSyncRole();

  // 设置同步输入重放回调：从控收到 SyncInputEvent 时在视图层重放，由 Chrome 在 UI 线程注册
  using SyncReplayCallback = std::function<void(std::vector<uint8_t>)>;
  void SetSyncReplayCallback(SyncReplayCallback callback);

  // 设置粘贴重放回调：从控收到 SyncPaste 时插入文本到当前聚焦的输入框
  using SyncPasteReplayCallback = std::function<void(std::u16string)>;
  void SetSyncPasteReplayCallback(SyncPasteReplayCallback callback);

  // 发送请求（期望响应）
  bool SendRequest(Topic topic, std::vector<uint8_t> data = {});

  // 发送事件（不期望响应）
  bool SendEvent(Topic topic, std::vector<uint8_t> data = {});

  // 注册消息处理器
  void RegisterHandler(std::unique_ptr<MessageHandler> handler);

  // 使用函数注册处理器
  void RegisterHandler(Topic topic, HandlerFunc func);
  void RegisterHandler(std::vector<Topic> topics, HandlerFunc func);

  // 添加/移除监听器
  void AddListener(EventBusListener* listener);
  void RemoveListener(EventBusListener* listener);

 private:
  friend class base::NoDestructor<EventBus>;

  EventBus();
  ~EventBus() override;

  // TransportDelegate 实现
  void OnConnected() override;
  void OnDisconnected() override;
  void OnMessageReceived(Message message) override;
  void OnError(const std::string& error) override;

  // 内部方法
  void DoHandshake();
  void HandleHandshakeResponse(const Message& message);
  void DispatchMessage(const Message& message);
  void SetState(EventBusState state);
  void NotifyListeners(EventBusState state);
  void NotifyHandshakeComplete();

  std::string env_id_;
  EventBusState state_ = EventBusState::kUninitialized;

  std::unique_ptr<Transport> transport_;

  // 消息处理器，按主题分组
  std::map<Topic, std::vector<MessageHandler*>> handlers_;
  std::vector<std::unique_ptr<MessageHandler>> owned_handlers_;

  // 监听器
  std::vector<EventBusListener*> listeners_;

  // 是否已完成握手
  bool handshake_complete_ = false;

  // 等待响应的请求 ID
  uint32_t handshake_request_id_ = 0;

  SyncReplayCallback sync_replay_callback_;
  SyncPasteReplayCallback sync_paste_replay_callback_;

  SEQUENCE_CHECKER(sequence_checker_);

  base::WeakPtrFactory<EventBus> weak_factory_{this};
};

}  // namespace eventbus
}  // namespace simprint

#endif  // SIMPRINT_EVENTBUS_EVENTBUS_H_
