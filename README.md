# Simprint Browser Kernel

`main` is the documentation-only landing branch for the Simprint Chromium
kernel project. It intentionally contains no kernel implementation, patch
pipeline, or Chromium source tree.

`main` 是 Simprint Chromium 内核项目的纯文档入口分支。内核实现、版本适配
和 Chromium 源码不会放在此分支。

## Kernel branches / 内核分支

| Branch | Chromium baseline | Purpose |
| --- | --- | --- |
| [`simprint/m144`](https://github.com/Simprint/simprint-browser-kernel/tree/simprint/m144) | `144.0.7559.118` | Chromium 144 integration and migration branch |

Each supported Chromium milestone uses a Git branch named `simprint/mXXX`.
Branches own the complete version-specific integration state; milestones are
not copied into version directories on `main`.

每个受支持的 Chromium 大版本使用一个 `simprint/mXXX` 分支。每个分支负责该
版本完整的接入状态，不在 `main` 中建立重复的版本目录。

## Target architecture / 目标架构

```text
Chromium upstream tag
        │
        ▼
simprint/mXXX              Simprint Core
        │                       │
        ├── integration commits ├── stable versioned API
        ├── kernel adapter      └── independently released
        └── buildable source state
```

The milestone branch itself is the build input. A build must not depend on
replaying a repository-wide patch queue every time. Shared product behavior
belongs in Simprint Core; Chromium-version-specific integration belongs in the
milestone branch and its adapter.

版本分支本身应当成为构建输入，不应在每次构建时重新执行整个 patch 队列。通用
产品能力进入 Simprint Core；依赖 Chromium 版本的接入代码保留在对应版本分支
及其 Adapter 中。

## Current migration status / 当前迁移状态

`simprint/m144` currently preserves the legacy overlay and patch implementation
as the migration starting point. It is not yet a complete Chromium source fork.
The migration is complete only when the branch represents an integrated,
directly buildable Chromium tree and the legacy patch queue is no longer the
source of truth.

`simprint/m144` 当前仍保留旧 overlay/patch 实现，作为迁移起点；它还不是完整的
Chromium 源码分支。只有当该分支成为可直接构建的集成源码，并且旧 patch 队列
不再是真实来源时，迁移才算完成。
