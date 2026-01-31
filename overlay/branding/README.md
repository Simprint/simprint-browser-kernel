# Overlay Branding

对 Chromium 源码的修改：产品名、可执行文件名、产品图标、BRANDING、install_static 支持。

## 配置（branding.config）

复制 `branding.config.example` 为 `branding.config`，按需修改。仅放定制项；输出目录由 driver 的 --out-dir 或环境变量 SIMPRINT_OUT_DIR 指定。

- `chrome_executable_name`：可执行文件名（对应补丁 001），如 `"simprint"` → simprint.exe
- `is_simprint_branded`：是否启用 Simprint 图标与 BRANDING（对应补丁 002、003），`true` / `false`
- `branding_path_component`：模板用，主题路径与图标名（如 `simprint` → theme/simprint/win、simprint.ico）
- `product_display_name`：模板用，产品显示名（BRANDING 等，如 `Simprint`）

deploy 阶段会将本配置同步到 Chromium 的 `args.gn`（路径由 SIMPRINT_OUT_DIR 指定，默认 `out/Default`）。

## 补丁与模板（patches/）

| 文件 | 说明 |
|------|------|
| 001-executable-name.patch | 自定义可执行文件名：gn 参数 `chrome_executable_name` |
| 002-simprint-icon.patch.j2 | **Jinja2 模板**：图标与 branding，apply 时用 branding.config 渲染（`{{ branding_path_component }}`、`{{ product_display_name }}`） |
| 003-install-static-simprint.patch | install_static 支持 Simprint，避免链接缺失符号 |

顺序见 `apply_order.txt`。`.patch.j2` 在 apply 时先读 config 再渲染，再应用；需 `uv sync` 安装 jinja2。

## 资源

- 应用 002 后，目录 `chrome/app/theme/simprint/` 与 `simprint/win/` 由补丁自动创建。
- 图标（.ico）不在补丁中：deploy 阶段会从 Chromium `theme/chromium/win/` 复制到 `theme/simprint/win/` 并重命名；发布前可替换为自己的图标。

## 统一入口

由 driver 调用：`python overlay/branding/run.py apply|deploy|build`。apply 打补丁；deploy 执行 `scripts/deploy_resources.py`（复制图标）与 `scripts/sync_gn_args.py`（将 branding.config 同步到 args.gn）；build 当前无操作。
