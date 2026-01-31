# Simprint 指纹浏览器 · 定制与发布内核

本仓库承担 **Simprint 指纹浏览器** 定制与发布的核心角色：仅包含补丁、文档、脚本和配置，不包含 Chromium 源码。目录与远程仓库名均为 **simprint-browser-kernel**。

## 目录结构

```
simprint-browser-kernel/
├── driver/                        # 流水线驱动：按 order.txt 调用各单元
│   ├── driver.config, order.txt, config.py, runner.py, cli.py, __main__.py
├── integration/                   # 接入层：最少 patch + 构建胶水
│   ├── patches/, apply_order.txt, run.py, README.md
├── overlay/                       # 对 Chromium 源码的修改（无需接入层）
│   └── branding/                  # 产品名、图标、主题
│       ├── patches/, scripts/, branding.config, run.py, README.md
├── projects/                      # 依赖接入层的高阶定制功能（仅 fingerprint、shared；无 branding）
│   ├── fingerprint/               # 指纹等（预留）
│   │   ├── apply_order.txt, patches/, README.md, run.py
│   └── shared/                    # 公共库区（预留）
│       ├── apply_order.txt, patches/, README.md, run.py
├── docs/
│   └── PROJECT_STRUCTURE.md
└── README.md
```

- **driver/**：流水线驱动，按 driver/order.txt 依次调用 integration、overlay、projects 下各单元的 run.py。
- **integration/**：与 Chromium 的接入（最少 patch、BUILD 链库等），当前无 patch。
- **overlay/**：对源码的修改（产品名、图标、主题等），不依赖接入层。
- **projects/**：高阶定制功能（指纹、鉴权、同步等），依赖接入层。

详细约定见 **`docs/PROJECT_STRUCTURE.md`**。

## Chromium 源码位置

Chromium 源码与构建产物**不放在本目录**，需单独通过 gclient 等工具维护（例如在 `../simprint-browser/` 下）。

- **Chromium 根目录**：在 `driver/driver.config` 或环境变量 `SIMPRINT_CHROMIUM_ROOT` 中指定（如 `../simprint-browser/src`）。

## 使用方式

1. 克隆或拉取本仓库到本地。
2. 复制 `driver/driver.config.example` 为 `driver/driver.config` 并填写 Chromium 路径，或设置环境变量 `SIMPRINT_CHROMIUM_ROOT`。
3. **应用补丁与资源**（在仓库根目录执行，需 Python 3）：
   - **一键**：`python -m driver apply-and-prepare`（先 apply 再 deploy）。
   - 分步：`python -m driver apply`，再 `python -m driver deploy`。
   - 仅处理某单元：`python -m driver apply --project branding`、`python -m driver deploy --project branding`。
   - 构建阶段（预留）：`python -m driver build`。
4. 在 Chromium 目录中执行 gn gen、autoninja 等构建。

详见 `docs/PROJECT_STRUCTURE.md`、各单元 README。
