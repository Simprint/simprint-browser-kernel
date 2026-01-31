# Integration 接入层

与 Chromium 的接入：最少 patch + 构建胶水。不承载业务逻辑。

- **patches/**：仅接入用 patch（如 init hook、BUILD 链库）。当前为空，将来在此添加。
- **apply_order.txt**：patch 应用顺序。
- 由 driver 调用：`python integration/run.py apply|deploy|build`。
