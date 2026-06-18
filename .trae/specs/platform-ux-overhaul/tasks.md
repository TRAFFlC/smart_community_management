# Tasks

## Phase 1: 安全与基础交互修复（P0）

- [x] Task 1: 修复全局 SQL 注入风险
  - [x] SubTask 1.1: 搜索所有字符串拼接 SQL（`"LIKE '%" + searchText + "%'"` 模式），改为 QSqlQuery::prepare + bindValue
  - [x] SubTask 1.2: 重点检查 createSystemPage（user/role/dict/log）和 createArchivePage 中的搜索逻辑
  - [x] SubTask 1.3: 编译验证无错误

- [x] Task 2: 登录页移除预填凭据
  - [x] SubTask 2.1: 移除 LoginWidget.cpp 中 `m_usernameEdit->setText("admin")` 和 `m_passwordEdit->setText("admin123")`
  - [x] SubTask 2.2: 将输入框改为占位符提示，演示账号信息保留在底部提示条

- [x] Task 3: 通知铃铛未读数角标
  - [x] SubTask 3.1: 在 MainWindow 顶栏铃铛按钮上添加红色数字角标 QLabel
  - [x] SubTask 3.2: 查询当前用户未读通知数 `SELECT COUNT(*) FROM nt_notification WHERE user_id = ? AND is_read = 0`
  - [x] SubTask 3.3: 在 switchPage 和页面刷新时更新角标数字
  - [x] SubTask 3.4: 角标为0时隐藏，>99时显示"99+"

- [x] Task 4: 面包屑可点击导航
  - [x] SubTask 4.1: 将 m_breadcrumbLabel 替换为多个可点击的 QLabel 链接
  - [x] SubTask 4.2: 点击"首页"跳转到工作台，点击模块名跳转到该模块第一个子页面

- [x] Task 5: 全局刷新按钮
  - [x] SubTask 5.1: 在顶栏添加刷新按钮（SVG 图标）
  - [x] SubTask 5.2: 点击后清除当前页面的 m_pageCache 缓存并重新创建页面
  - [x] SubTask 5.3: 添加旋转动画反馈

## Phase 2: 表格交互统一（P0）

- [x] Task 6: 统一状态标签样式
  - [x] SubTask 6.1: 搜索所有手动 setBackground/setForeground 的状态项，改为使用 createTagItem
  - [x] SubTask 6.2: 确保 createTagItem 使用统一的圆角背景色（alpha=30）+ 前景色

- [x] Task 7: 表格操作列改为胶囊按钮
  - [x] SubTask 7.1: 创建 setCellWidget 替代 setItem 方式，在操作列放置 QPushButton
  - [x] SubTask 7.2: 按钮样式：圆角、带背景色、hover 变深、cursor=pointinghand
  - [x] SubTask 7.3: 优先改造工单/事件/投诉/民意/督办/志愿 6 个有状态流转的页面

- [x] Task 8: 空状态提示
  - [x] SubTask 8.1: 在所有 loadXxx 函数中，当 table->rowCount() == 0 时显示 createEmptyState
  - [x] SubTask 8.2: 空状态包含图标+提示文字+引导操作（如"暂无工单，点击新建创建第一个工单"）

- [x] Task 9: 操作确认弹窗
  - [x] SubTask 9.1: 在工单受理/派单/完成/评价、事件审核/分派/完成、投诉受理/回复、民意回复、议题投票、志愿签到/签退操作前添加 QMessageBox::question 确认
  - [x] SubTask 9.2: 确认文案具体化（如"确认将工单派单给 张三？"而非通用的"确认操作？"）

- [x] Task 10: 操作成功 Toast 提示
  - [x] SubTask 10.1: 创建 showToast(const QString& msg) 静态函数，在页面右上角显示 2 秒自动消失的提示
  - [x] SubTask 10.2: 所有写入操作（insert/update）成功后调用 showToast

- [x] Task 10b: 表格分页与结果计数
  - [x] SubTask 10b.1: 创建 PaginationBar 通用分页组件（上/下一页、每页条数20/50/100、页码跳转、总条数显示）
  - [x] SubTask 10b.2: 为所有数据表格的加载函数注入 COUNT 查询和 LIMIT/OFFSET 分页逻辑
  - [x] SubTask 10b.3: 在每个页面布局中添加 PaginationBar（共 9 个分页控件覆盖 30+ 个表格）
  - [x] SubTask 10b.4: 搜索/筛选条件变更时自动重置到第 1 页

## Phase 3: 系统管理 CRUD（P1）

- [x] Task 11: 用户管理 CRUD
  - [x] SubTask 11.1: 添加"新增用户"按钮和对话框（username/real_name/phone/user_type/password/角色多选）
  - [x] SubTask 11.2: 添加"编辑"操作列按钮，可修改用户信息
  - [x] SubTask 11.3: 添加"禁用/启用"切换操作
  - [x] SubTask 11.4: 添加"重置密码"操作（弹框输入新密码）

- [x] Task 12: 角色管理 CRUD
  - [x] SubTask 12.1: 添加"新增角色"按钮和对话框（role_name/role_key/role_domain/data_scope/菜单权限树）
  - [x] SubTask 12.2: 添加"编辑"操作，可修改角色信息和菜单权限

- [x] Task 13: 菜单管理 CRUD
  - [x] SubTask 13.1: 添加"新增菜单"按钮（parent_id 下拉/menu_name/path/icon/sort_order）
  - [x] SubTask 13.2: 添加"编辑"和"删除"操作

- [x] Task 14: 字典管理 CRUD
  - [x] SubTask 14.1: 添加"新增字典类型"和"新增字典项"按钮
  - [x] SubTask 14.2: 添加"编辑"操作

- [x] Task 15: 操作日志分页
  - [x] SubTask 15.1: 移除 `LIMIT 100` 硬编码
  - [x] SubTask 15.2: 添加分页控件

## Phase 4: 统计分析增强（P1）

- [x] Task 16: 工单统计增强
  - [x] SubTask 16.1: 添加月度趋势折线图（QLineSeries，近6个月）
  - [x] SubTask 16.2: 添加优先级分布柱状图
  - [x] SubTask 16.3: 添加统计卡片（总工单数/平均处理时长/完结率/超时率）

- [x] Task 17: 事件统计增强
  - [x] SubTask 17.1: 添加月度趋势折线图
  - [x] SubTask 17.2: 添加处理效率柱状图（各状态平均停留时长）
  - [x] SubTask 17.3: 添加超时事件统计卡片

- [x] Task 18: 综合看板增强
  - [x] SubTask 18.1: 柱状图时间跨度添加 7天/30天/90天 切换按钮
  - [x] SubTask 18.2: 添加网格员绩效排名表（事件处理数+完结率+平均时长）

- [x] Task 19: 考核管理周期动态化
  - [x] SubTask 19.1: 从 kf_assessment_config 表查询 distinct cycle 填充下拉框
  - [x] SubTask 19.2: 移除写死的 2026年4/5/6月

- [x] Task 20: 数据导出 CSV
  - [x] SubTask 20.1: 创建 exportToCsv(const QString& filename, QTableWidget* table) 辅助函数
  - [x] SubTask 20.2: 在工单/事件/服务统计页面添加"导出"按钮
  - [x] SubTask 20.3: 使用 QFileDialog 选择保存路径

## Phase 5: 细节优化（P2）

- [x] Task 21: 全局搜索功能
  - [x] SubTask 21.1: 顶栏搜索框连接 textChanged 信号
  - [x] SubTask 21.2: 搜索菜单名（模糊匹配 sys_menu）+ 工单号 + 事件号
  - [x] SubTask 21.3: 结果以下拉 QFrame 列表展示，点击跳转

- [x] Task 22: 顶栏用户下拉菜单
  - [x] SubTask 22.1: 用户名旁下拉箭头改为 QToolButton + QMenu
  - [x] SubTask 22.2: 菜单项：个人中心（占位）、修改密码（弹框）、退出登录

- [x] Task 23: AI 快捷问题直接发送
  - [x] SubTask 23.1: 快捷问题按钮点击后直接调用 sendMessage 而非仅 setText

- [x] Task 24: 工单详情对话框
  - [x] SubTask 24.1: 表格行双击弹出详情对话框
  - [x] SubTask 24.2: 详情包含：基本信息 + 状态时间线（垂直布局显示各状态变更时间）

- [x] Task 25: 志愿报名确认弹窗
  - [x] SubTask 25.1: 报名按钮点击后先弹 QMessageBox::question 确认
  - [x] SubTask 25.2: 确认后执行报名并显示成功 Toast

# Task Dependencies
- Task 7 依赖 Task 6（先统一标签再改操作列）
- Task 9 依赖 Task 7（确认弹窗在操作按钮上触发）
- Task 10 依赖 Task 9（Toast 在确认操作后显示）
- Task 11-15 可并行
- Task 16-20 可并行
- Task 21-25 可并行
- Phase 2 依赖 Phase 1 的 Task 1（SQL修复后再改交互）
