// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMPRINT_PROXY_PROXY_CONFIG_SERVICE_H_
#define SIMPRINT_PROXY_PROXY_CONFIG_SERVICE_H_

#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/sequence_checker.h"
#include "net/proxy_resolution/proxy_config_service.h"
#include "net/proxy_resolution/proxy_config_with_annotation.h"

namespace simprint {

// Custom ProxyConfigService for Simprint browser that allows dynamic
// proxy configuration updates via IPC.
class SimprintProxyConfigService : public net::ProxyConfigService {
 public:
  SimprintProxyConfigService();
  ~SimprintProxyConfigService() override;

  SimprintProxyConfigService(const SimprintProxyConfigService&) = delete;
  SimprintProxyConfigService& operator=(const SimprintProxyConfigService&) =
      delete;

  // ProxyConfigService implementation
  void AddObserver(Observer* observer) override;
  void RemoveObserver(Observer* observer) override;
  ConfigAvailability GetLatestProxyConfig(
      net::ProxyConfigWithAnnotation* config) override;

  // Update proxy configuration (called from IPC handler)
  void UpdateProxyConfig(const net::ProxyConfig& config);

  // Clear proxy configuration (use direct connection)
  void ClearProxyConfig();

 private:
  // Notify all observers that configuration has changed
  void NotifyObservers();

  // Current proxy configuration
  net::ProxyConfigWithAnnotation current_config_;

  // Configuration availability status
  ConfigAvailability availability_;

  // Observer list
  base::ObserverList<Observer>::Unchecked observers_;

  // Sequence checker
  SEQUENCE_CHECKER(sequence_checker_);

  base::WeakPtrFactory<SimprintProxyConfigService> weak_factory_{this};
};

}  // namespace simprint

#endif  // SIMPRINT_PROXY_PROXY_CONFIG_SERVICE_H_
