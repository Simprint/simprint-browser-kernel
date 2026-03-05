// Copyright 2024 The Simprint Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_REVIEW_REVIEW_UI_H_
#define CHROME_BROWSER_UI_WEBUI_REVIEW_REVIEW_UI_H_

#include <memory>

#include "chrome/browser/ui/webui/review/review_page.mojom.h"
#include "content/public/browser/webui_config.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "ui/webui/mojo_web_ui_controller.h"

class ReviewPageHandler;

class ReviewUI;

// WebUI config for simprint://review
class ReviewUIConfig : public content::DefaultWebUIConfig<ReviewUI> {
 public:
  ReviewUIConfig();
  ~ReviewUIConfig() override = default;
};

// WebUI controller for simprint://review
class ReviewUI : public ui::MojoWebUIController {
 public:
  explicit ReviewUI(content::WebUI* web_ui);
  ~ReviewUI() override;

  ReviewUI(const ReviewUI&) = delete;
  ReviewUI& operator=(const ReviewUI&) = delete;

  // Instantiates the implementor of the mojom::ReviewPageHandler mojo
  // interface passing the pending receiver that will be internally bound.
  void BindInterface(
      mojo::PendingReceiver<mojom::review_page::ReviewPageHandler> receiver);

 private:
  std::unique_ptr<ReviewPageHandler> page_handler_;

  WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif  // CHROME_BROWSER_UI_WEBUI_REVIEW_REVIEW_UI_H_
