// Copyright 2024 Simprint. All rights reserved.
// 认证管理器 - 封装 EventBus 认证功能的便捷接口

#ifndef SIMPRINT_AUTH_AUTH_MANAGER_H_
#define SIMPRINT_AUTH_AUTH_MANAGER_H_

#include <functional>
#include <string>

#include "base/memory/weak_ptr.h"
#include "simprint/eventbus/eventbus.h"

namespace simprint {
namespace auth {

// 认证管理器：提供便捷的认证状态查询和监听接口
class AuthManager {
 public:
  // 获取单例
  static AuthManager& GetInstance();

  AuthManager(const AuthManager&) = delete;
  AuthManager& operator=(const AuthManager&) = delete;

  // 初始化认证管理器（在 EventBus 初始化后调用）
  void Initialize();

  // 是否已认证
  bool IsAuthenticated() const;

  // 获取访问令牌
  std::string GetAccessToken() const;

  // 获取用户 ID
  std::string GetUserId() const;

  // 获取用户名
  std::string GetUsername() const;

  // 请求刷新认证信息（从 Tauri 查询最新状态）
  void RefreshAuthInfo();

  // 认证状态变化回调
  using AuthStatusChangeCallback = std::function<void(bool is_authenticated)>;

  // 添加认证状态变化监听器
  void AddAuthStatusChangeListener(AuthStatusChangeCallback callback);

 private:
  friend class base::NoDestructor<AuthManager>;

  AuthManager();
  ~AuthManager();

  // 处理认证状态变化
  void OnAuthStatusChanged(const eventbus::EventBus::AuthInfo& auth_info);

  // 认证状态变化监听器列表
  std::vector<AuthStatusChangeCallback> listeners_;

  base::WeakPtrFactory<AuthManager> weak_factory_{this};
};

}  // namespace auth
}  // namespace simprint

#endif  // SIMPRINT_AUTH_AUTH_MANAGER_H_
