// Copyright 2024 Simprint. All rights reserved.
// EventBus 消息实现

#include "simprint/eventbus/message.h"

#include <atomic>
#include <array>

#include "base/containers/span.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/numerics/byte_conversions.h"
#include "base/values.h"

// SAFETY: This file handles binary protocol parsing with known-size fields.
// All buffer accesses are bounds-checked before use.
#pragma allow_unsafe_buffers

namespace simprint {
namespace eventbus {

namespace {

// Magic 序列作为 span
constexpr std::array<uint8_t, 4> kMagicArray = {0x53, 0x49, 0x4D, 0x00};

// 全局消息 ID 计数器
std::atomic<uint32_t> g_msg_id_counter{1};

uint32_t NextMsgId() {
  return g_msg_id_counter.fetch_add(1, std::memory_order_seq_cst);
}

// 写入小端序 uint32
void WriteUint32LE(std::vector<uint8_t>& buf, uint32_t value) {
  buf.push_back(static_cast<uint8_t>(value & 0xFF));
  buf.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
  buf.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
  buf.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

// 读取小端序 uint32
uint32_t ReadUint32LE(const uint8_t* data) {
  return static_cast<uint32_t>(data[0]) |
         (static_cast<uint32_t>(data[1]) << 8) |
         (static_cast<uint32_t>(data[2]) << 16) |
         (static_cast<uint32_t>(data[3]) << 24);
}

// 写入小端序 uint16
void WriteUint16LE(std::vector<uint8_t>& buf, uint16_t value) {
  buf.push_back(static_cast<uint8_t>(value & 0xFF));
  buf.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

// 读取小端序 uint16
uint16_t ReadUint16LE(const uint8_t* data) {
  return static_cast<uint16_t>(data[0]) |
         (static_cast<uint16_t>(data[1]) << 8);
}

// 写入小端序 int32
void WriteInt32LE(std::vector<uint8_t>& buf, int32_t value) {
  WriteUint32LE(buf, static_cast<uint32_t>(value));
}

// 读取小端序 int32
int32_t ReadInt32LE(const uint8_t* data) {
  return static_cast<int32_t>(ReadUint32LE(data));
}

// 比较 magic 序列
bool CheckMagic(const uint8_t* data) {
  return data[0] == kMagicArray[0] &&
         data[1] == kMagicArray[1] &&
         data[2] == kMagicArray[2] &&
         data[3] == kMagicArray[3];
}

}  // namespace

// Message 实现

Message::Message() = default;
Message::~Message() = default;
Message::Message(const Message&) = default;
Message& Message::operator=(const Message&) = default;
Message::Message(Message&&) = default;
Message& Message::operator=(Message&&) = default;

Message Message::Request(Topic topic, std::vector<uint8_t> data) {
  Message msg;
  msg.msg_id = NextMsgId();
  msg.msg_type = MessageType::kRequest;
  msg.topic = topic;
  msg.error_code = 0;
  msg.data = std::move(data);
  return msg;
}

Message Message::Event(Topic topic, std::vector<uint8_t> data) {
  Message msg;
  msg.msg_id = NextMsgId();
  msg.msg_type = MessageType::kEvent;
  msg.topic = topic;
  msg.error_code = 0;
  msg.data = std::move(data);
  return msg;
}

Message Message::Response(uint32_t request_id,
                          Topic topic,
                          ErrorCode error_code,
                          std::vector<uint8_t> data) {
  Message msg;
  msg.msg_id = request_id;
  msg.msg_type = MessageType::kResponse;
  msg.topic = topic;
  msg.error_code = static_cast<int32_t>(error_code);
  msg.data = std::move(data);
  return msg;
}

Message Message::SuccessResponse(uint32_t request_id,
                                 Topic topic,
                                 std::vector<uint8_t> data) {
  return Response(request_id, topic, ErrorCode::kSuccess, std::move(data));
}

std::vector<uint8_t> Message::Encode() const {
  // 构建 payload (简单的二进制格式)
  // msg_id(4) + msg_type(1) + topic(2) + error_code(4) + data_len(4) + data
  std::vector<uint8_t> payload;
  payload.reserve(15 + data.size());

  WriteUint32LE(payload, msg_id);
  payload.push_back(static_cast<uint8_t>(msg_type));
  WriteUint16LE(payload, static_cast<uint16_t>(topic));
  WriteInt32LE(payload, error_code);
  WriteUint32LE(payload, static_cast<uint32_t>(data.size()));
  payload.insert(payload.end(), data.begin(), data.end());

  // 构建完整帧
  std::vector<uint8_t> frame;
  frame.reserve(kHeaderSize + payload.size());

  // Magic
  frame.insert(frame.end(), kMagicArray.begin(), kMagicArray.end());
  // Version
  frame.push_back(kVersion);
  // Length
  WriteUint32LE(frame, static_cast<uint32_t>(payload.size()));
  // Payload
  frame.insert(frame.end(), payload.begin(), payload.end());

  return frame;
}

std::optional<Message> Message::Decode(const uint8_t* data, size_t len) {
  if (len < kHeaderSize) {
    return std::nullopt;
  }

  // 验证 Magic
  if (!CheckMagic(data)) {
    return std::nullopt;
  }

  // 验证 Version
  if (data[4] != kVersion) {
    return std::nullopt;
  }

  // 读取 payload 长度
  uint32_t payload_len = ReadUint32LE(data + 5);
  if (len < kHeaderSize + payload_len) {
    return std::nullopt;
  }

  // 解析 payload
  const uint8_t* payload = data + kHeaderSize;
  if (payload_len < 15) {  // 最小 payload 大小
    return std::nullopt;
  }

  Message msg;
  msg.msg_id = ReadUint32LE(payload);
  msg.msg_type = static_cast<MessageType>(payload[4]);
  msg.topic = static_cast<Topic>(ReadUint16LE(payload + 5));
  msg.error_code = ReadInt32LE(payload + 7);

  uint32_t data_len = ReadUint32LE(payload + 11);
  if (payload_len < 15 + data_len) {
    return std::nullopt;
  }

  msg.data.assign(payload + 15, payload + 15 + data_len);

  return msg;
}

std::optional<std::pair<Message, size_t>> Message::TryDecode(const uint8_t* data,
                                                              size_t len) {
  if (len < kHeaderSize) {
    return std::nullopt;
  }

  // 验证 Magic
  if (!CheckMagic(data)) {
    return std::nullopt;
  }

  // 读取 payload 长度
  uint32_t payload_len = ReadUint32LE(data + 5);
  size_t total_len = kHeaderSize + payload_len;

  if (len < total_len) {
    return std::nullopt;  // 数据不足
  }

  auto msg = Decode(data, total_len);
  if (!msg) {
    return std::nullopt;
  }

  return std::make_pair(std::move(*msg), total_len);
}

// HandshakeData 实现

HandshakeData HandshakeData::Browser(const std::string& env_id) {
  HandshakeData data;
  data.version = kVersion;
  data.env_id = env_id;
  data.client_type = "browser";
  return data;
}

std::vector<uint8_t> HandshakeData::ToBytes() const {
  // 使用简单的 JSON 格式
  base::Value::Dict dict;
  dict.Set("version", static_cast<int>(version));
  dict.Set("env_id", env_id);
  dict.Set("client_type", client_type);

  std::string json;
  base::JSONWriter::Write(base::Value(std::move(dict)), &json);

  return std::vector<uint8_t>(json.begin(), json.end());
}

std::optional<HandshakeData> HandshakeData::FromBytes(const uint8_t* data,
                                                       size_t len) {
  std::string json(reinterpret_cast<const char*>(data), len);
  auto value = base::JSONReader::Read(json, base::JSON_PARSE_RFC);
  if (!value || !value->is_dict()) {
    return std::nullopt;
  }

  const auto& dict = value->GetDict();
  HandshakeData result;

  if (auto v = dict.FindInt("version")) {
    result.version = static_cast<uint8_t>(*v);
  }
  if (const std::string* v = dict.FindString("env_id")) {
    result.env_id = *v;
  }
  if (const std::string* v = dict.FindString("client_type")) {
    result.client_type = *v;
  }

  return result;
}

}  // namespace eventbus
}  // namespace simprint
