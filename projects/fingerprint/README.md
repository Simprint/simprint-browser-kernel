# Fingerprint 单元

指纹相关定制（预留）：canvas、webgl、navigator 等。本单元为**业务代码**，通过 **integration** 建立的接入点**接入** Chromium，从而修改或扩展上述功能。接入方式多样，如静态库、hook 某个对象、替换实现等；实现方式为代码 + 构建产物，而非通过 patch 直接改 Chromium 源码。统一入口为 `run.py`，由 driver 按 order.txt 调用。
