# Overlay Auth - 启动认证检查模块

## 功能概述

Auth 模块为 Simprint 浏览器内核添加启动时的强制认证检查功能，确保只有通过 Tauri 启动且已登录的用户才能使用浏览器。

## 主要功能

### 1. 启动认证检查
- 浏览器启动时通过 EventBus IPC 向 Tauri 查询用户登录状态
- 未登录或 IPC 异常 → 静默退出浏览器（fail-safe 机制）
- 直接启动（未通过 Tauri）→ 静默退出浏览器

### 2. LaunchConfig IPC 传递
- Tauri 在握手完成后通过 IPC 发送启动配置（env_uuid、user_data_dir、proxy 等）
- 替代原有的命令行参数传递方式，提升安全性和扩展性

### 3. 认证信息查询
- 提供 AuthManager 便捷接口查询认证状态
- 支持认证状态变化监听（登录/登出广播）
- 缓存机制避免频繁 IPC 查询

## 模块结构

```
overlay/auth/
├── sources/
│   └── simprint/
│       └── auth/
│           ├── BUILD.gn                    # 编译配置
│           ├── auth_manager.h              # 认证管理器头文件
│           ├── auth_manager.cc             # 认证管理器实现
│           ├── startup_auth_checker.h      # 启动认证检查器头文件
│           ├── startup_auth_checker.cc     # 启动认证检查器实现
│           └── README.md                   # 使用文档
├── patches/
│   ├── 001-chrome-browser-build-gn.patch           # 添加 simprint/auth 依赖
│   ├── 002-chrome-browser-main.patch               # 集成启动认证检查
│   ├── 003-eventbus-topics-h.patch                 # 添加 kLaunchConfig Topic
│   ├── 004-eventbus-h.patch                        # 添加 AuthInfo 结构和认证方法
│   ├── 005-eventbus-cc.patch                       # 实现 LaunchConfig/AuthRequest/AuthResponse 处理
│   ├── 006-sync-input-capture-handler.patch        # 移除 SIMPRINT_LOG 调用
│   └── 007-sync-input-replay.patch                 # 移除 SIMPRINT_LOG 调用
├── scripts/
│   └── deploy_resources.py                # 部署脚本：复制 simprint/auth 源文件
├── apply_order.txt                         # 补丁应用顺序
├── run.py                                  # 主运行脚本
└── README.md                               # 本文件

```

## 技术实现

### 新增文件
- `simprint/auth/auth_manager.{h,cc}`: 认证管理器，封装 EventBus 认证功能
- `simprint/auth/startup_auth_checker.{h,cc}`: 启动认证检查器，强制认证检查
- `simprint/auth/BUILD.gn`: 编译配置
- `simprint/auth/README.md`: 使用文档

### 修改文件
- `chrome/browser/BUILD.gn`: 添加 `//simprint/auth` 依赖
- `chrome/browser/chrome_browser_main.cc`: 在 `PostBrowserStart()` 中集成启动认证检查
- `simprint/eventbus/topics.h`: 添加 `kLaunchConfig = 0x0900` Topic
- `simprint/eventbus/eventbus.h`: 添加 `AuthInfo` 结构、认证回调、认证请求方法
- `simprint/eventbus/eventbus.cc`: 实现 LaunchConfig/AuthRequest/AuthResponse 处理逻辑
- `chrome/browser/ui/views/frame/sync_input_capture_handler.cc`: 移除跨文件 SIMPRINT_LOG 调用
- `chrome/browser/ui/views/frame/sync_input_replay.cc`: 移除跨文件 SIMPRINT_LOG 调用

## 依赖关系

Auth 模块依赖以下模块（需要在 `driver/order.txt` 中先执行）：
- **syner**: 提供 EventBus IPC 通信基础设施和 console_log

## 使用方法

### 1. 在 driver/order.txt 中添加
```
overlay/syner
overlay/auth
```

### 2. 运行 driver
```bash
python driver/run.py apply_deploy
```

### 3. 编译 Chromium
```bash
cd <chromium_src>
autoninja -C out/Default chrome
```

## 认证流程

```
浏览器启动
    ↓
检查 --simprint-env-id 参数
    ↓
有参数 → 初始化 EventBus → 发送 AuthRequest → 等待 AuthResponse (5秒超时)
    ↓                                                      ↓
    ↓                                          is_authenticated = true → 继续运行
    ↓                                          is_authenticated = false → 静默退出
    ↓                                          超时/IPC 异常 → 静默退出
    ↓
无参数 → 静默退出
```

## 相关提交

- simprint-browser commit `879a342`: 添加启动认证检查和 LaunchConfig 支持
- simprint-browser commit `9be2688`: 移除 sync_input_*.cc 中的 SIMPRINT_LOG 调用

## 注意事项

1. **依赖顺序**: Auth 模块必须在 syner 模块之后执行
2. **安全策略**: 采用 fail-safe 机制，任何异常情况都会导致浏览器退出
3. **IPC 超时**: 默认 5 秒超时，可在 `chrome_browser_main.cc` 中调整
4. **日志输出**: 使用 `simprint::LogToConsole()` 输出调试日志到 Tauri 控制台

## 测试验证

1. **通过 Tauri 启动 + 已登录**: 浏览器正常运行
2. **通过 Tauri 启动 + 未登录**: 浏览器立即退出
3. **直接双击浏览器**: 浏览器立即退出
4. **IPC 异常/超时**: 浏览器在 5 秒后退出
