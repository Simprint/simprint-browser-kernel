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
| 007-resource-allowlist-pdb.patch | resource_allowlist 保持使用 `chrome.dll.pdb`（与上游一致；若上游已是该内容则补丁为 no-op），便于 is_official_build 下 enable_resource_allowlist_generation 正确依赖 PDB |
| 008-variations-dedupe-by-name.py | **可执行脚本**：variations 的 generate_ui_string_overrider 按 (hash, name) 去重；若目标文件已含该逻辑则跳过，避免上游已有修复时重复插入。apply 阶段执行该 .py。 |
| 009-disable-startup-infobars.patch | 禁用启动时的三个横幅提示：Google API keys missing、Session Restore、Default browser prompt |

顺序见 `apply_order.txt`。`.patch.j2` 在 apply 时先读 config 再渲染，再应用；需 `uv sync` 安装 jinja2。

## 资源

- 应用 002 后，目录 `chrome/app/theme/{branding_path_component}/` 与 `{branding_path_component}/win/` 由补丁自动创建。
- 图标（.ico）：deploy 从配置读取品牌名，优先从 `overlay/branding/icons/` 复制到 `theme/{品牌}/win/`（如 simprint.ico），缺失时从 Chromium `theme/chromium/win/` 复制并重命名。
- **磁贴图（tiles）与 VisualElements**：deploy 从 `overlay/branding/icons/tiles/` 复制 Logo.png、SmallLogo.png 等到 `theme/{品牌}/win/tiles/`，缺失时从 Chromium 复制。构建时 `chrome/BUILD.gn` 的 `visual_elements_resources` 从 `theme/$branding_path_component/win/tiles/` 取图并输出到构建目录，最终进入安装包/ZIP 的 **VersionDir/VisualElements/**（开始菜单磁贴）。若构建用的是 `out/Release`，需保证 args.gn 中有 `is_simprint_branded = true`，否则会使用 `theme/chromium/win/tiles/`（Chrome 图）。sync_gn_args 会同时写入 `out/Default` 与 `out/Release`，避免用错构建目录导致 VisualElements 仍是 Chrome 图。
- 字符串与矢量图标：deploy 会复制/生成 `components_{品牌}_strings.grd`、`vector_icons/{品牌}/`、`chrome/app/{品牌}_strings.grd` 及 `{品牌}_strings_*.xtb`，品牌名来自 `branding_path_component`。
- **overlay/branding/strings/**：存放通用名 `strings.grd` 与 `strings_*.xtb`，内容使用占位符 `__PRODUCT_DISPLAY_NAME__`；deploy 时拷贝为 `{branding_path_component}_strings.*` 并替换占位符为 `product_display_name`，便于换品牌（如改为 prase）时只改配置。
