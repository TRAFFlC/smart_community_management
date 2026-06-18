# Tasks

- [x] Task 1: API Key 加密存储
  - [x] SubTask 1.1: 在 `src/utils/Utils.h` 中新增 `simpleEncrypt/simpleDecrypt` 工具函数（使用 Qt 内置 `QCryptographicHash` + 固定派生密钥 + 简单可逆变换，不引入额外依赖）
  - [x] SubTask 1.2: 修改 `src/services/AIService.h` 的 `loadConfig()`：读取到 `ai_api_key` 时先尝试解密，若解密失败（旧明文）则视为明文并使用新算法加密后写回数据库
  - [x] SubTask 1.3: 修改 `src/services/AIService.h` 的 `saveConfig()`：写入 `ai_api_key` 前先加密
  - [x] SubTask 1.4: 编译验证，确认无新增语法错误

- [x] Task 2: 编号生成降低碰撞概率
  - [x] SubTask 2.1: 在 `src/utils/Utils.h` 中修改 `generateOrderNo()`：使用毫秒时间戳 + 6 位随机数 + 进程内自增序列（`static QAtomicInt`）
  - [x] SubTask 2.2: 同步修改 `generateEventNo()` 与 `generateServiceOrderNo()`，保持编号前缀不同、算法一致
  - [x] SubTask 2.3: 编译验证，确认无新增语法错误

- [x] Task 3: 数据库路径使用标准可写目录
  - [x] SubTask 3.1: 修改 `src/main.cpp`：使用 `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)` 构造数据库目录，并确保目录存在
  - [x] SubTask 3.2: 将数据库文件名保持为 `smart_community.db`，路径改为应用数据目录
  - [x] SubTask 3.3: 编译验证，确认无新增语法错误

- [x] Task 4: 移除 `using namespace UiKit` 全局污染
  - [x] SubTask 4.1: 修改 `src/widgets/MainWindow.cpp`：移除文件顶部 `using namespace UiKit;`，按需使用 `UiKit::` 前缀或局部 `using` 声明
  - [x] SubTask 4.2: 修改 `src/pages/*.cpp`（GovernancePage、SystemPage、PropertyPage、ServicePage、ReportPage、ArchivePage、DashboardTodoMessagePage、buildPropertySubPages）：移除 `using namespace UiKit;`，按需替换为 `UiKit::` 前缀
  - [x] SubTask 4.3: 编译验证，确认无新增语法错误

- [x] Task 5: 拆分 `PagesCommon.h` 中的 Qt Charts 依赖
  - [x] SubTask 5.1: 从 `src/PagesCommon.h` 中移除所有 Qt Charts 头（`QChart`、`QChartView`、`QPieSeries`、`QBarSeries`、`QBarSet`、`QLineSeries`、`QScatterSeries`、`QCategoryAxis`、`QValueAxis`、`QtCharts`）
  - [x] SubTask 5.2: 在 `src/pages/ReportPage.cpp` 顶部显式包含所需的 Qt Charts 头
  - [x] SubTask 5.3: 编译验证，确认 `ReportPage.cpp` 仍能正常编译且其他页面不再依赖 Charts 头

# Task Dependencies

- Task 1、Task 2、Task 3、Task 4、Task 5 互相独立，可并行实施。
- 所有 Task 已完成并统一通过项目编译验证（cmake + Release）。
