# 品牌字符串（通用名 + Jinja2 模板）

本目录存放 **通用名** 的 grd/xtb 及对应的 **Jinja2 模板**（.j2），deploy 时优先从模板渲染并写入 Chromium 树。

## 模板（deploy 使用）

- **strings.grd.j2**：主 grd 模板，渲染为 `chrome/app/{branding_path_component}_strings.grd`。
- **resources/strings_*.xtb.j2**：各语言 xtb 模板，渲染为 `chrome/app/resources/{branding_path_component}_strings_*.xtb`。
- 模板变量来自 **branding.config**：`${ product_display_name }`、`${ branding_path_component }`（使用 `${ }` 避免与 grd 内 ICU 复数语法 `{ }` 冲突）。

当存在 `strings.grd.j2` 时，deploy 使用模板；否则回退为从 Chromium 复制 chromium_strings.*。

## 非模板源文件（可选）

- **strings.grd** / **resources/strings_*.xtb**：通用名源文件，可作编辑或生成 .j2 的基准；deploy 不直接拷贝它们。
