# Account 模块

## 功能概述

Account 模块实现了账号自动导入功能，将环境绑定的账号自动导入到浏览器的密码管理器中。

## 主要功能

1. **账号数据接收**：通过 EventBus 的 LaunchConfig 消息接收账号数据
2. **密码导入**：使用 Chromium 的 PasswordImporter API 将账号导入到密码管理器
3. **自动打开标签页**：导入成功后自动打开账号对应的网站

## 实现流程

1. 从 LaunchConfig 消息解析账号数据（url, username, password）
2. 构造 CSVPassword 列表
3. 在 UI 线程创建 SavedPasswordsPresenter 和 PasswordImporter
4. 导入密码到 Profile Store
5. 导入成功后使用 chrome::AddTabAt 打开标签页

## 技术细节

### 数据流

```
前端 → Tauri (AccountConfig[]) → EventBus IPC (JSON) → 浏览器 (CSVPassword[]) → PasswordImporter
```

### 关键 API

- `password_manager::CSVPassword`：表示密码条目的数据结构
- `password_manager::PasswordImporter`：密码导入器
- `password_manager::SavedPasswordsPresenter`：密码管理器展示层
- `chrome::AddTabAt`：打开新标签页

### 依赖的头文件

- `components/password_manager/core/browser/import/csv_password.h`
- `components/password_manager/core/browser/import/password_importer.h`
- `components/password_manager/core/browser/ui/saved_passwords_presenter.h`
- `chrome/browser/password_manager/account_password_store_factory.h`
- `chrome/browser/password_manager/profile_password_store_factory.h`
- `chrome/browser/affiliations/affiliation_service_factory.h`

## 文件结构

```
account/
├── patches/
│   └── 001-eventbus-account-import.patch  # EventBus 账号导入补丁
├── apply_order.txt                         # 补丁应用顺序
├── run.py                                  # 模块执行脚本
└── README.md                               # 本文档
```

## 使用方式

### 前端调用

```typescript
await invoke('launch_environment', {
  // ... 其他参数
  accounts: [
    {
      account: 'user@example.com',
      password_encrypted: 'base64_encrypted_password',
      platform_url: 'https://example.com',
      platform_name: 'Example Site'
    }
  ]
});
```

### Tauri 处理

Tauri 会自动解密 `password_encrypted` 字段，并通过 EventBus 传递给浏览器。

### 浏览器处理

浏览器在接收到 LaunchConfig 消息后，会自动导入账号到密码管理器，并打开对应的网站标签页。

## 注意事项

1. 账号密码使用 AES-256-GCM 加密传输
2. 导入操作在 UI 线程执行，避免阻塞
3. 导入失败不会影响浏览器正常启动
4. 只有导入成功才会打开标签页
