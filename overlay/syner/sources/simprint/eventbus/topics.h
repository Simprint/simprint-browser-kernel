// Copyright 2024 Simprint. All rights reserved.
// EventBus 主题定义

#ifndef SIMPRINT_EVENTBUS_TOPICS_H_
#define SIMPRINT_EVENTBUS_TOPICS_H_

#include <cstdint>

namespace simprint {
namespace eventbus {

// 消息主题
enum class Topic : uint16_t {
  // ========== 系统消息 (0x0000 - 0x00FF) ==========
  kHandshake = 0x0001,    // 握手
  kHeartbeat = 0x0002,    // 心跳（预留）
  kDisconnect = 0x0003,   // 断开连接

  // ========== 配置相关 (0x0100 - 0x01FF) ==========
  kConfigInit = 0x0100,    // 配置初始化
  kConfigUpdate = 0x0101,  // 配置更新

  // ========== 指纹相关 (0x0200 - 0x02FF) ==========
  kFingerprintApply = 0x0200,  // 应用指纹配置
  kFingerprintQuery = 0x0201,  // 查询指纹状态

  // ========== 代理相关 (0x0300 - 0x03FF) ==========
  kProxySet = 0x0300,     // 设置代理
  kProxyBypass = 0x0301,  // 代理旁路规则

  // ========== RPA 相关 (0x0400 - 0x04FF) ==========
  kRpaCommand = 0x0400,  // RPA 命令
  kRpaResult = 0x0401,   // RPA 结果
  kRpaEvent = 0x0402,    // RPA 事件

  // ========== 页面事件 (0x0500 - 0x05FF) ==========
  kPageLoad = 0x0500,         // 页面加载完成
  kPageClose = 0x0501,        // 页面关闭
  kNavigationStart = 0x0502,  // 导航开始

  // ========== 鉴权相关 (0x0600 - 0x06FF) ==========
  kAuthRequest = 0x0600,   // 鉴权请求
  kAuthResponse = 0x0601,  // 鉴权响应

  // ========== 窗口控制 (0x0700 - 0x07FF) ==========
  kWindowSetBounds = 0x0700,  // 设置浏览器主窗口位置和大小

  // ========== 同步输入 (0x0800 - 0x08FF) ==========
  kSyncInputEvent = 0x0800,  // 同步输入事件：主控发送，Tauri 转发给从控
  kSyncRole = 0x0801,        // 同步角色：0=关闭 1=主控 2=从控
  kSyncInputDebug = 0x0802,  // 同步输入调试日志：C++ 发往 Tauri 打印
  kSyncPaste = 0x0803,       // 粘贴：主控发送剪贴板文本，从控 InsertText
};

// 消息类型
enum class MessageType : uint8_t {
  kRequest = 1,   // 请求（需要响应）
  kResponse = 2,  // 响应
  kEvent = 3,     // 事件（单向通知）
};

// 错误码
enum class ErrorCode : int32_t {
  kSuccess = 0,

  // 传输层错误 (1xxx)
  kConnectionFailed = 1001,
  kConnectionLost = 1002,
  kSendFailed = 1003,

  // 协议错误 (2xxx)
  kInvalidMessage = 2001,
  kUnknownTopic = 2002,
  kDecodeFailed = 2003,

  // 业务错误 (3xxx)
  kInvalidConfig = 3001,
  kPermissionDenied = 3002,
};

}  // namespace eventbus
}  // namespace simprint

#endif  // SIMPRINT_EVENTBUS_TOPICS_H_
