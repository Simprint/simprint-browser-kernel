# Overlay NTP

对 Chromium 源码的修改：自定义新标签页 UI、IP 检测、响应式布局。

## 补丁（patches/）

| 文件 | 说明 |
|------|------|
| 001-browser-build-gn.patch | browser_generated_files 添加 simprint_ip mojo 依赖 |
| 002-interface-binders.patch | 注册 SimprintIpHandler Mojo 接口绑定 |
| 003-resources-build-gn.patch | new_tab_page/BUILD.gn 添加 simprint_ip mojo TS 依赖 |
| 004-app-css.patch | app.css 修改 customizeButtons 位置与样式 |
| 005-new-tab-page-gni.patch | new_tab_page.gni 添加 simprint_ntp.ts、simprint_ip_proxy.ts |
| 006-new-tab-page-html.patch | new_tab_page.html 添加 canvas 背景和光晕样式 |
| 007-new-tab-page-ts.patch | new_tab_page.ts 导入 simprint_ntp.js |
| 008-customize-buttons-css.patch | customize_buttons.css 修改按钮样式与延迟显示 |
| 009-ui-build-gn.patch | ui/BUILD.gn 添加 simprint_ip_handler 源文件 |
| 010-webui-ntp-build-gn.patch | webui/new_tab_page/BUILD.gn 添加 simprint_ip mojo 依赖 |
| 011-new-tab-page-ui-cc.patch | new_tab_page_ui.cc 添加 SimprintIpHandler 实例化 |
| 012-new-tab-page-ui-h.patch | new_tab_page_ui.h 添加 SimprintIpHandler 声明与成员 |

顺序见 `apply_order.txt`。

## 新增源文件

由 deploy 阶段复制到 Chromium 对应位置：

| 本地路径 | 目标路径 |
|----------|----------|
| simprint_ip/BUILD.gn | chrome/browser/ui/webui/new_tab_page/simprint_ip/ |
| simprint_ip/simprint_ip.mojom | chrome/browser/ui/webui/new_tab_page/simprint_ip/ |
| simprint_ip/simprint_ip_handler.cc | chrome/browser/ui/webui/new_tab_page/simprint_ip/ |
| simprint_ip/simprint_ip_handler.h | chrome/browser/ui/webui/new_tab_page/simprint_ip/ |
| resources/simprint_ntp.ts | chrome/browser/resources/new_tab_page/ |
| resources/simprint_ip_proxy.ts | chrome/browser/resources/new_tab_page/ |

## 统一入口

由 driver 调用：`uv run python overlay/ntp/run.py apply|deploy|apply_deploy|build`。

- **apply**：打补丁。
- **deploy**：执行 `scripts/deploy_resources.py`，复制 simprint_ip/ 和 resources/ 到 Chromium。
- **apply_deploy**：先 apply 再 deploy，一次完成。
- **build**：当前无操作。

## 补丁调试与修复

参考 `overlay/branding/README.md` 中的"补丁调试与修复步骤"。
