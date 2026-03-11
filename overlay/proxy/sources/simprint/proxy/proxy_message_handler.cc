// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "simprint/proxy/proxy_message_handler.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "content/public/browser/network_service_instance.h"
#include "services/network/public/mojom/network_service.mojom.h"
#include "simprint/eventbus/topics.h"
#include "simprint/proxy/network_context_configurator.h"
#include "simprint/proxy/proxy_config_parser.h"
#include "simprint/proxy/proxy_config_service.h"
#include "simprint/simprint_browser_context.h"

namespace simprint {

ProxyMessageHandler::ProxyMessageHandler(
    SimprintProxyConfigService* proxy_service,
    NetworkContextConfigurator* network_context_configurator)
    : proxy_service_(proxy_service),
      network_context_configurator_(network_context_configurator) {
  DCHECK(proxy_service_);
  DCHECK(network_context_configurator_);
}

ProxyMessageHandler::~ProxyMessageHandler() = default;

std::optional<eventbus::HandlerResponse> ProxyMessageHandler::HandleMessage(
    const eventbus::Message& message) {
  // These are event messages, no response needed
  switch (message.topic) {
    case eventbus::Topic::kProxySet:
      HandleProxySet(message.data);
      break;
    case eventbus::Topic::kLaunchConfig:
      HandleLaunchConfig(message.data);
      break;
    default:
      LOG(WARNING) << "Unexpected topic in ProxyMessageHandler: "
                   << static_cast<int>(message.topic);
      break;
  }

  return std::nullopt;  // No response for event messages
}

std::vector<eventbus::Topic> ProxyMessageHandler::GetTopics() const {
  return {eventbus::Topic::kProxySet, eventbus::Topic::kLaunchConfig};
}

void ProxyMessageHandler::ApplyProxyConfig(
    const ParsedProxyConfig& parsed_config) {
  LOG(INFO) << "Applying Simprint proxy config"
            << ", mode=" << parsed_config.mode
            << ", has_bypass=" << !parsed_config.bypass_list.empty()
            << ", auth_entries=" << parsed_config.auth_entries.size();

  if (parsed_config.HasAuthentication()) {
    std::vector<ProxyAuthConfig> auth_entries;
    auth_entries.reserve(parsed_config.auth_entries.size());
    for (const auto& auth_entry : parsed_config.auth_entries) {
      ProxyAuthConfig auth_config;
      auth_config.challenger = auth_entry.proxy_endpoint;
      auth_config.credentials = auth_entry.credentials;
      auth_entries.push_back(std::move(auth_config));
    }
    network_context_configurator_->UpdateProxyAuthEntries(
        std::move(auth_entries));
  } else {
    network_context_configurator_->ClearProxyAuthConfig();
  }

  LOG(INFO) << "Updating proxy configuration";
  proxy_service_->UpdateProxyConfig(parsed_config.config);
  content::GetNetworkService()->ClearHttpAuthCache();
  content::GetNetworkService()->CloseAllConnections();
}

bool ProxyMessageHandler::HandleProxySet(const std::vector<uint8_t>& data) {
  LOG(INFO) << "Received proxy configuration update";

  // Convert data to string
  std::string json_data(data.begin(), data.end());

  // Handle empty config (clear proxy)
  if (json_data.empty() || json_data == "null") {
    LOG(INFO) << "Clearing proxy configuration";
    proxy_service_->ClearProxyConfig();
    network_context_configurator_->ClearProxyAuthConfig();
    content::GetNetworkService()->ClearHttpAuthCache();
    content::GetNetworkService()->CloseAllConnections();
    return true;
  }

  // Parse proxy configuration
  auto parsed_config = ProxyConfigParser::ParseProxyConfig(json_data);
  if (!parsed_config) {
    LOG(ERROR) << "Failed to parse proxy configuration from JSON";
    return false;
  }

  // Validate configuration
  if (!ProxyConfigParser::ValidateConfig(parsed_config->config)) {
    LOG(ERROR) << "Invalid proxy configuration";
    return false;
  }

  ApplyProxyConfig(*parsed_config);
  return true;
}

void ProxyMessageHandler::HandleLaunchConfig(const std::vector<uint8_t>& data) {
  LOG(INFO) << "Received launch configuration";
  auto* simprint_context = SimprintBrowserContext::GetInstance();

  // Convert data to string
  std::string json_data(data.begin(), data.end());

  // Parse JSON
  auto parsed_json = base::JSONReader::ReadDict(json_data, base::JSON_PARSE_RFC);
  if (!parsed_json) {
    LOG(ERROR) << "Failed to parse launch config JSON";
    proxy_service_->ClearProxyConfig();
    network_context_configurator_->ClearProxyAuthConfig();
    simprint_context->MarkLaunchConfigProcessed();
    return;
  }

  const base::Value::Dict& dict = *parsed_json;

  // Check if proxy configuration is present
  const base::Value::Dict* proxy_dict = dict.FindDict("proxy");
  if (!proxy_dict) {
    LOG(INFO) << "No proxy configuration in launch config";
    proxy_service_->ClearProxyConfig();
    network_context_configurator_->ClearProxyAuthConfig();
    simprint_context->MarkLaunchConfigProcessed();
    return;
  }

  LOG(INFO) << "Launch config contains proxy payload";

  // Convert proxy dict back to JSON string
  auto proxy_json_opt = base::WriteJson(*proxy_dict);
  if (!proxy_json_opt) {
    LOG(ERROR) << "Failed to serialize proxy config";
    proxy_service_->ClearProxyConfig();
    network_context_configurator_->ClearProxyAuthConfig();
    simprint_context->MarkLaunchConfigProcessed();
    return;
  }
  std::string proxy_json = std::move(*proxy_json_opt);

  // Handle proxy configuration
  std::vector<uint8_t> proxy_data(proxy_json.begin(), proxy_json.end());
  if (!HandleProxySet(proxy_data)) {
    proxy_service_->ClearProxyConfig();
    network_context_configurator_->ClearProxyAuthConfig();
  }
  simprint_context->MarkLaunchConfigProcessed();
}

}  // namespace simprint
