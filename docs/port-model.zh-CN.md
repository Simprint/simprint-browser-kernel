# Chromium 分支、Core 与 Port 维护模型

## 基本规则

一个 Git 分支只表示一个 Chromium 基线：

```text
simprint/m144 -> Chromium 144.0.7559.118
simprint/m145 -> Chromium 145.x
simprint/m146 -> Chromium 146.x
```

不要在同一分支中增加 `chromium-144/`、`chromium-145/` 等平行目录。当前分支的基线、Core API 与能力状态统一记录在 `port/manifest.toml`。

## 代码边界

```text
Chromium Browser  -> Browser Port  --+
Chromium Content  -> Content Port  ---|
Blink Renderer    -> Renderer Port ---+--> Simprint Core
Network Service   -> Network Port  ---|
Chromium UI       -> UI Port       --+
```

- Core 保存不依赖 Chromium 的配置、协议、策略和通用算法。
- Port 保存 Chromium 类型转换、生命周期、线程、Mojo 和接入代码。
- Hook 可以很多，但应尽量只调用 Port，不在 Chromium 文件中展开产品逻辑。
- 无法抽离的 Chromium 状态机修改属于 Port，不伪装成通用 Core。

CI 会拒绝 Core 直接包含 `chrome/`、`content/`、`net/`、`services/`、Blink 或 UI 头文件。Core 接口按 `core/include/simprint/kernel_api/vN` 显式版本化。

## 能力状态

`port/manifest.toml` 中每项能力有一个状态：

- `legacy-patch`：仍依赖历史 patch 的文本顺序，尚不能独立组合；
- `port`：Chromium 接入已经收敛到当前分支的 Port；
- `core`：实现完全位于稳定 Core 接口之后。

迁移过程中不要通过删除 `driver/order.txt` 的前置项来假设模块已经解耦。使用以下命令查看语义依赖和文本顺序依赖的差异：

```bash
uv run python -m driver plan --features fingerprint
uv run python -m driver plan --features launch-cookies --json
```

只有 `blocked_by_text_order` 为空，能力组合才真正摆脱 legacy patch 前缀。

## 新 Chromium 版本

1. 从对应 Chromium tag 创建新分支，例如 `simprint/m145`。
2. 保持 `core/`、`port/`、`driver/` 路径不变。
3. 更新 `port/manifest.toml` 的精确版本和 milestone。
4. 逐能力迁移 Port/Hook；Core API 不变时不要复制 Core。
5. 运行 `python -m driver validate`。
6. 对完整 Chromium 源码执行 apply、GN 生成、编译和契约测试。

验证目标是能力契约，而不是旧 patch 能否原样应用。如果上游已经提供相同能力，应删除对应下游接入；如果接入点改变，只修改当前版本 Port。

## Core 更新

- 兼容更新：由依赖滚动提交更新各受支持分支，并运行验证矩阵。
- 新增可选能力：旧 Port 可以不声明支持。
- 破坏性更新：新增 Core API major；旧 Chromium 分支继续固定旧 major。
- 安全或稳定性修复：回移到仍受支持的 Core release line。

每个分支都必须验证 Core 更新，但接口兼容时不需要人工重新移植实现。

## 自动验证

本地：

```bash
uv sync --frozen
uv run python -m driver validate
uv run python -m unittest discover -s tests -v
```

GitHub Actions workflow `Chromium port contract` 会在 push、pull request 和手工触发时运行同一组检查。手工运行时可以选择文本或 JSON 报告。

该 workflow 验证仓库级契约和 patch 完整性，不等价于完整 Chromium 编译。每个正式 milestone 分支仍应在具备 Chromium checkout 的构建 runner 上增加 GN/Autoninja 和浏览器契约测试。
