# Simprint 指纹浏览器 · 定制与发布内核

本仓库承担 **Simprint 指纹浏览器** 定制与发布的核心角色：仅包含补丁、文档、脚本和配置，不包含 Chromium 源码。目录与远程仓库名均为 **simprint-browser-kernel**。

## 目录结构

```
simprint-browser-kernel/        # 本仓库（目录/仓库名）
├── config/                    # 配置（Chromium 路径、打包/发布选项等）
├── docs/                      # 文档（结构约定、移植说明、配套方案等）
├── patches/                   # 补丁集：按功能域分子目录，便于扩展
│   ├── APPLY_ORDER            # 全局应用顺序
│   ├── branding/              # 品牌与外观（名称、图标等）
│   ├── fingerprint/           # 指纹相关（预留）
│   └── …                      # 其他功能域按需新建
└── scripts/
    ├── apply/                 # 按顺序应用补丁
    ├── build/                 # 构建封装（可选）
    ├── pack/                  # 打包发行
    └── release/               # 发布
```

详细约定与扩展方式见 **`docs/PROJECT_STRUCTURE.md`**。

## Chromium 源码位置

Chromium 源码与构建产物**不放在本目录**，需单独通过 gclient 等工具维护（例如在 `../simprint-browser/` 下）。

- **Chromium 根目录**：在 `config/kernel.config` 或环境变量中指定（如 `SIMPRINT_CHROMIUM_ROOT`），默认可指向 `../simprint-browser/src`。
- 脚本中的“源码路径”均指该配置的 Chromium 根目录（其下含 `chrome/VERSION`、`out/` 等）。

## 使用方式

1. 克隆或拉取本仓库到本地。
2. 在 `config/kernel.config` 或环境中设置 Chromium 根目录路径（如 `SIMPRINT_CHROMIUM_ROOT`）。
3. 应用补丁：在内核仓库根目录执行 `scripts/apply/apply-all.sh`（或按 `patches/README.md` 手动应用）。
4. 在 Chromium 目录中执行 gclient sync、gn、autoninja 等构建；本目录的 `scripts/build` 可封装上述步骤。
5. 使用 `scripts/pack` 打包、`scripts/release` 发布。

详见 `docs/PROJECT_STRUCTURE.md`、`docs/CUSTOMIZATION_AND_RELEASE_PLAN.md`。
