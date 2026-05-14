# Syner Overlay

**同步功能模块**：EventBus IPC 通信 + 输入同步（键盘、鼠标、IME）

## 功能概述

### EventBus IPC 通信
- 跨进程消息传递（基于 Windows Named Pipe）
- 窗口控制（一键排列布局）
- 主控/从控角色管理
- 消息序列化与反序列化

### 输入同步
- ✅ 键盘事件（KeyDown/KeyUp）
- ✅ 鼠标事件（Move/Click/Drag/Wheel）
- ✅ 粘贴同步
- ✅ IME 输入（中文、日文等输入法）
- ✅ 弹出层支持（标签页搜索、下拉菜单等）

## 目录结构

```
overlay/syner/
├── run.py                             # 主运行脚本
├── apply_order.txt                    # 补丁应用顺序
├── scripts/
│   └── deploy_resources.py           # 部署源文件脚本
├── patches/                           # 12 个补丁文件
│   ├── 001-chrome-browser-build-gn.patch
│   ├── 002-chrome-browser-main.patch
│   ├── 003-chrome-ui-build-gn.patch
│   ├── 004-browser-view-cc.patch
│   ├��─ 005-browser-view-h.patch
│   ├── 006-chrome-views-delegate-cc.patch
│   ├── 007-chrome-views-delegate-h.patch
│   ├── 008-render-widget-host-view-aura.patch
│   ├── 009-textfield-cc.patch
│   ├── 010-views-delegate-h.patch
│   ├── 011-native-widget-aura.patch
│   └── 012-widget-cc.patch
└── sources/                           # 新增源文件
    ├── simprint/
    │   ├── eventbus/                  # EventBus 通信模块
    │   │   ├── BUILD.gn
    │   │   ├── eventbus.{cc,h}
    │   │   ├── handler.{cc,h}
    │   │   ├── message.{cc,h}
    │   │   ├── topics.h
    │   │   ├── transport.h
    │   │   └── transport_win.cc
    │   └── console_log/               # 控制台日志模块
    │       ├── BUILD.gn
    │       ├── console_log.{cc,h}
    └── chrome/browser/ui/views/frame/ # 同步输入处理
        ├── sync_input_capture_handler.{cc,h}
        └── sync_input_replay.{cc,h}
```

## 技术细节

### EventBus 架构
- **Transport 层**：Windows Named Pipe 通信
- **Message 层**：消息序列化/反序列化
- **Handler 层**：消息处理回调
- **EventBus 层**：统一的事件总线接口

### 输入同步机制
1. **主控端捕获**：
   - 在 `BrowserView` 中挂接事件监听器
   - 捕获键盘、鼠标、IME 事件
   - 通过 EventBus 发送到从控端

2. **从控端重放**：
   - 接收 EventBus 消息
   - 根据 `root_index` 定位目标窗口
   - 重放输入事件到正确的控件

3. **弹出层支持**：
   - 通过 `root window` 匹配机制
   - 支持主窗口和所有弹出层
   - 自动处理 WebUI 子窗口

## 统计数据

```
源文件：      17 个
补丁文件：    12 个
源代码行数：  2,957 行
补丁总大小：  66 KB
模块总大小：  195 KB
```

## 基于提交

从 `45c01f3f68a83` (NTP) 到 `105f9bc65d5dc` (IME 同步) 的 8 个提交：

1. `45b4782f0fc36` - EventBus IPC 通信模块基础
2. `268091fe4179e` - EventBus 窗口控制
3. `39d4217b5496f` - 一阶段同步输入（主控捕获与从控重放）
4. `a17e130f840a0` - 网页渲染区鼠标同步
5. `783ec899c43d9` - 鼠标滚轮事件同步
6. `11486b2989289` - 粘贴同步、Enter 键修复
7. `8360ad1cd29e3` - 输入挂接优化、日志收敛
8. `105f9bc65d5dc` - 弹出层 IME 输入同步

## 依赖关系

- **前置模块**：branding, ntp
- **后续模块**：auth, fingerprint, review, account, proxy, cookie
- **执行方式**：应按 `driver/order.txt` 的整体顺序运行，不建议将本模块作为独立公开工作流单独执行

## 注意事项

1. 仅支持 Windows 平台（使用 Named Pipe）
2. 需要在主程序初始化时启动 EventBus
3. 同步功能需要多个浏览器实例配合使用
4. 主控端和从控端通过角色配置区分
