# 智慧社区管理平台 - C++ Qt 技术架构说明

> 本文档是对 `spec.md` 第11节"技术架构建议"的修正与补充。原 spec.md 中描述的 Java Spring Boot + Vue Web 架构已调整为 **C++ Qt 6 桌面应用** 架构。

## 1. 技术栈总览

| 层级 | 技术选型 | 说明 |
|------|----------|------|
| 编程语言 | C++17 | 现代 C++ 标准 |
| GUI 框架 | Qt 6 Widgets | 桌面界面开发 |
| 数据库 | SQLite (Qt SQL) | 本地嵌入式数据库 |
| 图表可视化 | Qt Charts | 数据统计图表 |
| 构建系统 | CMake 3.16+ | 跨平台构建 |
| 开发工具 | Qt Creator / VS Code | IDE |

## 2. 与原 Web 方案的差异对比

| 维度 | 原方案 (Java + Vue) | 现方案 (C++ Qt) |
|------|---------------------|-----------------|
| 应用类型 | Web 应用（浏览器访问） | 桌面应用（本地安装） |
| 架构模式 | 前后端分离 | 单层单体应用 |
| 后端框架 | Spring Boot | 无（业务逻辑直接在 C++ 中实现） |
| 前端框架 | Vue 3 + Element Plus | Qt Widgets |
| 数据库 | MySQL（外部服务） | SQLite（本地文件） |
| 通信协议 | RESTful API | 函数调用（同进程） |
| 部署方式 | Tomcat/Jar + Nginx | 编译为原生可执行文件 |
| 权限实现 | Spring Security + JWT | 自定义 RBAC（内存+数据库） |

## 3. 代码模块结构

```
smart_community_management/
├── CMakeLists.txt              # CMake 构建配置
├── sql/
│   └── schema.sql              # SQLite 表结构定义 (45+张表)
├── resources/
│   ├── resources.qrc           # Qt 资源文件
│   └── style.qss               # 全局样式表 (QSS)
└── src/
    ├── main.cpp                # 程序入口点
    ├── models/
    │   ├── Models.h            # 数据模型结构体 (45+个struct)
    │   └── Constants.h         # 常量枚举与标签映射
    ├── utils/
    │   └── Utils.h             # 工具函数 (密码哈希、编号生成、脱敏)
    ├── database/
    │   └── DatabaseManager.h   # 数据库管理器 (单例，通用CRUD)
    ├── services/
    │   ├── AuthService.h       # 认证服务 (登录、权限、菜单)
    │   └── DemoDataService.h   # 演示数据初始化 (15个账号+业务数据)
    └── widgets/
        ├── LoginWidget.h/cpp   # 登录界面
        └── MainWindow.h/cpp    # 主窗口 (侧边栏+内容区+图表)
```

## 4. 核心设计模式

### 4.1 单例模式

以下核心服务采用单例模式：

```cpp
// DatabaseManager - 数据库管理
auto& db = DatabaseManager::instance();

// AuthService - 认证服务
auto& auth = AuthService::instance();
```

### 4.2 数据访问层

DatabaseManager 提供通用 CRUD 方法：

```cpp
// 查询
QSqlQuery query(const QString& sql, const QVariantMap& params = {});

// 插入，返回新ID
qint64 insert(const QString& table, const QVariantMap& data);

// 更新
bool update(const QString& table, qint64 id, const QVariantMap& data);

// 软删除
bool softDelete(const QString& table, qint64 id);
```

### 4.3 UI 架构

- **LoginWidget**: 独立登录窗口，登录成功后 emit 信号
- **MainWindow**: 主窗口，包含左侧导航栏和右侧内容堆叠区
- **页面懒加载**: 通过 `getOrCreatePage()` 按需创建页面并缓存

## 5. 数据库设计要点

### 5.1 SQLite 特性利用

- 使用 `AUTOINCREMENT` 自增主键
- 使用 `DATETIME DEFAULT CURRENT_TIMESTAMP` 自动时间戳
- 使用 `VARCHAR` 代替 TEXT（性能更好）
- 不强制外键约束（应用层保证一致性）

### 5.2 软删除机制

所有业务表包含 `del_flag` 字段：
```sql
del_flag INTEGER DEFAULT 0  -- 0=正常, 1=已删除
```

### 5.3 索引优化

核心索引包括：
- 用户表: username(唯一), phone, status
- 工单表: order_no(唯一), estate_id, reporter_id, status, create_time
- 事件表: event_no(唯一), grid_id, status, create_time

## 6. 权限体系实现

### 6.1 RBAC 模型

```
sys_user → sys_user_role → sys_role → sys_role_menu → sys_menu
```

### 6.2 数据权限

7级数据范围（data_scope）：
| 值 | 范围 | 说明 |
|----|------|------|
| 1 | 平台级 | 查看全部数据 |
| 2 | 街道级 | 查看本街道及下属社区 |
| 3 | 社区级 | 查看本社区及下属小区 |
| 4 | 小区级 | 查看本小区数据 |
| 5 | 楼栋级 | 查看本楼栋数据 |
| 6 | 个人级 | 仅查看本人数据 |
| 7 | 协同级 | 仅查看协同数据 |

### 6.3 动态菜单

登录时根据用户角色查询关联菜单，构建树形结构：
```cpp
QList<SysMenu> currentUserMenus();  // 返回当前用户的菜单树
```

## 7. 编译与部署

### 7.1 Windows 编译

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="D:/Qt/6.5.0/mingw_64/lib/cmake"
cmake --build . --config Release
```

### 7.2 打包部署

使用 `windeployqt` 打包依赖：
```bash
windeployqt --release SmartCommunity.exe
```

### 7.3 跨平台支持

| 平台 | 编译器 | 命令 |
|------|--------|------|
| Windows | MinGW-w64 / MSVC | cmake -G "MinGW Makefiles" |
| Linux | GCC / Clang | cmake .. |
| macOS | Xcode CLT | cmake .. -G "Unix Makefiles" |

## 8. 演示数据说明

### 8.1 内置账号

| 账号 | 密码 | 角色 | 用途 |
|------|------|------|------|
| admin | admin123 | 平台超级管理员 | 展示全部功能 |
| zhangsan | 123456 | 居民户主 | 居民视角演示 |
| wuye_kefu | 123456 | 物业客服 | 工单受理演示 |
| wuye_jingli | 123456 | 物业经理 | 物业管理演示 |
| wangge_zhao | 123456 | 网格员 | 事件上报演示 |
| shequ_lifang | 123456 | 社区工作人员 | 事件审核演示 |
| shequ_shuji | 123456 | 社区书记 | 治理看板演示 |
| jiedao_worker | 123456 | 街道工作人员 | 辖区统计演示 |
| yewei_zhao | 123456 | 业委会成员 | 公共收益演示 |

### 8.2 初始化流程

首次运行时自动检测数据库是否为空，若为空则执行：
1. 创建所有表结构
2. 初始化角色和菜单
3. 初始化组织架构
4. 创建演示账号
5. 填充基础档案数据
6. 生成演示工单和事件

## 9. 与原 spec.md 的功能对应关系

| spec.md 模块 | Qt 实现状态 | 说明 |
|--------------|-------------|------|
| M01 统一认证 | ✅ 完成 | LoginWidget + AuthService |
| M02 基础档案 | ✅ 完成 | 组织、小区、房屋、居民、车辆等表格展示 |
| M03 小区管理 | ✅ 完成 | 工单列表、公告列表 |
| M04 社区治理 | ✅ 完成 | 事件列表、流转记录 |
| M05 社区服务 | ✅ 完成 | 志愿活动、服务订单列表 |
| M06 流程引擎 | ⚠️ 简化 | 状态流转通过硬编码实现 |
| M07 消息通知 | ⚠️ 简化 | 数据库存储，UI暂不展示实时推送 |
| M08 统计分析 | ✅ 完成 | Qt Charts 饼图/柱状图 |
| M09 系统管理 | ✅ 完成 | 用户、角色、菜单、字典、日志表格 |
| M10 智能助手 | ⚠️ 简化 | 知识库数据存储，UI展示表格 |

## 10. 课程答辩建议

### 10.1 演示路线

1. **平台管理员视角**：登录 admin → 查看用户管理 → 角色权限 → 操作日志
2. **居民视角**：登录 zhangsan → 查看我的房屋 → 提交报修 → 查看进度
3. **物业视角**：登录 wuye_kefu → 工单中心 → 受理派单 → 处理反馈
4. **社区治理视角**：登录 shequ_lifang → 事件中心 → 审核分派 → 督办跟踪
5. **统计看板**：展示工单分类饼图、事件趋势柱状图

### 10.2 亮点强调

- **多角色协同**：同一套系统，不同角色看到不同界面和数据
- **流程闭环**：报修从提交到评价的完整链路
- **权限分层**：7级数据权限，精细控制可见范围
- **数据完整性**：45+张表覆盖全业务域
- **开箱即用**：SQLite 本地数据库，无需额外配置

## 11. 注意事项

1. **Qt 版本要求**：必须安装 Qt 6.5+，确保包含 Qt Charts 模块
2. **中文编码**：所有源文件使用 UTF-8 编码，QString 使用 QStringLiteral 宏
3. **数据库路径**：默认在当前目录创建 `smart_community.db`，可修改为绝对路径
4. **密码安全**：演示项目使用 SHA256+盐，生产环境应使用 BCrypt
5. **内存管理**：Qt 父子对象机制自动管理内存，注意不要重复 delete

---

*本文档最后更新：2026-06-15*
*适用版本：C++ Qt 6 实现 v1.0*
