# Auth 模块创建完成

## 模块结构

已成功创建 `overlay/auth` 模块，包含以下内容：

### 1. 源文件 (sources/)
- `simprint/auth/BUILD.gn` - 编译配置
- `simprint/auth/auth_manager.h/cc` - 认证管理器
- `simprint/auth/startup_auth_checker.h/cc` - 启动认证检查器
- `simprint/auth/README.md` - 使用文档

### 2. 补丁文件 (patches/)
- `001-chrome-browser-build-gn.patch` - 添加 simprint/auth 依赖
- `002-chrome-browser-main.patch` - 集成启动认证检查
- `003-eventbus-topics-h.patch` - 添加 kLaunchConfig Topic
- `004-eventbus-h.patch` - 添加 AuthInfo 结构和认证方法
- `005-eventbus-cc.patch` - 实现认证和配置处理逻辑
- `006-sync-input-capture-handler.patch` - 移除 SIMPRINT_LOG 调用
- `007-sync-input-replay.patch` - 移除 SIMPRINT_LOG 调用

### 3. 脚本文件
- `run.py` - 主运行脚本
- `scripts/deploy_resources.py` - 部署脚本
- `apply_order.txt` - 补丁应用顺序
- `README.md` - 模块说明文档

## 使用方法

### 1. 更新 driver/order.txt

在 `simprint-browser-kernel/driver/order.txt` 中添加：

```
overlay/syner
overlay/auth
```

注意：auth 必须在 syner 之后，因为它依赖 EventBus 和 console_log。

### 2. 运行 driver

```bash
cd simprint-browser-kernel
python driver/run.py apply_deploy
```

这将：
1. 按顺序应用所有模块的补丁
2. 部署所有模块的源文件到 Chromium 源码目录

### 3. 编译 Chromium

```bash
cd <chromium_src>
autoninja -C out/Default chrome
```

## 功能说明

### 启动认证检查
- 浏览器启动时检查 `--simprint-env-id` 参数
- 有参数：通过 EventBus IPC 向 Tauri 查询认证状态（5秒超时）
  - 已登录 → 继续运行
  - 未登录/超时/IPC异常 → 静默退出
- 无参数：立即静默退出

### LaunchConfig IPC 传递
- Tauri 在握手完成后通过 IPC 发送启动配置
- 包含：env_uuid、user_data_dir、proxy、kernel_version、extensions、custom_flags
- 替代原有的命令行参数传递方式

### 认证信息查询
- 提供 AuthManager 便捷接口
- 支持认证状态变化监听
- 缓存机制避免频繁 IPC 查询

## 对应的 Git 提交

此模块基于 simprint-browser 仓库的以下提交：
- `879a342` - feat(auth): 添加启动认证检查和 LaunchConfig 支持
- `9be2688` - Remove SIMPRINT_LOG calls from sync input handlers

## 验证

模块创建完成后，可以通过以下方式验证：

1. 检查文件结构：
```bash
ls -R simprint-browser-kernel/overlay/auth/
```

2. 查看补丁内容：
```bash
cat simprint-browser-kernel/overlay/auth/patches/*.patch
```

3. 测试运行：
```bash
cd simprint-browser-kernel
python overlay/auth/run.py apply_deploy
```

## 注意事项

1. **依赖顺序**：auth 模块必须在 syner 模块之后执行
2. **补丁冲突**：如果 Chromium 源码已经应用过这些修改，补丁可能会失败
3. **清理**：如需重新应用，先恢复 Chromium 源码到干净状态
