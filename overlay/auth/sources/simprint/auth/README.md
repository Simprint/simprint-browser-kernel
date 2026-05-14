# EventBus 认证功能使用指南

## 概述

浏览器端通过 EventBus 与 Tauri 进行认证信息交互，采用客户端-服务器模式：
- **浏览器（客户端）**：主动请求认证信息
- **Tauri（服务器）**：响应认证请求，并在登录/登出时主动广播状态变化

## 架构设计

### 消息流程

```
浏览器内核                    Tauri (服务端)
    |                              |
    |--- 连接 + 握手 -------------->|
    |<-- LaunchConfig -------------|  (握手后自动发送配置)
    |                              |
    |--- AuthRequest ------------->|  (按需查询)
    |<-- AuthResponse -------------|
    |                              |
    |                              | [用户登录/登出]
    |<-- AuthResponse (广播) ------|  (状态变化通知)
```

### 主题定义

- `kAuthRequest (0x0600)`: 浏览器请求认证信息
- `kAuthResponse (0x0601)`: Tauri 返回认证信息
- `kLaunchConfig (0x0900)`: Tauri 在握手后发送启动配置

## 数据结构

### AuthInfo

```cpp
struct AuthInfo {
  bool is_authenticated = false;  // 是否已认证
  std::string access_token;       // 访问令牌
  std::string user_id;            // 用户 ID
  std::string username;           // 用户名
};
```

### AuthResponse JSON 格式

Tauri 发送的 AuthResponse 消息体为 JSON 格式：

```json
{
  "is_authenticated": true,
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "user_info": {
    "user_id": "12345",
    "username": "john_doe"
  }
}
```

## 注意事项

1. **初始化顺序**：必须在 EventBus 初始化后才能初始化 AuthManager
2. **线程安全**：所有 EventBus 操作都在同一个序列上执行，无需额外的线程同步
3. **缓存机制**：AuthInfo 会在浏览器端缓存，避免频繁查询
4. **状态同步**：Tauri 在登录/登出时会主动广播状态变化，浏览器端会自动更新缓存
5. **按需查询**：如果需要确保获取最新状态，可以调用 `RefreshAuthInfo()` 主动查询

## Tauri 端实现

Tauri 端的相关实现位于：
- `src-tauri/src/infrastructure/eventbus/manager.rs`: EventBus 管理器
- `src-tauri/src/services/auth/credential.rs`: 凭证服务
- `src-tauri/src/services/auth/login.rs`: 登录服务

认证判断逻辑：检查 `access_token` 是否存在（通过 `is_login()` 函数）
