# Checklist

## API Key 加密存储
- [x] `Utils.h` 中新增加解密工具函数，不引入外部依赖
- [x] `AIService::loadConfig()` 读取到旧明文 `ai_api_key` 时能自动加密覆盖
- [x] `AIService::loadConfig()` 读取到密文时能正确解密为明文 `m_apiKey`
- [x] `AIService::saveConfig()` 写入数据库前对 `ai_api_key` 加密
- [x] AI 问答功能仍可正常调用（内存中 `m_apiKey` 为明文）

## 编号生成降低碰撞概率
- [x] `generateOrderNo()` 输出不再使用秒级时间戳 + 4 位随机数格式
- [x] `generateEventNo()` 与 `generateServiceOrderNo()` 使用与工单编号一致的改进算法
- [x] 并发/连续调用生成的编号不重复

## 数据库路径使用标准可写目录
- [x] `main.cpp` 使用 `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)` 作为数据库目录
- [x] 启动时自动创建该目录（若不存在）
- [x] 数据库文件名为 `smart_community.db`

## 命名空间按需引入
- [x] `MainWindow.cpp` 无文件顶部 `using namespace UiKit;`
- [x] 所有 `src/pages/*.cpp` 无文件顶部 `using namespace UiKit;`
- [x] 移除后项目仍能正常编译，UiKit 工具函数调用无歧义

## PagesCommon.h 拆分 Qt Charts 依赖
- [x] `PagesCommon.h` 中不再包含 Qt Charts 头
- [x] `ReportPage.cpp` 内部包含所需 Qt Charts 头
- [x] 项目编译通过

## 整体编译与回归
- [x] 项目编译通过，无新增语法/链接错误（cmake -B build -S . ; cmake --build build --config Release --parallel 4）
- [ ] 登录功能正常（建议运行时手动验证）
- [ ] 数据库初始化与种子数据写入正常（建议运行时手动验证）
