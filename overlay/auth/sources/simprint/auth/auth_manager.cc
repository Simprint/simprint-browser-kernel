// Copyright 2024 Simprint. All rights reserved.
// 认证管理器实现

#include "simprint/auth/auth_manager.h"

#include "base/logging.h"
#include "base/no_destructor.h"

namespace simprint {
namespace auth {

// static
AuthManager& AuthManager::GetInstance() {
  static base::NoDestructor<AuthManager> instance;
  return *instance;
}

AuthManager::AuthManager() = default;

AuthManager::~AuthManager() = default;

void AuthManager::Initialize() {
  // 设置认证状态变化回调
  eventbus::EventBus::GetInstance().SetAuthStatusCallback(
      [this](const eventbus::EventBus::AuthInfo& auth_info) {
        OnAuthStatusChanged(auth_info);
      });

  LOG(INFO) << "AuthManager: Initialized";
}

bool AuthManager::IsAuthenticated() const {
  return eventbus::EventBus::GetInstance().GetCachedAuthInfo().is_authenticated;
}

std::string AuthManager::GetAccessToken() const {
  return eventbus::EventBus::GetInstance().GetCachedAuthInfo().access_token;
}

std::string AuthManager::GetUserId() const {
  return eventbus::EventBus::GetInstance().GetCachedAuthInfo().user_id;
}

std::string AuthManager::GetUsername() const {
  return eventbus::EventBus::GetInstance().GetCachedAuthInfo().username;
}

void AuthManager::RefreshAuthInfo() {
  LOG(INFO) << "AuthManager: Requesting auth info refresh";
  eventbus::EventBus::GetInstance().RequestAuthInfo();
}

void AuthManager::AddAuthStatusChangeListener(AuthStatusChangeCallback callback) {
  listeners_.push_back(std::move(callback));
}

void AuthManager::OnAuthStatusChanged(const eventbus::EventBus::AuthInfo& auth_info) {
  LOG(INFO) << "AuthManager: Auth status changed, is_authenticated="
            << auth_info.is_authenticated;

  // 通知所有监听器
  for (const auto& listener : listeners_) {
    listener(auth_info.is_authenticated);
  }
}

}  // namespace auth
}  // namespace simprint
