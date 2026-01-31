# Simprint 补丁集

本目录按**功能域**分子目录存放补丁，便于扩展无数功能；应用时按**全局顺序**（见 `APPLY_ORDER`）在 Chromium 源码根目录执行。

## 目录与全局顺序

| 目录 | 说明 |
|------|------|
| `APPLY_ORDER` | 全局应用顺序：一行一个 patch 相对路径（如 `branding/001-executable-name.patch`） |
| `branding/` | 品牌与外观（可执行名、图标、BRANDING） |
| `fingerprint/` | 指纹相关（预留：canvas、webgl、navigator 等） |
| … | 其他功能域按需新建，并在 `APPLY_ORDER` 中追加 |

每域内见该域 `README.md`（补丁列表、应用后需做、生成方式）。

## 如何应用

**Chromium 源码根目录** = `simprint-browser/src`（含 `chrome/VERSION`、`build/` 的目录）。

### 方式一：脚本按顺序应用（推荐）

在内核仓库根目录执行（会读 `APPLY_ORDER` 与 `config/kernel.config` 或 `SIMPRINT_CHROMIUM_ROOT`）：

```bash
cd /path/to/simprint-browser-kernel
./scripts/apply/apply-all.sh
# 或 Windows: scripts\apply\apply-all.bat
```

### 方式二：手动按顺序应用

在 Chromium 源码根目录执行，补丁路径为 `a/`、`b/` 时用 `-p1`：

```bash
cd /path/to/simprint-browser/src
PATCHES=/path/to/simprint-browser-kernel/patches

patch -p1 < "$PATCHES/branding/001-executable-name.patch"
patch -p1 < "$PATCHES/branding/002-simprint-icon.patch"
# 后续按 APPLY_ORDER 中顺序继续
```

相对路径示例（内核与 simprint-browser 平级时）：

```bash
cd /d/Documents/cprojects/simprint/simprint-browser/src
patch -p1 < ../../simprint-browser-kernel/patches/branding/001-executable-name.patch
patch -p1 < ../../simprint-browser-kernel/patches/branding/002-simprint-icon.patch
```

## 新增功能域

1. 在 `patches/` 下新建目录，如 `patches/network/`。
2. 放入该域的 `.patch` 与 `README.md`。
3. 在 `APPLY_ORDER` 末尾追加该域补丁路径（一行一个，按域内顺序）。

## 冲突与移植

- 若 `patch -p1` 报错或冲突，按提示手工合入后，再按各域 README 中「生成/更新补丁」更新对应 patch。
- 换 Chromium 版本时，按 `APPLY_ORDER` 重新应用；若有 API/路径变化，见 `docs/MIGRATION.md` 做适配后更新补丁。
