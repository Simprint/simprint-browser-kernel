<div align="center">
  <h1>Simprint Browser Kernel</h1>
  <p>面向 Simprint Chromium 构建的补丁流水线、overlay 模块与发布准备层。</p>
  <p>
    <img alt="Python 3.11+" src="https://img.shields.io/badge/python-3.11%2B-3776ab?style=flat-square&labelColor=0f172a" />
    <img alt="Package Manager uv" src="https://img.shields.io/badge/package%20manager-uv-22c55e?style=flat-square&labelColor=0f172a" />
    <img alt="Base Chromium" src="https://img.shields.io/badge/base-Chromium-2563eb?style=flat-square&labelColor=0f172a" />
  </p>
  <p>
    <a href="./README.md">English</a> | <strong>简体中文</strong>
  </p>
</div>

---

## Introduction

Simprint Browser Kernel 是 Simprint 用来定制 Chromium 的浏览器内核层。这个仓库本身不包含 Chromium 源码，而是提供补丁执行流程、overlay 模块、资源部署逻辑以及发布前准备步骤，并将这些变更应用到独立维护的 Chromium 源码树上。

这个仓库的存在，是为了把面向 Chromium 的定制逻辑从桌面客户端和运行时仓库中拆出来。通过将品牌化、补丁编排以及浏览器侧功能改动收敛到一个独立仓库中，Simprint 可以更明确地演进浏览器内核层，也更容易在后续升级上游 Chromium 时保持边界清晰。

## Why Simprint Browser Kernel?

在 Chromium 之上维护产品，通常会因为补丁、脚本和零散改动分布在多个位置而迅速变得难以维护。这会让升级成本变高、审查过程脆弱，也会让构建行为越来越不可复现。

Simprint Browser Kernel 的目标，就是把这一层整理得更明确。它把 Chromium 视为外部源码依赖，保留一套清晰的浏览器定制路径，并通过一个轻量的 driver 流程按固定顺序执行各类 overlay。

## Features

- **外部 Chromium 工作流**：Chromium 源码不放在本仓库中，而是将 Simprint 相关变更应用到独立维护的源码树。
- **基于 driver 的流水线**：通过 `python -m driver` 执行 `apply`、`deploy`、`build` 以及组合准备流程，并按固定顺序运行。
- **Overlay 模块化组织**：将浏览器定制拆分为 `branding`、`ntp`、`syner`、`auth`、`fingerprint`、`review`、`account`、`proxy`、`cookie` 等单元。
- **补丁与资源部署**：既支持对源码打补丁，也支持复制资源、同步构建参数等后续部署工作。
- **可模板化的定制方式**：通过仓库内的脚本和配置文件管理浏览器品牌与定制逻辑，而不是依赖零散的手工修改。
- **结构化定制流程**：通过明确的 driver 与 overlay 边界组织浏览器内核定制逻辑。

## Quick Start

### Prerequisites

- Python 3.11+
- `uv`
- 一个在本仓库之外单独维护的 Chromium 源码树

### Run locally

```bash
uv sync
cp driver/driver.config.example driver/driver.config
uv run python -m driver apply-and-prepare
```

完成准备后，请在外部 Chromium 源码树中按你现有的 `gn` 和 `autoninja` 工作流继续构建。

## Status

Simprint Browser Kernel 目前正在作为 Simprint 整体开源重构工作的一部分持续整理。

这个仓库已经在用于管理 Simprint 面向 Chromium 的定制逻辑，但围绕目录结构、文档和发布约定的内容，仍在继续收敛，以便后续支持长期的外部协作。

## Contributing

欢迎提交 Issue 和 Pull Request。

当前较有价值的贡献方向包括：

- 补丁组织方式与升级安全性改进
- 构建与发布流程整理
- Overlay 模块文档补充
- Chromium 升级验证与回归检查
- 降低浏览器内核维护成本的工具化能力

## License

本项目采用 GNU Affero General Public License v3.0 (AGPLv3) 进行许可。

如果你希望在不履行 AGPLv3 义务的前提下使用 Simprint Browser Kernel，包括分发修改版本或以闭源服务形式提供修改版本，请联系获取商业许可。
