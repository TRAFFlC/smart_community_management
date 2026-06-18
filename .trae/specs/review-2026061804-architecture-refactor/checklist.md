# Checklist

## Service/Repository 分层
- [ ] 存在通用的 `BaseRepository` 类
- [x] `EventService` 已创建并封装事件相关 SQL
- [x] `GovernancePage.cpp` 中事件 SQL 已迁移到 `EventService`
- [x] `DashboardTodoMessagePage.cpp` 中事件计数 SQL 已迁移到 `EventService`
- [ ] `WorkOrderService` / `ResidentService` / `PropertyService` 等核心 Service 已创建
- [ ] 页面代码中不再出现未封装 SQL（当前仅 EventPage 达成）

## 页面子类化
- [x] `EventPage` 替代 `GovernancePage` 事件分支
- [ ] `PlanPage` / `RecordPage` / `GroupPage` / `VisitPage` / `SupervisionPage` / `OpinionPage` / `AssessmentPage`
- [ ] `UserPage` / `RolePage` / `DictPage` / `LogPage` 替代 `SystemPage` 对应分支
- [ ] `PropertyPage` 子页面拆分完成
- [ ] `PageFactory` 路由完全更新
- [ ] 原巨型文件中对应代码已删除

## Lambda 重构
- [x] `EventPage` 无 `std::make_shared<std::function<void()>>` 自引用模式
- [x] `EventPage` 深层嵌套 lambda 已拆分为成员函数
- [ ] 其他页面仍保留原 lambda 模式，待后续子类化时重构

## 模型行为化
- [x] `WoWorkOrder` / `GeEvent` / `CmResident` / `CmHouse` / `CmEstate` 具备 `fromQuery(QSqlQuery&)`
- [x] 上述模型具备 `toVariantMap()`
- [x] `EventService` 使用 `GeEvent::fromQuery`
- [ ] 其他 Service 使用模型工厂方法

## 枚举助手统一
- [x] `Constants.h` 已添加 `EnumHelper<T>`
- [x] 重复 switch-case 已替换为统一助手（原函数作为转发保留）
- [x] 既有引用点编译通过

## 编译与回归
- [x] 项目完整编译通过
- [ ] 登录与核心页面功能正常（建议运行时手动验证）
- [ ] 数据库初始化与种子数据正常（建议运行时手动验证）
