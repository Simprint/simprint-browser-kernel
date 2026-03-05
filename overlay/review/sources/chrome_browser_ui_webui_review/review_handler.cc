// Copyright 2024 The Simprint Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/review/review_handler.h"

#include <array>

#include "base/json/json_reader.h"
#include "chrome/browser/profiles/profile.h"
#include "net/base/load_flags.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"

#if defined(SIMPRINT_FINGERPRINT_ENABLED)
#include "simprint/fingerprint/fingerprint_config.h"
#endif

namespace {

// List of IP detection APIs to try in order
constexpr std::array<const char*, 3> kIpApis = {
    "https://ipapi.co/json/",
    "https://ip-api.com/json/",
    "https://ipinfo.io/json",
};

constexpr net::NetworkTrafficAnnotationTag kTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation("review_page_ip_detection", R"(
      semantics {
        sender: "Review Page IP Detection"
        description:
          "Fetches the user's public IP address and location information "
          "from third-party services for display on the review page."
        trigger: "User opens the review page (simprint://review)."
        data: "No user data is sent. The service sees the request IP address."
        destination: OTHER
      }
      policy {
        cookies_allowed: NO
        setting: "This feature cannot be disabled by settings."
        policy_exception_justification:
          "Not implemented. This is a diagnostic feature."
      })");

}  // namespace

ReviewPageHandler::ReviewPageHandler(
    mojo::PendingReceiver<mojom::review_page::ReviewPageHandler> handler,
    Profile* profile)
    : handler_(this, std::move(handler)), profile_(profile) {}

ReviewPageHandler::~ReviewPageHandler() = default;

void ReviewPageHandler::GetFingerprintData(GetFingerprintDataCallback callback) {
  LOG(INFO) << "ReviewPageHandler::GetFingerprintData called";
  auto data = mojom::review_page::FingerprintData::New();

#if defined(SIMPRINT_FINGERPRINT_ENABLED)
  auto* config_manager = simprint::fingerprint::FingerprintConfigManager::GetInstance();
  LOG(INFO) << "config_manager: " << (config_manager ? "exists" : "null");
  if (config_manager) {
    LOG(INFO) << "IsConfigLoaded: " << (config_manager->IsConfigLoaded() ? "true" : "false");
  }
  if (config_manager && config_manager->IsConfigLoaded()) {
    const auto& config = config_manager->GetConfig();
    LOG(INFO) << "Config loaded successfully";

    // 输出完整配置内容
    LOG(INFO) << "=== Full Config Dump ===";
    LOG(INFO) << "user_agent: " << (config.user_agent ? *config.user_agent : "null");
    LOG(INFO) << "platform: " << (config.platform ? *config.platform : "null");
    LOG(INFO) << "language: " << (config.language ? *config.language : "null");
    LOG(INFO) << "timezone: " << (config.timezone ? *config.timezone : "null");
    LOG(INFO) << "env_id: " << (config.env_id ? *config.env_id : "null");
    LOG(INFO) << "env_name: " << (config.env_name ? *config.env_name : "null");
    if (config.font_list) {
      LOG(INFO) << "font_list size: " << config.font_list->size();
      for (size_t i = 0; i < config.font_list->size(); ++i) {
        LOG(INFO) << "  font[" << i << "]: " << (*config.font_list)[i];
      }
    } else {
      LOG(INFO) << "font_list: null";
    }
    LOG(INFO) << "========================";

    // Basic info
    if (config.user_agent)
      data->user_agent = *config.user_agent;
    if (config.platform)
      data->platform = *config.platform;
    if (config.language)
      data->language = *config.language;

    // Hardware
    if (config.hardware_concurrency)
      data->hardware_concurrency = *config.hardware_concurrency;
    if (config.device_memory)
      data->device_memory = *config.device_memory;
    if (config.max_touch_points)
      data->max_touch_points = *config.max_touch_points;

    // Screen
    if (config.resolution)
      data->resolution = *config.resolution;
    if (config.color_depth)
      data->color_depth = *config.color_depth;
    if (config.device_pixel_ratio)
      data->device_pixel_ratio = *config.device_pixel_ratio;

    // WebGL
    if (config.webgl_vendor)
      data->webgl_vendor = *config.webgl_vendor;
    if (config.webgl_renderer)
      data->webgl_renderer = *config.webgl_renderer;

    // Fingerprint protection modes
    if (config.canvas)
      data->canvas = *config.canvas;
    if (config.webgl_image)
      data->webgl_image = *config.webgl_image;
    if (config.webgl_info)
      data->webgl_info = *config.webgl_info;
    if (config.audio_context)
      data->audio_context = *config.audio_context;
    if (config.client_rects)
      data->client_rects = *config.client_rects;
    if (config.font_list)
      data->font_list = *config.font_list;

    // Network and location
    if (config.webrtc)
      data->webrtc = *config.webrtc;
    if (config.geolocation)
      data->geolocation = *config.geolocation;
    if (config.geolocation_prompt)
      data->geolocation_prompt = *config.geolocation_prompt;

    // Device info
    if (config.device_name)
      data->device_name = *config.device_name;
    if (config.mac_address)
      data->mac_address = *config.mac_address;
    if (config.do_not_track)
      data->do_not_track = *config.do_not_track;
    if (config.port_scan_protection)
      data->port_scan_protection = *config.port_scan_protection;

    // Media
    if (config.media_devices)
      data->media_devices = *config.media_devices;

    // Environment info
    if (config.env_id)
      data->env_id = *config.env_id;
    if (config.env_name)
      data->env_name = *config.env_name;
  }
#endif

  LOG(INFO) << "ReviewPageHandler::GetFingerprintData returning data";
  std::move(callback).Run(std::move(data));
}

void ReviewPageHandler::GetIpInfo(GetIpInfoCallback callback) {
  pending_ip_callback_ = std::move(callback);
  current_api_index_ = 0;
  TryNextApi();
}

void ReviewPageHandler::TryNextApi() {
  if (current_api_index_ >= kIpApis.size()) {
    // All APIs failed
    auto info = mojom::review_page::IpInfo::New();
    info->success = false;
    info->error = "All IP detection services failed";
    std::move(pending_ip_callback_).Run(std::move(info));
    return;
  }

  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(kIpApis[current_api_index_]);
  resource_request->method = "GET";
  resource_request->load_flags = net::LOAD_BYPASS_CACHE | net::LOAD_DISABLE_CACHE;
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;

  url_loader_ = network::SimpleURLLoader::Create(std::move(resource_request),
                                                   kTrafficAnnotation);
  url_loader_->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
      profile_->GetURLLoaderFactory().get(),
      base::BindOnce(&ReviewPageHandler::OnIpFetchComplete,
                     weak_ptr_factory_.GetWeakPtr()));
}

void ReviewPageHandler::OnIpFetchComplete(
    std::unique_ptr<std::string> response_body) {
  if (!response_body) {
    // Try next API
    current_api_index_++;
    TryNextApi();
    return;
  }

  auto parsed = base::JSONReader::Read(*response_body, base::JSON_PARSE_RFC);
  if (!parsed || !parsed->is_dict()) {
    // Try next API
    current_api_index_++;
    TryNextApi();
    return;
  }

  const base::Value::Dict& dict = parsed->GetDict();
  auto info = mojom::review_page::IpInfo::New();

  // Parse based on current API
  if (current_api_index_ == 0) {
    // ipapi.co format
    const std::string* ip = dict.FindString("ip");
    const std::string* country = dict.FindString("country_name");
    const std::string* country_code = dict.FindString("country_code");
    const std::string* city = dict.FindString("city");
    const std::string* region = dict.FindString("region");
    const std::string* org = dict.FindString("org");
    const std::string* timezone = dict.FindString("timezone");

    if (ip) info->ip = *ip;
    if (country) info->country = *country;
    if (country_code) info->country_code = *country_code;
    if (city) info->city = *city;
    if (region) info->region = *region;
    if (org) info->isp = *org;
    if (timezone) info->timezone = *timezone;
  } else if (current_api_index_ == 1) {
    // ip-api.com format
    const std::string* ip = dict.FindString("query");
    const std::string* country = dict.FindString("country");
    const std::string* country_code = dict.FindString("countryCode");
    const std::string* city = dict.FindString("city");
    const std::string* region = dict.FindString("regionName");
    const std::string* isp = dict.FindString("isp");
    const std::string* timezone = dict.FindString("timezone");

    if (ip) info->ip = *ip;
    if (country) info->country = *country;
    if (country_code) info->country_code = *country_code;
    if (city) info->city = *city;
    if (region) info->region = *region;
    if (isp) info->isp = *isp;
    if (timezone) info->timezone = *timezone;
  } else if (current_api_index_ == 2) {
    // ipinfo.io format
    const std::string* ip = dict.FindString("ip");
    const std::string* country = dict.FindString("country");
    const std::string* city = dict.FindString("city");
    const std::string* region = dict.FindString("region");
    const std::string* org = dict.FindString("org");
    const std::string* timezone = dict.FindString("timezone");

    if (ip) info->ip = *ip;
    if (country) info->country_code = *country;  // ipinfo only gives code
    if (city) info->city = *city;
    if (region) info->region = *region;
    if (org) info->isp = *org;
    if (timezone) info->timezone = *timezone;
  }

  info->success = info->ip.has_value() && !info->ip->empty();
  if (!info->success) {
    // Try next API
    current_api_index_++;
    TryNextApi();
    return;
  }

  std::move(pending_ip_callback_).Run(std::move(info));
}
