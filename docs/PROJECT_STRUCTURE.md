# Simprint 定制仓库项目结构

本文档约定目录组织方式：**driver** 驱动流水线；**integration** 为接入层；**overlay** 为对 Chromium 源码的修改；**projects** 为依赖接入层的高阶定制功能。

---

## 一、整体原则

1. **顶层按角色分组**：
   - **driver/**：流水线驱动，读 driver.config、order.txt，按顺序调用 integration、overlay、projects 下各单元的 run.py。
   - **integration/**：与 Chromium 的接入（最少 patch、BUILD 链库），不承载业务。
   - **overlay/**：对 Chromium 源码的修改（产品名、图标、主题等），无需接入层即可完成。
   - **projects/**：依赖接入层的高阶定制功能（指纹、鉴权、同步等）。
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
├── projects/                      # 高阶定制功能（仅 fingerprint、shared；branding 在 overlay/ 下）
│   ├── fingerprint/
│   │   ├── apply_order.txt
│   │   ├── patches/
│   │   │   └── .gitkeep
│   │   ├── README.md
│   │   └── run.py
│   └── shared/
│       ├── apply_order.txt
│       ├── patches/
│       │   └── .gitkeep
│       ├── README.md
│       └── run.py
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

## 四、各阶段含义

| 阶段  | 含义           | 各单元可做 |
|-------|----------------|------------|
| apply | 对 Chromium 打补丁或写入 | integration/overlay/projects 的 patches 或模板渲染 |
| deploy | 部署资源、写 args.gn 等 | 复制图标、sync_gn_args 等 |
| build | 编译或产出物   | 当前多为 no-op；将来可编静态/动态库 |

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
