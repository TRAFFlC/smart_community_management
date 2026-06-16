# 智慧社区管理系统 - 项目开发规范

## 核心原则

### 1. 禁止硬编码数据

**所有页面展示的数据必须来源于数据库查询，严禁在 UI 代码中硬编码演示数据。**

- 禁止在 `MainWindow.cpp` 或任何 UI 代码中使用 `struct XxxData { const char* ... }` + 数组初始化的方式填充表格
- 禁止在统计卡片中写死数值（如 `QStringLiteral("12")`、`QStringLiteral("¥126,800")`）
- 所有数据必须通过 `QSqlQuery` 从 SQLite 数据库中查询

**正确做法：**
```cpp
// 统计卡片 - 从数据库查询
QSqlQuery cntQ("SELECT COUNT(*) FROM cm_visitor WHERE date(arrive_time) = date('now')");
int todayCount = cntQ.next() ? cntQ.value(0).toInt() : 0;
statsRow->addWidget(createMiniCard("今日访客", QString::number(todayCount), "#1677ff", page));

// 表格数据 - 从数据库查询
QSqlQuery q("SELECT name, phone, host_name, arrive_time, leave_time, status FROM cm_visitor WHERE del_flag = 0");
while (q.next()) {
    table->insertRow(row);
    table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
    // ...
}
```

**错误做法：**
```cpp
// 禁止！硬编码数据
struct VisitorData { const char* name; const char* phone; ... };
VisitorData visitors[] = { {"王先生", "138****5678", ...}, ... };
for (int i = 0; i < 4; ++i) { ... }
```

### 2. 新增页面必须先建表

添加新功能页面时，必须遵循以下流程：

1. 在 `sql/schema.sql` 中创建对应的数据库表
2. 在 `src/services/DemoDataService.h` 中添加种子数据初始化函数
3. 在 `DemoDataService::initIfEmpty()` 中调用新初始化函数
4. 在 `MainWindow.cpp` 的 `getOrCreatePage()` 中使用 `QSqlQuery` 查询数据

### 3. 种子数据规范

- 种子数据统一在 `DemoDataService` 中管理
- 使用 `DatabaseManager::insert()` 方法插入
- 种子数据应覆盖各种状态和边界情况
- 数量适中（每表 5-20 条），足以演示功能

### 4. 数据库表命名规范

| 前缀 | 含义 | 示例 |
|------|------|------|
| `sys_` | 系统管理 | sys_user, sys_role |
| `cm_` | 社区/基础档案 | cm_estate, cm_visitor |
| `wo_` | 工单管理 | wo_work_order |
| `ge_` | 社区治理/事件 | ge_event, ge_opinion |
| `sv_` | 社区服务 | sv_volunteer |
| `nt_` | 通知公告 | nt_announcement |
| `oc_` | 业委会/自治 | oc_topic, oc_public_income |
| `ev_` | 评价 | ev_evaluation |
| `ai_` | 智能助手 | ai_knowledge |
| `kf_` | 考核 | kf_assessment_config |
| `pm_` | 物业管理(缴费/停车) | pm_bill, pm_monthly_card |

### 5. 通用字段规范

每张表必须包含以下审计字段：

```sql
create_by      INTEGER,          -- 创建人ID
create_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
update_by      INTEGER,
update_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
del_flag       INTEGER DEFAULT 0  -- 逻辑删除标志
```

### 6. UI 代码规范

- 搜索/筛选控件必须连接到数据刷新逻辑
- 按钮必须连接到实际功能（禁止创建无响应按钮）
- 面包屑导航必须随页面切换更新
- 表格数据必须支持动态查询和筛选
