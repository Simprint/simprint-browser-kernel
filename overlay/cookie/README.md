# Overlay Cookie

**启动 Cookie 注入与 LaunchConfig 后处理模块**

本模块用于承接 `simprint/eventbus/eventbus.cc` 中与以下能力相关的后续增量：

- 启动阶段 Cookie 注入
- Cookie 注入完成后的账号导入编排
- 浏览器窗口就绪后的 URL 重试打开

当前已落入的 patch 区间：

- `4fb8aadb1ceab45e405905887476e2e5be65ee46..9baa250511ea5c70d718c0ad558400ca2a52985b`

## 目录结构

```text
overlay/cookie/
├── apply_order.txt
├── README.md
├── run.py
├── patches/
│   └── .gitkeep
├── scripts/
│   └── deploy_resources.py
└── sources/
    └── .gitkeep
```

## 说明

- 当前模块仅建立标准 overlay 结构，便于后续通过命令工具导入 patch。
- 新模块应追加在 `driver/order.txt` 末尾执行，避免影响现有 patch 栈上下文。
- 若后续需要新增源文件，可放入 `sources/`，并由 `deploy_resources.py` 部署到 Chromium 树。
