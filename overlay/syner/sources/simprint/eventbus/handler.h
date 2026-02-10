// Copyright 2024 Simprint. All rights reserved.
// EventBus 消息处理器接口

#ifndef SIMPRINT_EVENTBUS_HANDLER_H_
#define SIMPRINT_EVENTBUS_HANDLER_H_

#include <functional>
#include <memory>
#include <vector>

#include "simprint/eventbus/message.h"
#include "simprint/eventbus/topics.h"

namespace simprint {
namespace eventbus {

// 消息处理器返回的响应
struct HandlerResponse {
  ErrorCode error_code = ErrorCode::kSuccess;
  std::vector<uint8_t> data;

  HandlerResponse();
  HandlerResponse(ErrorCode code, std::vector<uint8_t> response_data);
  ~HandlerResponse();
  HandlerResponse(const HandlerResponse&);
  HandlerResponse& operator=(const HandlerResponse&);
  HandlerResponse(HandlerResponse&&);
  HandlerResponse& operator=(HandlerResponse&&);

  static HandlerResponse Success(std::vector<uint8_t> data = {});
  static HandlerResponse Error(ErrorCode code, std::vector<uint8_t> data = {});
};

// 消息处理器接口
class MessageHandler {
 public:
  virtual ~MessageHandler() = default;

  // 处理消息，返回响应（仅对 Request 类型消息）
  // 返回 std::nullopt 表示不发送响应
  virtual std::optional<HandlerResponse> HandleMessage(const Message& message) = 0;

  // 获取该处理器关心的主题列表
  virtual std::vector<Topic> GetTopics() const = 0;
};

// 函数式处理器
using HandlerFunc = std::function<std::optional<HandlerResponse>(const Message&)>;

// 简单的函数式处理器包装
class FunctionHandler : public MessageHandler {
 public:
  FunctionHandler(std::vector<Topic> topics, HandlerFunc func);
  ~FunctionHandler() override;

  std::optional<HandlerResponse> HandleMessage(const Message& message) override;
  std::vector<Topic> GetTopics() const override;

 private:
  std::vector<Topic> topics_;
  HandlerFunc func_;
};

}  // namespace eventbus
}  // namespace simprint

#endif  // SIMPRINT_EVENTBUS_HANDLER_H_
