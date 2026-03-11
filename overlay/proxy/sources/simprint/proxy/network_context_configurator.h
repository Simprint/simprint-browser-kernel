// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMPRINT_PROXY_NETWORK_CONTEXT_CONFIGURATOR_H_
#define SIMPRINT_PROXY_NETWORK_CONTEXT_CONFIGURATOR_H_

#include <map>
#include <optional>
#include <vector>

#include "base/sequence_checker.h"
#include "net/base/auth.h"
#include "services/network/public/mojom/network_context.mojom-forward.h"
#include "simprint/proxy/proxy_config_parser.h"

namespace simprint {

struct ProxyAuthConfig {
  ProxyAuthConfig();
  ProxyAuthConfig(const ProxyAuthConfig&);
  ProxyAuthConfig& operator=(const ProxyAuthConfig&);
  ~ProxyAuthConfig();

  bool HasCredentials() const;

  ProxyEndpoint challenger;
  net::AuthCredentials credentials;
};

// Centralizes Simprint-specific network configuration that needs to be shared
// across startup-time NetworkContext creation and runtime network auth flows.
class NetworkContextConfigurator {
 public:
  NetworkContextConfigurator();
  ~NetworkContextConfigurator();

  NetworkContextConfigurator(const NetworkContextConfigurator&) = delete;
  NetworkContextConfigurator& operator=(const NetworkContextConfigurator&) =
      delete;

  void ConfigureNetworkContextParams(
      network::mojom::NetworkContextParams* network_context_params) const;

  void UpdateProxyAuthEntries(std::vector<ProxyAuthConfig> auth_entries);
  void ClearProxyAuthConfig();

 std::optional<net::AuthCredentials> GetProxyAuthCredentials(
      const net::AuthChallengeInfo& auth_info,
      bool first_auth_attempt) const;

 private:
  std::vector<network::mojom::Socks5AuthEntryPtr> BuildSocks5AuthEntries()
      const;
  void PushSocks5AuthEntriesToNetworkService() const;

  std::map<ProxyEndpoint, net::AuthCredentials> proxy_auth_configs_;

  SEQUENCE_CHECKER(sequence_checker_);
};

}  // namespace simprint

#endif  // SIMPRINT_PROXY_NETWORK_CONTEXT_CONFIGURATOR_H_
