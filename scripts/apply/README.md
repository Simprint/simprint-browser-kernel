# 应用补丁（apply）

按 `patches/APPLY_ORDER` 中顺序，在 Chromium 源码根目录应用全部补丁。

## 用法

在内核仓库根目录执行（Chromium 路径由 `config/kernel.config` 或环境变量 `SIMPRINT_CHROMIUM_ROOT` 指定）：

```bash
./scripts/apply/apply-all.sh
# 或 Windows: scripts\apply\apply-all.bat
```

脚本会跳过 `APPLY_ORDER` 中以 `#` 开头的行和空行，依次对每个 patch 执行 `patch -p1`；若某 patch 失败会中止并提示。

## 扩展

- 需「只应用某域」或「跳过某域」时，可在脚本中增加参数（如 `--only branding`、`--skip fingerprint`），或维护多份 APPLY_ORDER 由参数选择。
