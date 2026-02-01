# Integration 接入层

**目的**：在 Chromium 中通过**最少 patch** 建立**可接入的入口**，不承载业务逻辑。这样 **projects** 中的代码可以直接接入 Chromium，对各项功能做出修改或扩展，而非通过大量 patch 改 Chromium 源码——复杂功能单靠 patch 难以实现且难维护。

- **patches/**：仅接入用 patch（建立可接入的入口）。当前为空，将来在此添加。
- **apply_order.txt**：patch 应用顺序。
- 由 driver 调用：`python integration/run.py apply|deploy|build`。
