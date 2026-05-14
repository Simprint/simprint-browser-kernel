# Overlay NTP

对 Chromium 源码的修改：自定义新标签页背景动画。

## 补丁（patches/）

| 文件 | 说明 |
|------|------|
| 001-browser-build-gn.patch | browser_generated_files 添加 simprint_ip mojo 依赖 |
| 002-interface-binders.patch | 注册 SimprintIpHandler Mojo 接口绑定 |
| 003-resources-build-gn.patch | new_tab_page/BUILD.gn 添加 simprint_ip mojo TS 依赖 |
| 005-new-tab-page-gni.patch | new_tab_page.gni 添加 simprint_ntp.ts |
| 006-new-tab-page-html.patch | new_tab_page.html 添加 canvas 背景和光晕样式 |
| 007-new-tab-page-ts.patch | new_tab_page.ts 导入 simprint_ntp.js |
| 009-ui-build-gn.patch | ui/BUILD.gn 添加 simprint_ip_handler 源文件 |
| 010-webui-ntp-build-gn.patch | webui/new_tab_page/BUILD.gn 添加 simprint_ip mojo 依赖 |
| 011-new-tab-page-ui-cc.patch | new_tab_page_ui.cc 添加 SimprintIpHandler 实例化 |
| 012-new-tab-page-ui-h.patch | new_tab_page_ui.h 添加 SimprintIpHandler 声明与成员 |

顺序见 `apply_order.txt`。

**注意**: 补丁 004 (app-css) 和 008 (customize-buttons-css) 已移除，customizer 按钮恢复默认位置和样式。

## 新增源文件

由 deploy 阶段复制到 Chromium 对应位置：

| 本地路径 | 目标路径 |
|----------|----------|
| simprint_ip/BUILD.gn | chrome/browser/ui/webui/new_tab_page/simprint_ip/ |
| simprint_ip/simprint_ip.mojom | chrome/browser/ui/webui/new_tab_page/simprint_ip/ |
| simprint_ip/simprint_ip_handler.cc | chrome/browser/ui/webui/new_tab_page/simprint_ip/ |
| simprint_ip/simprint_ip_handler.h | chrome/browser/ui/webui/new_tab_page/simprint_ip/ |
| resources/simprint_ntp.ts | chrome/browser/resources/new_tab_page/ |

**注意**: `simprint_ip_proxy.ts` 已移除，不再部署。

## 功能说明

- **背景动画**: 六边形网格和流动光带动画效果
- **simprint_ip**: 后端 IP 检测功能（保留但前端不再使用）
