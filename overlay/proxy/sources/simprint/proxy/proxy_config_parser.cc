// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "simprint/proxy/proxy_config_parser.h"

#include <set>
#include <tuple>

#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "net/base/proxy_chain.h"
#include "net/base/proxy_server.h"

namespace simprint {

namespace {

bool IsValidProxyServer(const net::ProxyServer& proxy) {
  return proxy.is_valid() && !proxy.GetHost().empty() && proxy.GetPort() != 0;
}

bool IsValidProxyList(const net::ProxyList& proxies) {
  for (const net::ProxyChain& proxy_chain : proxies.AllChains()) {
    if (!proxy_chain.IsValid()) {
      return false;
    }

    if (proxy_chain.is_direct()) {
      continue;
    }

    for (size_t i = 0; i < proxy_chain.length(); ++i) {
      if (!IsValidProxyServer(proxy_chain.GetProxyServer(i))) {
        return false;
      }
    }
  }

  return true;
}

bool IsSupportedAuthScheme(const ProxyEndpoint& endpoint) {
  return endpoint.scheme == "http" || endpoint.scheme == "https" ||
         endpoint.scheme == "socks5";
}

void CollectProxyEndpointsFromList(const net::ProxyList& proxy_list,
                                   std::set<ProxyEndpoint>* endpoints) {
  for (const net::ProxyChain& proxy_chain : proxy_list.AllChains()) {
    if (!proxy_chain.IsValid()) {
      continue;
    }

    for (size_t i = 0; i < proxy_chain.length(); ++i) {
      const net::ProxyServer& proxy = proxy_chain.GetProxyServer(i);
      if ((!proxy.is_http() && !proxy.is_https() && !proxy.is_socks()) ||
          !IsValidProxyServer(proxy)) {
        continue;
      }
      std::string scheme = "http";
      if (proxy.is_https()) {
        scheme = "https";
      } else if (proxy.is_socks()) {
        scheme = proxy.scheme() == net::ProxyServer::SCHEME_SOCKS5 ? "socks5"
                                                                   : "socks4";
      }
      ProxyEndpoint endpoint;
      endpoint.scheme = scheme;
      endpoint.host_port_pair = net::HostPortPair(proxy.GetHost(), proxy.GetPort());
      endpoints->insert(std::move(endpoint));
    }
  }
}

std::set<ProxyEndpoint> CollectSupportedAuthEndpoints(
    const net::ProxyConfig& config) {
  std::set<ProxyEndpoint> endpoints;
  switch (config.proxy_rules().type) {
    case net::ProxyConfig::ProxyRules::Type::PROXY_LIST:
      CollectProxyEndpointsFromList(config.proxy_rules().single_proxies,
                                    &endpoints);
      break;
    case net::ProxyConfig::ProxyRules::Type::PROXY_LIST_PER_SCHEME:
      CollectProxyEndpointsFromList(config.proxy_rules().proxies_for_http,
                                    &endpoints);
      CollectProxyEndpointsFromList(config.proxy_rules().proxies_for_https,
                                    &endpoints);
      CollectProxyEndpointsFromList(config.proxy_rules().proxies_for_ftp,
                                    &endpoints);
      CollectProxyEndpointsFromList(config.proxy_rules().fallback_proxies,
                                    &endpoints);
      break;
    case net::ProxyConfig::ProxyRules::Type::EMPTY:
      break;
  }
  return endpoints;
}

bool ParseBypassListStrict(std::string_view bypass_list,
                           net::ProxyHostMatchingRules* rules) {
  DCHECK(rules);
  rules->Clear();

  for (const auto& token : base::SplitStringPiece(
           bypass_list, ";,", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY)) {
    if (!rules->AddRuleFromString(token)) {
      LOG(ERROR) << "Invalid bypass rule: " << token;
      return false;
    }
  }

  return true;
}

std::optional<ProxyEndpoint> ParseProxyEndpoint(std::string_view key) {
  const auto scheme_split = key.find("://");
  if (scheme_split == std::string_view::npos || scheme_split == 0) {
    LOG(ERROR) << "Invalid proxy auth endpoint: " << key;
    return std::nullopt;
  }

  const std::string scheme =
      base::ToLowerASCII(std::string(key.substr(0, scheme_split)));
  const std::string_view host_port = key.substr(scheme_split + 3);
  if (host_port.empty()) {
    LOG(ERROR) << "Invalid proxy auth endpoint: " << key;
    return std::nullopt;
  }

  const size_t colon = host_port.rfind(':');
  if (colon == std::string_view::npos || colon == 0 ||
      colon == host_port.size() - 1) {
    LOG(ERROR) << "Invalid proxy auth endpoint: " << key;
    return std::nullopt;
  }

  const std::string host(host_port.substr(0, colon));
  int port = 0;
  if (!base::StringToInt(host_port.substr(colon + 1), &port) || host.empty() ||
      port <= 0 || port > 65535) {
    LOG(ERROR) << "Invalid proxy auth endpoint: " << key;
    return std::nullopt;
  }

  ProxyEndpoint endpoint;
  endpoint.scheme = scheme;
  endpoint.host_port_pair = net::HostPortPair(host, static_cast<uint16_t>(port));
  if (endpoint.host_port_pair.host().empty() ||
      endpoint.host_port_pair.port() == 0) {
    LOG(ERROR) << "Invalid proxy auth endpoint: " << key;
    return std::nullopt;
  }

  return endpoint;
}

std::optional<ParsedProxyAuthEntry> ParseAuthEntry(
    std::string_view proxy_endpoint_key,
    const base::Value& auth_value) {
  const base::Value::Dict* auth_dict = auth_value.GetIfDict();
  if (!auth_dict) {
    LOG(ERROR) << "Proxy auth entry must be a dictionary";
    return std::nullopt;
  }

  const std::string* username = auth_dict->FindString("username");
  const std::string* password = auth_dict->FindString("password");
  if (!username || username->empty() || !password) {
    LOG(ERROR) << "Proxy auth entry missing username/password";
    return std::nullopt;
  }

  auto proxy_endpoint = ParseProxyEndpoint(proxy_endpoint_key);
  if (!proxy_endpoint) {
    return std::nullopt;
  }

  ParsedProxyAuthEntry auth_entry;
  auth_entry.proxy_endpoint = std::move(*proxy_endpoint);
  auth_entry.credentials = net::AuthCredentials(
      base::UTF8ToUTF16(*username), base::UTF8ToUTF16(*password));
  return auth_entry;
}

}  // namespace

ProxyEndpoint::ProxyEndpoint() = default;
ProxyEndpoint::ProxyEndpoint(const ProxyEndpoint&) = default;
ProxyEndpoint& ProxyEndpoint::operator=(const ProxyEndpoint&) = default;
ProxyEndpoint::~ProxyEndpoint() = default;

bool ProxyEndpoint::operator<(const ProxyEndpoint& other) const {
  return std::tuple(scheme, host_port_pair.host(), host_port_pair.port()) <
         std::tuple(other.scheme, other.host_port_pair.host(),
                    other.host_port_pair.port());
}

bool ProxyEndpoint::operator==(const ProxyEndpoint& other) const {
  return scheme == other.scheme && host_port_pair == other.host_port_pair;
}

ParsedProxyAuthEntry::ParsedProxyAuthEntry() = default;
ParsedProxyAuthEntry::ParsedProxyAuthEntry(const ParsedProxyAuthEntry&) =
    default;
ParsedProxyAuthEntry& ParsedProxyAuthEntry::operator=(
    const ParsedProxyAuthEntry&) = default;
ParsedProxyAuthEntry::~ParsedProxyAuthEntry() = default;

ParsedProxyConfig::ParsedProxyConfig() = default;
ParsedProxyConfig::ParsedProxyConfig(const ParsedProxyConfig&) = default;
ParsedProxyConfig& ParsedProxyConfig::operator=(const ParsedProxyConfig&) =
    default;
ParsedProxyConfig::~ParsedProxyConfig() = default;

bool ParsedProxyConfig::HasAuthentication() const {
  return !auth_entries.empty();
}

std::string ProxyEndpointToString(const ProxyEndpoint& endpoint) {
  return endpoint.scheme + "://" + endpoint.host_port_pair.ToString();
}

std::optional<ParsedProxyConfig> ProxyConfigParser::ParseProxyConfig(
    const std::string& json) {
  auto parsed_json = base::JSONReader::ReadDict(json, base::JSON_PARSE_RFC);
  if (!parsed_json) {
    LOG(ERROR) << "Failed to parse proxy config JSON";
    return std::nullopt;
  }

  ParsedProxyConfig parsed_config;
  const base::Value::Dict& dict = *parsed_json;
  const std::string* mode = dict.FindString("mode");
  if (!mode || mode->empty()) {
    LOG(ERROR) << "Missing required field 'mode' in proxy config";
    return std::nullopt;
  }

  parsed_config.mode = base::ToLowerASCII(*mode);
  if (parsed_config.mode == "fixed_servers") {
    const std::string* server = dict.FindString("server");
    if (!server || server->empty()) {
      LOG(ERROR) << "Missing required field 'server' for fixed proxy config";
      return std::nullopt;
    }

    parsed_config.server = *server;
    parsed_config.config.proxy_rules().ParseFromString(parsed_config.server);

    const std::string* bypass_list = dict.FindString("bypass_list");
    if (bypass_list && !bypass_list->empty()) {
      parsed_config.bypass_list = *bypass_list;
      if (!ParseBypassListStrict(parsed_config.bypass_list,
                                 &parsed_config.config.proxy_rules()
                                      .bypass_rules)) {
        return std::nullopt;
      }
    }
  } else {
    LOG(ERROR) << "Unsupported proxy mode: " << parsed_config.mode;
    return std::nullopt;
  }

  if (const base::Value::Dict* auth_dict = dict.FindDict("auth")) {
    const std::set<ProxyEndpoint> supported_auth_endpoints =
        CollectSupportedAuthEndpoints(parsed_config.config);

    for (const auto [proxy_endpoint_key, auth_value] : *auth_dict) {
      auto auth_entry = ParseAuthEntry(proxy_endpoint_key, auth_value);
      if (!auth_entry) {
        return std::nullopt;
      }
      if (!IsSupportedAuthScheme(auth_entry->proxy_endpoint)) {
        LOG(ERROR) << "Unsupported proxy auth endpoint scheme: "
                   << ProxyEndpointToString(auth_entry->proxy_endpoint);
        return std::nullopt;
      }
      if (supported_auth_endpoints.find(auth_entry->proxy_endpoint) ==
          supported_auth_endpoints.end()) {
        LOG(ERROR) << "Proxy auth endpoint not found in configured proxy rules: "
                   << ProxyEndpointToString(auth_entry->proxy_endpoint);
        return std::nullopt;
      }
      parsed_config.auth_entries.push_back(std::move(*auth_entry));
    }
  }

  LOG(INFO) << "Parsed Simprint proxy config"
            << ", mode=" << parsed_config.mode
            << ", has_bypass=" << !parsed_config.bypass_list.empty()
            << ", auth_entries=" << parsed_config.auth_entries.size();

  return parsed_config;
}

std::optional<net::ProxyConfig> ProxyConfigParser::ParseFromJson(
    const std::string& json) {
  auto parsed_config = ParseProxyConfig(json);
  if (!parsed_config) {
    return std::nullopt;
  }

  return parsed_config->config;
}

bool ProxyConfigParser::ValidateConfig(const net::ProxyConfig& config) {
  switch (config.proxy_rules().type) {
    case net::ProxyConfig::ProxyRules::Type::EMPTY:
      return false;
    case net::ProxyConfig::ProxyRules::Type::PROXY_LIST: {
      const net::ProxyList& proxies = config.proxy_rules().single_proxies;
      if (proxies.IsEmpty()) {
        return false;
      }
      return IsValidProxyList(proxies);
    }
    case net::ProxyConfig::ProxyRules::Type::PROXY_LIST_PER_SCHEME: {
      const auto has_valid_proxy_list = [](const net::ProxyList& proxies) {
        return proxies.IsEmpty() || IsValidProxyList(proxies);
      };
      const bool has_any_proxy =
          !config.proxy_rules().proxies_for_http.IsEmpty() ||
          !config.proxy_rules().proxies_for_https.IsEmpty() ||
          !config.proxy_rules().proxies_for_ftp.IsEmpty() ||
          !config.proxy_rules().fallback_proxies.IsEmpty();

      return has_any_proxy &&
             has_valid_proxy_list(config.proxy_rules().proxies_for_http) &&
             has_valid_proxy_list(config.proxy_rules().proxies_for_https) &&
             has_valid_proxy_list(config.proxy_rules().proxies_for_ftp) &&
             has_valid_proxy_list(config.proxy_rules().fallback_proxies);
    }
  }

  return false;
}

}  // namespace simprint
