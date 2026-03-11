# Simprint Proxy Configuration

Dynamic proxy configuration for Simprint browser.

## Directory Structure

```
proxy/
├── BUILD.gn                            # Build configuration
├── proxy_config_service.h/cc           # Core proxy configuration service
├── proxy_config_parser.h/cc            # JSON configuration parser
├── proxy_message_handler.h/cc          # EventBus message handler
└── network_context_configurator.h/cc   # Network context configurator
```

## Components

### ProxyConfigService
- Implements `net::ProxyConfigService` interface
- Manages current proxy configuration
- Notifies observers on configuration changes
- Thread-safe using `SEQUENCE_CHECKER`

### ProxyConfigParser
- Parses JSON proxy configuration
- Validates proxy settings
- Supports HTTP, HTTPS, and SOCKS5 proxies
- Handles proxy authentication

### ProxyMessageHandler
- Implements `eventbus::MessageHandler` interface
- Handles `Topic::kProxySet` messages
- Handles `Topic::kLaunchConfig` messages
- Automatically registered with EventBus

### NetworkContextConfigurator
- Configures network context with proxy settings
- Integrates with Chromium network stack

## Usage

### From C++

```cpp
#include "simprint/proxy/proxy_config_service.h"

// Get proxy service
auto* context = simprint::SimprintBrowserContext::GetInstance();
auto* proxy_service = context->proxy_config_service();

// Update configuration
net::ProxyConfig config;
config.proxy_rules().type = net::ProxyConfig::ProxyRules::Type::PROXY_LIST;
config.proxy_rules().single_proxies.SetSingleProxyServer(
    net::ProxyServer::FromURI("socks5://127.0.0.1:1080",
                              net::ProxyServer::SCHEME_SOCKS5));
proxy_service->UpdateProxyConfig(config);

// Clear proxy
proxy_service->ClearProxyConfig();
```

### From Tauri (via EventBus)

```rust
// Send proxy configuration
let proxy_config = ProxyConfig {
    proxy_type: "socks5".to_string(),
    host: "127.0.0.1".to_string(),
    port: 1080,
    username: Some("user".to_string()),
    password: Some("pass".to_string()),
};

let config_json = serde_json::to_string(&proxy_config)?;
eventbus_manager.send_to_env(
    &env_uuid,
    Topic::ProxySet,
    config_json.into_bytes()
).await?;
```

## JSON Format

```json
{
  "proxy_type": "socks5",
  "host": "127.0.0.1",
  "port": 1080,
  "username": "user",
  "password": "pass"
}
```

Supported proxy types:
- `http` - HTTP proxy
- `https` - HTTPS proxy
- `socks5` - SOCKS5 proxy

Username and password are optional.

## Message Flow

### Startup Configuration

```
Tauri → Topic::kLaunchConfig
  ↓
EventBus dispatch
  ↓
ProxyMessageHandler::HandleLaunchConfig()
  ↓
Parse proxy field from JSON
  ↓
ProxyMessageHandler::HandleProxySet()
  ↓
ProxyConfigService::UpdateProxyConfig()
  ↓
Notify ProxyConfigMonitor
  ↓
Mojo IPC → Network Service
```

### Runtime Update

```
Tauri → Topic::kProxySet
  ↓
EventBus dispatch
  ↓
ProxyMessageHandler::HandleProxySet()
  ↓
ProxyConfigParser::ParseFromJson()
  ↓
ProxyConfigService::UpdateProxyConfig()
  ↓
Apply to network requests
```

## Integration

The proxy module is integrated into Simprint browser through:

1. **SimprintBrowserContext** - Creates and manages proxy services
2. **EventBus** - Registers ProxyMessageHandler for IPC messages
3. **ProxyConfigMonitor** - Observes configuration changes
4. **Network Service** - Applies proxy configuration to requests

## Dependencies

```gn
deps = [
  "//base",
  "//chrome/browser/net",
  "//net",
  "//simprint/eventbus",
  "//url",
]
```

## Testing

### Unit Tests
- `proxy_config_service_unittest.cc` - Test configuration service
- `proxy_config_parser_unittest.cc` - Test JSON parsing

### Integration Tests
- Test startup proxy configuration
- Test runtime proxy updates
- Test proxy authentication
- Test error handling

## See Also

- [Parent README](../README.md) - Simprint browser overview
- [EventBus](../eventbus/README.md) - EventBus framework
- [Implementation Guide](../../../../plan/FINAL_IMPLEMENTATION.md) - Detailed implementation
