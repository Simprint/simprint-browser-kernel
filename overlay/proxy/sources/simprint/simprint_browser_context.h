// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMPRINT_SIMPRINT_BROWSER_CONTEXT_H_
#define SIMPRINT_SIMPRINT_BROWSER_CONTEXT_H_

#include <memory>

#include "base/no_destructor.h"
#include "base/time/time.h"

namespace simprint {

class ProxyMessageHandler;
class NetworkContextConfigurator;
class SimprintProxyConfigService;

// Global context for Simprint browser features
class SimprintBrowserContext {
 public:
  static SimprintBrowserContext* GetInstance();

  SimprintBrowserContext(const SimprintBrowserContext&) = delete;
  SimprintBrowserContext& operator=(const SimprintBrowserContext&) = delete;

  // Initialize the context (called during browser startup)
  void Initialize();

  // Register handlers with EventBus after it is initialized.
  void RegisterEventBusHandlers();

  void MarkLaunchConfigProcessed();
  void WaitForInitialLaunchConfig(base::TimeDelta timeout);

  // Shutdown the context (called during browser shutdown)
  void Shutdown();

  // Get the proxy configuration service
  SimprintProxyConfigService* proxy_config_service() {
    return proxy_config_service_.get();
  }

  // Get the proxy message handler
  ProxyMessageHandler* proxy_message_handler() {
    return proxy_message_handler_.get();
  }

  NetworkContextConfigurator* network_context_configurator() {
    return network_context_configurator_.get();
  }

 private:
  friend class base::NoDestructor<SimprintBrowserContext>;

  class WaitState;

  SimprintBrowserContext();
  ~SimprintBrowserContext();

  std::unique_ptr<SimprintProxyConfigService> proxy_config_service_;
  std::unique_ptr<NetworkContextConfigurator> network_context_configurator_;
  std::unique_ptr<ProxyMessageHandler> proxy_message_handler_;
  std::unique_ptr<WaitState> wait_state_;
  bool launch_config_processed_ = false;
  bool handlers_registered_ = false;
  bool initialized_ = false;
};

}  // namespace simprint

#endif  // SIMPRINT_SIMPRINT_BROWSER_CONTEXT_H_
