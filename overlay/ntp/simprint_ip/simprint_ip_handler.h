// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_NEW_TAB_PAGE_SIMPRINT_IP_SIMPRINT_IP_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_NEW_TAB_PAGE_SIMPRINT_IP_SIMPRINT_IP_HANDLER_H_

#include <memory>
#include <string>

#include "chrome/browser/ui/webui/new_tab_page/simprint_ip/simprint_ip.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "services/network/public/cpp/simple_url_loader.h"

class Profile;

namespace network {
class SharedURLLoaderFactory;
}  // namespace network

// Handles IP detection requests sent from the NTP JS.
// Uses multiple IP detection APIs for redundancy.
class SimprintIpHandler : public simprint_ip::mojom::SimprintIpHandler {
 public:
  SimprintIpHandler(
      mojo::PendingReceiver<simprint_ip::mojom::SimprintIpHandler> handler,
      Profile* profile);
  ~SimprintIpHandler() override;

  SimprintIpHandler(const SimprintIpHandler&) = delete;
  SimprintIpHandler& operator=(const SimprintIpHandler&) = delete;

  // simprint_ip::mojom::SimprintIpHandler:
  void GetIpInfo(GetIpInfoCallback callback) override;

 private:
  // Tries the next IP API in the list.
  void TryNextApi();

  // Callback when an IP fetch completes.
  void OnIpFetchComplete(std::unique_ptr<std::string> response_body);

  mojo::Receiver<simprint_ip::mojom::SimprintIpHandler> handler_;
  raw_ptr<Profile> profile_;
  std::unique_ptr<network::SimpleURLLoader> url_loader_;

  // Current API index being tried.
  size_t current_api_index_ = 0;

  // Pending callback to return results.
  GetIpInfoCallback pending_callback_;
};

#endif  // CHROME_BROWSER_UI_WEBUI_NEW_TAB_PAGE_SIMPRINT_IP_SIMPRINT_IP_HANDLER_H_
