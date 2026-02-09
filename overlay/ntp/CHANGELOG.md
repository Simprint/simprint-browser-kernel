# NTP 修改日志

## 2026-02-10 - 简化 NTP，仅保留背景动画

### 移除的功能
- ❌ 左下角环境统计面板（指纹保护、今日访问、在线时长）
- ❌ 右下角 IP 卡片（IP 地址、位置、时区、ISP）
- ❌ 右下角指纹卡片（Canvas、WebGL 等防护状态）
- ❌ 响应式布局代码
- ❌ Customizer 按钮位置偏移和样式修改

### 保留的功能
- ✅ 背景 Canvas 动画（六边形网格 + 流动光带）
- ✅ simprint_ip 后端功能（保留但前端不再调用）

### 修改的文件

#### 删除的文件
- `patches/004-app-css.patch` - customizer 按钮位置偏移
- `patches/008-customize-buttons-css.patch` - customizer 按钮样式和延迟显示
- `resources/simprint_ip_proxy.ts` - IP 检测前端代理

#### 修改的文件
- `resources/simprint_ntp.ts` - 从 952 行简化到 120 行，仅保留背景动画
- `patches/005-new-tab-page-gni.patch` - 移除 simprint_ip_proxy.ts 引用
- `apply_order.txt` - 移除补丁 004 和 008
- `README.md` - 更新文档说明

### 代码统计
- **删除代码**: ~830 行（simprint_ntp.ts 中的面板代码）
- **删除补丁**: 2 个（004, 008）
- **保留补丁**: 10 个
- **最终 simprint_ntp.ts**: 120 行（纯背景动画）

### 使用方式
```bash
# 应用修改
cd simprint-browser-kernel
uv run python -m driver apply --project ntp
uv run python -m driver deploy --project ntp

# 或一次性完成
uv run python -m driver apply-and-prepare --project ntp
```

### 效果
新标签页现在只显示：
- 干净的背景动画（六边形网格 + 流动光带）
- Chromium 原生的搜索框和快捷方式
- Customizer 按钮恢复到右下角默认位置
