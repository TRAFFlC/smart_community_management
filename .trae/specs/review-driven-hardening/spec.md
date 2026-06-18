# 审查报告驱动的安全加固 Spec

## Why

Qwen 在 `d:\smart_community_management\.trae\diagnoses\2026061802.md` 中对项目进行了全面审查。经过逐条代码验证，其中部分分析是准确的且属于真实的安全/正确性问题，应当在当前阶段立即修复，而非留到后期重构。本 spec 仅聚焦"经验证正确且需要立刻完善"的问题，排除架构重构、工程优化、视觉打磨等非紧急事项。

## What Changes

- **修复 AIService 上下文查询的权限过滤缺失**：`queryWorkOrderContext`、`queryEventContext`、`queryResidentContext`、`queryOverviewContext` 均未使用 `user` 参数进行数据权限过滤，普通居民通过 AI 问答可看到全平台工单统计、居民总数等敏感数据。需按 `roleDomain` / `dataScope` 限制查询范围。
- **升级密码哈希算法**：`Utils::hashPassword` 当前为 `SHA256(password + "smart_community_salt")` 裸哈希 + 固定盐，改为 PBKDF2-SHA512 + 每用户随机盐 + 多轮迭代，输出格式 `salt:hash`。`verifyPassword` 需兼容旧哈希（无 `:` 分隔符时回退到旧算法），避免现有数据库用户无法登录。
- **加固 DatabaseManager 的 SQL 拼接**：`count(table, where)` 中 `where` 参数直接拼入 SQL 字符串，`insert(table, data)` 中列名来自 `QVariantMap` 的 key 直接拼入。增加表名白名单校验和列名合法性校验（仅允许字母/数字/下划线），防御潜在的 SQL 注入。

## Impact

- Affected specs: 无直接关联的既有 spec（本 spec 为独立的安全加固）
- Affected code:
  - [src/services/AIService.h](file:///d:/smart_community_management/src/services/AIService.h) — 4 个上下文查询方法
  - [src/utils/Utils.h](file:///d:/smart_community_management/src/utils/Utils.h) — `hashPassword` / `verifyPassword`
  - [src/database/DatabaseManager.h](file:///d:/smart_community_management/src/database/DatabaseManager.h) — `insert` / `count` / `update`
  - 间接影响：所有调用 `Utils::hashPassword` 的位置（DemoDataService、SystemPage、MainWindow 用户管理、ChangePasswordDialog）无需改动，因为接口签名不变。

## ADDED Requirements

### Requirement: AIService 上下文查询的数据权限过滤

The system SHALL filter AI context queries by the requesting user's role domain and data scope, so that residents cannot retrieve platform-wide statistics or other users' business data through AI问答.

#### Scenario: 居民通过 AI 查询工单信息
- **WHEN** 一个 `roleDomain` 为居民域的用户通过 AI 助手提问
- **THEN** AI 上下文构建时，工单查询应仅返回该用户本人提交的工单（`create_by = user.id`），不返回全平台工单总数和最近工单列表

#### Scenario: 居民通过 AI 查询居民/小区概况
- **WHEN** 一个居民用户通过 AI 助手提问涉及"居民""小区"等关键词
- **THEN** 上下文中的居民总数、小区数量、房屋总数等平台级聚合数据不应对居民可见，应替换为该用户所在小区/楼栋的局部信息或返回权限提示

#### Scenario: 工作人员通过 AI 查询
- **WHEN** 一个工作人员（物业/社区/街道/平台）通过 AI 助手提问
- **THEN** 上下文查询应按其 `dataScope` 限制范围（如物业仅看本小区、社区仅看本社区、街道看下属社区汇总）

### Requirement: 密码哈希算法升级

The system SHALL store user passwords using PBKDF2-SHA512 with a per-user random salt and at least 10000 iterations, output in `salt:hash` format.

#### Scenario: 新用户创建或密码重置
- **WHEN** 系统通过 `Utils::hashPassword(password)` 生成密码哈希
- **THEN** 返回值应为 `salt:hash` 格式，其中 salt 为 16 字节随机数的十六进制，hash 为 PBKDF2-SHA512 迭代不少于 10000 轮的结果

#### Scenario: 验证旧格式密码（兼容）
- **WHEN** `Utils::verifyPassword(password, storedHash)` 被调用且 `storedHash` 不包含 `:` 分隔符
- **THEN** 系统应回退到旧的 SHA256 + 固定盐算法进行验证，确保现有数据库用户仍可登录

#### Scenario: 验证新格式密码
- **WHEN** `Utils::verifyPassword(password, storedHash)` 被调用且 `storedHash` 为 `salt:hash` 格式
- **THEN** 系统应解析 salt，用相同参数重新计算 PBKDF2-SHA512 并与 storedHash 比对

### Requirement: DatabaseManager SQL 拼接加固

The system SHALL validate table names and column names against a whitelist/character policy before interpolating them into SQL strings, to prevent SQL injection through malicious keys.

#### Scenario: insert/update 接收合法列名
- **WHEN** `DatabaseManager::insert(table, data)` 或 `update(table, id, data)` 被调用
- **THEN** 系统应校验 `table` 在已知表名白名单内，且 `data` 的每个 key 仅包含字母、数字、下划线；校验失败时打印警告并返回失败

#### Scenario: count 接收 where 子句
- **WHEN** `DatabaseManager::count(table, where, params)` 被调用
- **THEN** 系统应校验 `table` 在白名单内（`where` 子句因需支持复杂条件，保持参数化绑定方式，但在文档/注释中明确要求调用方不得拼接用户输入）

## MODIFIED Requirements

### Requirement: Utils::hashPassword / verifyPassword 接口

接口签名保持不变（`QString hashPassword(const QString&)` / `bool verifyPassword(const QString&, const QString&)`），但内部实现替换为 PBKDF2-SHA512 + 随机盐。所有现有调用方无需修改。

## REMOVED Requirements

无。

---

## 附：Qwen 报告条目评估结论

### 经验证正确且需要立即完善（本 spec 范围）

| # | 问题 | 验证结论 |
|---|------|----------|
| 9 | AIService 上下文查询无权限过滤 | **正确**。`queryWorkOrderContext`/`queryEventContext`/`queryResidentContext`/`queryOverviewContext` 均未使用 `user` 参数过滤，居民可看全平台数据。真实安全漏洞。 |
| 2 | 密码安全 SHA256 + 固定盐 | **正确**。`Utils.h#L13-L18` 确为裸 SHA256 + `"smart_community_salt"`。虽注释标注"演示用"，但属于真实安全问题，且改动可控（接口不变）。 |
| 3 | SQL 注入风险 | **部分正确**。`count()` 的 `where` 直接拼接，`insert()` 列名来自 map key 拼接。当前调用方均为硬编码，实际风险低，但应做防御性加固。 |

### 经验证正确但不需要立即完善（排除出本 spec）

| # | 问题 | 排除理由 |
|---|------|----------|
| 1 | MainWindow God Class 7000+ 行 | 架构重构，工作量大，属于后期专项任务，非紧急安全/正确性问题。 |
| 5 | 70 行重复 #include | 工程优化，影响编译时间但不影响功能，非紧急。 |
| 6 | enum 而非 enum class | 代码质量问题，改为 enum class 涉及全项目大量调用点修改，风险/收益不匹配。 |
| 7 | DemoDataService 1622 行单体 | 工程优化，拆分为 .h/.cpp 不影响功能，非紧急。 |
| 8 | eventFilter 万能分发器 | 设计问题，当前可正常工作，重构风险较高，非紧急。 |

### 经验证分析不准确（排除出本 spec）

| # | 问题 | 排除理由 |
|---|------|----------|
| 4 | 资源泄漏 / 悬空指针 | **分析不准确**。`m_pageCache` 中的页面均为 `m_contentStack` 的子 QObject，MainWindow 析构时 Qt 父子机制会自动 delete 所有子对象，不存在泄漏。`refreshCurrentPage` 中 `deleteLater()` 是 Qt 推荐的安全删除方式，不会产生悬空指针。 |

### 用户明确暂不处理（排除出本 spec）

| # | 问题 | 排除理由 |
|---|------|----------|
| 10 | 登录表单硬编码 admin/admin123 | 用户明确说明为开发期后门，"暂时不管"。 |

### 需求/视觉/产品层面（排除出本 spec）

需求覆盖度、角色差异化、视觉细节等问题属于产品/设计层面，非代码安全/正确性问题，不在本安全加固 spec 范围内。
