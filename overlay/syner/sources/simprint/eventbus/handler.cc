// Copyright 2024 Simprint. All rights reserved.
// EventBus 消息处理器实现

#include "simprint/eventbus/handler.h"

namespace simprint {
namespace eventbus {

// HandlerResponse 实现

HandlerResponse::HandlerResponse() = default;

HandlerResponse::HandlerResponse(ErrorCode code, std::vector<uint8_t> response_data)
    : error_code(code), data(std::move(response_data)) {}

HandlerResponse::~HandlerResponse() = default;
HandlerResponse::HandlerResponse(const HandlerResponse&) = default;
HandlerResponse& HandlerResponse::operator=(const HandlerResponse&) = default;
HandlerResponse::HandlerResponse(HandlerResponse&&) = default;
HandlerResponse& HandlerResponse::operator=(HandlerResponse&&) = default;

HandlerResponse HandlerResponse::Success(std::vector<uint8_t> data) {
  return HandlerResponse(ErrorCode::kSuccess, std::move(data));
}

HandlerResponse HandlerResponse::Error(ErrorCode code, std::vector<uint8_t> data) {
  return HandlerResponse(code, std::move(data));
}

// FunctionHandler 实现

FunctionHandler::FunctionHandler(std::vector<Topic> topics, HandlerFunc func)
    : topics_(std::move(topics)), func_(std::move(func)) {}

FunctionHandler::~FunctionHandler() = default;

std::optional<HandlerResponse> FunctionHandler::HandleMessage(const Message& message) {
  return func_(message);
}

std::vector<Topic> FunctionHandler::GetTopics() const {
  return topics_;
}

}  // namespace eventbus
}  // namespace simprint
