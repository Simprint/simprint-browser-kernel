# Overlay: Fingerprint

**指纹防护模块** - 浏览器指纹伪装与配置系统

## 功能概述

本模块实现了完整的浏览器指纹防护系统，包括：

### 1. 配置系统 (`simprint/config`)
- `FingerprintConfigStorage`: 指纹配置的存储和管理
- 支持动态加载和更新指纹配置
- 提供线程安全的配置访问接口

### 2. 指纹伪装 (`simprint/fingerprint`)
- **Canvas 指纹**: 修改 Canvas 2D/WebGL 渲染输出
- **WebGL 指纹**: 伪装 GPU 信息、渲染器、供应商
- **WebGPU 指纹**: 伪装 GPU 适配器信息
- **Audio 指纹**: 修改 AudioBuffer 和 WebAudio 输出
- **字体指纹**: 白名单机制拦截字体检测（FontFaceSet.check() + DOM 尺寸测量）
- **Screen 指纹**: 伪装屏幕分辨率、色深、像素比
- **Navigator 指纹**: 伪装 UA、语言、平台、内存、硬件并发
- **Network 指纹**: 伪装 IP、端口、WebRTC 本地 IP
- **Geolocation**: 伪装地理位置
- **Window 尺寸**: 控制窗口初始大小和位置

### 3. 防御机制
- **双层字体拦截**:
  - API 层：`FontFace.load()` + `FontFaceSet.check()`
  - 渲染层：`CSSFontSelector::GetFontData()` + `FontCache::GetFontData()`
- **Canvas 噪声注入**: 在像素级别添加不可见噪声
- **WebGL 参数伪装**: 修改 UNMASKED_VENDOR_WEBGL 等参数
- **Audio 噪声**: 修改音频样本数据

## 提交范围

基于 `git diff 879a34261b0ba..1a044afbb2149`（auth 终点 → fingerprint 最新）

包含以下提交：
- `9be2688`: 移除 SIMPRINT_LOG 调用（syner 模块清理）
- `947f2bf`: 集成指纹配置系统（84 文件，+4238/-173 行）
- `b2b202e`: 字体列表配置存储（10 文件，+379/-30 行）
- `78482d8`: 字体白名单拦截 API 层（3 文件，+58/-13 行）
- `1a044af`: FontCache 层面拦截非白名单字体（完善 browserscan 防御）

## 文件结构

```
overlay/fingerprint/
├── patches/           # 68 个补丁文件
│   ├── 001-030: Chrome/Components/Content/Net/Services/Simprint
│   └── 031-068: Blink (Core/Modules/Platform)
├── sources/
│   └── simprint/      # 新增的 simprint 模块源文件
│       ├── config_*   # 配置系统（3 文件）
│       └── fingerprint_*  # 指纹模块（13 文件）
├── scripts/
│   └── deploy_resources.py  # 部署源文件 + 删除 console_log
├── apply_order.txt    # 补丁应用顺序
├── run.py            # 主入口脚本
└── README.md         # 本文件
```

## 使用方法

由 driver 自动调用，或手动执行：

```bash
# 应用补丁
python overlay/fingerprint/run.py apply

# 部署资源文件
python overlay/fingerprint/run.py deploy

# 应用补丁 + 部署资源
python overlay/fingerprint/run.py apply_deploy
```

## 技术细节

### 字体检测防御

针对两种检测方法：
1. **API 检测**: `FontFaceSet.check()` - 在 `font_face_set.cc` 中拦截
2. **DOM 尺寸检测**: `offsetWidth/offsetHeight` - 在 `CSSFontSelector` 和 `FontCache` 中拦截

### Canvas 指纹防御

- 在 `HTMLCanvasElement::toDataURL()` 和 `toBlob()` 中注入噪声
- 在 `OffscreenCanvas` 中同样处理
- 噪声算法基于种子，确保同一会话内一致性

### WebGL 指纹防御

- 拦截 `getParameter()` 调用，伪装关键参数
- 修改 `WEBGL_debug_renderer_info` 扩展返回值
- 伪装 GPU 供应商和渲染器字符串

## 依赖关系

- 依赖 `simprint/auth` 模块（启动认证）
- 依赖 `simprint/eventbus` 模块（事件通信）
- 被 Chrome Browser、Content、Blink 各层调用

## 注意事项

1. 本模块必须在 `auth` 和 `syner` 之后应用
2. 删除了 `simprint/console_log` 模块（已废弃）
3. 补丁包含对 `sync_input_*.cc` 的修改（移除 SIMPRINT_LOG）
4. 字体白名单配置需要在运行时动态加载
