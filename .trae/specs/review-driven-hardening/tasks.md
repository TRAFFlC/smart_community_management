# Tasks

- [x] Task 1: 加固 AIService 上下文查询的权限过滤
  - [x] SubTask 1.1: 在 `AIService.h` 中为 `queryWorkOrderContext` 增加按 `user` 的权限过滤：居民域用户仅查 `create_by = user.id` 的工单；工作人员按 `dataScope` 限制范围（物业限本小区、社区限本社区等）
  - [x] SubTask 1.2: 为 `queryEventContext` 增加权限过滤：居民仅可见与自己相关的事件；工作人员按 `dataScope` 限制
  - [x] SubTask 1.3: 为 `queryResidentContext` 增加权限过滤：居民不应看到平台级居民总数/小区数/房屋总数等聚合数据，改为返回"您所在小区"的局部信息或权限提示；工作人员按 `dataScope` 限制
  - [x] SubTask 1.4: `queryOverviewContext` 调用上述方法时传递 `user`，确保权限过滤链路完整
  - [x] SubTask 1.5: 编译验证，确认无语法错误

- [x] Task 2: 升级密码哈希算法为 PBKDF2-SHA512 + 随机盐
  - [x] SubTask 2.1: 修改 `Utils::hashPassword`：生成 16 字节随机盐，使用 `QCryptographicHash` 或 `QPasswordDigestor`（Qt6）实现 PBKDF2-SHA512，迭代不少于 10000 轮，返回 `salt:hash` 格式字符串
  - [x] SubTask 2.2: 修改 `Utils::verifyPassword`：解析 `salt:hash` 格式，用相同参数重新计算并比对；若 `storedHash` 不含 `:` 则回退到旧 SHA256+固定盐算法（兼容已有数据）
  - [x] SubTask 2.3: 确认所有调用方（DemoDataService、SystemPage、MainWindow 用户管理、ChangePasswordDialog）接口签名未变，无需修改
  - [x] SubTask 2.4: 编译验证，确认无语法错误

- [x] Task 3: 加固 DatabaseManager 的 SQL 拼接
  - [x] SubTask 3.1: 在 `DatabaseManager.h` 中增加表名白名单（覆盖项目所有 `sys_`/`cm_`/`wo_`/`ge_`/`sv_`/`nt_`/`oc_`/`ev_`/`ai_`/`kf_`/`pm_` 前缀表）或正则校验函数 `isValidTableName`/`isValidColumnName`
  - [x] SubTask 3.2: 在 `insert()` 和 `update()` 中校验 `table` 和 `data` 的 key 合法性，校验失败打印警告并返回失败
  - [x] SubTask 3.3: 在 `count()` 中校验 `table` 合法性；在 `where` 参数的文档注释中明确要求调用方不得拼接用户输入
  - [x] SubTask 3.4: 编译验证，确认无语法错误

# Task Dependencies

- Task 1、Task 2、Task 3 互相独立，可并行实施。
- 所有 Task 完成后统一进行编译验证。

# 编译验证结论

- 编译阶段（.cpp → .obj）全部通过，本次修改的 3 个头文件（Utils.h、AIService.h、DatabaseManager.h）无语法错误。
- 链接阶段遇到**预先存在的重复定义问题**：`src/widgets/MainWindow.cpp` 与 `src/pages/{Service,Property,Governance,System,Dashboard,Report}Page.cpp` 中存在相同方法（如 `createServicePage`）的重复定义。这是项目重构过程中的遗留问题，与本次安全加固修改无关。
