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

## 使用方法

### 方法 1: 使用 AuthManager（推荐）

AuthManager 提供了便捷的封装接口：

```cpp
#include "simprint/auth/auth_manager.h"

// 1. 初始化（在 EventBus 初始化后调用）
simprint::auth::AuthManager::GetInstance().Initialize();

// 2. 查询认证状态
bool is_authenticated = simprint::auth::AuthManager::GetInstance().IsAuthenticated();
if (is_authenticated) {
  std::string user_id = simprint::auth::AuthManager::GetInstance().GetUserId();
  std::string username = simprint::auth::AuthManager::GetInstance().GetUsername();
  std::string token = simprint::auth::AuthManager::GetInstance().GetAccessToken();

  LOG(INFO) << "User logged in: " << username << " (ID: " << user_id << ")";
}

// 3. 刷新认证信息（从 Tauri 查询最新状态）
simprint::auth::AuthManager::GetInstance().RefreshAuthInfo();

// 4. 监听认证状态变化
simprint::auth::AuthManager::GetInstance().AddAuthStatusChangeListener(
    [](bool is_authenticated) {
      if (is_authenticated) {
        LOG(INFO) << "User logged in";
        // 执行登录后的操作
      } else {
        LOG(INFO) << "User logged out";
        // 执行登出后的操作
      }
    });
```

### 方法 2: 直接使用 EventBus

如果需要更底层的控制：

```cpp
#include "simprint/eventbus/eventbus.h"

// 1. 设置认证状态变化回调
simprint::eventbus::EventBus::GetInstance().SetAuthStatusCallback(
    [](const simprint::eventbus::EventBus::AuthInfo& auth_info) {
      LOG(INFO) << "Auth status changed: " << auth_info.is_authenticated;
      if (auth_info.is_authenticated) {
        LOG(INFO) << "User: " << auth_info.username;
        LOG(INFO) << "User ID: " << auth_info.user_id;
        LOG(INFO) << "Token: " << (auth_info.access_token.empty() ? "empty" : "present");
      }
    });

// 2. 请求认证信息
simprint::eventbus::EventBus::GetInstance().RequestAuthInfo();

// 3. 获取缓存的认证信息
const auto& auth_info = simprint::eventbus::EventBus::GetInstance().GetCachedAuthInfo();
if (auth_info.is_authenticated) {
  // 使用认证信息
}
```

## 集成示例

### 在浏览器启动时初始化

在 `chrome_browser_main.cc` 的 `PostBrowserStart()` 中：

```cpp
void ChromeBrowserMainParts::PostBrowserStart() {
  #if BUILDFLAG(IS_WIN)
  // Initialize Simprint EventBus
  const base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch("simprint-env-id")) {
    std::string env_id = command_line->GetSwitchValueASCII("simprint-env-id");
    simprint::eventbus::EventBus::GetInstance().Initialize(env_id);

    // 初始化认证管理器
    simprint::auth::AuthManager::GetInstance().Initialize();

    // 可选：立即请求认证信息
    simprint::auth::AuthManager::GetInstance().RefreshAuthInfo();
  }
  #endif
}
```

### 在需要认证的功能中使用

```cpp
void SomeFeature::DoSomethingThatRequiresAuth() {
  if (!simprint::auth::AuthManager::GetInstance().IsAuthenticated()) {
    LOG(WARNING) << "User not authenticated, cannot proceed";
    // 显示登录提示或禁用功能
    return;
  }

  std::string token = simprint::auth::AuthManager::GetInstance().GetAccessToken();
  // 使用 token 调用需要认证的 API
}
```

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
