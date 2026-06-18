# 2026061804 审查报告 — 架构重构专项 Spec

## Why

Qwen 在 `d:\smart_community_management\.trae\diagnoses\2026061804.md` 中识别出项目存在典型的"大泥球"架构问题：UI 构建、业务逻辑、数据访问完全混杂在巨型页面文件中，导致代码不可测试、不可维护、不可复用。这些问题属实，但需要系统性的架构重构，无法在一次迭代内安全完成。本 spec 作为后续专项规划，明确重构目标、范围和优先级，待当前安全/正确性修复完成后择机实施。

## What Changes

- **拆分巨型页面文件（问题 #1）**：将 `GovernancePage.cpp`（1738 行）、`SystemPage.cpp`（1866 行）、`DashboardTodoMessagePage.cpp`（991 行）等巨型文件中的每个子功能拆分为独立的页面类（如 `EventPage`、`UserPage`、`WorkOrderPage` 继承自 `BasePage`），每个类拥有自己的 `loadData()`、`buildToolbar()`、`buildTable()` 等方法，目标单文件不超过 400 行。
- **提取 Service/Repository 层（问题 #2）**：将散落在各页面 lambda 中的 SQL 查询提取到业务 Service 类（如 `WorkOrderService`、`EventService`、`ResidentService`）和数据访问 Repository 类中，页面仅负责 UI 构建和事件转发。
- **重构嵌套 Lambda（问题 #3）**：将 `shared_ptr<std::function>` 自引用模式和 4-5 层嵌套 lambda 替换为页面子类的成员函数，用 Qt 信号/槽机制替代深层回调。
- **为 Model 类添加行为（问题 #6）**：在 `Models.h` 中为高频模型（`WoWorkOrder`、`GeEvent`、`CmResident` 等）添加 `fromQuery(QSqlQuery&)` 静态工厂和 `toVariantMap()` 序列化方法，配合 Service 层统一 ORM 映射。
- **统一枚举助手（问题 #7）**：消除 `Constants.h` 中 20+ 个命名空间内重复的 `label(int v)` / `color(int v)` 函数，使用 `Q_ENUM` + 元对象系统或统一模板 `EnumHelper<T>` 生成标签与颜色。

## 不在本次范围

- API Key 加密、编号生成、数据库路径、using namespace、PagesCommon.h Charts 依赖等已在 `review-2026061804-fixes` spec 中处理。

## Impact

- Affected specs: 所有涉及页面构建和 SQL 查询的既有 spec。
- Affected code:
  - [src/pages/GovernancePage.cpp](file:///d:/smart_community_management/src/pages/GovernancePage.cpp)
  - [src/pages/SystemPage.cpp](file:///d:/smart_community_management/src/pages/SystemPage.cpp)
  - [src/pages/DashboardTodoMessagePage.cpp](file:///d:/smart_community_management/src/pages/DashboardTodoMessagePage.cpp)
  - [src/pages/buildPropertySubPages.cpp](file:///d:/smart_community_management/src/pages/buildPropertySubPages.cpp)
  - [src/pages/PropertyPage.cpp](file:///d:/smart_community_management/src/pages/PropertyPage.cpp)
  - [src/pages/ServicePage.cpp](file:///d:/smart_community_management/src/pages/ServicePage.cpp)
  - [src/pages/ReportPage.cpp](file:///d:/smart_community_management/src/pages/ReportPage.cpp)
  - [src/pages/ArchivePage.cpp](file:///d:/smart_community_management/src/pages/ArchivePage.cpp)
  - [src/services/DemoDataService.cpp](file:///d:/smart_community_management/src/services/DemoDataService.cpp)
  - [src/models/Models.h](file:///d:/smart_community_management/src/models/Models.h)
  - [src/models/Constants.h](file:///d:/smart_community_management/src/models/Constants.h)
  - 新增：`src/services/*Service.h/cpp`、`src/repositories/*Repository.h/cpp`、`src/pages/subpages/*.h/cpp`

## ADDED Requirements

### Requirement: 页面子类化

The system SHALL split monolithic `createXxxPage` factory functions into dedicated page subclasses, each responsible for a single business sub-module.

#### Scenario: 打开网格事件管理页
- **WHEN** 用户导航到"网格事件管理"
- **THEN** `PageFactory` 创建 `EventPage` 实例，而非在 `GovernancePage.cpp` 中通过 `if (sub == "event")` 分支处理

### Requirement: Service/Repository 分层

The system SHALL encapsulate all SQL queries inside Service/Repository classes, and Pages SHALL NOT contain SQL strings.

#### Scenario: 工单页面加载数据
- **WHEN** 工单页面需要刷新列表
- **THEN** 页面调用 `WorkOrderService::queryWorkOrders(params)`，由 Service 内部执行参数化 SQL 并返回模型列表

### Requirement: 模型类具备 ORM 行为

The system SHALL provide `fromQuery(QSqlQuery&)` and `toVariantMap()` methods on key model structs, so that data mapping is centralized.

#### Scenario: 从查询结果构造工单对象
- **WHEN** `WorkOrderService` 执行列表查询
- **THEN** 使用 `WoWorkOrder::fromQuery(query)` 将 `QSqlQuery` 当前行转换为模型对象，而非在多处重复 `query.value(0).toInt()`

### Requirement: 枚举标签/颜色统一生成

The system SHALL eliminate repetitive `label(int v)` / `color(int v)` functions in `Constants.h` by using a common helper or Qt meta-object facilities.

#### Scenario: 新增业务状态枚举
- **WHEN** 开发者新增一个状态枚举
- **THEN** 只需定义枚举值和对应标签映射，无需再手写 switch-case

## MODIFIED Requirements

无。

## REMOVED Requirements

无。

---

## 附：Qwen 报告架构类问题评估

| # | 问题 | 验证结论 | 规划处理 |
|---|------|----------|----------|
| 1 | 巨型单文件 — God Function 泛滥 | **属实**。5 个文件超 1000 行，最大 1866 行。 | 页面子类化专项 |
| 2 | UI 层直接嵌入 SQL | **属实**。约 100+ 处 SQL 散落在 UI 代码。 | Service/Repository 分层专项 |
| 3 | 无限嵌套 Lambda | **属实**。shared_ptr function 自引用、4-5 层嵌套。 | 随页面子类化同步重构 |
| 6 | Models.h 模型类无行为 | **属实**。583 行纯数据结构。 | 模型行为化专项 |
| 7 | Constants.h 枚举模式重复 | **部分属实**。label() 大量重复。 | 统一枚举助手专项 |
