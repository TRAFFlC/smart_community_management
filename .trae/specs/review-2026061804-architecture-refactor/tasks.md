# Tasks

- [x] Task 1: 建立 Service/Repository 基础框架（试点完成）
  - [ ] SubTask 1.1: 设计通用的 `BaseRepository` 类（可选，当前由各 Service 直接操作 DatabaseManager）
  - [x] SubTask 1.2: 建立 `EventService` 作为核心 Service 类试点
  - [x] SubTask 1.3: 将 `GovernancePage.cpp` 中事件相关 SQL 迁移到 `EventService`
  - [x] SubTask 1.4: 将 `DashboardTodoMessagePage.cpp` 中事件计数 SQL 迁移到 `EventService`
  - [ ] SubTask 1.5: 建立 `WorkOrderService` / `ResidentService` / `PropertyService` 等（待后续扩展）

- [x] Task 2: 页面子类化 — 治理域（试点完成）
  - [x] SubTask 2.1: 创建 `EventPage` 类，替代 `GovernancePage` 中 `sub == "event"` 分支
  - [ ] SubTask 2.2: 创建 `PlanPage`、`RecordPage`、`GroupPage`、`VisitPage`、`SupervisionPage`、`OpinionPage`、`AssessmentPage`
  - [x] SubTask 2.3: 更新 `PageFactory` 路由，事件分支已路由到 `EventPage`
  - [x] SubTask 2.4: 治理域事件子页面编译通过

- [ ] Task 3: 页面子类化 — 系统域
  - [ ] SubTask 3.1: 创建 `UserPage`、`RolePage`、`DictPage`、`LogPage`
  - [ ] SubTask 3.2: 更新 `PageFactory` 路由，逐步删除 `SystemPage.cpp` 中已迁移的代码
  - [ ] SubTask 3.3: 确保系统域所有子页面编译通过

- [ ] Task 4: 页面子类化 — 物业/服务/档案域
  - [ ] SubTask 4.1: 拆分 `buildPropertySubPages.cpp` 中各子页面为独立类
  - [ ] SubTask 4.2: 拆分 `ServicePage.cpp` 中志愿服务、便民服务、就业服务子页面
  - [ ] SubTask 4.3: 拆分 `ArchivePage.cpp` 中组织、小区、房屋、居民、车辆、设施等子页面

- [x] Task 5: 重构嵌套 Lambda 与回调（试点完成）
  - [x] SubTask 5.1: `EventPage` 已将 `shared_ptr<std::function>` 自引用模式替换为成员函数
  - [x] SubTask 5.2: `EventPage` 操作处理改为成员函数，无深层嵌套 lambda
  - [ ] SubTask 5.3: 其他页面（SystemPage、PropertyPage 等）仍保留原 lambda 模式，待后续子类化时同步重构

- [x] Task 6: 模型类行为化（试点完成）
  - [x] SubTask 6.1: 为 `WoWorkOrder`、`GeEvent`、`CmResident`、`CmHouse`、`CmEstate` 添加 `fromQuery(QSqlQuery&)`
  - [x] SubTask 6.2: 为上述模型添加 `toVariantMap()` 序列化方法
  - [x] SubTask 6.3: `EventService` 已使用 `GeEvent::fromQuery`

- [x] Task 7: 统一枚举助手（已完成）
  - [x] SubTask 7.1: 设计 `EnumHelper<T>` 模板
  - [x] SubTask 7.2: 将 `Constants.h` 中重复的 switch-case 替换为统一助手，保留原函数作为转发
  - [x] SubTask 7.3: 既有引用点编译通过

# Task Dependencies

- Task 1（Service/Repository 框架）是 Task 2/3/4 的前提。
- Task 2/3/4 可并行按业务域推进。
- Task 5 与 Task 2/3/4 同步进行，在子类化时消除 lambda 嵌套。
- Task 6 可与 Task 1 并行启动，但需在 Service 中落地。
- Task 7 相对独立，已完成。
