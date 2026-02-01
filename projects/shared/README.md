# Shared 公共库区（预留）

用于**跨内核版本复用**的公共能力：与指纹浏览器软件通讯、认证、同步器、RPA 等。这些代码通过 **integration** 层在 Chromium 中建立的接入点接入 Chromium，而非通过 patch 改 Chromium 源码。接入方式多样，如静态/动态库、hook、替换实现等。

当前仅预留目录与统一入口 `run.py`；**不实现**编译逻辑。实现时可在本目录下按能力分子目录（如 auth、comm、sync、rpa），在 build 阶段产出到例如 `out/`。driver 的 order.txt 中建议将 shared 放在 fingerprint 等之前以便先构建公共库。
