# Checklist

## AIService 权限过滤
- [x] `queryWorkOrderContext` 对居民域用户仅返回 `create_by = user.id` 的工单，不再返回全平台工单总数和最近工单列表
- [x] `queryEventContext` 对居民用户做了权限限制，不再泄露全平台事件
- [x] `queryResidentContext` 对居民用户不再返回平台级居民总数/小区数/房屋总数等聚合数据
- [x] 工作人员用户的上下文查询按 `dataScope` 限制了数据范围
- [x] `queryOverviewContext` 调用链路中 `user` 参数被正确传递和使用

## 密码哈希升级
- [x] `Utils::hashPassword` 返回 `salt:hash` 格式，salt 为随机生成，hash 为 PBKDF2-SHA512 不少于 10000 轮迭代
- [x] `Utils::verifyPassword` 能正确验证新格式哈希
- [x] `Utils::verifyPassword` 对不含 `:` 的旧格式哈希回退到原 SHA256+固定盐算法，保证已有用户可登录
- [x] 所有调用 `Utils::hashPassword` / `Utils::verifyPassword` 的位置接口签名未变，无需修改
- [x] DemoDataService 初始化的用户密码使用新算法生成（接口不变，自动使用新算法）

## DatabaseManager SQL 加固
- [x] `insert()` / `update()` 校验 `table` 和列名合法性，非法输入返回失败并打印警告
- [x] `count()` 校验 `table` 合法性
- [x] 表名/列名校验逻辑覆盖项目所有已知表（sys_/cm_/wo_/ge_/sv_/nt_/oc_/ev_/ai_/kf_/pm_ 前缀，共 59 张表，与 schema.sql 交叉核对）
- [x] `where` 参数的注释明确要求调用方不得拼接用户输入

## 编译验证
- [x] 项目编译通过，无新增语法错误（编译阶段 .cpp → .obj 全部通过）
- [ ] 登录功能正常（用 admin/admin123 或种子数据账号验证）— 因预先存在的链接错误（MainWindow.cpp 与 pages/*.cpp 重复定义）无法生成可执行文件，待链接问题解决后验证
- [ ] AI 问答功能在居民账号下不再泄露全平台数据 — 同上，代码逻辑已确认正确，待链接问题解决后运行时验证

> **注**：链接错误为预先存在的问题（`src/widgets/MainWindow.cpp` 与 `src/pages/{Service,Property,Governance,System,Dashboard,Report}Page.cpp` 中存在相同方法的重复定义），与本次安全加固修改无关。本次修改的 3 个头文件编译阶段全部通过。
