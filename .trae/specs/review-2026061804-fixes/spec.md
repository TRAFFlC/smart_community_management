# 2026061804 审查报告问题修复 Spec

## Why

Qwen 在 `d:\smart_community_management\.trae\diagnoses\2026061804.md` 中对项目进行了全面审查，指出架构、代码质量、安全、正确性等多类问题。经逐条代码验证，其中安全与正确性问题真实存在且可在当前阶段独立修复；架构重构类问题属实，但属于大型专项工程，需单独规划。本 spec 聚焦**经验证属实、可在本次迭代内安全落地**的问题，避免一次引入大规模重构风险。

## What Changes

- **修复 API Key 明文存储（问题 #8）**：`AIService` 将 `ai_api_key` 以明文写入 `sys_config` 表，任何能访问 `.db` 文件的人均可提取。改为使用简易对称加密（AES-256-CBC 或 Qt 的 `QSslCipher`/`QCA` 不可用时的 XOR+固定密钥兜底）存储，读取时解密。保留旧明文配置的自动迁移。
- **修复编号生成并发冲突（问题 #9）**：`Utils::generateOrderNo` / `generateEventNo` / `generateServiceOrderNo` 使用秒级时间戳 + 4 位随机数，在并发场景下有碰撞概率。改为毫秒级时间戳 + 6 位随机数 + 自增序列号，显著降低碰撞概率。
- **修复数据库路径硬编码（问题 #10）**：`main.cpp` 中 `db.initialize("smart_community.db")` 使用当前工作目录，易导致多实例冲突或数据丢失。改为使用 `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)`，并将数据库文件命名为 `smart_community.db`。
- **移除 `using namespace UiKit` 全局污染（问题 #4）**：将 `MainWindow.cpp` 及所有 `Page.cpp` 顶层的 `using namespace UiKit;` 改为按需 `using UiKit::xxx;` 或直接使用 `UiKit::` 前缀，减少命名空间污染。
- **拆分 `PagesCommon.h` 中的 Qt Charts 依赖（问题 #5 的一部分）**：将 `PagesCommon.h` 中所有 Qt Charts 相关头文件移到实际使用它们的 `ReportPage.cpp` 内部，其他页面不再强制包含 Charts 头，降低编译依赖。

## 不在本次范围的已确认问题

以下问题经核实属实，但属于大型架构重构，改动面广、风险高，建议作为后续专项迭代处理：

| 问题 | 排除理由 |
|------|----------|
| #1 巨型单文件（God Function） | 拆分 `GovernancePage.cpp`/`SystemPage.cpp`/`DemoDataService.cpp` 等需要重新设计页面类继承体系，属于架构专项。 |
| #2 UI 层直接嵌入 SQL | 全面提取 Service/Repository 层涉及所有页面，与 #1 强相关，应随页面拆分同步进行。 |
| #3 无限嵌套 Lambda | 重构为类成员函数需配合 #1 的页面子类化。 |
| #6 Models.h 模型类无行为 | 为所有 30+ 个模型添加 `fromQuery/toVariantMap` 工作量大，建议配合 #2 的 Service 层重构统一进行。 |
| #7 Constants.h 枚举模式重复 | 已改进为 `enum Type : int`，`label()` 重复虽可进一步用模板/宏消除，但属于代码风格优化，非紧急。 |

## Impact

- Affected specs: 无直接关联既有 spec，本 spec 为独立的审查修复。
- Affected code:
  - [src/services/AIService.h](file:///d:/smart_community_management/src/services/AIService.h) — API Key 加解密读写
  - [src/utils/Utils.h](file:///d:/smart_community_management/src/utils/Utils.h) — 编号生成函数
  - [src/main.cpp](file:///d:/smart_community_management/src/main.cpp) — 数据库路径
  - [src/widgets/MainWindow.cpp](file:///d:/smart_community_management/src/widgets/MainWindow.cpp) — 移除 `using namespace UiKit`
  - [src/pages/*.cpp](file:///d:/smart_community_management/src/pages/) — 移除 `using namespace UiKit`
  - [src/PagesCommon.h](file:///d:/smart_community_management/src/PagesCommon.h) — 移除 Charts 头
  - [src/pages/ReportPage.cpp](file:///d:/smart_community_management/src/pages/ReportPage.cpp) — 新增 Charts 头包含

## ADDED Requirements

### Requirement: API Key 加密存储

The system SHALL NOT store AI API keys in plaintext in the SQLite database. Keys shall be encrypted before persistence and decrypted when loaded, with automatic migration of existing plaintext keys.

#### Scenario: 首次保存 API Key
- **WHEN** 用户在系统设置中输入并保存 AI API Key
- **THEN** `AIService::saveConfig` 在写入 `sys_config` 前对 Key 进行加密，数据库中保存的是密文

#### Scenario: 启动时加载 API Key
- **WHEN** 应用启动，`AIService::loadConfig` 读取 `sys_config`
- **THEN** 若读取到密文则解密；若读取到旧版明文则自动加密覆盖；最终内存中的 `m_apiKey` 为明文以供网络请求使用

### Requirement: 编号生成降低碰撞概率

The system SHALL generate order/event/service numbers using a combination of millisecond timestamp, random component, and an atomic sequence, such that concurrent generation within the same millisecond produces distinct values with high probability.

#### Scenario: 并发创建工单
- **WHEN** 多个工单在短时间内连续创建
- **THEN** 每个工单编号唯一，不再出现 `WO202606181200001234` 这种仅秒级时间戳 + 4 位随机数的格式

### Requirement: 数据库路径使用标准可写目录

The system SHALL initialize the SQLite database under the platform-appropriate writable application data directory instead of the current working directory.

#### Scenario: 应用首次启动
- **WHEN** `main.cpp` 调用 `DatabaseManager::initialize`
- **THEN** 数据库文件路径为 `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)/smart_community.db`

### Requirement: 命名空间按需引入

The system SHALL avoid top-level `using namespace UiKit;` in `.cpp` files. Helper functions from UiKit shall be introduced via explicit `UiKit::` qualification or targeted `using` declarations.

#### Scenario: 编译页面代码
- **WHEN** 编译 `MainWindow.cpp` 及各 `Page.cpp`
- **THEN** 不存在文件顶层的 `using namespace UiKit;`，且 `UiKit` 工具函数仍正常可用

### Requirement: 减少 PagesCommon.h 依赖

The system SHALL move Qt Charts-specific includes from the common precompiled-style header into the only consumer translation unit.

#### Scenario: 编译非图表页面
- **WHEN** 编译不展示图表的页面（如 `SystemPage.cpp`）
- **THEN** 不再强制包含 Qt Charts 头文件

## MODIFIED Requirements

无。

## REMOVED Requirements

无。

---

## 附：Qwen 2026061804 报告条目评估结论

| # | 问题 | 验证结论 | 本次处理 |
|---|------|----------|----------|
| 1 | 巨型单文件 — God Function 泛滥 | **属实**。`GovernancePage.cpp` 1738 行、`SystemPage.cpp` 1866 行、`DemoDataService.cpp` 1618 行、`DashboardTodoMessagePage.cpp` 991 行。 | 不处理，架构专项 |
| 2 | UI 层直接嵌入 SQL | **属实**。`GovernancePage.cpp:93`、`DashboardTodoMessagePage.cpp:65` 等均有 SQL。 | 不处理，随 #1 专项重构 |
| 3 | 无限嵌套 Lambda / shared_ptr 自引用 | **属实**。`GovernancePage.cpp:87-89` 等存在 4-5 层嵌套 lambda。 | 不处理，随 #1 专项重构 |
| 4 | `using namespace UiKit` 全局污染 | **属实**。多个 `.cpp` 文件顶部存在。 | **本次修复** |
| 5 | `PagesCommon.h` 巨型预编译头 | **部分属实**。包含 62 个头，其中 Qt Charts 仅 `ReportPage.cpp` 使用。 | **部分修复**：移除 Charts 头 |
| 6 | `Models.h` 模型类无行为 | **属实**。583 行纯数据结构，无 `fromQuery/toVariantMap` 等方法。 | 不处理，配合 #2 专项 |
| 7 | `Constants.h` 枚举模式重复冗余 | **部分属实**。已改为 `enum Type : int`，但 `label()` 重复。 | 不处理，低优先级风格优化 |
| 8 | 密码/密钥明文存储 | **属实**。`AIService.h:165` 将 `ai_api_key` 明文存入数据库。 | **本次修复** |
| 9 | 编号生成有并发冲突风险 | **属实**。`Utils.h` 秒级时间戳 + 4 位随机数。 | **本次修复** |
| 10 | 数据库路径硬编码为相对路径 | **属实**。`main.cpp:59` 使用当前工作目录。 | **本次修复** |
