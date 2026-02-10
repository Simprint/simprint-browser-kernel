// Copyright 2024 Simprint. All rights reserved.
// EventBus 消息定义

#ifndef SIMPRINT_EVENTBUS_MESSAGE_H_
#define SIMPRINT_EVENTBUS_MESSAGE_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "simprint/eventbus/topics.h"

namespace simprint {
namespace eventbus {

// 协议常量
constexpr uint8_t kMagic[4] = {0x53, 0x49, 0x4D, 0x00};  // "SIM\0"
constexpr uint8_t kVersion = 0x01;
constexpr size_t kHeaderSize = 9;  // Magic(4) + Version(1) + Length(4)

// EventBus 消息
struct Message {
  uint32_t msg_id = 0;
  MessageType msg_type = MessageType::kEvent;
  Topic topic = Topic::kHandshake;
  int32_t error_code = 0;
  std::vector<uint8_t> data;

  Message();
  ~Message();
  Message(const Message&);
  Message& operator=(const Message&);
  Message(Message&&);
  Message& operator=(Message&&);

  // 创建请求消息
  static Message Request(Topic topic, std::vector<uint8_t> data);

  // 创建事件消息
  static Message Event(Topic topic, std::vector<uint8_t> data);

  // 创建响应消息
  static Message Response(uint32_t request_id,
                          Topic topic,
                          ErrorCode error_code,
                          std::vector<uint8_t> data = {});

  // 创建成功响应
  static Message SuccessResponse(uint32_t request_id,
                                 Topic topic,
                                 std::vector<uint8_t> data = {});

  // 编码消息为字节流
  std::vector<uint8_t> Encode() const;

  // 从字节流解码消息
  static std::optional<Message> Decode(const uint8_t* data, size_t len);

  // 尝试从缓冲区解码消息，返回消息和消耗的字节数
  static std::optional<std::pair<Message, size_t>> TryDecode(const uint8_t* data,
                                                              size_t len);
};

// 握手消息数据
struct HandshakeData {
  uint8_t version = kVersion;
  std::string env_id;
  std::string client_type;  // "browser" 或 "tauri"

  // 创建浏览器端握手数据
  static HandshakeData Browser(const std::string& env_id);

  // 序列化为字节
  std::vector<uint8_t> ToBytes() const;

  // 从字节反序列化
  static std::optional<HandshakeData> FromBytes(const uint8_t* data, size_t len);
};

}  // namespace eventbus
}  // namespace simprint

#endif  // SIMPRINT_EVENTBUS_MESSAGE_H_
