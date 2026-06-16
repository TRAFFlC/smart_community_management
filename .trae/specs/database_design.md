# 智慧社区管理平台 —— 数据库设计草案

## 1. 文档目标

本文档定义《智慧社区管理平台》核心数据库设计，包括：

- 核心 ER 关系总览
- 主要表字段建议（含字段类型、约束说明）
- 字典与状态码规范
- 索引与性能建议

技术选型：MySQL 8.0+，字符集 utf8mb4，排序规则 utf8mb4_general_ci。

---

## 2. 核心 ER 关系总览

### 2.1 实体关系图（文字描述）

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│  sys_user    │───>│ sys_user_   │<───│  sys_role    │
│  (用户)      │    │ role(关联)  │    │  (角色)      │
└──────┬───────┘    └─────────────┘    └──────┬──────┘
       │                                      │
       │ N:1                                  │ N:1
       ▼                                      ▼
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│ sys_org      │───>│ sys_role_   │<───│ sys_menu     │
│ (组织)       │    │ menu(关联)  │    │ (菜单/权限)  │
└──────┬───────┘    └─────────────┘    └─────────────┘
       │
       │ 1:N (街道→社区→小区)
       ▼
┌─────────────────────────────────────────────────────────┐
│                    基础档案域                             │
│                                                          │
│  cm_estate(小区) ──1:N──> cm_building(楼栋)              │
│       ──1:N──> cm_unit(单元) ──1:N──> cm_house(房屋)     │
│                                                          │
│  cm_house <──M:N──> cm_resident(居民)                    │
│       通过 cm_house_resident(人房关系表)                   │
│                                                          │
│  cm_family(家庭) ──1:N──> cm_resident                    │
│                                                          │
│  cm_vehicle(车辆) <──M:N──> cm_resident                   │
│       通过 cm_vehicle_owner(车辆-车主表)                   │
│                                                          │
│  cm_parking_space(车位) ──N:1──> cm_estate               │
│                                                          │
│  cm_facility(设施设备) ──N:1──> cm_estate                │
│                                                          │
│  cm_grid(网格) ──N:1──> cm_community(社区)               │
│                                                          │
│  cm_special_group(特殊群体) ──1:N──> cm_resident          │
│                                                          │
│  cm_property_company(物业公司) ──1:N──> cm_estate        │
│                                                          │
│  cm_owner_committee(业委会) ──N:1──> cm_estate           │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                    业务运营域                             │
│                                                          │
│  wo_work_order(工单) ──N:1──> cm_house                   │
│       ──N:1──> sys_user(报修人)                          │
│       ──N:1──> sys_user(处理人)                          │
│                                                          │
│  ge_event(治理事件) ──N:1──> cm_grid                     │
│       ──N:1──> sys_user(上报人)                          │
│       ──N:1──> sys_user(承办人)                          │
│                                                          │
│  ge_inspection(巡查记录) ──N:1──> cm_grid                │
│       ──N:1──> sys_user(巡查人)                          │
│                                                          │
│  sv_volunteer_activity(志愿活动) ──N:1──> cm_community   │
│  sv_volunteer_record(志愿记录) ──N:1──> sys_user         │
│                                                          │
│  sv_service_order(服务订单) ──N:1──> sys_user(申请人)     │
│       ──N:1──> sv_service_provider(服务商)               │
│                                                          │
│  nt_announcement(公告) ──N:1──> cm_estate/cm_community   │
│                                                          │
│  oc_topic(业委会议题) ──N:1──> cm_owner_committee        │
│  oc_vote(投票记录) ──N:1──> oc_topic                     │
│                                                          │
│  msg_notification(消息通知) ──N:1──> sys_user            │
│                                                          │
│  eval_evaluation(评价) ──N:1──> wo_work_order/sv_order   │
│                                                          │
│  ai_chat_log(对话记录) ──N:1──> sys_user                │
│  ai_knowledge(知识库)                                     │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                    系统管理域                             │
│                                                          │
│  sys_dict_type(字典类型) ──1:N──> sys_dict_data(字典数据) │
│  sys_config(系统参数)                                     │
│  sys_operation_log(操作日志) ──N:1──> sys_user           │
│  sys_login_log(登录日志) ──N:1──> sys_user               │
│  sys_file(文件) ──N:1──> sys_user                        │
└─────────────────────────────────────────────────────────┘
```

### 2.2 核心实体关系矩阵

| 关系 | 类型 | 说明 |
| --- | --- | --- |
| 用户 ↔ 角色 | M:N | 通过 sys_user_role 关联 |
| 角色 ↔ 菜单权限 | M:N | 通过 sys_role_menu 关联 |
| 用户 ↔ 组织 | M:N | 通过 sys_user_org 关联（一人可属于多组织） |
| 街道 → 社区 | 1:N | 组织层级，parent_id 实现 |
| 社区 → 小区 | 1:N | 组织层级 |
| 小区 → 楼栋 | 1:N | |
| 楼栋 → 单元 | 1:N | |
| 单元 → 房屋 | 1:N | |
| 房屋 ↔ 居民 | M:N | 通过 cm_house_resident，含关系类型 |
| 家庭 → 居民 | 1:N | 一个家庭多个居民 |
| 居民 ↔ 车辆 | M:N | 通过 cm_vehicle_owner |
| 小区 → 车位 | 1:N | |
| 小区 → 设施 | 1:N | |
| 社区 → 网格 | 1:N | |
| 物业公司 → 小区 | 1:N | 一个物业公司可管多个小区 |
| 业委会 → 小区 | 1:1 | 一个小区一个业委会 |
| 工单 → 房屋 | N:1 | |
| 工单 → 用户(报修/处理) | N:1 | |
| 事件 → 网格 | N:1 | |
| 事件 → 用户(上报/承办) | N:1 | |
| 志愿活动 → 社区 | N:1 | |
| 志愿记录 → 用户 | N:1 | |
| 服务订单 → 用户(申请人) | N:1 | |
| 服务订单 → 服务商 | N:1 | |
| 公告 → 组织/小区 | N:1 | |
| 议题 → 业委会 | N:1 | |
| 投票 → 议题 | N:1 | |
| 评价 → 工单/服务订单 | N:1 | |

---

## 3. 主表字段设计

### 3.1 通用字段约定

所有业务表均包含以下通用字段：

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT AUTO_INCREMENT | 主键 |
| create_by | BIGINT | 创建人ID |
| create_time | DATETIME | 创建时间，默认 CURRENT_TIMESTAMP |
| update_by | BIGINT | 更新人ID |
| update_time | DATETIME | 更新时间，默认 CURRENT_TIMESTAMP ON UPDATE |
| del_flag | TINYINT | 逻辑删除标志，0=正常，1=已删除 |
| remark | VARCHAR(500) | 备注 |

### 3.2 SYS — 系统管理域

#### sys_user（用户表）

| 字段名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| id | BIGINT | PK, AUTO_INCREMENT | 主键 |
| username | VARCHAR(64) | UNIQUE, NOT NULL | 登录账号 |
| password | VARCHAR(128) | NOT NULL | 加密密码(BCrypt) |
| nickname | VARCHAR(64) | | 昵称/显示名 |
| real_name | VARCHAR(64) | | 真实姓名 |
| avatar | VARCHAR(256) | | 头像URL |
| phone | VARCHAR(20) | | 手机号 |
| email | VARCHAR(128) | | 邮箱 |
| id_card | VARCHAR(18) | | 身份证号（加密存储） |
| gender | TINYINT | | 性别：0未知 1男 2女 |
| birthday | DATE | | 出生日期 |
| status | TINYINT | NOT NULL, DEFAULT 0 | 状态：0正常 1禁用 2锁定 |
| user_type | TINYINT | NOT NULL, DEFAULT 0 | 用户类型：0居民 1工作人员 2服务商 3管理员 |
| last_login_time | DATETIME | | 最后登录时间 |
| last_login_ip | VARCHAR(64) | | 最后登录IP |

#### sys_role（角色表）

| 字段名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| id | BIGINT | PK | |
| role_name | VARCHAR(64) | NOT NULL | 角色名称 |
| role_key | VARCHAR(64) | UNIQUE, NOT NULL | 角色标识(如 property_manager) |
| role_domain | VARCHAR(32) | NOT NULL | 角色域：resident/property/governance/service/supervision/platform |
| data_scope | TINYINT | NOT NULL | 数据权限范围：1平台 2街道 3社区 4小区 5楼栋 6个人 7协同 |
| sort_order | INT | DEFAULT 0 | 排序 |
| status | TINYINT | DEFAULT 0 | 0正常 1禁用 |

#### sys_user_role（用户-角色关联表）

| 字段名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| user_id | BIGINT | PK, FK | |
| role_id | BIGINT | PK, FK | |

#### sys_menu（菜单/权限表）

| 字段名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| id | BIGINT | PK | |
| parent_id | BIGINT | DEFAULT 0 | 父菜单ID |
| menu_name | VARCHAR(64) | NOT NULL | 菜单名称 |
| menu_type | TINYINT | NOT NULL | 类型：1目录 2菜单 3按钮 4接口 |
| path | VARCHAR(256) | | 路由路径 |
| component | VARCHAR(256) | | 前端组件路径 |
| permission | VARCHAR(128) | | 权限标识(如 archive:house:list) |
| icon | VARCHAR(64) | | 图标 |
| sort_order | INT | DEFAULT 0 | 排序 |
| visible | TINYINT | DEFAULT 1 | 是否可见 |
| status | TINYINT | DEFAULT 0 | 0正常 1禁用 |

#### sys_role_menu（角色-菜单关联表）

| 字段名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| role_id | BIGINT | PK, FK | |
| menu_id | BIGINT | PK, FK | |

#### sys_org（组织表）

| 字段名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| id | BIGINT | PK | |
| parent_id | BIGINT | DEFAULT 0 | 父组织ID |
| org_name | VARCHAR(128) | NOT NULL | 组织名称 |
| org_type | TINYINT | NOT NULL | 类型：1平台 2街道 3社区 4物业公司 5业委会 6服务商 7社会组织 |
| org_code | VARCHAR(64) | UNIQUE | 组织编码 |
| leader | VARCHAR(64) | | 负责人 |
| phone | VARCHAR(20) | | 联系电话 |
| address | VARCHAR(256) | | 地址 |
| ancestors | VARCHAR(1024) | | 祖先层级(如 0,1,2)，方便查询子级 |
| sort_order | INT | DEFAULT 0 | |
| status | TINYINT | DEFAULT 0 | |

#### sys_user_org（用户-组织关联表）

| 字段名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| user_id | BIGINT | PK, FK | |
| org_id | BIGINT | PK, FK | |
| is_primary | TINYINT | DEFAULT 0 | 是否主组织 |

#### sys_dict_type / sys_dict_data（数据字典）

**sys_dict_type:**

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| dict_name | VARCHAR(128) | 字典名称 |
| dict_type | VARCHAR(128) | UNIQUE, 字典类型标识 |
| status | TINYINT | 0正常 1禁用 |
| remark | VARCHAR(500) | 备注 |

**sys_dict_data:**

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| dict_type | VARCHAR(128) | FK, 字典类型 |
| dict_label | VARCHAR(128) | 字典标签 |
| dict_value | VARCHAR(128) | 字典值 |
| sort_order | INT | 排序 |
| css_class | VARCHAR(64) | 前端样式类 |
| status | TINYINT | 0正常 1禁用 |

#### sys_operation_log（操作日志）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| user_id | BIGINT | 操作人 |
| username | VARCHAR(64) | 操作人账号 |
| module | VARCHAR(64) | 所属模块 |
| operation | VARCHAR(128) | 操作描述 |
| method | VARCHAR(256) | 请求方法 |
| request_url | VARCHAR(256) | 请求URL |
| request_method | VARCHAR(16) | GET/POST/PUT/DELETE |
| request_params | TEXT | 请求参数(JSON) |
| response_result | TEXT | 返回结果(JSON) |
| ip | VARCHAR(64) | IP地址 |
| status | TINYINT | 0成功 1失败 |
| error_msg | TEXT | 错误信息 |
| cost_time | BIGINT | 耗时(ms) |
| operation_time | DATETIME | 操作时间 |

#### sys_login_log（登录日志）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| user_id | BIGINT | 用户ID |
| username | VARCHAR(64) | 登录账号 |
| ip | VARCHAR(64) | 登录IP |
| location | VARCHAR(256) | 登录地点 |
| browser | VARCHAR(128) | 浏览器 |
| os | VARCHAR(128) | 操作系统 |
| status | TINYINT | 0成功 1失败 |
| message | VARCHAR(256) | 提示消息 |
| login_time | DATETIME | 登录时间 |

#### sys_file（文件表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| file_name | VARCHAR(256) | 原始文件名 |
| file_path | VARCHAR(512) | 存储路径 |
| file_size | BIGINT | 文件大小(字节) |
| file_type | VARCHAR(64) | MIME类型 |
| file_ext | VARCHAR(16) | 文件扩展名 |
| module | VARCHAR(64) | 所属模块 |
| biz_id | BIGINT | 关联业务ID |
| upload_by | BIGINT | 上传人 |
| upload_time | DATETIME | 上传时间 |

### 3.3 CM — 基础档案域

#### cm_estate（小区表）

| 字段名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| id | BIGINT | PK | |
| org_id | BIGINT | FK | 所属社区组织 |
| property_company_id | BIGINT | FK | 所属物业公司 |
| estate_name | VARCHAR(128) | NOT NULL | 小区名称 |
| estate_code | VARCHAR(64) | UNIQUE | 小区编码 |
| address | VARCHAR(256) | | 地址 |
| total_area | DECIMAL(12,2) | | 总面积(㎡) |
| total_buildings | INT | | 总楼栋数 |
| total_houses | INT | | 总户数 |
| built_year | INT | | 建成年份 |
| green_rate | DECIMAL(5,2) | | 绿化率(%) |
| parking_spaces | INT | | 停车位总数 |
| longitude | DECIMAL(10,7) | | 经度 |
| latitude | DECIMAL(10,7) | | 纬度 |
| status | TINYINT | DEFAULT 0 | 0正常 1停用 |

#### cm_building（楼栋表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| estate_id | BIGINT | FK, 所属小区 |
| building_name | VARCHAR(64) | 楼栋名称/编号 |
| building_code | VARCHAR(32) | 楼栋编码 |
| total_units | INT | 单元数 |
| total_floors | INT | 总楼层数 |
| has_elevator | TINYINT | 是否有电梯 |
| elevator_count | INT | 电梯数 |

#### cm_unit（单元表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| building_id | BIGINT | FK |
| unit_name | VARCHAR(32) | 单元名称/编号 |
| unit_code | VARCHAR(32) | 单元编码 |

#### cm_house（房屋表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| unit_id | BIGINT | FK, 所属单元 |
| estate_id | BIGINT | FK, 所属小区(冗余，方便查询) |
| house_code | VARCHAR(32) | UNIQUE, 房屋编号 |
| floor | INT | 楼层 |
| room_number | VARCHAR(16) | 房号 |
| area | DECIMAL(8,2) | 建筑面积(㎡) |
| house_type | VARCHAR(32) | 户型(如 三室两厅) |
| house_status | TINYINT | 状态：0空置 1自住 2出租 3已售 |
| owner_name | VARCHAR(64) | 产权人姓名(冗余) |
| owner_phone | VARCHAR(20) | 产权人电话(加密存储) |

#### cm_resident（居民档案表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| user_id | BIGINT | FK, 关联系统用户 |
| name | VARCHAR(64) | NOT NULL, 姓名 |
| id_card | VARCHAR(64) | 身份证号(加密存储) |
| gender | TINYINT | 性别 |
| phone | VARCHAR(64) | 手机号(加密存储) |
| phone_display | VARCHAR(20) | 手机号脱敏展示(如138****5678) |
| birthday | DATE | 出生日期 |
| photo | VARCHAR(256) | 照片URL |
| nationality | VARCHAR(32) | 民族 |
| education | VARCHAR(32) | 学历 |
| occupation | VARCHAR(64) | 职业 |
| political_status | VARCHAR(16) | 政治面貌 |
| is_special | TINYINT | 是否特殊群体：0否 1是 |
| special_type | VARCHAR(64) | 特殊群体类型(多选逗号分隔) |
| emergency_contact | VARCHAR(64) | 紧急联系人 |
| emergency_phone | VARCHAR(64) | 紧急联系电话(加密) |
| status | TINYINT | 0正常 1迁出 2注销 |

#### cm_family（家庭表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| family_name | VARCHAR(64) | 家庭名称 |
| house_id | BIGINT | FK, 关联房屋 |
| head_resident_id | BIGINT | FK, 户主居民ID |
| member_count | INT | 家庭成员数 |

#### cm_family_member（家庭成员关联表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| family_id | BIGINT | FK |
| resident_id | BIGINT | FK |
| relation | VARCHAR(32) | 与户主关系：配偶/子女/父母/其他 |
| is_head | TINYINT | 是否户主 |

#### cm_house_resident（人房关系表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| house_id | BIGINT | FK |
| resident_id | BIGINT | FK |
| relation_type | TINYINT | 关系类型：1产权人 2居住人 3租户 4家庭成员 |
| start_date | DATE | 关系开始日期 |
| end_date | DATE | 关系结束日期(NULL=当前有效) |
| status | TINYINT | 0有效 1失效 |

#### cm_vehicle（车辆表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| plate_number | VARCHAR(16) | UNIQUE, 车牌号 |
| vehicle_brand | VARCHAR(64) | 品牌 |
| vehicle_color | VARCHAR(32) | 颜色 |
| vehicle_type | TINYINT | 类型：1轿车 2SUV 3电动车 4其他 |
| estate_id | BIGINT | FK, 所属小区 |
| parking_space_id | BIGINT | FK, 绑定车位(可空) |
| status | TINYINT | 0正常 1注销 |

#### cm_vehicle_owner（车辆-车主关联表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| vehicle_id | BIGINT | FK |
| resident_id | BIGINT | FK |
| relation | VARCHAR(32) | 关系：车主/使用人 |

#### cm_parking_space（车位表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| estate_id | BIGINT | FK |
| space_code | VARCHAR(32) | 车位编号 |
| area_name | VARCHAR(64) | 区域名称(如A区、B区) |
| space_type | TINYINT | 1产权 2租赁 3临时 |
| status | TINYINT | 0空闲 1已分配 2维修中 |

#### cm_facility（设施设备表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| estate_id | BIGINT | FK |
| facility_name | VARCHAR(128) | 设施名称 |
| facility_type | TINYINT | 类型：1电梯 2消防 3门禁 4摄像头 5健身 6照明 7其他 |
| facility_code | VARCHAR(64) | 设备编号 |
| location | VARCHAR(256) | 安装位置 |
| install_date | DATE | 安装日期 |
| warranty_expire | DATE | 保修到期 |
| last_maintenance | DATE | 最近维护日期 |
| next_maintenance | DATE | 下次维护计划 |
| status | TINYINT | 0正常 1故障 2报废 |

#### cm_grid（网格表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| community_org_id | BIGINT | FK, 所属社区 |
| grid_name | VARCHAR(64) | 网格名称 |
| grid_code | VARCHAR(32) | 网格编码 |
| description | VARCHAR(256) | 网格描述 |
| grid_worker_id | BIGINT | FK, 负责网格员 |
| cover_estates | VARCHAR(512) | 覆盖小区ID列表(JSON) |
| longitude | DECIMAL(10,7) | 中心经度 |
| latitude | DECIMAL(10,7) | 中心纬度 |
| status | TINYINT | 0正常 1停用 |

#### cm_property_company（物业公司表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| org_id | BIGINT | FK, 关联组织 |
| company_name | VARCHAR(128) | 公司名称 |
| legal_person | VARCHAR(64) | 法人 |
| license_no | VARCHAR(64) | 营业执照号 |
| service_scope | VARCHAR(256) | 服务范围 |
| contact_phone | VARCHAR(20) | 联系电话 |
| contract_start | DATE | 合同开始 |
| contract_end | DATE | 合同到期 |
| status | TINYINT | 0正常 1停用 |

#### cm_owner_committee（业委会表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| org_id | BIGINT | FK, 关联组织 |
| estate_id | BIGINT | FK, 所属小区 |
| committee_name | VARCHAR(128) | 业委会名称 |
| established_date | DATE | 成立日期 |
| term_start | DATE | 任期开始 |
| term_end | DATE | 任期结束 |
| member_count | INT | 成员数 |
| status | TINYINT | 0正常 1换届中 2已解散 |

#### cm_owner_committee_member（业委会成员表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| committee_id | BIGINT | FK |
| user_id | BIGINT | FK |
| position | VARCHAR(32) | 职务：主任/副主任/委员 |
| join_date | DATE | 加入日期 |

#### cm_special_group（特殊群体档案表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| resident_id | BIGINT | FK, 关联居民 |
| group_type | TINYINT | 类型：1独居老人 2残障人士 3低保户 4重点帮扶 5其他 |
| detail | VARCHAR(256) | 详细说明 |
| care_level | TINYINT | 关怀等级：1一般 2重点 3特殊 |
| care_frequency | VARCHAR(32) | 走访频率(如 weekly/monthly) |
| care_worker_id | BIGINT | FK, 负责关怀人员 |
| last_visit_time | DATETIME | 最近走访时间 |

#### cm_social_org（社会组织表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| org_id | BIGINT | FK |
| org_name | VARCHAR(128) | 组织名称 |
| org_category | TINYINT | 类型：1志愿团队 2服务机构 3合作单位 |
| contact_person | VARCHAR(64) | 联系人 |
| contact_phone | VARCHAR(20) | 联系电话 |
| service_scope | VARCHAR(256) | 服务范围 |
| status | TINYINT | 0正常 1停用 |

### 3.4 WO — 工单域

#### wo_work_order（工单表）

| 字段名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| id | BIGINT | PK | |
| order_no | VARCHAR(32) | UNIQUE | 工单编号(自动生成) |
| estate_id | BIGINT | FK | 所属小区 |
| house_id | BIGINT | FK | 关联房屋 |
| reporter_id | BIGINT | FK | 报修人 |
| reporter_name | VARCHAR(64) | | 报修人姓名 |
| reporter_phone | VARCHAR(20) | | 联系电话 |
| order_type | TINYINT | NOT NULL | 类型：1水电维修 2公共设施 3环境卫生 4安全秩序 5其他 |
| priority | TINYINT | DEFAULT 1 | 优先级：1普通 2紧急 3特急 |
| title | VARCHAR(256) | NOT NULL | 工单标题 |
| description | TEXT | | 详细描述 |
| location_desc | VARCHAR(256) | | 位置描述 |
| images | VARCHAR(1024) | | 图片URL列表(JSON) |
| status | TINYINT | NOT NULL | 状态：0待受理 1已受理 2已派单 3处理中 4已完成 5已关闭 6已评价 7已退回 |
| accept_by | BIGINT | FK | 受理人 |
| accept_time | DATETIME | | 受理时间 |
| assign_to | BIGINT | FK | 执行人 |
| assign_time | DATETIME | | 派单时间 |
| finish_time | DATETIME | | 完成时间 |
| close_time | DATETIME | | 关闭时间 |
| result_desc | TEXT | | 处理结果描述 |
| result_images | VARCHAR(1024) | | 处理结果图片(JSON) |
| sla_deadline | DATETIME | | SLA截止时间 |
| is_escalated | TINYINT | DEFAULT 0 | 是否已升级 |
| escalate_to_event_id | BIGINT | | 升级为事件的ID |
| source | TINYINT | DEFAULT 0 | 来源：0居民报修 1巡检发现 2电话受理 3系统自动 |

### 3.5 GE — 社区治理域

#### ge_event（治理事件表）

| 字段名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| id | BIGINT | PK | |
| event_no | VARCHAR(32) | UNIQUE | 事件编号 |
| grid_id | BIGINT | FK | 所属网格 |
| community_org_id | BIGINT | FK | 所属社区 |
| reporter_id | BIGINT | FK | 上报人 |
| reporter_name | VARCHAR(64) | | 上报人姓名 |
| event_category | TINYINT | NOT NULL | 类别：1民生服务 2环境卫生 3设施安全 4邻里纠纷 5特殊帮扶 6市容秩序 7突发预警 |
| priority | TINYINT | DEFAULT 1 | 优先级：1一般 2重要 3紧急 4特急 |
| title | VARCHAR(256) | NOT NULL | 事件标题 |
| description | TEXT | | 事件描述 |
| location | VARCHAR(256) | | 发生地点 |
| longitude | DECIMAL(10,7) | | 经度 |
| latitude | DECIMAL(10,7) | | 纬度 |
| images | VARCHAR(1024) | | 图片(JSON) |
| status | TINYINT | NOT NULL | 状态：0待审核 1已审核 2已分派 3处理中 4已完成 5已退回 6已归档 7已升级 |
| reviewer_id | BIGINT | FK | 审核人 |
| review_time | DATETIME | | 审核时间 |
| assign_to | BIGINT | FK | 承办人 |
| assign_org_id | BIGINT | FK | 承办组织 |
| assign_time | DATETIME | | 分派时间 |
| finish_time | DATETIME | | 完成时间 |
| archive_time | DATETIME | | 归档时间 |
| result_desc | TEXT | | 处理结果 |
| result_images | VARCHAR(1024) | | 处理结果图片(JSON) |
| sla_deadline | DATETIME | | SLA截止时间 |
| is_supervised | TINYINT | DEFAULT 0 | 是否督办 |
| is_coordinated | TINYINT | DEFAULT 0 | 是否联动处置 |
| source | TINYINT | DEFAULT 0 | 来源：0网格员上报 1居民上报 2物业上报 3巡查发现 |

#### ge_event_flow（事件流转记录表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| event_id | BIGINT | FK |
| action | VARCHAR(64) | 操作：审核/分派/处理/退回/升级/督办/归档 |
| operator_id | BIGINT | 操作人 |
| operator_name | VARCHAR(64) | 操作人姓名 |
| from_status | TINYINT | 原状态 |
| to_status | TINYINT | 新状态 |
| comment | TEXT | 备注说明 |
| images | VARCHAR(1024) | 附件图片(JSON) |
| action_time | DATETIME | 操作时间 |

#### ge_inspection（巡查记录表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| grid_id | BIGINT | FK |
| inspector_id | BIGINT | FK, 巡查人 |
| plan_id | BIGINT | FK, 关联巡查计划(可空) |
| start_time | DATETIME | 开始时间 |
| end_time | DATETIME | 结束时间 |
| duration | INT | 巡查时长(分钟) |
| route_points | TEXT | 巡查轨迹点(JSON) |
| issue_count | INT | 发现问题数 |
| summary | TEXT | 巡查总结 |
| status | TINYINT | 0进行中 1已完成 |

#### ge_inspection_plan（巡查计划表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| grid_id | BIGINT | FK |
| plan_name | VARCHAR(128) | 计划名称 |
| frequency | VARCHAR(32) | 频率(daily/weekly/monthly) |
| inspector_id | BIGINT | FK, 执行人 |
| start_date | DATE | 开始日期 |
| end_date | DATE | 结束日期 |
| status | TINYINT | 0正常 1停用 |

#### ge_visit_record（走访记录表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| special_group_id | BIGINT | FK, 关联特殊群体 |
| resident_id | BIGINT | FK |
| visitor_id | BIGINT | FK, 走访人 |
| visit_time | DATETIME | 走访时间 |
| visit_type | TINYINT | 1定期走访 2临时走访 3紧急走访 |
| content | TEXT | 走访内容 |
| images | VARCHAR(1024) | 走访照片(JSON) |
| found_issues | VARCHAR(512) | 发现问题 |
| follow_up | VARCHAR(256) | 后续措施 |

#### ge_supervision（督办表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| event_id | BIGINT | FK |
| supervisor_id | BIGINT | FK, 督办人 |
| supervise_to | BIGINT | FK, 被督办人 |
| deadline | DATETIME | 要求完成时间 |
| reason | VARCHAR(256) | 督办原因 |
| status | TINYINT | 0已下发 1已反馈 2已结案 |
| feedback | TEXT | 反馈内容 |
| feedback_time | DATETIME | 反馈时间 |

### 3.6 SV — 社区服务域

#### sv_volunteer（志愿者表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| user_id | BIGINT | FK |
| resident_id | BIGINT | FK |
| skills | VARCHAR(256) | 特长(逗号分隔) |
| available_time | VARCHAR(128) | 可用时间段 |
| total_hours | DECIMAL(8,1) | 累计服务时长 |
| status | TINYINT | 0正常 1冻结 |
| register_time | DATETIME | 注册时间 |

#### sv_volunteer_activity（志愿活动表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| community_org_id | BIGINT | FK |
| title | VARCHAR(256) | 活动标题 |
| description | TEXT | 活动描述 |
| activity_type | TINYINT | 类型：1环保 2助老 3文教 4治安 5其他 |
| location | VARCHAR(256) | 活动地点 |
| start_time | DATETIME | 开始时间 |
| end_time | DATETIME | 结束时间 |
| need_count | INT | 需求人数 |
| enrolled_count | INT | 已报名人数 |
| publisher_id | BIGINT | FK, 发布人 |
| status | TINYINT | 0草稿 1招募中 2进行中 3已结束 4已取消 |

#### sv_volunteer_signup（志愿报名表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| activity_id | BIGINT | FK |
| volunteer_id | BIGINT | FK |
| signup_time | DATETIME | 报名时间 |
| status | TINYINT | 0待审核 1已审核 2已拒绝 3已签到 4已签退 5缺席 |
| checkin_time | DATETIME | 签到时间 |
| checkout_time | DATETIME | 签退时间 |
| hours | DECIMAL(4,1) | 实际服务时长 |

#### sv_service_provider（服务商表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| org_id | BIGINT | FK |
| user_id | BIGINT | FK, 服务商账号 |
| provider_name | VARCHAR(128) | 服务商名称 |
| service_types | VARCHAR(256) | 服务类型(多选) |
| contact_person | VARCHAR(64) | 联系人 |
| contact_phone | VARCHAR(20) | 联系电话 |
| service_area | VARCHAR(256) | 服务区域 |
| rating | DECIMAL(2,1) | 平均评分 |
| total_orders | INT | 总订单数 |
| status | TINYINT | 0正常 1审核中 2停用 |

#### sv_service_order（服务订单表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| order_no | VARCHAR(32) | UNIQUE, 订单编号 |
| applicant_id | BIGINT | FK, 申请人 |
| provider_id | BIGINT | FK, 服务商 |
| service_type | TINYINT | 服务类型：1维修 2家政 3配送 4养老 5法律 6其他 |
| title | VARCHAR(256) | 服务标题 |
| description | TEXT | 服务描述 |
| appointment_time | DATETIME | 预约时间 |
| address | VARCHAR(256) | 服务地址 |
| status | TINYINT | 0待接单 1已接单 2已预约 3服务中 4已完成 5已评价 6已取消 |
| accept_time | DATETIME | 接单时间 |
| finish_time | DATETIME | 完成时间 |
| proof_images | VARCHAR(1024) | 完成凭证图片(JSON) |
| rating | TINYINT | 评分(1-5) |
| comment | VARCHAR(512) | 评价内容 |

#### sv_job_posting（就业岗位表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| publisher_id | BIGINT | FK |
| community_org_id | BIGINT | FK |
| title | VARCHAR(256) | 岗位名称 |
| company | VARCHAR(128) | 公司名 |
| salary_range | VARCHAR(64) | 薪资范围 |
| requirements | TEXT | 岗位要求 |
| description | TEXT | 岗位描述 |
| headcount | INT | 招聘人数 |
| deadline | DATE | 截止日期 |
| status | TINYINT | 0正常 1已关闭 |

### 3.7 NT — 消息通知域

#### nt_announcement（公告表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| title | VARCHAR(256) | 标题 |
| content | TEXT | 内容(富文本) |
| announcement_type | TINYINT | 类型：1小区公告 2社区公告 3物业公告 4系统公告 |
| target_scope | VARCHAR(32) | 发布范围(如 estate:1,community:2) |
| target_ids | VARCHAR(512) | 目标ID列表(JSON) |
| publisher_id | BIGINT | FK, 发布人 |
| publish_time | DATETIME | 发布时间 |
| is_top | TINYINT | 是否置顶 |
| read_count | INT | 阅读数 |
| status | TINYINT | 0草稿 1已发布 2已下架 |

#### nt_notification（消息通知表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| user_id | BIGINT | FK, 接收人 |
| title | VARCHAR(256) | 标题 |
| content | VARCHAR(1024) | 内容 |
| notification_type | TINYINT | 类型：1系统消息 2工单提醒 3审批待办 4超时预警 5活动通知 6公告推送 |
| biz_type | VARCHAR(64) | 关联业务类型 |
| biz_id | BIGINT | 关联业务ID |
| is_read | TINYINT | 0未读 1已读 |
| read_time | DATETIME | 阅读时间 |

### 3.8 OC — 业委会域

#### oc_topic（议题表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| committee_id | BIGINT | FK |
| estate_id | BIGINT | FK |
| title | VARCHAR(256) | 议题标题 |
| content | TEXT | 议题内容 |
| topic_type | TINYINT | 类型：1公共收益 2物业监督 3设施改造 4其他 |
| publisher_id | BIGINT | FK, 发起人 |
| publish_time | DATETIME | 发布时间 |
| need_vote | TINYINT | 是否需要投票 |
| vote_start | DATETIME | 投票开始时间 |
| vote_end | DATETIME | 投票结束时间 |
| vote_result | TINYINT | 投票结果：0未投票 1通过 2未通过 |
| status | TINYINT | 0讨论中 1投票中 2已公示 3已关闭 |

#### oc_vote（投票记录表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| topic_id | BIGINT | FK |
| voter_id | BIGINT | FK, 投票人(业主) |
| choice | TINYINT | 选择：1赞成 2反对 3弃权 |
| vote_time | DATETIME | 投票时间 |

#### oc_public_income（公共收益公示表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| estate_id | BIGINT | FK |
| period | VARCHAR(32) | 公示周期(如 2026-Q1) |
| income_amount | DECIMAL(12,2) | 收入金额 |
| expense_amount | DECIMAL(12,2) | 支出金额 |
| balance | DECIMAL(12,2) | 结余 |
| detail | TEXT | 明细说明(JSON) |
| publisher_id | BIGINT | FK |
| publish_time | DATETIME | 发布时间 |
| status | TINYINT | 0公示中 1已归档 |

### 3.9 EV — 评价域

#### ev_evaluation（评价表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| biz_type | VARCHAR(32) | 业务类型：work_order/service_order/complaint |
| biz_id | BIGINT | 业务ID |
| evaluator_id | BIGINT | FK, 评价人 |
| rating | TINYINT | 评分(1-5) |
| content | VARCHAR(512) | 评价内容 |
| reply | VARCHAR(512) | 回复内容 |
| reply_by | BIGINT | 回复人 |
| reply_time | DATETIME | 回复时间 |
| eval_time | DATETIME | 评价时间 |

### 3.10 AI — 智能助手域

#### ai_knowledge（知识库表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| category | VARCHAR(64) | 知识分类 |
| question | VARCHAR(512) | 问题 |
| answer | TEXT | 回答 |
| keywords | VARCHAR(256) | 关键词 |
| priority | INT | 优先级 |
| hit_count | INT | 命中次数 |
| status | TINYINT | 0正常 1下架 |

#### ai_chat_log（对话日志表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| user_id | BIGINT | FK |
| session_id | VARCHAR(64) | 会话ID |
| role | VARCHAR(16) | user/assistant/system |
| content | TEXT | 消息内容 |
| intent | VARCHAR(64) | 识别到的意图 |
| matched_knowledge_id | BIGINT | 匹配的知识ID(可空) |
| is_useful | TINYINT | 是否有用反馈(NULL=未反馈 0无用 1有用) |
| create_time | DATETIME | |

### 3.11 KF — 考核域

#### kf_assessment_config（考核配置表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| indicator_name | VARCHAR(128) | 指标名称 |
| indicator_type | TINYINT | 类型：1事件处理 2工单处理 3巡查完成率 4走访完成率 |
| target_value | DECIMAL(8,2) | 目标值 |
| weight | DECIMAL(5,2) | 权重(%) |
| assessment_period | VARCHAR(16) | 考核周期：monthly/quarterly/yearly |
| status | TINYINT | 0正常 1停用 |

#### kf_assessment_result（考核结果表）

| 字段名 | 类型 | 说明 |
| --- | --- | --- |
| id | BIGINT | PK |
| config_id | BIGINT | FK |
| target_org_id | BIGINT | FK, 被考核组织 |
| target_user_id | BIGINT | FK, 被考核人(可空) |
| period | VARCHAR(32) | 考核周期(如 2026-06) |
| actual_value | DECIMAL(8,2) | 实际值 |
| score | DECIMAL(5,2) | 得分 |
| rank | INT | 排名 |
| assess_time | DATETIME | 考核时间 |

---

## 4. 字典与状态码规范

### 4.1 核心字典类型清单

| dict_type | 说明 | 示例值 |
| --- | --- | --- |
| sys_user_type | 用户类型 | 0居民 1工作人员 2服务商 3管理员 |
| sys_user_status | 用户状态 | 0正常 1禁用 2锁定 |
| sys_org_type | 组织类型 | 1平台 2街道 3社区 4物业 5业委会 6服务商 7社会组织 |
| sys_menu_type | 菜单类型 | 1目录 2菜单 3按钮 4接口 |
| sys_data_scope | 数据权限范围 | 1平台 2街道 3社区 4小区 5楼栋 6个人 7协同 |
| cm_house_status | 房屋状态 | 0空置 1自住 2出租 3已售 |
| cm_house_relation | 人房关系类型 | 1产权人 2居住人 3租户 4家庭成员 |
| cm_vehicle_type | 车辆类型 | 1轿车 2SUV 3电动车 4其他 |
| cm_facility_type | 设施类型 | 1电梯 2消防 3门禁 4摄像头 5健身 6照明 7其他 |
| cm_special_type | 特殊群体类型 | 1独居老人 2残障人士 3低保户 4重点帮扶 5其他 |
| cm_care_level | 关怀等级 | 1一般 2重点 3特殊 |
| wo_order_type | 工单类型 | 1水电维修 2公共设施 3环境卫生 4安全秩序 5其他 |
| wo_priority | 工单优先级 | 1普通 2紧急 3特急 |
| wo_status | 工单状态 | 0待受理 1已受理 2已派单 3处理中 4已完成 5已关闭 6已评价 7已退回 |
| ge_event_category | 事件类别 | 1民生服务 2环境卫生 3设施安全 4邻里纠纷 5特殊帮扶 6市容秩序 7突发预警 |
| ge_event_priority | 事件优先级 | 1一般 2重要 3紧急 4特急 |
| ge_event_status | 事件状态 | 0待审核 1已审核 2已分派 3处理中 4已完成 5已退回 6已归档 7已升级 |
| sv_activity_type | 志愿活动类型 | 1环保 2助老 3文教 4治安 5其他 |
| sv_activity_status | 活动状态 | 0草稿 1招募中 2进行中 3已结束 4已取消 |
| sv_order_type | 服务类型 | 1维修 2家政 3配送 4养老 5法律 6其他 |
| sv_order_status | 服务订单状态 | 0待接单 1已接单 2已预约 3服务中 4已完成 5已评价 6已取消 |
| nt_announcement_type | 公告类型 | 1小区公告 2社区公告 3物业公告 4系统公告 |
| nt_notification_type | 通知类型 | 1系统消息 2工单提醒 3审批待办 4超时预警 5活动通知 6公告推送 |
| oc_topic_type | 议题类型 | 1公共收益 2物业监督 3设施改造 4其他 |
| oc_vote_choice | 投票选项 | 1赞成 2反对 3弃权 |
| kf_indicator_type | 考核指标类型 | 1事件处理 2工单处理 3巡查完成率 4走访完成率 |

### 4.2 SLA 时限配置建议

| 业务类型 | 优先级 | SLA时限 | 说明 |
| --- | --- | --- | --- |
| 工单-普通 | 1 | 48小时 | 水电维修等日常问题 |
| 工单-紧急 | 2 | 4小时 | 漏水、断电等 |
| 工单-特急 | 3 | 2小时 | 电梯困人、燃气泄漏 |
| 事件-一般 | 1 | 72小时 | 一般性社区事件 |
| 事件-重要 | 2 | 24小时 | 需协调多方的事件 |
| 事件-紧急 | 3 | 8小时 | 设施安全隐患 |
| 事件-特急 | 4 | 2小时 | 突发预警事件 |
| 投诉 | — | 48小时 | 居民投诉 |
| 督办 | — | 按事件SLA | 随事件SLA |

---

## 5. 索引与性能建议

### 5.1 核心索引建议

| 表 | 建议索引 |
| --- | --- |
| sys_user | UNIQUE(username), INDEX(phone), INDEX(status) |
| sys_org | INDEX(parent_id), INDEX(org_type), INDEX(ancestors) |
| cm_house | INDEX(estate_id), INDEX(unit_id), UNIQUE(house_code) |
| cm_resident | INDEX(user_id), INDEX(id_card), INDEX(phone_display) |
| cm_house_resident | INDEX(house_id), INDEX(resident_id), INDEX(status) |
| wo_work_order | UNIQUE(order_no), INDEX(estate_id), INDEX(reporter_id), INDEX(status), INDEX(sla_deadline), INDEX(create_time) |
| ge_event | UNIQUE(event_no), INDEX(grid_id), INDEX(status), INDEX(sla_deadline), INDEX(create_time), INDEX(reporter_id) |
| ge_event_flow | INDEX(event_id), INDEX(action_time) |
| sv_service_order | UNIQUE(order_no), INDEX(applicant_id), INDEX(provider_id), INDEX(status) |
| nt_notification | INDEX(user_id), INDEX(is_read), INDEX(notification_type) |
| sys_operation_log | INDEX(user_id), INDEX(operation_time), INDEX(module) |

### 5.2 分区建议

- `sys_operation_log` 和 `sys_login_log`：按月分区(operation_time)，保留最近12个月在线数据。
- `nt_notification`：可按user_id取模分区，提升个人消息查询效率。
- `ai_chat_log`：按月分区(create_time)。

### 5.3 统计预计算建议

对于报表看板的频繁查询，建议引入统计中间表：

| 表名 | 说明 |
| --- | --- |
| stat_daily_work_order | 每日工单统计(数量、完结率、超时率)，定时任务刷新 |
| stat_daily_event | 每日事件统计(分类、时效)，定时任务刷新 |
| stat_daily_volunteer | 每日志愿活动统计 |
| stat_daily_service | 每日服务订单统计 |

---

## 6. 数据库初始化策略

### 6.1 初始化数据

- 超级管理员账号（admin）。
- 基础角色数据（居民、物业客服、物业经理、维修人员、网格员、社区工作人员、社区书记、街道工作人员、街道领导、业委会成员、便民服务商、平台管理员、审计管理员等）。
- 基础菜单数据（按模块清单配置）。
- 基础字典数据（按字典清单初始化）。
- 演示组织层级（1个街道 → 2个社区 → 各1-2个小区 → 楼栋/房屋示例数据）。

### 6.2 命名规范

- 表名：小写，模块前缀 + 业务名，如 `sys_user`, `cm_house`, `wo_work_order`。
- 字段名：小写下划线，如 `create_time`, `estate_id`。
- 索引名：`idx_表名_字段名`，如 `idx_wo_work_order_status`。
- 唯一索引：`uk_表名_字段名`，如 `uk_sys_user_username`。
- 外键不强制建立物理外键约束（使用逻辑外键，应用层保证一致性），降低维护成本。

---

## 7. 扩展性预留

| 扩展点 | 设计预留 |
| --- | --- |
| 多租户 | sys_org 作为租户维度，所有业务表预留 org_id / estate_id 字段 |
| 外部系统联动 | ge_event 表预留 external_system / external_id 字段 |
| 物联网设备 | cm_facility 预留 device_code / protocol_type 字段，后续扩展 IoT 模块 |
| 在线支付 | wo_work_order / sv_service_order 预留 payment_status / payment_no 字段 |
| 短信通知 | nt_notification 预留 send_channel 字段(1站内 2短信 3邮件) |
| 地图服务 | cm_estate / cm_grid 预留经纬度字段(已包含) |
| 流程引擎 | M06 流程引擎建议第一期硬编码核心流程，预留 wf_ 前缀表空间 |
