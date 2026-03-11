// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "simprint/proxy/proxy_config_service_wrapper.h"

#include "base/check.h"
#include "simprint/proxy/proxy_config_service.h"

namespace simprint {

ProxyConfigServiceWrapper::ProxyConfigServiceWrapper(
    SimprintProxyConfigService* proxy_service)
    : proxy_service_(proxy_service) {
  DCHECK(proxy_service_);
  // Register ourselves as an observer of the underlying service
  proxy_service_->AddObserver(this);
}

ProxyConfigServiceWrapper::~ProxyConfigServiceWrapper() {
  if (proxy_service_) {
    proxy_service_->RemoveObserver(this);
  }
}

void ProxyConfigServiceWrapper::AddObserver(Observer* observer) {
  DCHECK(!observer_) << "Only one observer is supported";
  observer_ = observer;
}

void ProxyConfigServiceWrapper::RemoveObserver(Observer* observer) {
  if (observer_ == observer) {
    observer_ = nullptr;
  }
}

net::ProxyConfigService::ConfigAvailability
ProxyConfigServiceWrapper::GetLatestProxyConfig(
    net::ProxyConfigWithAnnotation* config) {
  DCHECK(config);
  return proxy_service_->GetLatestProxyConfig(config);
}

void ProxyConfigServiceWrapper::OnProxyConfigChanged(
    const net::ProxyConfigWithAnnotation& config,
    ConfigAvailability availability) {
  // Forward the notification to our observer
  if (observer_) {
    observer_->OnProxyConfigChanged(config, availability);
  }
}

}  // namespace simprint
