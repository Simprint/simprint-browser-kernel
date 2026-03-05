// Copyright 2024 The Simprint Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/review/review_ui.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/review/review_handler.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/review_resources.h"
#include "chrome/grit/review_resources_map.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"
#include "ui/webui/webui_util.h"

ReviewUIConfig::ReviewUIConfig()
    : DefaultWebUIConfig(content::kChromeUIScheme,
                         chrome::kChromeUIReviewHost) {}

ReviewUI::ReviewUI(content::WebUI* web_ui) : MojoWebUIController(web_ui) {
  // Set up the chrome://review/ source.
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      Profile::FromWebUI(web_ui), chrome::kChromeUIReviewHost);

  webui::SetupWebUIDataSource(source, kReviewResources,
                              IDR_REVIEW_REVIEW_HTML);

  // CSP for inline styles and data URIs
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::StyleSrc,
      "style-src 'self' 'unsafe-inline';");
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ImgSrc,
      "img-src 'self' data:;");
}

ReviewUI::~ReviewUI() = default;

void ReviewUI::BindInterface(
    mojo::PendingReceiver<mojom::review_page::ReviewPageHandler> receiver) {
  LOG(INFO) << "ReviewUI::BindInterface called";
  page_handler_ = std::make_unique<ReviewPageHandler>(
      std::move(receiver), Profile::FromWebUI(web_ui()));
  LOG(INFO) << "ReviewUI::BindInterface completed";
}

WEB_UI_CONTROLLER_TYPE_IMPL(ReviewUI)
