// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/new_tab_page/simprint_ip/simprint_ip_handler.h"

#include <memory>
#include <string>
#include <vector>

#include "base/containers/span.h"
#include "base/json/json_reader.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "net/base/load_flags.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "url/gurl.h"

namespace {

// IP detection API endpoints (multiple for redundancy)
struct IpApiConfig {
  const char* url;
  const char* ip_field;
  const char* country_field;
  const char* country_code_field;
  const char* city_field;
  const char* region_field;
  const char* isp_field;
  const char* timezone_field;
  const char* status_field;  // nullptr if no status field
  const char* status_success_value;  // nullptr if no status check needed
};

constexpr IpApiConfig kIpApis[] = {
    // ip-api.com - Full geolocation, 45 requests/minute free
    {
        "http://ip-api.com/json/?fields=status,message,country,countryCode,region,regionName,city,isp,query,timezone",
        "query", "country", "countryCode", "city", "regionName", "isp",
        "timezone", "status", "success"
    },
    // ipapi.co - Full geolocation, 1000 requests/day free
    {
        "https://ipapi.co/json/",
        "ip", "country_name", "country_code", "city", "region", "org",
        "timezone", nullptr, nullptr
    },
    // api.ip.sb - Full geolocation, no hard limit
    {
        "https://api.ip.sb/geoip",
        "ip", "country", "country_code", "city", "region", "isp",
        "timezone", nullptr, nullptr
    },
    // realip.cc - Chinese service with full geolocation
    {
        "https://realip.cc/",
        "ip", "country", "iso_code", "city", "province", "isp",
        "time_zone", nullptr, nullptr
    },
    // ipbase.com - Full geolocation
    {
        "https://api.ipbase.com/v1/json/",
        "ip", "country_name", "country_code", "city", "region_name", nullptr,
        "time_zone", nullptr, nullptr
    },
    // iprust.io - Full geolocation
    {
        "https://iprust.io/ip.json",
        "ip", "country_long", "country_short", "city", "region", nullptr,
        "timezone", nullptr, nullptr
    },
};

constexpr auto kIpApisSpan = base::span(kIpApis);

// Maximum response size (16KB should be more than enough for IP info JSON)
constexpr int kMaxResponseSize = 16 * 1024;

// Helper function to get nested JSON value by dot-separated path
const std::string* GetNestedString(const base::Value::Dict& dict,
                                   const char* path) {
  if (!path || !*path) {
    return nullptr;
  }

  std::string path_str(path);
  size_t dot_pos = path_str.find('.');
  if (dot_pos == std::string::npos) {
    return dict.FindString(path);
  }

  std::string first_key = path_str.substr(0, dot_pos);
  std::string rest_path = path_str.substr(dot_pos + 1);
  const base::Value::Dict* nested = dict.FindDict(first_key);
  if (!nested) {
    return nullptr;
  }
  return GetNestedString(*nested, rest_path.c_str());
}

}  // namespace

SimprintIpHandler::SimprintIpHandler(
    mojo::PendingReceiver<simprint_ip::mojom::SimprintIpHandler> handler,
    Profile* profile)
    : handler_(this, std::move(handler)), profile_(profile) {}

SimprintIpHandler::~SimprintIpHandler() = default;

void SimprintIpHandler::GetIpInfo(GetIpInfoCallback callback) {
  current_api_index_ = 0;
  pending_callback_ = std::move(callback);
  TryNextApi();
}

void SimprintIpHandler::TryNextApi() {
  if (current_api_index_ >= kIpApisSpan.size()) {
    // All APIs failed
    auto info = simprint_ip::mojom::IpInfo::New();
    info->success = false;
    info->error = "All IP detection services failed";
    std::move(pending_callback_).Run(std::move(info));
    return;
  }

  const IpApiConfig& config = kIpApisSpan[current_api_index_];

  // Define network traffic annotation for this request
  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("simprint_ip_detection", R"(
        semantics {
          sender: "Simprint IP Detection"
          description:
            "Fetches the user's public IP address and geolocation information "
            "to display on the New Tab Page."
          trigger:
            "Opening a new tab in Simprint browser."
          data:
            "No user data is sent. The request only fetches the public IP "
            "address as seen by the external service."
          destination: OTHER
          internal {
            contacts {
              email: "nickel@nickel.workers.dev"
            }
          }
          user_data {
            type: NONE
          }
          last_reviewed: "2025-02-01"
        }
        policy {
          cookies_allowed: NO
          setting:
            "This feature is part of Simprint browser's New Tab Page and "
            "cannot be disabled separately."
          policy_exception_justification:
            "This is a core feature of Simprint browser for displaying "
            "network information to the user."
        })");

  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(config.url);
  resource_request->method = "GET";
  resource_request->load_flags =
      net::LOAD_BYPASS_CACHE | net::LOAD_DISABLE_CACHE;
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;

  url_loader_ = network::SimpleURLLoader::Create(std::move(resource_request),
                                                 traffic_annotation);
  url_loader_->SetTimeoutDuration(base::Seconds(10));  // 10 second timeout

  auto url_loader_factory = profile_->GetURLLoaderFactory();
  url_loader_->DownloadToString(
      url_loader_factory.get(),
      base::BindOnce(&SimprintIpHandler::OnIpFetchComplete,
                     base::Unretained(this)),
      kMaxResponseSize);
}

void SimprintIpHandler::OnIpFetchComplete(
    std::unique_ptr<std::string> response_body) {
  const IpApiConfig& config = kIpApisSpan[current_api_index_];

  if (!response_body || response_body->empty()) {
    // Try next API
    current_api_index_++;
    TryNextApi();
    return;
  }

  // Parse JSON response
  auto json_result = base::JSONReader::ReadAndReturnValueWithError(
      *response_body, base::JSON_PARSE_RFC);
  if (!json_result.has_value() || !json_result->is_dict()) {
    // Try next API
    current_api_index_++;
    TryNextApi();
    return;
  }

  const base::Value::Dict& dict = json_result->GetDict();

  // Check API status if applicable
  if (config.status_field) {
    const std::string* status = dict.FindString(config.status_field);
    // Handle both string "success" and boolean true
    if (status) {
      if (*status != config.status_success_value) {
        current_api_index_++;
        TryNextApi();
        return;
      }
    } else {
      // Check for boolean status
      std::optional<bool> bool_status = dict.FindBool(config.status_field);
      if (bool_status.has_value() && !bool_status.value()) {
        current_api_index_++;
        TryNextApi();
        return;
      }
    }
  }

  // Extract IP information
  auto info = simprint_ip::mojom::IpInfo::New();

  const std::string* ip = GetNestedString(dict, config.ip_field);
  if (!ip || ip->empty()) {
    // No IP found, try next API
    current_api_index_++;
    TryNextApi();
    return;
  }

  info->success = true;
  info->error = "";
  info->ip = *ip;

  const std::string* country = GetNestedString(dict, config.country_field);
  info->country = country ? *country : "";

  const std::string* country_code =
      GetNestedString(dict, config.country_code_field);
  info->country_code = country_code ? *country_code : "";

  const std::string* city = GetNestedString(dict, config.city_field);
  info->city = city ? *city : "";

  const std::string* region = GetNestedString(dict, config.region_field);
  info->region = region ? *region : "";

  const std::string* isp = GetNestedString(dict, config.isp_field);
  info->isp = isp ? *isp : "";

  const std::string* timezone = GetNestedString(dict, config.timezone_field);
  info->timezone = timezone ? *timezone : "";

  std::move(pending_callback_).Run(std::move(info));
}
