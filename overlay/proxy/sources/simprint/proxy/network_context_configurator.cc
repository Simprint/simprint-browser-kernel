// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "simprint/proxy/network_context_configurator.h"

#include "base/check.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/network_service_instance.h"
#include "net/socket/socks5_auth_cache.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "services/network/public/mojom/network_service.mojom.h"

namespace simprint {

ProxyAuthConfig::ProxyAuthConfig() = default;
ProxyAuthConfig::ProxyAuthConfig(const ProxyAuthConfig&) = default;
ProxyAuthConfig& ProxyAuthConfig::operator=(const ProxyAuthConfig&) = default;
ProxyAuthConfig::~ProxyAuthConfig() = default;

bool ProxyAuthConfig::HasCredentials() const {
  return !credentials.Empty();
}

NetworkContextConfigurator::NetworkContextConfigurator() {
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

NetworkContextConfigurator::~NetworkContextConfigurator() = default;

void NetworkContextConfigurator::ConfigureNetworkContextParams(
    network::mojom::NetworkContextParams* network_context_params) const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(network_context_params);

  if (!network_context_params->http_auth_static_network_context_params) {
    network_context_params->http_auth_static_network_context_params =
        network::mojom::HttpAuthStaticNetworkContextParams::New();
  }

  network_context_params->socks5_auth_entries = BuildSocks5AuthEntries();
}

void NetworkContextConfigurator::UpdateProxyAuthEntries(
    std::vector<ProxyAuthConfig> auth_entries) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  proxy_auth_configs_.clear();
  net::SOCKS5AuthCache::GetInstance().ClearCredentials();
  for (const auto& auth_entry : auth_entries) {
    if (auth_entry.HasCredentials()) {
      proxy_auth_configs_.insert_or_assign(auth_entry.challenger,
                                           auth_entry.credentials);
      if (auth_entry.challenger.scheme == "socks5") {
        net::SOCKS5AuthCache::GetInstance().SetCredentials(
            auth_entry.challenger.host_port_pair, auth_entry.credentials);
      }
    }
  }

  PushSocks5AuthEntriesToNetworkService();
  LOG(INFO) << "Updated Simprint proxy auth entries: "
            << proxy_auth_configs_.size();
}

void NetworkContextConfigurator::ClearProxyAuthConfig() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  proxy_auth_configs_.clear();
  net::SOCKS5AuthCache::GetInstance().ClearCredentials();
  PushSocks5AuthEntriesToNetworkService();
  LOG(INFO) << "Cleared Simprint proxy auth entries";
}

std::optional<net::AuthCredentials>
NetworkContextConfigurator::GetProxyAuthCredentials(
    const net::AuthChallengeInfo& auth_info,
    bool first_auth_attempt) const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!first_auth_attempt || !auth_info.is_proxy) {
    return std::nullopt;
  }

  for (const auto& [configured_endpoint, credentials] : proxy_auth_configs_) {
    if (configured_endpoint.scheme == auth_info.challenger.scheme() &&
        configured_endpoint.host_port_pair.host() ==
            auth_info.challenger.host() &&
        configured_endpoint.host_port_pair.port() ==
            auth_info.challenger.port()) {
      return credentials;
    }
  }

  for (const auto& [configured_endpoint, credentials] : proxy_auth_configs_) {
    if (configured_endpoint.host_port_pair.host() == auth_info.challenger.host() &&
        configured_endpoint.host_port_pair.port() == auth_info.challenger.port()) {
      return credentials;
    }
  }

  return std::nullopt;
}

std::vector<network::mojom::Socks5AuthEntryPtr>
NetworkContextConfigurator::BuildSocks5AuthEntries() const {
  std::vector<network::mojom::Socks5AuthEntryPtr> auth_entries;

  for (const auto& [endpoint, credentials] : proxy_auth_configs_) {
    if (endpoint.scheme != "socks5" || credentials.Empty()) {
      continue;
    }

    auto auth_entry = network::mojom::Socks5AuthEntry::New();
    auth_entry->host = endpoint.host_port_pair.host();
    auth_entry->port = endpoint.host_port_pair.port();
    auth_entry->username = base::UTF16ToUTF8(credentials.username());
    auth_entry->password = base::UTF16ToUTF8(credentials.password());
    auth_entries.push_back(std::move(auth_entry));
  }

  return auth_entries;
}

void NetworkContextConfigurator::PushSocks5AuthEntriesToNetworkService() const {
  content::GetNetworkService()->SetSocks5AuthEntries(BuildSocks5AuthEntries());
}

}  // namespace simprint
