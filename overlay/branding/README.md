# Overlay Branding

对 Chromium 源码的修改：产品名、可执行文件名、产品图标、BRANDING、install_static 支持。

## 配置（branding.config）

复制 `branding.config.example` 为 `branding.config`，按需修改。仅放定制项；输出目录由 driver 的 --out-dir 或环境变量 SIMPRINT_OUT_DIR 指定。

- `chrome_executable_name`：可执行文件名（对应补丁 001），如 `"simprint"` → simprint.exe
- `is_simprint_branded`：是否启用 Simprint 图标与 BRANDING（对应补丁 002、003），`true` / `false`
- `branding_path_component`：apply 模板与 **deploy** 共用；用于 theme/{品牌}/、*_strings.grd、vector_icons/{品牌}/、win 图标名（如 `simprint` → simprint.ico）。**deploy 不再硬编码品牌名，均从此配置读取。**
- `product_display_name`：模板用，产品显示名（BRANDING 等，如 `Simprint`）

deploy 阶段会将本配置同步到 Chromium 的 `args.gn`（路径由 SIMPRINT_OUT_DIR 指定，默认 `out/Default`）。

## 补丁与模板（patches/）

| 文件 | 说明 |
|------|------|
| 001-executable-name.patch.j2 | **Jinja2 模板**：可执行文件名，apply 时用 branding.config 的 `{{ chrome_executable_name }}` 写入 chrome_build.gni，无需依赖 args.gn |
| 002-simprint-icon.patch.j2 | **Jinja2 模板**：图标与 branding，apply 时用 branding.config 渲染（`{{ branding_path_component }}`、`{{ product_display_name }}`） |
| 003-install-static-simprint.patch | install_static 支持 Simprint，避免链接缺失符号 |
| 004-installer-string-rc-simprint.patch | installer 字符串资源支持 simprint 品牌 |
| 005-mini-installer-archive-exe.patch | mini_installer_archive 使用 `chrome_executable_name`（simprint.exe/dll），避免 gn 报 “input not generated” |
| 006a-reorder-imports-exe-name-sig.patch | reorder_imports 函数签名增加 exe_name 参数 |
| 006b-reorder-imports-exe-name-body.patch | reorder_imports 脚本与 BUILD.gn 使用 exe_name，避免生成 chrome.exe.pdb 而期望 simprint.exe.pdb |
| 007-resource-allowlist-pdb.patch | resource_allowlist 使用 `${chrome_executable_name}.dll.pdb`，使 is_official_build 下 enable_resource_allowlist_generation 在 Simprint 构建中正确依赖 PDB |

顺序见 `apply_order.txt`。`.patch.j2` 在 apply 时先读 config 再渲染，再应用；需 `uv sync` 安装 jinja2。

## 资源

- 应用 002 后，目录 `chrome/app/theme/{branding_path_component}/` 与 `{branding_path_component}/win/` 由补丁自动创建。
- 图标（.ico）：deploy 从配置读取品牌名，优先从 `overlay/branding/icons/` 复制到 `theme/{品牌}/win/`（如 simprint.ico），缺失时从 Chromium `theme/chromium/win/` 复制并重命名。
- 磁贴图（tiles）：deploy 从 `overlay/branding/icons/tiles/` 复制到 `theme/{品牌}/win/tiles/`，缺失时从 Chromium 复制。
- 字符串与矢量图标：deploy 会复制/生成 `components_{品牌}_strings.grd`、`vector_icons/{品牌}/`、`chrome/app/{品牌}_strings.grd` 及 `{品牌}_strings_*.xtb`，品牌名来自 `branding_path_component`。
- **overlay/branding/strings/**：存放通用名 `strings.grd` 与 `strings_*.xtb`，内容使用占位符 `__PRODUCT_DISPLAY_NAME__`；deploy 时拷贝为 `{branding_path_component}_strings.*` 并替换占位符为 `product_display_name`，便于换品牌（如改为 prase）时只改配置。

**推荐流程**：执行一次 `apply_deploy`（或先 `apply` 再 `deploy`）后，Chromium 树即可直接 `autoninja -C out\Release chrome`，无需额外手动步骤。

## 统一入口

由 driver 调用：`python overlay/branding/run.py apply|deploy|apply_deploy|build`。

- **apply**：打补丁（含 .patch.j2 渲染）。
- **deploy**：执行 `scripts/deploy_resources.py`（从 branding.config 读 `branding_path_component`，复制图标、tiles、strings、vector_icons 等）与 `scripts/sync_gn_args.py`（将 config 同步到 args.gn）。**需先 apply**，否则 theme/{品牌}/BRANDING 不存在会报错。
- **apply_deploy**：先 apply 再 deploy，一次完成，完成后即可构建。
- **build**：当前无操作。
