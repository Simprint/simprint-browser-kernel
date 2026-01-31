# Simprint 内核项目结构（可扩展）

本文档约定在「无数各种功能」下如何组织补丁、文档、脚本与配置，便于按功能模块扩展且不混乱。

---

## 一、整体原则

1. **按功能/领域分目录**：补丁、文档、脚本都尽量按「功能域」拆分（如 branding、fingerprint、network），避免单层目录下文件爆炸。
2. **统一入口与顺序**：补丁有全局应用顺序（如 `patches/APPLY_ORDER` 或各子目录 README 约定）；脚本有统一入口（如 `scripts/apply/all.sh`、`scripts/pack/pack.sh`），内部再按模块调用。
3. **配置与代码分离**：路径、开关、渠道等放 `config/`，脚本只读配置不写死路径。
4. **文档跟功能走**：每个功能域可有自己的 README 或 `docs/features/<域>.md`，全局移植/发布说明放 `docs/` 根下。

---

## 二、推荐目录结构（扩展版）

```
simprint-browser-kernel/
├── README.md
├── .gitignore
│
├── config/                          # 配置（集中管理，脚本只读）
│   ├── kernel.config.example        # Chromium 路径等
│   ├── pack.config.json.example     # 打包选项（可选）
│   ├── release.config.json.example  # 发布渠道等（可选）
│   └── features.optional.json       # 可选：哪些功能域启用（可选）
│
├── patches/                         # 补丁：按功能域分子目录，便于无数功能
│   ├── README.md                    # 如何应用、如何生成、全局顺序说明
│   ├── APPLY_ORDER                  # 可选：一行一个 patch 路径，按顺序应用
│   ├── branding/                    # 品牌与外观（名称、图标、产品名等）
│   │   ├── README.md
│   │   ├── 001-executable-name.patch
│   │   └── 002-simprint-icon.patch
│   ├── fingerprint/                 # 指纹相关（canvas、webgl、navigator 等）
│   │   ├── README.md
│   │   └── ...
│   ├── network/                     # 网络/代理/请求头等（示例）
│   │   ├── README.md
│   │   └── ...
│   └── ...                          # 其他功能域按需新建
│
├── docs/                            # 文档
│   ├── PROJECT_STRUCTURE.md         # 本文件：项目结构约定
│   ├── CUSTOMIZATION_AND_RELEASE_PLAN.md  # 定制与发布方案
│   ├── MIGRATION.md                 # 移植说明（定制点清单、新版本适配）
│   ├── REPO_NAME.md
│   └── features/                    # 可选：按功能域写说明
│       ├── branding.md
│       ├── fingerprint.md
│       └── ...
│
├── scripts/                         # 脚本：按职责分子目录
│   ├── apply/                       # 应用补丁（按 APPLY_ORDER 或各域 README）
│   │   ├── README.md
│   │   ├── apply-all.sh
│   │   └── apply-all.bat
│   ├── build/                       # 构建封装（gn、autoninja）
│   ├── pack/                        # 打包发行
│   └── release/                     # 发布
│
└── (可选) features/                 # 可选：功能清单与简要说明，便于导航
    ├── README.md                    # 功能列表 + 对应 patches/docs 链接
    ├── branding.md
    └── ...
```

- **Chromium 源码**仍在仓库外（如 `simprint-browser/src`），由 `config/kernel.config` 或 `SIMPRINT_CHROMIUM_ROOT` 指定。
- **独立库**（如 `third_party/simprint_core`）仍在 Chromium 树内，本仓库只通过补丁或文档描述如何接入。

---

## 三、补丁（patches/）约定

| 内容 | 位置 | 说明 |
|------|------|------|
| 按功能分子目录 | `patches/<域>/` | 如 `branding/`、`fingerprint/`、`network/`，每域内可再细分或按 001、002 编号。 |
| 全局应用顺序 | `patches/README.md` 或 `patches/APPLY_ORDER` | 明确先应用哪一域、再应用哪一域；同域内按文件名数字顺序。 |
| 每域说明 | `patches/<域>/README.md` | 该域包含哪些 patch、依赖关系、gn 参数、应用后需做的步骤。 |

**应用方式示例**：

- 方式 A：在 Chromium 根目录执行 `scripts/apply/apply-all.sh`，脚本读取 `APPLY_ORDER` 或按约定顺序依次 `patch -p1 < ...`。
- 方式 B：仍手动按 `patches/README.md` 列出的顺序执行 `patch -p1 < ...`。

**新增功能域时**：在 `patches/` 下新建目录（如 `patches/fingerprint/`），放入 patch 与 README，并在 `patches/README.md`（或 `APPLY_ORDER`）中把该域加入全局顺序。

---

## 四、文档（docs/）约定

| 内容 | 位置 | 说明 |
|------|------|------|
| 项目结构 | `docs/PROJECT_STRUCTURE.md` | 本文件。 |
| 定制与发布方案 | `docs/CUSTOMIZATION_AND_RELEASE_PLAN.md` | 打包、发布、构建、配置的职责与路径。 |
| 移植说明 | `docs/MIGRATION.md` | 所有定制点清单、新版本移植时的适配要点（API 变化、文件重命名等）。 |
| 按功能域说明 | `docs/features/<域>.md`（可选） | 某功能的背景、设计、与 patches 的对应关系；可与 `patches/<域>/README.md` 二选一或互补。 |

后续功能增多时，优先在 `docs/MIGRATION.md` 中追加「定制点 + 移植注意」，复杂功能再单独加 `docs/features/<域>.md`。

---

## 五、脚本（scripts/）约定

| 目录 | 职责 | 扩展方式 |
|------|------|----------|
| `scripts/apply/` | 按顺序应用全部或部分补丁 | 新增「按域应用」或「跳过某域」参数即可。 |
| `scripts/build/` | 封装 gn、autoninja，读 config 中的 Chromium 路径 | 保持单入口，参数可增加（如 Debug/Release、target）。 |
| `scripts/pack/` | 收集运行时、打 zip、版本命名 | 新增子脚本或参数（如多 target、多平台）。 |
| `scripts/release/` | 上传、版本清单、发布前检查 | 新增渠道或存储后端时加配置与分支逻辑。 |

脚本统一从 `config/` 读路径与选项，不写死；新功能若需新参数，优先加配置文件项而非改脚本签名。

---

## 六、配置（config/）约定

| 文件 | 用途 |
|------|------|
| `kernel.config`（或 .example） | Chromium 根目录、可选 out 目录名等。 |
| `pack.config.json`（可选） | 打包输出目录、是否带 PDB、排除列表等。 |
| `release.config.json`（可选） | 渠道列表、上传 endpoint 等。 |
| `features.optional.json`（可选） | 启用哪些 patch 域（便于做「最小集」构建）。 |

后续功能若需开关（如「是否打指纹补丁」），可在此增加配置项，由 `scripts/apply` 或构建脚本读取。

---

## 七、与「无数功能」的衔接

- **新功能 = 新域**：在 `patches/` 下新建 `<新域>/`，必要时在 `docs/features/` 或 `docs/MIGRATION.md` 中补一笔。
- **顺序与依赖**：在 `patches/README.md` 或 `APPLY_ORDER` 中固定全局顺序，有依赖的域排在后面。
- **独立库**：若某功能用独立库（如 `simprint_core`），补丁只做「接入点」修改，逻辑在库内；文档中注明「该功能实现见 third_party/simprint_core 与 patches/xxx」。

按上述结构扩展，可长期保持「补丁按域、文档跟功能、脚本统一入口、配置集中」，便于维护和协作。
