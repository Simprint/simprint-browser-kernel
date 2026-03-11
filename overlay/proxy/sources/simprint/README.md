# Simprint Browser Components

This directory contains core components for the Simprint browser.

## Directory Structure

```
simprint/
├── auth/                               # Authentication module
├── config/                             # Configuration management
├── eventbus/                           # EventBus IPC framework
├── fingerprint/                        # Browser fingerprinting
├── proxy/                              # Proxy configuration (NEW)
│   ├── proxy_config_service.h/cc       # Core proxy service
│   ├── proxy_config_parser.h/cc        # JSON parser
│   ├── proxy_message_handler.h/cc      # EventBus handler
│   └── network_context_configurator.h/cc
├── simprint_browser_context.h/cc       # Global context manager
└── BUILD.gn
```

## Modules

### EventBus
IPC communication framework between Tauri and Chromium.
- Named pipe transport
- Message routing and handling
- Topic-based pub/sub

See [eventbus/README.md](eventbus/README.md) for details.

### Proxy
Dynamic proxy configuration with runtime updates.
- HTTP, HTTPS, SOCKS5 support
- Proxy authentication
- EventBus integration

See [proxy/README.md](proxy/README.md) for details.

### Auth
Authentication and authorization.
- User authentication
- Token management
- Permission control

### Config
Configuration management.
- Launch configuration
- Runtime configuration updates

### Fingerprint
Browser fingerprinting and anti-detection.
- Canvas fingerprinting
- WebGL fingerprinting
- Audio context fingerprinting

## Global Context

`SimprintBrowserContext` is a singleton that manages the lifecycle of all Simprint components:

```cpp
auto* context = simprint::SimprintBrowserContext::GetInstance();

// Initialize during browser startup
context->Initialize();

// Access components
auto* proxy_service = context->proxy_config_service();
auto* proxy_handler = context->proxy_message_handler();

// Shutdown during browser exit
context->Shutdown();
```

## Integration Points

### Chrome Browser Main
- `chrome_browser_main.cc` initializes SimprintBrowserContext
- Integrated into browser startup/shutdown lifecycle

### EventBus
- All modules register message handlers with EventBus
- Enables IPC communication with Tauri

### Network Stack
- Proxy module integrates with Chromium's network stack
- ProxyConfigMonitor observes configuration changes

## Build Configuration

Each module has its own BUILD.gn:

```gn
# Root BUILD.gn
source_set("browser_context") {
  deps = [
    "//simprint/eventbus",
    "//simprint/proxy",
  ]
}

# proxy/BUILD.gn
source_set("proxy") {
  deps = [
    "//base",
    "//net",
    "//simprint/eventbus",
  ]
}
```

## Development Guidelines

### Adding New Modules

1. Create a subdirectory under `simprint/`
2. Add BUILD.gn with appropriate dependencies
3. Implement EventBus message handlers if needed
4. Register with SimprintBrowserContext if needed
5. Add README.md documenting the module

### Code Style

- Follow Chromium C++ style guide
- Use `simprint` namespace
- Use `SEQUENCE_CHECKER` for thread safety
- Add comprehensive logging

### Testing

- Write unit tests for each component
- Add integration tests for cross-module functionality
- Test EventBus message handling

## Documentation

- [Proxy Implementation](../../../plan/FINAL_IMPLEMENTATION.md)
- [Conflict Resolution](../../../plan/CONFLICT_RESOLUTION.md)
- [Directory Structure](../../../plan/DIRECTORY_STRUCTURE.md)
- [Tauri Implementation](../../../plan/TAURI_IMPLEMENTATION.md)

## License

Copyright 2024 The Chromium Authors. All rights reserved.
