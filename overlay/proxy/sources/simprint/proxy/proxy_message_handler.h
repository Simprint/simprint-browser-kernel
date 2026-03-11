// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMPRINT_PROXY_PROXY_MESSAGE_HANDLER_H_
#define SIMPRINT_PROXY_PROXY_MESSAGE_HANDLER_H_

#include "base/memory/raw_ptr.h"
#include "simprint/eventbus/handler.h"
#include "simprint/proxy/proxy_config_parser.h"

namespace simprint {

class NetworkContextConfigurator;
class SimprintProxyConfigService;

// MessageHandler implementation for proxy configuration messages
// Handles Topic::kProxySet and Topic::kLaunchConfig
class ProxyMessageHandler : public eventbus::MessageHandler {
 public:
  ProxyMessageHandler(SimprintProxyConfigService* proxy_service,
                      NetworkContextConfigurator* network_context_configurator);
  ~ProxyMessageHandler() override;

  ProxyMessageHandler(const ProxyMessageHandler&) = delete;
  ProxyMessageHandler& operator=(const ProxyMessageHandler&) = delete;

  // MessageHandler implementation
  std::optional<eventbus::HandlerResponse> HandleMessage(
      const eventbus::Message& message) override;
  std::vector<eventbus::Topic> GetTopics() const override;

 private:
  // Handle proxy configuration update (Topic::kProxySet)
  bool HandleProxySet(const std::vector<uint8_t>& data);

  // Handle launch configuration (Topic::kLaunchConfig)
  void HandleLaunchConfig(const std::vector<uint8_t>& data);

  void ApplyProxyConfig(const ParsedProxyConfig& parsed_config);

  // Pointer to proxy configuration service
  raw_ptr<SimprintProxyConfigService> proxy_service_;
  raw_ptr<NetworkContextConfigurator> network_context_configurator_;
};

}  // namespace simprint

#endif  // SIMPRINT_PROXY_PROXY_MESSAGE_HANDLER_H_
