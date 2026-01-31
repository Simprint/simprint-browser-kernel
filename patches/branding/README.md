# 品牌与外观（branding）

本域补丁：可执行文件名、产品图标、BRANDING 等。

## 补丁列表（按应用顺序）

| 文件 | 说明 |
|------|------|
| `001-executable-name.patch` | 自定义可执行文件名：gn 参数 `chrome_executable_name`（默认 `chrome`），可改为如 `simprint` 得到 simprint.exe |
| `002-simprint-icon.patch` | Simprint 图标与 branding：`is_simprint_branded`、`chrome/app/theme/simprint/`、Windows 图标路径 |

## 应用后需做

1. **启用自定义名称**：gn args 增加 `chrome_executable_name = "simprint"`。
2. **启用 Simprint 图标**：gn args 增加 `is_simprint_branded = true`，并在 Chromium 树内 `chrome/app/theme/simprint/win/` 下放置图标（至少 `simprint.ico`，可先复制 `../chromium/win/chromium.ico` 做测试）。

## 生成/更新补丁

在 Chromium 根目录（`src`）修改后：

```bash
cd /path/to/simprint-browser/src
git diff build/config/chrome_build.gni chrome/BUILD.gn > /path/to/simprint-browser-kernel/patches/branding/001-executable-name.patch
# 或按修改范围生成 002 等
```

生成后应用时在 `src/` 下用 `patch -p1`。
