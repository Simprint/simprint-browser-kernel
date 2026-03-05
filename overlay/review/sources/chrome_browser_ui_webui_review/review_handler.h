// Copyright 2024 The Simprint Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_REVIEW_REVIEW_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_REVIEW_REVIEW_HANDLER_H_

#include <memory>
#include <string>

#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/webui/review/review_page.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "services/network/public/cpp/simple_url_loader.h"

class Profile;

namespace network {
class SharedURLLoaderFactory;
}  // namespace network

// Handles review page requests sent from the JS.
class ReviewPageHandler : public mojom::review_page::ReviewPageHandler {
 public:
  ReviewPageHandler(
      mojo::PendingReceiver<mojom::review_page::ReviewPageHandler> handler,
      Profile* profile);
  ~ReviewPageHandler() override;

  ReviewPageHandler(const ReviewPageHandler&) = delete;
  ReviewPageHandler& operator=(const ReviewPageHandler&) = delete;

  // mojom::review_page::ReviewPageHandler:
  void GetFingerprintData(GetFingerprintDataCallback callback) override;
  void GetIpInfo(GetIpInfoCallback callback) override;

 private:
  // Tries the next IP API in the list.
  void TryNextApi();

  // Callback when an IP fetch completes.
  void OnIpFetchComplete(std::unique_ptr<std::string> response_body);

  mojo::Receiver<mojom::review_page::ReviewPageHandler> handler_;
  raw_ptr<Profile> profile_;
  std::unique_ptr<network::SimpleURLLoader> url_loader_;

  // Current API index being tried.
  size_t current_api_index_ = 0;

  // Pending callback to return IP results.
  GetIpInfoCallback pending_ip_callback_;

  base::WeakPtrFactory<ReviewPageHandler> weak_ptr_factory_{this};
};

#endif  // CHROME_BROWSER_UI_WEBUI_REVIEW_REVIEW_HANDLER_H_
