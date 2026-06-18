# Tasks

## Phase 1: 设计 Token 与全局 QSS

- [x] Task 1: 重写 `resources/style.qss`，建立新设计系统
  - [x] SubTask 1.1: 在 QSS 顶部集中定义颜色 token（深 slate 侧边栏、近白内容区、新强调色、语义色、中性灰阶）
  - [x] SubTask 1.2: 定义字体层级（H1 24px/700, H2 18px/600, body 14px/400, caption 12px, data 32px/700）
  - [x] SubTask 1.3: 定义圆角 token（全局 4px，按钮/输入框/标签统一）
  - [x] SubTask 1.4: 重写 QWidget、QTableWidget、QHeaderView、QPushButton、QLineEdit、QComboBox、QTabWidget、QGroupBox、QTreeWidget、QScrollBar、QMenu、QMessageBox、QToolTip 样式
  - [x] SubTask 1.5: 编译验证无样式错误

## Phase 2: 框架层重构

- [x] Task 2: 重构侧边栏
  - [x] SubTask 2.1: 将 `setupUI()` 中侧边栏背景改为深 slate（#0f172a），宽度保持 240px 或收窄至 220px
  - [x] SubTask 2.2: 调整 `createSidebarItem()`：激活态使用左侧 3px 强调色条 + 半透明高亮背景，hover 使用更淡的高亮
  - [x] SubTask 2.3: 调整分组标题 `groupHeader`：字体更小、字间距正常、颜色更暗，不再使用大写/字母间距伪装专业感
  - [x] SubTask 2.4: 折叠/展开箭头改用 SVG 图标或更克制的三角符号
  - [x] SubTask 2.5: Logo 区同步新配色

- [x] Task 3: 重构顶栏与面包屑
  - [x] SubTask 3.1: 顶栏高度保持 56px，背景改为与内容区一致或纯白，底部细边框改为 #e2e8f0
  - [x] SubTask 3.2: 搜索框改为 4px 圆角、更浅的背景、新的 focus 颜色
  - [x] SubTask 3.3: 刷新/通知/用户按钮使用新的 hover 背景色与图标尺寸
  - [x] SubTask 3.4: 面包屑 `updateBreadcrumb()` 改为普通文本路径，可点击项使用下划线 hover，不再使用蓝色链接色

## Phase 3: 工作台重新设计

- [x] Task 4: 重写 `createDashboardPage()`
  - [x] SubTask 4.1: 移除四张等宽统计卡片，改为顶部 3 个「大数字 + 标签」的轻量指标区（如：待处理工单 / 待审核事件 / 未读公告）
  - [x] SubTask 4.2: 快捷操作区改为无背景图标网格（3 列），文字标签位于图标下方，hover 仅改变图标颜色
  - [x] SubTask 4.3: 待办列表去掉 emoji，改用带语义色圆点的前置标记 + 文字 + 数量
  - [x] SubTask 4.4: 社区动态与数据概览改为两栏布局：左侧时间线（左侧色条区分类型），右侧小尺寸饼图
  - [x] SubTask 4.5: 所有模块使用统一内边距（24px）与间距（16/24px）

## Phase 4: 表格与列表统一

- [x] Task 5: 统一表格样式
  - [x] SubTask 5.1: 修改 `TABLE_STYLE` 常量：表头背景 #f1f5f9、文字 #475569、11px、字重 600；数据行 44px 行高；斑马纹 #fafafa；hover #f1f5f9
  - [x] SubTask 5.2: 修改 `createTagItem()`：圆角 4px、背景使用低饱和实色（alpha=15% 左右）、文字使用同色系深色、字号 12px、字重 500
  - [x] SubTask 5.3: 修改 `createPillButton()`：从圆角胶囊改为 4px 圆角小按钮，或改为 icon-only 按钮（新方向）
  - [x] SubTask 5.4: 修改 `createActionCell()`：按钮间距 8px，容器背景透明
  - [x] SubTask 5.5: 修改 `createEmptyState()`：图标颜色改为 #cbd5e1，提示文字使用新的辅助色

- [x] Task 6: 分页条与结果计数样式
  - [x] SubTask 6.1: 修改 `PaginationBar`（如有源码）或内联分页控件：按钮 4px 圆角、页码选中态使用强调色、紧凑内边距
  - [x] SubTask 6.2: 总条数字号 12px、颜色 #64748b

## Phase 5: 页面头部与操作列

- [x] Task 7: 重构页面头部 `createPageHeader()`
  - [x] SubTask 7.1: 移除渐变背景与左侧色条，改为白色/透明背景 + 底部 1px 分隔线
  - [x] SubTask 7.2: 标题 20px/700，描述 13px/400 #64748b
  - [x] SubTask 7.3: 头部高度从 72px 降至 64px
  - [x] SubTask 7.4: 各页面调用点只需传 title/desc，color 参数可选或用于小标识点

- [x] Task 8: 替换表格操作列胶囊按钮
  - [x] SubTask 8.1: 在工单/事件/投诉/民意/督办/志愿等状态流转页面，将 `createPillButton` 替换为统一的小按钮（4px 圆角、白底细边框、hover 背景变化）或文字链接
  - [x] SubTask 8.2: 确保操作列按钮颜色不再五花八门，最多使用主/成功/警告/危险四种语义色
  - [x] SubTask 8.3: 保留原有确认弹窗与 Toast 提示逻辑不变

## Phase 6: 图标、表单与登录页

- [x] Task 9: 移除 emoji 与统一图标
  - [x] SubTask 9.1: 全局搜索 `📋`, `📌`, `📢`, `✓`, `⚠️` 等 emoji 并替换为 SVG 图标或色块
  - [x] SubTask 9.2: 统计图表配色同步到新设计系统（不再使用原 #1677ff / #52c41a / #faad14 / #ff4d4f 高饱和组合）
  - [x] SubTask 9.3: 如资源允许，将现有 SVG 图标规范化为 2px 线宽、24x24 视图框

- [x] Task 10: 表单与弹窗样式
  - [x] SubTask 10.1: 输入框/下拉框/日期框圆角统一为 4px，focus 边框使用新强调色
  - [x] SubTask 10.2: 主按钮使用新强调色，危险按钮保留但降低饱和度
  - [x] SubTask 10.3: 弹窗标题 16px/600，减少顶部与底部多余间距

- [x] Task 11: 登录页视觉同步
  - [x] SubTask 11.1: 读取 `src/widgets/LoginWidget.cpp`
  - [x] SubTask 11.2: 将登录页主色/按钮色/输入框样式同步到新设计系统
  - [x] SubTask 11.3: 移除或替换 emoji/装饰元素

## Phase 7: 验证与走查

- [x] Task 12: 编译与运行时验证
  - [x] SubTask 12.1: `cmake --build . --config Release` 编译通过
  - [x] SubTask 12.2: 运行时无 QPainter 错误、无 QString::arg 警告
  - [x] SubTask 12.3: 逐页检查视觉一致性，记录与规格偏差

## Phase 8: 验证问题修复（P0）

- [x] Task 13: 统一圆角收敛
  - [x] SubTask 13.1: 在 MainWindow.cpp 中搜索 `border-radius: 8px`、`border-radius: 10px`、`border-radius: 12px`，统一收敛到 4-6px（卡片/面板最大 6px，按钮/输入框/标签 4px）
  - [x] SubTask 13.2: 在 LoginWidget.cpp 中搜索 8px/10px/12px/16px 圆角，统一收敛到 4-6px
  - [x] SubTask 13.3: 保留 QMessageBox 等系统弹窗的默认圆角或统一为 6px

- [x] Task 14: 状态标签与 Constants.h 颜色收敛
  - [x] SubTask 14.1: 修改 `src/models/Constants.h` 中 WorkOrderStatus::color 和 EventStatus::color，使用新设计系统低饱和色：Pending/PendingReview 用 #d97706，Processing/Accepted/Assigned/Reviewed 用 #2563eb，Completed/Evaluated 用 #15803d，Rejected/Escalated 用 #b91c1c，Closed/Archived 用 #64748b
  - [x] SubTask 14.2: 修改 `createTagItem()` 实现 4px 圆角（通过 QSS delegate 或在绘制时设置，如使用 QTableWidgetItem 无法直接圆角，可考虑用 QLabel 作为 cell widget，或保持现状并在返回中说明 Qt Widgets 限制）
  - [x] SubTask 14.3: 确保所有状态标签调用点颜色来自 Constants.h 或 moduleColor

- [x] Task 15: 统计图表配色统一
  - [x] SubTask 15.1: 在 createReportPage 中找到 `mkPieChart` 或类似未指定颜色的饼图辅助函数
  - [x] SubTask 15.2: 为所有饼图系列显式设置切片颜色，使用新配色：#b45309, #2563eb, #15803d, #64748b, #cbd5e1, #d97706, #0d9488, #7c3aed
  - [x] SubTask 15.3: 检查折线图/柱状图配色是否已统一

- [x] Task 16: 登录页与表单细节收敛
  - [x] SubTask 16.1: 将 LoginWidget.cpp 中登录输入框/按钮圆角统一为 4px
  - [x] SubTask 16.2: 将 AI 问答输入框等局部 8px 圆角输入框统一为 4px
  - [x] SubTask 16.3: 统一弹窗标题为 `font-weight: 600`（避免使用 bold/700）

- [x] Task 17: 运行时验证
  - [x] SubTask 17.1: 运行 `build/SmartCommunity.exe`，检查登录页与工作台
  - [x] SubTask 17.2: 逐页切换主要模块，确认无 QPainter 错误、无 QString::arg 警告
  - [x] SubTask 17.3: 使用 checklist.md 逐页核对视觉一致性，标记剩余偏差

# Task Dependencies

- Task 1 必须先完成（全局样式）
- Task 2 / Task 3 可并行，均依赖 Task 1
- Task 4 依赖 Task 1
- Task 5 / Task 6 可并行，依赖 Task 1
- Task 7 / Task 8 可并行，依赖 Task 5
- Task 9 / Task 10 / Task 11 可并行，依赖 Task 1
- Task 12 依赖所有前述任务
- Task 13 / Task 14 / Task 15 / Task 16 可并行，均依赖 Task 12
- Task 17 依赖 Task 13-16
