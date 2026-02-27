# Simprint 指纹浏览器 · 定制与发布内核

本仓库承担 **Simprint 指纹浏览器** 定制与发布的核心角色：仅包含补丁、文档、脚本和配置，不包含 Chromium 源码。目录与远程仓库名均为 **simprint-browser-kernel**。

## 目录结构

```
simprint-browser-kernel/
├── driver/                        # 流水线驱动：按 order.txt 调用各单元
│   ├── driver.config, order.txt, config.py, runner.py, cli.py, __main__.py
├── integration/                   # 接入层：最少 patch，建立可接入入口（链库、hook 等）
│   ├── patches/, apply_order.txt, run.py, README.md
├── overlay/                       # 对 Chromium 源码的修改（无需接入层）
│   └── branding/                  # 产品名、图标、主题
│       ├── patches/, scripts/, branding.config, run.py, README.md
├── projects/                      # 业务代码，通过接入层入口接入 Chromium
│   ├── fingerprint/               # 指纹等（预留）
│   │   ├── 代码、BUILD、run.py 等
│   └── shared/                    # 公共库区（预留）
│       ├── 代码、BUILD、run.py 等
├── docs/
│   └── PROJECT_STRUCTURE.md
└── README.md
```

- **driver/**：流水线驱动，按 driver/order.txt 依次调用 integration、overlay、projects 下各单元的 run.py。
- **integration/**：**接入层**。通过最少 patch 在 Chromium 中建立**可接入的入口**，不承载业务逻辑；目的是让 projects 中的代码能够挂进来，而非用 patch 直接改 Chromium 业务。
- **overlay/**：对源码的修改（产品名、图标、主题等），不依赖接入层，主要通过 patch 完成。
- **projects/**：**业务代码**。通过 integration 提供的接入点**接入** Chromium，从而修改或扩展功能。接入方式多样，如静态库、hook 某个对象、替换实现等；复杂功能在此用代码实现，而非通过 patch 改 Chromium 源码。

详细约定见 **`docs/PROJECT_STRUCTURE.md`**。

## Chromium 源码位置

Chromium 源码与构建产物**不放在本目录**，需单独通过 gclient 等工具维护（例如在 `../simprint-browser/` 下）。

- **Chromium 根目录**：在 `driver/driver.config` 或环境变量 `SIMPRINT_CHROMIUM_ROOT` 中指定（如 `../simprint-browser/src`）。

## 使用方式

本仓库推荐使用 **uv** 管理 Python 依赖（含 Jinja2，用于 `.patch.j2` 模板渲染）。以下为基于 uv 的详细步骤。

### 环境准备

1. **安装 uv**（若尚未安装）：
   ```bash
   # Windows (PowerShell)
   irm https://astral.sh/uv/install.ps1 | iex
   # 或 macOS/Linux
   curl -LsSf https://astral.sh/uv/install.sh | sh
   ```

2. **克隆本仓库**：
   ```bash
   git clone <本仓库地址> simprint-browser-kernel
   cd simprint-browser-kernel
   ```

3. **安装依赖**（在仓库根目录执行）：
   ```bash
   uv sync
   ```
   会创建虚拟环境并安装 `pyproject.toml` 中的依赖（如 jinja2），并生成/更新 `uv.lock`。

### 配置

4. **指定 Chromium 源码路径**（二选一）：
   - **推荐**：复制 `driver/driver.config.example` 为 `driver/driver.config`，编辑并填写：
     ```ini
     SIMPRINT_CHROMIUM_ROOT=../simprint-browser/src
     ```
     路径为 gclient 拉取后的 `src` 目录（含 `chrome/VERSION`、`out/` 等）。
   - 或设置环境变量：`SIMPRINT_CHROMIUM_ROOT=<Chromium src 绝对或相对路径>`。

5. **Branding 可选配置**：若需修改产品名、可执行文件名、图标等，复制 `overlay/branding/branding.config.example` 为 `overlay/branding/branding.config` 并按需编辑。未配置时使用示例中的默认值。

### 应用补丁与部署资源

以下命令均在 **simprint-browser-kernel 仓库根目录** 执行，使用 `uv run` 以自动使用当前项目的虚拟环境。

6. **一键应用补丁并部署资源**（推荐）：
   ```bash
   uv run python -m driver apply-and-prepare
   ```
   会先对 Chromium 源码执行 apply（打补丁，含 `.patch.j2` 渲染），再执行 deploy（部署图标、字符串、同步 args.gn 等）。

7. **分步执行**：
   ```bash
   uv run python -m driver apply    # 仅应用补丁
   uv run python -m driver deploy   # 仅部署资源（需先 apply）
   ```

8. **仅处理 branding 单元**：
   ```bash
   uv run python -m driver apply --project branding
   uv run python -m driver deploy --project branding
   # 或一次性
   uv run python -m driver apply-and-prepare --project branding
   ```

9. **构建阶段**（预留，当前无实际操作）：
   ```bash
   uv run python -m driver build
   ```

### 构建 Chromium

10. 在 **Chromium 源码目录**（即 `SIMPRINT_CHROMIUM_ROOT` 所指的 `src`）中执行常规构建，例如：
    ```bash
    gn gen out/Default
    autoninja -C out/Default chrome
    ```
    或使用你已有的 `args.gn` 与输出目录。deploy 阶段会将 branding 相关选项同步到 `args.gn`（若指定了 `SIMPRINT_OUT_DIR`，则对应该 out 目录）。

---

更多约定见 `docs/PROJECT_STRUCTURE.md`，各单元说明见 `integration/`、`overlay/branding/`、`projects/*/` 下的 README。

```
gn gen out\Release --args="is_simprint_branded=true is_debug=false is_official_build=true chrome_pgo_phase=0 dawn_enable_webgpu_on_webgpu = true"

autoninja -C out\Release chrome  
```