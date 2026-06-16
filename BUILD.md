# 智慧社区管理平台 - C++ Qt 版

## 项目概述

基于 C++ Qt 6 开发的智慧社区管理桌面应用程序，实现小区管理与社区治理一体化平台。

**核心特性：**
- 多角色分层权限体系（居民、物业、社区、街道、业委会、服务商、平台管理员）
- 完整业务闭环（报事报修、社区事件、志愿服务）
- SQLite 本地数据库，开箱即用
- 丰富的演示数据，支持多场景演示

## 技术栈

| 组件 | 技术 |
|------|------|
| 语言 | C++17 |
| GUI 框架 | Qt 6 (Widgets) |
| 数据库 | SQLite (Qt SQL) |
| 图表 | Qt Charts |
| 构建系统 | CMake 3.16+ |

## 项目结构

```
smart_community_management/
├── CMakeLists.txt              # CMake 构建配置
├── sql/
│   └── schema.sql              # 数据库表结构定义 (45+张表)
├── resources/
│   ├── resources.qrc           # Qt 资源文件
│   └── style.qss               # 全局样式表
└── src/
    ├── main.cpp                # 程序入口
    ├── models/
    │   ├── Models.h            # 数据模型定义
    │   └── Constants.h         # 常量与枚举定义
    ├── utils/
    │   └── Utils.h             # 工具函数
    ├── database/
    │   └── DatabaseManager.h   # 数据库管理器
    ├── services/
    │   ├── AuthService.h       # 认证服务
    │   └── DemoDataService.h   # 演示数据初始化
    └── widgets/
        ├── LoginWidget.h/cpp   # 登录界面
        └── MainWindow.h/cpp    # 主窗口
```

## 环境要求

### 必需软件

1. **Qt 6.x** (推荐 Qt 6.5+)
   - 必须安装的模块：`qtbase` (Core, Gui, Widgets, Sql), `qtcharts`
   - 下载地址：https://www.qt.io/download

2. **CMake 3.16+**
   - 下载地址：https://cmake.org/download/

3. **C++ 编译器**
   - Windows: MinGW-w64 (GCC 10+) 或 MSVC 2019+
   - Linux: GCC 9+ 或 Clang 10+
   - macOS: Xcode Command Line Tools

## 编译运行

### 方法一：命令行构建（推荐）

```bash
# 进入项目目录
cd smart_community_management

# 创建构建目录
mkdir build && cd build

# 配置 CMake（指定 Qt 路径，如果不在 PATH 中）
# Windows 示例（假设 Qt 安装在 D:\Qt\6.5.0\mingw_64）
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="D:/Qt/6.5.0/mingw_64/lib/cmake"

# Linux/macOS 示例
cmake .. -DCMAKE_PREFIX_PATH="/opt/Qt/6.5.0/gcc_64/lib/cmake"

# 编译
cmake --build . --config Release

# 运行
# Windows
Release\SmartCommunity.exe
# Linux/macOS
./SmartCommunity
```

### 方法二：Qt Creator IDE

1. 打开 Qt Creator
2. 文件 → 打开文件或项目 → 选择 `CMakeLists.txt`
3. 配置 Kit（选择 Qt 6 的编译器）
4. 点击左下角的绿色三角形运行

### 常见问题

**Q: 找不到 Qt6Charts**
```
A: 确保安装了 Qt Charts 模块。在 Qt Online Installer 中勾选 "Qt Charts"。
```

**Q: 编译时提示找不到 Qt**
```
A: 设置 CMAKE_PREFIX_PATH 指向 Qt 安装目录下的 lib/cmake 文件夹。
```

**Q: 运行时提示缺少 DLL**
```
A: Windows 下需要将 Qt 的 bin 目录添加到 PATH，或使用 windeployqt 打包。
```

## 演示账号

| 账号 | 密码 | 角色 | 可见功能 |
|------|------|------|----------|
| admin | admin123 | 平台超级管理员 | 全部功能 |
| zhangsan | 123456 | 居民户主 | 报修、服务、公告 |
| wuye_kefu | 123456 | 物业客服 | 工单受理、档案管理 |
| wuye_jingli | 123456 | 物业经理 | 物业管理、统计报表 |
| wangge_zhao | 123456 | 网格员 | 事件上报、巡查管理 |
| shequ_lifang | 123456 | 社区工作人员 | 事件审核、居民档案 |
| shequ_shuji | 123456 | 社区书记 | 治理看板、督办管理 |
| jiedao_worker | 123456 | 街道工作人员 | 辖区统计、考核 |
| yewei_zhao | 123456 | 业委会成员 | 公共收益、议题投票 |

## 功能模块

### 已实现模块

| 模块 | 子功能 | 状态 |
|------|--------|------|
| 工作台 | 首页概览、待办中心、消息中心 | ✅ |
| 基础档案 | 组织、小区、楼栋房屋、居民、车辆、设施、网格、特殊群体 | ✅ |
| 小区管理 | 报事报修、投诉建议、巡检巡查、公告通知 | ✅ |
| 社区治理 | 事件中心、巡查管理、重点关怀、督办管理 | ✅ |
| 社区服务 | 志愿服务、便民服务、就业服务 | ✅ |
| 统计分析 | 工单统计、事件统计、服务统计、综合看板 | ✅ |
| 系统管理 | 用户、角色、菜单、字典、操作日志、智能问答 | ✅ |

### 核心业务流程

1. **报事报修闭环**：居民提交 → 物业受理 → 派单处理 → 完成评价 → 归档统计
2. **社区事件闭环**：网格员上报 → 社区审核 → 分派处理 → 督办跟踪 → 回访归档
3. **志愿服务流程**：活动发布 → 志愿者报名 → 签到签退 → 时长累计

## 数据库设计

- **45+ 张核心表**，覆盖系统管理、基础档案、工单、事件、服务、消息、业委会、评价、AI知识库、考核等全业务域
- **自动初始化**：首次运行自动创建表结构并填充演示数据
- **软删除机制**：所有业务表包含 `del_flag` 字段

## 权限体系

- **RBAC 模型**：用户-角色-菜单三级权限
- **数据权限**：7级数据范围（平台/街道/社区/小区/楼栋/个人/协同）
- **动态菜单**：根据用户角色自动展示对应功能

## 架构设计

```
┌─────────────────────────────────────────┐
│              UI Layer                    │
│  LoginWidget → MainWindow (Sidebar + Content) │
├─────────────────────────────────────────┤
│           Service Layer                  │
│  AuthService | DemoDataService           │
├─────────────────────────────────────────┤
│          Data Access Layer               │
│  DatabaseManager (SQLite CRUD)           │
├─────────────────────────────────────────┤
│           Model Layer                    │
│  Models.h (45+ structs)                  │
│  Constants.h (Enums & Labels)            │
└─────────────────────────────────────────┘
```

## 扩展开发

### 添加新页面

1. 在 `MainWindow.cpp` 的 `getOrCreatePage()` 中添加新的 key 分支
2. 创建对应的 `createXxxPage()` 方法
3. 在数据库中为新角色分配菜单权限

### 添加新业务模块

1. 在 `Models.h` 中定义数据结构
2. 在 `schema.sql` 中添加表定义
3. 使用 `DatabaseManager` 进行 CRUD 操作

## 课程答辩要点

1. **多角色协同**：展示不同角色登录后看到的界面差异
2. **流程闭环**：演示报修从提交到完成的完整流程
3. **权限分层**：展示数据权限隔离（如物业只能看自己小区的数据）
4. **数据完整性**：展示45+张表的数据库设计
5. **业务深度**：展示事件流转时间线、SLA时限、督办机制

## 许可证

本项目为课程实训项目，仅供学习使用。
