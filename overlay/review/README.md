# Overlay: Review

**Review WebUI 页面与窗口增强功能**

## 功能概述

本模块为 Simprint 浏览器添加内部审查页面和窗口增强功能，用于浏览器功能测试、调试和实例区分。

### 1. chrome://review WebUI 页面
- 内部审查页面用于功能测试和调试
- 提供 Mojo 接口与浏览器后端通信
- 支持实时查看和验证浏览器状态
- 配置加载成功后自动打开（可选）

### 2. 窗口图标徽章显示
- 在窗口图标底部添加带数字的徽章区分不同浏览器实例
- 使用 Skia 图形库绘制圆角矩形徽章
- 徽章显示 `--simprint-display-id` 参数传入的数字
- 图标文件命名规则：`{user-data-dir}/{simprint-env-id}.ico`
- 支持多尺寸图标生成（16/32/48/64px）

**徽章设计**：
- 位置：图标底部居中
- 形状：圆角矩形（药丸形状）
- 颜色：深蓝色背景 (#1E3A5F) + 白色文字
- 尺寸：自适应文本宽度，高度约占图标 80%

### 3. 环境信息集成
- FingerprintConfig 新增 `env_id` 和 `env_name` 字段
- 支持从 JSON 配置中解析环境标识
- 在窗口属性管理器中添加详细日志用于调试

## 提交范围

基于 `git diff bcffd0ea6d377^..bb84996b42a5b`（2026-03-05 提交）

包含以下提交：
- `bcffd0ea6`: 实现基于窗口图标的任务栏徽章显示功能
- `d7ea5e7a8`: 添加 chrome://review WebUI 页面
- `bc0ce86fa`: 配置加载成功后自动打开 chrome://review 页面
- `045ce2091`: 添加环境信息到指纹配置并增强窗口属性日志
- `bb84996b4`: 更新产品 logo 图片资源（未包含在迁移中）

## 文件结构

```
overlay/review/
├── patches/           # 16 个补丁文件
│   ├── 001-005: Chrome Browser（interface binders, resources, ui, window）
│   ├── 006-007: Chrome WebUI（build, configs）
│   ├── 008-011: Chrome Common（switches, webui url constants）
│   ├── 012-015: Simprint（eventbus, fingerprint config）
│   └── 016: Tools（resource_ids.spec）
├── sources/
│   ├── chrome_browser_simprint/          # 窗口图标管理器（3 文件）
│   ├── chrome_browser_resources_review/  # review 前端资源（4 文件）
│   └── chrome_browser_ui_webui_review/   # review 后端代码（6 文件）
├── scripts/
│   └── deploy_resources.py               # 部署源文件
├── apply_order.txt    # 补丁应用顺序
├── run.py            # 主入口脚本
└── README.md         # 本文件
```

## 技术细节

### 窗口图标徽章

**WindowIconManager** 模块负责生成带徽章的窗口图标：
- 从产品 logo 图片加载基础图标
- 使用 Skia Canvas 在底部绘制徽章
- 支持多尺寸图标（16/32/48/64px）
- 图标缓存到用户数据目录

**集成点**：
- `BrowserDesktopWindowTreeHostWin::OnFrameInitialized()` 中调用
- 通过 `--simprint-display-id` 命令行参数传入徽章数字
- 通过 `--simprint-env-id` 参数确定图标文件名

### chrome://review WebUI

**前端**（TypeScript + HTML）：
- `review.html`: 页面结构
- `review.ts`: 页面逻辑
- `review_ui.ts`: UI 组件

**后端**（C++）：
- `ReviewUI`: WebUI 配置和资源加载
- `ReviewHandler`: Mojo 接口实现，处理前后端通信
- `review_page.mojom`: Mojo 接口定义

**注册流程**：
1. `chrome_web_ui_configs.cc` 注册 ReviewUI
2. `chrome_browser_interface_binders_webui.cc` 绑定 Mojo 接口
3. `webui_url_constants.{cc,h}` 定义 URL 常量
4. `resource_ids.spec` 分配资源 ID

### 自动打开 review 页面

在 `simprint/eventbus/eventbus.cc` 的 `HandleHandshakeResponse()` 中：
- 配置加载成功后检查是否已存在 chrome://review 标签页
- 如果不存在则在第一个浏览器窗口中打开新标签页
- 避免重复打开

### 环境信息集成

**FingerprintConfig 扩展**：
```cpp
std::optional<std::string> env_id;
std::optional<std::string> env_name;
```

**日志增强**：
- `BrowserWindowPropertyManagerWin` 添加详细日志
- 输出窗口属性设置过程用于调试

## 依赖关系

- 依赖 `simprint/fingerprint` 模块（扩展配置结构）
- 依赖 `simprint/eventbus` 模块（自动打开逻辑）
- 依赖 Skia 图形库（窗口图标绘制）

## 注意事项

1. 本模块必须在 `fingerprint` 之后应用（因为修改了 fingerprint_config）
2. 窗口图标徽章功能仅支持 Windows 平台
3. 图标文件会缓存到用户数据目录，避免重复生成
4. chrome://review 页面为内部调试页面，不应暴露给最终用户
5. 自动打开 review 页面的逻辑可根据需要调整或禁用

## 命令行参数

- `--simprint-display-id=<number>`: 指定窗口图标徽章显示的数字
- `--simprint-env-id=<string>`: 指定环境 ID，用于图标文件命名

## 测试验证

1. **窗口图标徽章**: 启动浏览器时传入 `--simprint-display-id=1`，检查任务栏图标是否显示徽章
2. **chrome://review 页面**: 访问 `chrome://review`，检查页面是否正常加载
3. **自动打开**: 配置加载成功后，检查是否自动打开 chrome://review 标签页
4. **环境信息**: 检查 FingerprintConfig 是否正确解析 env_id 和 env_name
