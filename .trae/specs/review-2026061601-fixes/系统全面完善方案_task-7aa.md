# 数据完整性与筛选功能全面修复方案

## Context

全面交叉审查发现：9张数据库表无演示数据（含车辆、家庭等核心档案）、2处SQL字段名与schema不匹配导致运行时报错、12个模块缺少筛选下拉框、居民档案仅1条且关键字段为空、投诉建议页面因source=2无匹配永远为空。本计划修复所有数据缺失和筛选缺失问题。

## Task 1: 修复严重Bug（P0）

### 1.1 志愿报名INSERT字段错误
**文件**: `src/widgets/MainWindow.cpp` L2958-2960

`sv_volunteer_signup` schema只有 `activity_id, volunteer_id, signup_time, status, checkin_time, checkout_time, hours, create_time`，但代码INSERT了不存在的 `user_id`, `user_name`, `create_by`。

修复：
```cpp
DatabaseManager::instance().insert("sv_volunteer_signup", {
    {"activity_id", id}, {"volunteer_id", user.id},
    {"signup_time", QDateTime::currentDateTime()}, {"status", 0}
});
```

### 1.2 投诉建议页面永远为空
**文件**: `src/services/DemoDataService.h` initWorkOrders()

所有工单 `source = 0`，但投诉建议页面SQL用 `WHERE source = 2`。需在 initWorkOrders 末尾新增3条 `source = 2` 的投诉/建议工单：
```cpp
// 投诉工单(source=2)
db.insert("wo_work_order", {{"order_no", Utils::generateOrderNo()}, ...{"source", 2}, {"order_type", 0}, ...});
db.insert("wo_work_order", {{"order_no", Utils::generateOrderNo()}, ...{"source", 2}, {"order_type", 1}, ...});
db.insert("wo_work_order", {{"order_no", Utils::generateOrderNo()}, ...{"source", 2}, {"order_type", 0}, ...});
```

## Task 2: 居民档案数据丰富（P1）

**文件**: `src/services/DemoDataService.h` initArchiveData()

当前 cm_resident 仅有张三1条记录，且缺少 nationality/occupation/birthday/education 等显示字段。

### 2.1 补全张三字段
为现有张三记录补充：nationality="汉族", occupation="软件工程师", birthday="1990-03-15", education="本科", id_card="340104199003150000", political_status="群众"

### 2.2 新增8条居民档案
在 initArchiveData 中新增8条完整居民记录（李四、王五、赵六、孙七、周八、吴九、郑十、陈十一），每人包含全部字段。使用循环或结构体数组简化代码。

### 2.3 新增家庭关联数据
新增 `initFamilyData()` 函数：
- `cm_family`: 3个家庭（张家、李家、王家），关联house_id
- `cm_family_member`: 每家庭3-4人，关联resident_id，relation（户主/配偶/子女/父母）
- `cm_house_resident`: 将居民关联到房屋（每套房2-3人）

在 initIfEmpty() 中 initArchiveData 之后调用。

## Task 3: 车辆数据补全（P1）

**文件**: `src/services/DemoDataService.h`

新增 `initVehicleData()` 函数：
- `cm_vehicle`: 5条车辆（不同品牌/颜色/类型），关联estate_id和parking_space_id
  - 皖A·12345 比亚迪 白色 轿车
  - 皖A·23456 大众 黑色 SUV
  - 皖A·34567 丰田 银色 轿车
  - 皖A·45678 宝马 红色 SUV
  - 皖A·56789 本田 白色 MPV
- `cm_vehicle_owner`: 关联车辆和居民（vehicle_id → resident_id）

在 initIfEmpty() 中 initParkingData 之后调用（需先有车位数据再关联车辆）。

## Task 4: 月卡数据修复 + 投诉工单数据（P1-P2）

**文件**: `src/services/DemoDataService.h`

### 4.1 pm_monthly_card 补充 owner_phone
检查 initMonthlyCardData()，为每条月卡INSERT添加 `owner_phone` 字段。

### 4.2 投诉建议工单数据（已在Task1.2中处理）

## Task 5: 工单状态筛选完善

**文件**: `src/widgets/MainWindow.cpp`

### 5.1 报事报修 statusMap 扩展
L1097: `int statusMap[] = {0, 0, 1, 3, 4, 5};` → 添加 status=6(已评价) 选项
筛选下拉框新增 "已评价" 选项。

### 5.2 投诉建议 statusMap 同步扩展
L1300: 同上扩展。

## Task 6: 12个模块筛选下拉框补充

**文件**: `src/widgets/MainWindow.cpp`

每个模块在搜索框后添加QComboBox，连接到对应的load函数。模式统一：
```cpp
auto* filterCombo = new QComboBox(toolbar);
filterCombo->addItem(QStringLiteral("全部XXX"), -1);
filterCombo->addItem(QStringLiteral("选项A"), 0);
// ...
filterCombo->setMinimumWidth(120);
tbLayout->addWidget(filterCombo);
// 在load lambda中:
int filterVal = filterCombo->currentData().toInt();
if (filterVal >= 0) sql += " AND field = " + QString::number(filterVal);
// 连接信号:
connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadXxx(); });
```

| 模块 | 位置 | 筛选字段 | 选项 |
|------|------|----------|------|
| 组织管理 | L683 | org_type | 1=街道,2=社区,3=网格,4=物业,5=业委会,6=志愿,7=社会 |
| 小区管理 | L719 | status | 0=正常,1=停用 |
| 居民档案 | L820 | gender | 0=女,1=男 |
| 设施设备 | L902 | facility_type | 1=电梯,2=消防,3=监控,4=门禁,5=照明 |
| 网格管理 | L948 | status | 0=正常,1=停用 |
| 特殊群体 | L983 | group_type | 1=独居老人,2=残疾人,3=低保户,4=留守儿童 |
| 巡查计划 | L2307 | status | 0=待执行,1=进行中,2=已完成 |
| 关怀对象 | L2443 | care_level | 1=一般,2=重点,3=特殊 |
| 走访记录 | L2500 | visit_type | 1=定期走访,2=临时走访,3=电话慰问 |
| 考核管理 | L2753 | 考核周期（从kf_assessment_config的cycle字段动态获取） |
| 角色管理 | L3550 | role_domain | 从Constants.h的RoleDomain取值 |
| 数据字典 | L3612 | dict_type | 从sys_dict_type动态获取 |

## Task 7: 编译验证

每个Task完成后执行编译验证：
```powershell
cd d:\smart_community_management\build; $env:Path = "D:\Qt\Tools\CMake_64\bin;" + $env:Path; cmake --build .
```

## 涉及文件汇总

| 文件 | Task | 改动量 |
|------|------|--------|
| `src/services/DemoDataService.h` | 1,2,3,4 | +200行（投诉工单+8居民+3家庭+5车辆+月卡修复） |
| `src/widgets/MainWindow.cpp` | 1,5,6 | +150行（志愿修复+筛选完善+12个ComboBox） |
