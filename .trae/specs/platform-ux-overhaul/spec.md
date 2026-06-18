# 平台体验全面升级 Spec

## Why

Qwen3.7 从体验者视角对平台进行了全面审查，发现了大量影响用户体验的问题：视觉同质化严重（40+页面三段式结构）、操作反馈缺失（无确认弹窗、无成功提示、无加载状态）、交互细节缺失（无分页、无结果计数、无空状态、面包屑不可点击、全局搜索摆设、铃铛无未读数）、系统管理只读（用户/角色/菜单/字典无法增删改）、SQL注入风险遍布全局、统计分析过于单薄（仅饼图、无趋势对比、考核周期写死）、登录页预填凭据不够正式。这些问题让平台显得"堪堪能用"而非"眼前一亮"。

## What Changes

### A. 视觉设计升级（打破同质化）
- 重构页面头部组件：从"色条+图标+文字"升级为带模块色渐变背景的卡片式头部，增加模块标识色块
- 重构统计卡片：增加趋势指示器（↑↓ + 百分比），增加迷你 sparkline 折线图
- 重构表格行操作列：从文字链接改为胶囊按钮（带背景色+hover效果），增加图标
- 统一状态标签样式：全部使用 createTagItem 圆角背景，消除直角背景色不一致
- 重构空状态：所有表格为空时显示 createEmptyState（图标+提示+引导文字）

### B. 交互体验补全
- 表格分页：所有数据表格添加分页控件（每页20/50/100条切换 + 页码导航）
- 结果计数：每张表格底部显示"共 N 条记录"
- 面包屑可点击：面包屑各级别可点击导航到对应父级页面
- 全局搜索：顶栏搜索框连接到实际搜索（搜索菜单名+工单号+事件号，结果下拉列表跳转）
- 通知铃铛未读数：铃铛右上角显示红色未读数角标
- 刷新按钮：顶栏添加全局刷新按钮，刷新当前页面数据
- 操作确认：所有状态流转操作（派单/完成/评价/投票/签到等）添加二次确认弹窗
- 操作成功提示：所有写入操作后显示 Toast 成功提示
- 加载状态：数据查询时显示加载动画（QProgressBar busy 模式或旋转图标）

### C. 系统管理 CRUD 补全
- 用户管理：新增用户、编辑用户、禁用/启用用户、重置密码
- 角色管理：新增角色、编辑角色权限（菜单分配）
- 菜单管理：新增菜单项、编辑菜单、删除菜单、拖拽排序
- 字典管理：新增字典类型、新增字典项、编辑字典项
- 操作日志：移除 LIMIT 100 限制，添加分页

### D. 统计分析增强
- 工单统计：增加月度趋势折线图、工单优先级分布柱状图、平均处理时长统计卡片
- 事件统计：增加月度趋势折线图、事件处理效率柱状图、超时事件统计
- 服务统计：增加服务满意度统计、服务类型分布柱状图
- 综合看板：柱状图时间跨度从7天扩展到30天可切换、增加网格员绩效排名表
- 考核管理：周期下拉框改为动态查询 kf_assessment_config 表，不再写死
- 数据导出：统计页面添加"导出 Excel"按钮（导出 CSV 格式）

### E. 安全修复
- SQL注入修复：所有搜索/筛选查询从字符串拼接改为参数绑定（QSqlQuery::prepare + bindValue）

### F. 细节优化
- 登录页：移除 admin/admin123 预填，改为占位符提示
- 顶栏用户名下拉箭头：添加下拉菜单（个人中心、修改密码、退出登录）
- 志愿报名：添加成功确认弹窗"确认报名此活动？"
- AI 快捷问题：点击后直接发送而非仅填充输入框
- 工单详情：双击表格行弹出详情对话框（完整信息+状态时间线），而非仅靠操作列

## Impact
- Affected specs: use_cases.md（UC-19-001/002/004/007 补全）, priority_analysis.md（系统管理缺口关闭）
- Affected code: src/widgets/MainWindow.cpp（主要改动文件）, src/widgets/LoginWidget.cpp, resources/style.qss

## ADDED Requirements

### Requirement: 表格分页
The system SHALL provide pagination for all data tables with more than 20 rows, with page size selector (20/50/100) and page navigation.

#### Scenario: Large dataset browsing
- **WHEN** user opens a page with 500+ records
- **THEN** table shows first 20 rows with pagination controls at bottom
- **AND** user can change page size to 50 or 100
- **AND** user can navigate to any page via page numbers

### Requirement: 操作确认与反馈
The system SHALL show confirmation dialog before any state transition operation and show success toast after operation completes.

#### Scenario: Confirm before dispatch
- **WHEN** user clicks "派单" on a work order
- **THEN** a confirmation dialog appears asking "确认派单给 XXX？"
- **AND** after confirmation, a success toast "派单成功" appears for 2 seconds

### Requirement: 全局搜索
The system SHALL provide a global search in the top bar that searches across menus, work orders, events, and announcements.

#### Scenario: Search for a work order
- **WHEN** user types "WO2024" in the global search box
- **THEN** a dropdown shows matching work orders with order number and title
- **AND** clicking a result navigates to the work order page

### Requirement: 通知未读数角标
The system SHALL display an unread count badge on the notification bell icon.

#### Scenario: New notification received
- **WHEN** a new notification is inserted into nt_notification for the current user
- **THEN** the bell icon shows a red badge with the unread count
- **AND** clicking the bell navigates to the message center

### Requirement: 系统管理 CRUD
The system SHALL provide full CRUD operations for user management, role management, menu management, and dictionary management.

#### Scenario: Create a new user
- **WHEN** admin clicks "新增用户" in user management
- **THEN** a dialog appears with fields: username, real_name, phone, user_type, password, role assignment
- **AND** after submission, the new user appears in the table and can log in

### Requirement: 数据导出
The system SHALL allow exporting statistics data to CSV format.

#### Scenario: Export work order statistics
- **WHEN** user clicks "导出" on the work order statistics page
- **THEN** a CSV file is generated and saved with all work order data
- **AND** the file opens in Excel with proper column headers

## MODIFIED Requirements

### Requirement: 统计分析
统计页面 SHALL 展示多维度图表（饼图+折线图+柱状图），而非仅饼图。综合看板 SHALL 支持时间范围切换（7天/30天/90天）。考核管理 SHALL 动态加载考核周期。

### Requirement: 表格交互
所有表格 SHALL 统一使用圆角背景状态标签、胶囊式操作按钮、空状态提示、分页控件和结果计数。

### Requirement: SQL安全
所有数据库查询 SHALL 使用参数绑定（QSqlQuery::prepare + bindValue），禁止字符串拼接 SQL。

## REMOVED Requirements

### Requirement: 登录页预填凭据
**Reason**: 预填 admin/admin123 让系统显得不够正式
**Migration**: 改为占位符提示"请输入账号 / 请输入密码"，演示账号信息保留在底部提示条中
