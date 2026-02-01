# Simprint 定制仓库项目结构

本文档约定目录组织方式：**driver** 驱动流水线；**integration** 为接入层；**overlay** 为对 Chromium 源码的修改；**projects** 为通过接入层接入 Chromium 的业务代码。

---

## 一、整体原则

1. **顶层按角色分组**：
   - **driver/**：流水线驱动，读 driver.config、order.txt，按顺序调用 integration、overlay、projects 下各单元的 run.py。
   - **integration/**：**接入层**。通过最少 patch 在 Chromium 中建立**可接入的入口**，不承载业务逻辑；目的是让 projects 中的代码能够接入 Chromium，而非用 patch 直接改 Chromium 业务。复杂功能单靠 patch 难以实现且难维护，接入层只做「开口」，具体实现放在 projects。
   - **overlay/**：对 Chromium 源码的修改（产品名、图标、主题等），无需接入层即可完成，主要通过 patch 实现。
   - **projects/**：**业务代码**。通过 integration 建立的接入点**接入** Chromium，从而修改或扩展功能（如指纹、鉴权、同步）。接入方式多样，如静态库、hook 某个对象、替换实现等；实现方式为代码 + 构建产物，而非通过 patch 改 Chromium 源码。
   - **docs/**：仓库级文档。

2. **单元自包含**：integration、overlay/<名>、projects/<名> 各自提供 run.py，支持 apply、deploy、build。

3. **driver 薄编排**：只负责读配置、读顺序、解析单元路径（integration → integration/，branding → overlay/branding/，fingerprint → projects/fingerprint/），执行 run.py，不关心单元内部实现。

---

## 二、目录结构

```
simprint-browser-kernel/
├── .gitignore
├── README.md
│
├── driver/                        # 流水线驱动
│   ├── __init__.py
│   ├── __main__.py
│   ├── cli.py
│   ├── config.py
│   ├── driver.config
│   ├── driver.config.example
│   ├── order.txt
│   └── runner.py
│
├── integration/                   # 接入层
│   ├── apply_order.txt
│   ├── patches/
│   │   └── .gitkeep
│   ├── README.md
│   └── run.py
│
├── overlay/                       # 对源码的修改
│   └── branding/                 # 产品名、图标、主题
│       ├── apply_order.txt
│       ├── branding.config
│       ├── branding.config.example
│       ├── patches/
│       │   ├── 001-executable-name.patch
│       │   ├── 002-simprint-icon.patch
│       │   └── 003-install-static-simprint.patch
│       ├── README.md
│       ├── run.py
│       └── scripts/
│           ├── deploy_resources.py
│           └── sync_gn_args.py
│
├── projects/                      # 业务代码：静态库、hook 等，通过接入层入口接入 Chromium
│   ├── fingerprint/               # 指纹等（代码 + BUILD，非 patch 改 Chromium）
│   │   ├── 源码、BUILD、run.py、README.md 等
│   └── shared/                    # 公共库（静态/动态库，供 fingerprint 等与 Chromium 使用）
│       ├── 源码、BUILD、run.py、README.md 等
│
└── docs/
    └── PROJECT_STRUCTURE.md
```

---

## 三、单元路径解析

driver/runner 根据 order.txt 中的**名称**解析实际目录：

- **integration** → `integration/`
- **branding** → `overlay/branding/`（因 overlay/branding 存在）
- **fingerprint** → `projects/fingerprint/`
- **shared** → `projects/shared/`

即：先查 integration，再查 overlay/<名>，再查 projects/<名>。**branding 在 overlay/ 下，不在 projects/ 下；projects/ 下仅有 fingerprint、shared。**

---

## 三（补充）、接入层与 projects 的关系

- **integration（接入层）**：用**最少 patch** 在 Chromium 里「开口」，不写业务逻辑，只提供**可接入的入口**。
- **projects**：放**业务代码**。这些代码在 build 阶段编译，通过 integration 建立的入口**接入** Chromium，从而修改或扩展 Chromium 行为。接入方式多样，如静态库、hook 某个对象、替换某实现等——复杂功能用代码实现、通过接入点挂入，而非用 patch 直接改 Chromium 源码，便于维护与升级。

---

## 四、各阶段含义

| 阶段  | 含义           | 各单元可做 |
|-------|----------------|------------|
| apply | 对 Chromium 打补丁或写入 | **integration**：打接入用 patch（链库、hook 点等）。**overlay**：打品牌/主题等 patch。**projects**：若有少量接入用 patch 可在此打，但主要工作不依赖 patch。 |
| deploy | 部署资源、写 args.gn 等 | overlay 复制图标、sync_gn_args 等；projects 可部署头文件、配置等到 Chromium 树。 |
| build | 编译或产出物   | **projects**：编静态/动态库、hook 实现等，产出供 Chromium 通过 integration 接入点使用。integration/overlay 当前多为 no-op。 |

---

## 五、使用方式

在仓库根目录执行（需配置 Chromium 路径）：

```bash
python -m driver apply
python -m driver deploy
python -m driver build
python -m driver apply-and-prepare
python -m driver apply --project branding
python -m driver deploy --out-dir out/Release
```

Chromium 路径：`driver/driver.config` 中 `SIMPRINT_CHROMIUM_ROOT` 或环境变量 `SIMPRINT_CHROMIUM_ROOT`。  
输出目录：`--out-dir` 或环境变量 `SIMPRINT_OUT_DIR`，默认 `out/Default`。

---

## 六、新增单元

- **overlay 下新增**：在 overlay/ 下新建目录（如 overlay/other/），提供 run.py、patches/ 等，在 driver/order.txt 中追加名称（如 other）。
- **projects 下新增**：在 projects/ 下新建目录，同上，在 order.txt 中追加名称。

无需修改 driver 代码；driver 仅根据 order.txt 与路径解析规则调用 run.py。
