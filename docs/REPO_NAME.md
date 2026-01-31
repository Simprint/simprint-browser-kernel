# 关于仓库名称

本仓库承担 **Simprint 指纹浏览器** 定制与发布的核心角色，目录与远程仓库名均为 **simprint-browser-kernel**（“内核”表示定制与发布的核心层，与 Chromium 源码目录区分）。

## 克隆时建议使用目录名

```bash
git clone <远程地址> simprint-browser-kernel
```

## 目录名与 Chromium 的关系

- **simprint-browser-kernel**：本仓库（补丁、文档、脚本、配置）。
- **simprint-browser**：Chromium 源码目录（gclient 拉取，含 `src/`、`.gclient` 等）。

两者分离；脚本通过 `config/kernel.config` 或环境变量 `SIMPRINT_CHROMIUM_ROOT` 指向 Chromium 根目录（如 `../simprint-browser/src`）。
