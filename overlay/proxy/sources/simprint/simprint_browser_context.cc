// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "simprint/simprint_browser_context.h"

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/run_loop.h"
#include "base/timer/timer.h"
#include "simprint/eventbus/eventbus.h"
#include "simprint/proxy/network_context_configurator.h"
#include "simprint/proxy/proxy_config_service.h"
#include "simprint/proxy/proxy_message_handler.h"

namespace simprint {

class SimprintBrowserContext::WaitState {
 public:
  WaitState() = default;
  ~WaitState() = default;

  base::RunLoop* run_loop = nullptr;
};

// static
SimprintBrowserContext* SimprintBrowserContext::GetInstance() {
  static base::NoDestructor<SimprintBrowserContext> instance;
  return instance.get();
}

SimprintBrowserContext::SimprintBrowserContext() = default;

SimprintBrowserContext::~SimprintBrowserContext() = default;

void SimprintBrowserContext::Initialize() {
  if (initialized_) {
    LOG(WARNING) << "SimprintBrowserContext already initialized";
    return;
  }

  LOG(INFO) << "Initializing SimprintBrowserContext";

  // Create proxy configuration service
  proxy_config_service_ = std::make_unique<SimprintProxyConfigService>();
  network_context_configurator_ = std::make_unique<NetworkContextConfigurator>();

  // Create proxy message handler
  proxy_message_handler_ = std::make_unique<ProxyMessageHandler>(
      proxy_config_service_.get(), network_context_configurator_.get());
  wait_state_ = std::make_unique<WaitState>();
  launch_config_processed_ = false;

  initialized_ = true;
}

void SimprintBrowserContext::RegisterEventBusHandlers() {
  if (!initialized_) {
    LOG(WARNING)
        << "SimprintBrowserContext not initialized, cannot register handlers";
    return;
  }

  if (handlers_registered_) {
    return;
  }

  auto& eventbus = eventbus::EventBus::GetInstance();
  if (!eventbus.IsInitialized()) {
    LOG(WARNING) << "EventBus not initialized, proxy handler not registered";
    return;
  }

  eventbus.RegisterHandler(std::move(proxy_message_handler_));
  handlers_registered_ = true;
  LOG(INFO) << "Proxy message handler registered with EventBus";
}

void SimprintBrowserContext::MarkLaunchConfigProcessed() {
  if (launch_config_processed_) {
    return;
  }

  launch_config_processed_ = true;
  LOG(INFO) << "Initial Simprint LaunchConfig processed";

  if (wait_state_ && wait_state_->run_loop) {
    wait_state_->run_loop->Quit();
  }
}

void SimprintBrowserContext::WaitForInitialLaunchConfig(base::TimeDelta timeout) {
  if (launch_config_processed_) {
    return;
  }

  LOG(INFO) << "Waiting for initial Simprint LaunchConfig";

  base::RunLoop run_loop(base::RunLoop::Type::kNestableTasksAllowed);
  base::OneShotTimer timeout_timer;
  if (wait_state_) {
    wait_state_->run_loop = &run_loop;
  }
  timeout_timer.Start(
      FROM_HERE, timeout,
      base::BindOnce(
          [](base::RunLoop* loop) {
            LOG(WARNING) << "Timed out waiting for initial Simprint "
                            "LaunchConfig";
            loop->Quit();
          },
          &run_loop));
  run_loop.Run();
  if (wait_state_) {
    wait_state_->run_loop = nullptr;
  }
}

void SimprintBrowserContext::Shutdown() {
  if (!initialized_) {
    return;
  }

  LOG(INFO) << "Shutting down SimprintBrowserContext";

  proxy_message_handler_.reset();
  proxy_config_service_.reset();
  network_context_configurator_.reset();
  wait_state_.reset();
  launch_config_processed_ = false;
  handlers_registered_ = false;

  initialized_ = false;
}

}  // namespace simprint
