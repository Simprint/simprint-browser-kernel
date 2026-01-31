# Simprint 定制与发布配套方案

本文档约定：**补丁集 + 移植说明**、**独立库**、以及**打包 / 发布等配套脚本**的目录与职责，便于后续逐步落地。

---

## 一、整体目录约定（工作目录与 Chromium 分离）

**工作目录**（本仓库，建议目录/仓库名 **simprint-browser-kernel**）仅包含定制与发布所需内容；**Chromium 源码**单独放在其它目录（如 `simprint-browser/` 下的 gclient 拉取结果），通过配置或环境变量指定路径。可扩展结构（补丁按功能域、APPLY_ORDER、scripts/apply）见 **`docs/PROJECT_STRUCTURE.md`**。

```
simprint/                              # 项目根（示例）
├── simprint-browser-kernel/           # 本仓库
│   ├── config/                        # 配置（Chromium 路径、打包/发布选项等）
│   ├── docs/                          # 文档（结构约定、移植说明、本方案等）
│   ├── patches/                       # 补丁集：按功能域分子目录
│   │   ├── APPLY_ORDER                # 全局应用顺序
│   │   ├── branding/                  # 品牌与外观
│   │   ├── fingerprint/               # 指纹相关（预留）
│   │   └── …
│   └── scripts/
│       ├── apply/                     # 按顺序应用补丁
│       ├── pack/                      # 打包 / 发行
│       ├── release/                   # 发布
│       └── build/                     # 构建相关（可选）
│
└── simprint-browser/                  # Chromium 源码（gclient、src、.cipd 等，不纳入本仓库）
    ├── .gclient
    ├── src/                           # Chromium 源码根（含 chrome/VERSION、out/）
    └── …
```

- **Chromium 根目录**：由 `config/kernel.config` 或环境变量 `SIMPRINT_CHROMIUM_ROOT` 指定，例如 `../simprint-browser/src`。脚本中“源码路径”均指该目录。
- **只维护一个版本时**：补丁、独立库（位于 Chromium 的 `third_party/simprint_core`）、脚本都针对该配置下的一份源码工作。
- **多版本时**：可复制本工作目录或通过 `--chromium-root`、`--version` 等参数指向不同 Chromium 目录。

---

## 二、补丁集与移植说明

| 内容           | 位置 | 说明 |
|----------------|------|------|
| 补丁文件       | `patches/<域>/*.patch` | 按功能域分子目录（如 `branding/`、`fingerprint/`），每域内按编号或命名分文件。 |
| 全局应用顺序   | `patches/APPLY_ORDER` | 一行一个 patch 相对路径，按此顺序在 Chromium 根目录应用。 |
| 应用/生成说明  | `patches/README.md`、`patches/<域>/README.md` | 如何应用（含 `scripts/apply/apply-all.sh`）、如何生成/更新补丁。 |
| 移植说明       | `docs/MIGRATION.md` | 定制点清单、新版本移植时的适配要点（API 变化、文件重命名等）。 |

应用流程建议：**先** 在干净 Chromium 上执行 `scripts/apply/apply-all.sh` 或按 `APPLY_ORDER` 手动 `patch -p1`，**再** 参考 `docs/MIGRATION.md` 做手工适配（若有冲突或 API 变化）。

---

## 三、打包 / 发行脚本（参与打包发行）

目标：从 Chromium 构建产物生成「可分发」的压缩包或安装目录（例如 zip、安装器输入目录等）。

| 脚本/职责           | 建议路径 | 说明 |
|---------------------|----------|------|
| 主入口              | `scripts/pack/pack.sh` 或 `pack.bat` | 根据参数选择 Release/Debug、是否带符号等，调用下方脚本。 |
| 收集运行时文件      | `scripts/pack/collect_runtime.py` 或 `.sh` | 从配置的 Chromium 根目录下 `out/Release`（或 `out/Default`）拷贝 chrome.exe、DLL、resources、locales 等到临时目录（如 `dist/portable`）。 |
| 打 zip/安装目录      | `scripts/pack/archive.py` 或 `.sh` | 对 `dist/portable` 打 zip（或生成 NSIS/安装器所需目录结构）。 |
| 版本与命名          | `scripts/pack/version_from_src.py` | 读取 Chromium 根目录下 `chrome/VERSION`，生成包名或资源中的版本号（如 `simprint-146.0.7660.0-win64.zip`）。 |
| 配置（可选）        | `config/pack.config.json` 或 `.env` | 输出目录、是否包含 PDB、要排除的文件列表等。 |

调用关系示例：  
`pack.bat` → 读取 Chromium 根路径（config/环境变量）→ 调用 `collect_runtime.*` → 调用 `archive.*`，产物输出到工作目录下 `dist/` 或配置的路径。

---

## 四、发布脚本（参与发布）

目标：把「已打包好的产物」推到发布渠道（本地目录、内网、或外网），并可选地做版本记录。

| 脚本/职责           | 建议路径 | 说明 |
|---------------------|----------|------|
| 主入口              | `scripts/release/release.sh` 或 `release.bat` | 参数：版本号、渠道（如 local / staging / production）、可选的上传目标。 |
| 上传/拷贝           | `scripts/release/upload.py` 或 `.sh` | 根据渠道把指定 zip 或目录拷贝到固定路径、或调用 S3/OSS/内网存储 API。 |
| 版本清单            | `scripts/release/update_manifest.py` | 更新 `config/releases.json` 或类似清单（版本号、文件名、日期、渠道），便于客户端或文档引用。 |
| 发布前检查（可选）  | `scripts/release/check_release.py` | 校验 zip 存在、版本号与 Chromium 根目录下 `chrome/VERSION` 一致、必要文件齐全等。 |

调用关系示例：  
`release.bat` → `check_release.*`（可选）→ `upload.*` → `update_manifest.*`。

---

## 五、其他可选配套

| 类别         | 位置 | 说明 |
|--------------|------|------|
| 构建封装     | `scripts/build/build_simprint.bat` | 封装 `gn gen` + `autoninja`，在配置的 Chromium 根目录下执行，便于与 `scripts/pack` 约定路径。 |
| 环境检查     | `scripts/build/check_env.py` | 检查 VS、depot_tools、磁盘空间等，在 pack/release 前可调用。 |
| CI 配置      | `.github/workflows/` 或其它 CI 目录 | 需要时在 CI 中调用 `scripts/build` → `scripts/pack` → `scripts/release`（仅内网/安全渠道建议放 CI）。 |
| 配置集中管理 | `config/` | 除 `kernel.config`（Chromium 路径）、`pack.config.json`、`releases.json` 外，可放渠道列表、上传 endpoint 等，脚本通过 `config/` 读配置，避免写死在脚本里。 |

---

## 六、与「定制拆分」的衔接

- **补丁集**：只包含对 Chromium 源码的修改；应用补丁时在 **Chromium 根目录** 执行，脚本不修改 Chromium，只读 `chrome/VERSION` 和构建产物。
- **独立库**：位于 Chromium 树内 `third_party/simprint_core`，随 Chromium 一起构建；打包时无需单独处理，只需按现有方式收集 `out/Release` 等即可。
- **版本唯一**：当前仅维护一个 Chromium 版本，所有脚本默认针对配置中的 Chromium 根目录与一次构建产物；将来若多版本，可通过 `--chromium-root`、`--version` 等参数区分。

按上述方式在工作目录中添加「参与打包发行的脚本」「参与发布的脚本」及其他配套，即可与补丁集 + 移植说明 + 独立库的方案一致，且目录清晰、便于后续扩展多版本或自动化。
