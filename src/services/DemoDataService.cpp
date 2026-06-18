#include "DemoDataService.h"

#include <QChar>
#include <QDate>
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QPair>
#include <QSqlQuery>
#include <QStringList>
#include <QVariantMap>

void DemoDataService::initIfEmpty()
{
    auto &db = DatabaseManager::instance();

    // 检查是否已有数据
    QSqlQuery q("SELECT COUNT(*) FROM sys_user");
    if (q.exec() && q.next() && q.value(0).toInt() > 0)
        return;

    qDebug() << "Initializing demo data...";

    initRoles(db);
    initMenus(db);
    initOrganizations(db);
    initUsers(db);
    initDictData(db);
    initArchiveData(db);
    initFamilyData(db);
    initWorkOrders(db);
    initEvents(db);
    initAnnouncements(db);
    initVolunteerData(db);
    initServiceData(db);
    initParkingData(db);
    initVehicleData(db);
    initPublicIncomeData(db);
    initTopicData(db);
    initKnowledgeBase(db);
    initEvaluationData(db);
    initAssessmentData(db);
    initVisitorData(db);
    initBillData(db);
    initMonthlyCardData(db);
    initOpinionData(db);
    initInspectionData(db);
    initVisitRecordData(db);
    initSupervisionData(db);
    initOperationLogData(db);
    initVoteData(db);
    initNotificationData(db);

    qDebug() << "Demo data initialized successfully";
}

void DemoDataService::initRoles(DatabaseManager &db)
{
    auto insert = [&](const QString &name, const QString &key, const QString &domain, int scope, int sort)
    {
        db.insert("sys_role", {{"role_name", name}, {"role_key", key}, {"role_domain", domain}, {"data_scope", scope}, {"sort_order", sort}, {"status", 0}});
    };

    // 居民域
    insert(QStringLiteral("居民户主"), "resident_owner", "resident", 6, 1);
    insert(QStringLiteral("家庭成员"), "resident_family", "resident", 6, 2);
    insert(QStringLiteral("租户"), "resident_tenant", "resident", 6, 3);
    insert(QStringLiteral("楼栋长"), "building_leader", "resident", 5, 4);
    insert(QStringLiteral("志愿者"), "volunteer", "resident", 6, 5);

    // 物业域
    insert(QStringLiteral("物业客服"), "property_cs", "property", 4, 10);
    insert(QStringLiteral("物业管家"), "property_steward", "property", 4, 11);
    insert(QStringLiteral("维修人员"), "property_repair", "property", 4, 12);
    insert(QStringLiteral("保安人员"), "property_security", "property", 4, 13);
    insert(QStringLiteral("物业经理"), "property_manager", "property", 4, 14);
    insert(QStringLiteral("物业管理员"), "property_admin", "property", 4, 15);

    // 治理域
    insert(QStringLiteral("网格员"), "grid_worker", "governance", 3, 20);
    insert(QStringLiteral("社区工作人员"), "community_worker", "governance", 3, 21);
    insert(QStringLiteral("社区书记"), "community_secretary", "governance", 3, 22);
    insert(QStringLiteral("街道工作人员"), "street_worker", "governance", 2, 23);
    insert(QStringLiteral("街道领导"), "street_leader", "governance", 2, 24);

    // 服务域
    insert(QStringLiteral("便民服务商"), "service_provider", "service", 7, 30);
    insert(QStringLiteral("社会组织管理员"), "social_org_admin", "service", 7, 31);

    // 监督域
    insert(QStringLiteral("业委会成员"), "oc_member", "supervision", 4, 40);
    insert(QStringLiteral("业委会主任"), "oc_director", "supervision", 4, 41);

    // 平台域
    insert(QStringLiteral("平台超级管理员"), "super_admin", "platform", 1, 50);
    insert(QStringLiteral("安全审计管理员"), "audit_admin", "platform", 1, 51);
    insert(QStringLiteral("数据分析管理员"), "data_admin", "platform", 1, 52);
}

void DemoDataService::initMenus(DatabaseManager &db)
{
    auto addDir = [&](int id, const QString &name, const QString &icon, int sort)
    {
        db.insert("sys_menu", {{"id", id}, {"parent_id", 0}, {"menu_name", name}, {"menu_type", 1}, {"icon", icon}, {"sort_order", sort}, {"visible", 1}});
    };
    auto addMenu = [&](int id, int pid, const QString &name, const QString &path, const QString &perm, const QString &icon, int sort)
    {
        db.insert("sys_menu", {{"id", id}, {"parent_id", pid}, {"menu_name", name}, {"menu_type", 2}, {"path", path}, {"permission", perm}, {"icon", icon}, {"sort_order", sort}, {"visible", 1}});
    };

    // 1. 工作台
    addDir(1, QStringLiteral("工作台"), "dashboard", 1);
    addMenu(101, 1, QStringLiteral("首页概览"), "/dashboard", "dashboard:view", "home", 1);
    addMenu(102, 1, QStringLiteral("待办中心"), "/todo", "todo:view", "list", 2);
    addMenu(103, 1, QStringLiteral("消息中心"), "/message", "message:view", "bell", 3);

    // 2. 基础档案
    addDir(2, QStringLiteral("基础档案"), "archive", 2);
    addMenu(201, 2, QStringLiteral("组织管理"), "/archive/org", "archive:org:list", "tree", 1);
    addMenu(202, 2, QStringLiteral("小区管理"), "/archive/estate", "archive:estate:list", "building", 2);
    addMenu(203, 2, QStringLiteral("楼栋房屋"), "/archive/house", "archive:house:list", "house", 3);
    addMenu(204, 2, QStringLiteral("居民档案"), "/archive/resident", "archive:resident:list", "people", 4);
    addMenu(205, 2, QStringLiteral("车辆管理"), "/archive/vehicle", "archive:vehicle:list", "car", 5);
    addMenu(206, 2, QStringLiteral("设施设备"), "/archive/facility", "archive:facility:list", "tool", 6);
    addMenu(207, 2, QStringLiteral("网格管理"), "/archive/grid", "archive:grid:list", "grid", 7);
    addMenu(208, 2, QStringLiteral("特殊群体"), "/archive/special", "archive:special:list", "heart", 8);

    // 3. 小区管理
    addDir(3, QStringLiteral("小区管理"), "community", 3);
    addMenu(301, 3, QStringLiteral("报事报修"), "/property/workorder", "property:wo:list", "repair", 1);
    addMenu(302, 3, QStringLiteral("投诉建议"), "/property/complaint", "property:complaint:list", "edit", 2);
    addMenu(303, 3, QStringLiteral("巡检巡查"), "/property/inspection", "property:inspection:list", "search", 3);
    addMenu(304, 3, QStringLiteral("公告通知"), "/property/announcement", "property:announce:list", "notice", 4);
    addMenu(305, 3, QStringLiteral("访客管理"), "/property/visitor", "property:visitor:list", "user", 5);
    addMenu(306, 3, QStringLiteral("业委会议题"), "/property/topic", "property:topic:list", "flag", 6);
    addMenu(307, 3, QStringLiteral("停车管理"), "/property/parking", "property:parking:list", "car", 7);
    addMenu(308, 3, QStringLiteral("物业缴费"), "/property/billing", "property:billing:list", "money", 8);
    addMenu(309, 3, QStringLiteral("公共收益"), "/property/income", "property:income:list", "chart", 9);

    // 4. 社区治理
    addDir(4, QStringLiteral("社区治理"), "governance", 4);
    addMenu(401, 4, QStringLiteral("事件中心"), "/governance/event", "governance:event:list", "alert", 1);
    addMenu(402, 4, QStringLiteral("巡查管理"), "/governance/inspection", "governance:inspection:list", "route", 2);
    addMenu(403, 4, QStringLiteral("重点关怀"), "/governance/care", "governance:care:list", "heart", 3);
    addMenu(404, 4, QStringLiteral("督办管理"), "/governance/supervision", "governance:supervision:list", "clock", 4);
    addMenu(405, 4, QStringLiteral("民意收集"), "/governance/opinion", "governance:opinion:list", "chat", 5);
    addMenu(406, 4, QStringLiteral("考核管理"), "/governance/assessment", "governance:assessment:list", "chart", 6);

    // 5. 社区服务
    addDir(5, QStringLiteral("社区服务"), "service", 5);
    addMenu(501, 5, QStringLiteral("志愿服务"), "/service/volunteer", "service:volunteer:list", "star", 1);
    addMenu(502, 5, QStringLiteral("便民服务"), "/service/convenience", "service:convenience:list", "shop", 2);
    addMenu(503, 5, QStringLiteral("就业服务"), "/service/job", "service:job:list", "briefcase", 3);

    // 6. 统计分析
    addDir(6, QStringLiteral("统计分析"), "report", 6);
    addMenu(601, 6, QStringLiteral("工单统计"), "/report/workorder", "report:wo:view", "pie", 1);
    addMenu(602, 6, QStringLiteral("事件统计"), "/report/event", "report:event:view", "bar", 2);
    addMenu(603, 6, QStringLiteral("服务统计"), "/report/service", "report:service:view", "line", 3);
    addMenu(604, 6, QStringLiteral("综合看板"), "/report/dashboard", "report:dashboard:view", "monitor", 4);

    // 7. 系统管理
    addDir(7, QStringLiteral("系统管理"), "system", 7);
    addMenu(701, 7, QStringLiteral("用户管理"), "/system/user", "system:user:list", "user", 1);
    addMenu(702, 7, QStringLiteral("角色管理"), "/system/role", "system:role:list", "peoples", 2);
    addMenu(703, 7, QStringLiteral("菜单管理"), "/system/menu", "system:menu:list", "tree-table", 3);
    addMenu(704, 7, QStringLiteral("数据字典"), "/system/dict", "system:dict:list", "dict", 4);
    addMenu(705, 7, QStringLiteral("操作日志"), "/system/log", "system:log:list", "log", 5);
    addMenu(706, 7, QStringLiteral("智能问答"), "/system/ai", "system:ai:list", "robot", 6);

    // 构建 role_key -> role_id 映射，避免依赖自增 ID 导致角色错位
    QHash<QString, qint64> roleIdByKey;
    {
        QSqlQuery rq("SELECT id, role_key FROM sys_role WHERE del_flag = 0");
        while (rq.next()) {
            roleIdByKey.insert(rq.value(1).toString(), rq.value(0).toLongLong());
        }
    }

    // 为超级管理员分配所有菜单
    qint64 superAdminRoleId = roleIdByKey.value(QStringLiteral("super_admin"));
    QSqlQuery q("SELECT id FROM sys_menu");
    while (q.next())
    {
        db.insert("sys_role_menu", {{"role_id", superAdminRoleId}, {"menu_id", q.value(0).toInt()}});
    }

    // 为其他角色分配合理菜单（按 role_key 绑定，避免自增 ID 漂移）
    auto assignMenusToRole = [&](const QString& roleKey, QList<int> menuIds)
    {
        qint64 roleId = roleIdByKey.value(roleKey);
        for (int mid : menuIds)
        {
            db.insert("sys_role_menu", {{"role_id", roleId}, {"menu_id", mid}});
        }
    };

    // 居民角色: 工作台+小区管理+社区服务+智能问答
    assignMenusToRole(QStringLiteral("resident_owner"), {1, 101, 102, 103, 3, 301, 302, 304, 307, 308, 5, 501, 502, 503, 7, 706});
    // 物业客服
    assignMenusToRole(QStringLiteral("property_cs"), {1, 101, 102, 103, 2, 202, 203, 204, 205, 3, 301, 302, 303, 304, 307, 308, 6, 601, 604});
    // 物业经理
    assignMenusToRole(QStringLiteral("property_manager"), {1, 101, 102, 103, 2, 202, 203, 204, 205, 206, 3, 301, 302, 303, 304, 305, 306, 307, 308, 309, 6, 601, 604});
    // 网格员
    assignMenusToRole(QStringLiteral("grid_worker"), {1, 101, 102, 103, 2, 204, 207, 208, 4, 401, 402, 403, 6, 602});
    // 社区工作人员
    assignMenusToRole(QStringLiteral("community_worker"), {1, 101, 102, 103, 2, 201, 202, 203, 204, 207, 208, 4, 401, 402, 403, 404, 405, 5, 501, 502, 6, 602, 604});
    // 社区书记
    assignMenusToRole(QStringLiteral("community_secretary"), {1, 101, 102, 103, 2, 201, 202, 203, 204, 207, 208, 4, 401, 402, 403, 404, 405, 406, 6, 602, 604});
    // 街道工作人员
    assignMenusToRole(QStringLiteral("street_worker"), {1, 101, 102, 103, 4, 401, 404, 406, 6, 602, 604});
    // 业委会成员
    assignMenusToRole(QStringLiteral("oc_member"), {1, 101, 103, 3, 304, 306, 309, 6, 604});
}

void DemoDataService::initOrganizations(DatabaseManager &db)
{
    // 街道
    qint64 streetId = db.insert("sys_org", {{"parent_id", 0}, {"org_name", QStringLiteral("和平街道")}, {"org_type", 2}, {"org_code", "ORG-ST-001"}, {"leader", QStringLiteral("张建国")}, {"phone", "0551-66661234"}, {"address", QStringLiteral("和平路100号")}, {"ancestors", "0"}, {"sort_order", 1}});

    // 社区
    qint64 comm1Id = db.insert("sys_org", {{"parent_id", streetId}, {"org_name", QStringLiteral("翠苑社区")}, {"org_type", 3}, {"org_code", "ORG-CM-001"}, {"leader", QStringLiteral("李芳")}, {"phone", "0551-66662345"}, {"address", QStringLiteral("翠苑路88号")}, {"ancestors", QString("0,%1").arg(streetId)}, {"sort_order", 1}});
    qint64 comm2Id = db.insert("sys_org", {{"parent_id", streetId}, {"org_name", QStringLiteral("金都社区")}, {"org_type", 3}, {"org_code", "ORG-CM-002"}, {"leader", QStringLiteral("王强")}, {"phone", "0551-66663456"}, {"address", QStringLiteral("金都大道200号")}, {"ancestors", QString("0,%1").arg(streetId)}, {"sort_order", 2}});

    // 物业公司
    qint64 propId = db.insert("sys_org", {{"parent_id", 0}, {"org_name", QStringLiteral("恒达物业管理有限公司")}, {"org_type", 4}, {"org_code", "ORG-WY-001"}, {"leader", QStringLiteral("陈明")}, {"phone", "0551-88881234"}, {"address", QStringLiteral("科技园路50号")}, {"ancestors", "0"}, {"sort_order", 10}});

    // 业委会
    db.insert("sys_org", {{"parent_id", 0}, {"org_name", QStringLiteral("翠苑小区业主委员会")}, {"org_type", 5}, {"org_code", "ORG-YW-001"}, {"leader", QStringLiteral("赵德胜")}, {"phone", "13800001001"}, {"ancestors", "0"}, {"sort_order", 20}});

    // 服务商
    db.insert("sys_org", {{"parent_id", 0}, {"org_name", QStringLiteral("好帮手家政服务公司")}, {"org_type", 6}, {"org_code", "ORG-FW-001"}, {"leader", QStringLiteral("孙丽")}, {"phone", "13800002001"}, {"ancestors", "0"}, {"sort_order", 30}});

    // 社会组织
    db.insert("sys_org", {{"parent_id", 0}, {"org_name", QStringLiteral("翠苑志愿者服务队")}, {"org_type", 7}, {"org_code", "ORG-SH-001"}, {"leader", QStringLiteral("周洁")}, {"phone", "13800003001"}, {"ancestors", "0"}, {"sort_order", 40}});

    Q_UNUSED(comm1Id)
    Q_UNUSED(comm2Id)
    Q_UNUSED(propId)
}

void DemoDataService::initUsers(DatabaseManager &db)
{
    // 构建 role_key -> role_id 映射，避免依赖自增 ID 导致角色错位
    QHash<QString, qint64> roleIdByKey;
    {
        QSqlQuery rq("SELECT id, role_key FROM sys_role WHERE del_flag = 0");
        while (rq.next()) {
            roleIdByKey.insert(rq.value(1).toString(), rq.value(0).toLongLong());
        }
    }

    auto addUser = [&](const QString &username, const QString &password,
                       const QString &realName, const QString &phone,
                       int userType, const QString &roleKey, qint64 orgId = 0)
    {
        qint64 uid = db.insert("sys_user", {{"username", username}, {"password", Utils::hashPassword(password)}, {"nickname", realName}, {"real_name", realName}, {"phone", phone}, {"user_type", userType}, {"status", 0}});
        db.insert("sys_user_role", {{"user_id", uid}, {"role_id", roleIdByKey.value(roleKey)}});
        if (orgId > 0)
        {
            db.insert("sys_user_org", {{"user_id", uid}, {"org_id", orgId}, {"is_primary", 1}});
        }
        return uid;
    };

    // 平台管理员
    addUser("admin", "admin123", QStringLiteral("系统管理员"), "13800000001", 3, QStringLiteral("super_admin"));

    // 居民
    qint64 r1 = addUser("zhangsan", "123456", QStringLiteral("张三"), "13800001001", 0, QStringLiteral("resident_owner"));
    addUser("lisi", "123456", QStringLiteral("李四"), "13800001002", 0, QStringLiteral("resident_owner"));
    addUser("wangwu", "123456", QStringLiteral("王五(租户)"), "13800001003", 0, QStringLiteral("resident_tenant"));
    addUser("liulou", "123456", QStringLiteral("刘楼长"), "13800001004", 0, QStringLiteral("building_leader"));

    // 物业
    addUser("wuye_kefu", "123456", QStringLiteral("小美(客服)"), "13800002001", 1, QStringLiteral("property_cs"));
    addUser("wuye_guanjia", "123456", QStringLiteral("陈管家"), "13800002002", 1, QStringLiteral("property_steward"));
    addUser("wuye_weixiu", "123456", QStringLiteral("老张(维修)"), "13800002003", 1, QStringLiteral("property_repair"));
    addUser("wuye_baoan", "123456", QStringLiteral("小李(保安)"), "13800002004", 1, QStringLiteral("property_security"));
    addUser("wuye_jingli", "123456", QStringLiteral("陈明(经理)"), "13800002005", 1, QStringLiteral("property_manager"));

    // 社区治理
    addUser("wangge_zhao", "123456", QStringLiteral("赵网格"), "13800003001", 1, QStringLiteral("grid_worker"));
    addUser("shequ_lifang", "123456", QStringLiteral("李芳"), "13800003002", 1, QStringLiteral("community_worker"));
    addUser("shequ_shuji", "123456", QStringLiteral("周书记"), "13800003003", 1, QStringLiteral("community_secretary"));
    addUser("jiedao_worker", "123456", QStringLiteral("吴干事"), "13800003004", 1, QStringLiteral("street_worker"));
    addUser("jiedao_leader", "123456", QStringLiteral("马主任"), "13800003005", 1, QStringLiteral("street_leader"));

    // 业委会
    addUser("yewei_zhao", "123456", QStringLiteral("赵德胜"), "13800004001", 0, QStringLiteral("oc_member"));

    // 服务商
    addUser("fuwu_sun", "123456", QStringLiteral("孙丽"), "13800005001", 2, QStringLiteral("service_provider"));

    // 为张三创建居民档案和关联
    db.insert("cm_resident", {{"user_id", r1}, {"name", QStringLiteral("张三")}, {"phone", "13800001001"}, {"phone_display", "138****1001"}, {"gender", 1}, {"nationality", QStringLiteral("汉族")}, {"occupation", QStringLiteral("软件工程师")}, {"birthday", "1990-03-15"}, {"education", QStringLiteral("本科")}, {"id_card", "340104199003150000"}, {"political_status", QStringLiteral("群众")}, {"status", 0}});
}

void DemoDataService::initDictData(DatabaseManager &db)
{
    auto addDictType = [&](const QString &name, const QString &type)
    {
        db.insert("sys_dict_type", {{"dict_name", name}, {"dict_type", type}});
    };
    auto addDictItem = [&](const QString &type, const QString &label, const QString &value, int sort)
    {
        db.insert("sys_dict_data", {{"dict_type", type}, {"dict_label", label}, {"dict_value", value}, {"sort_order", sort}});
    };

    addDictType(QStringLiteral("用户类型"), "sys_user_type");
    addDictItem("sys_user_type", QStringLiteral("居民"), "0", 1);
    addDictItem("sys_user_type", QStringLiteral("工作人员"), "1", 2);
    addDictItem("sys_user_type", QStringLiteral("服务商"), "2", 3);
    addDictItem("sys_user_type", QStringLiteral("管理员"), "3", 4);

    addDictType(QStringLiteral("组织类型"), "sys_org_type");
    addDictItem("sys_org_type", QStringLiteral("平台"), "1", 1);
    addDictItem("sys_org_type", QStringLiteral("街道"), "2", 2);
    addDictItem("sys_org_type", QStringLiteral("社区"), "3", 3);
    addDictItem("sys_org_type", QStringLiteral("物业公司"), "4", 4);
    addDictItem("sys_org_type", QStringLiteral("业委会"), "5", 5);
    addDictItem("sys_org_type", QStringLiteral("服务商"), "6", 6);
    addDictItem("sys_org_type", QStringLiteral("社会组织"), "7", 7);

    addDictType(QStringLiteral("房屋状态"), "cm_house_status");
    addDictItem("cm_house_status", QStringLiteral("空置"), "0", 1);
    addDictItem("cm_house_status", QStringLiteral("自住"), "1", 2);
    addDictItem("cm_house_status", QStringLiteral("出租"), "2", 3);
    addDictItem("cm_house_status", QStringLiteral("已售"), "3", 4);

    addDictType(QStringLiteral("工单类型"), "wo_order_type");
    addDictItem("wo_order_type", QStringLiteral("水电维修"), "1", 1);
    addDictItem("wo_order_type", QStringLiteral("公共设施"), "2", 2);
    addDictItem("wo_order_type", QStringLiteral("环境卫生"), "3", 3);
    addDictItem("wo_order_type", QStringLiteral("安全秩序"), "4", 4);
    addDictItem("wo_order_type", QStringLiteral("其他"), "5", 5);

    addDictType(QStringLiteral("工单优先级"), "wo_priority");
    addDictItem("wo_priority", QStringLiteral("普通"), "1", 1);
    addDictItem("wo_priority", QStringLiteral("紧急"), "2", 2);
    addDictItem("wo_priority", QStringLiteral("特急"), "3", 3);

    addDictType(QStringLiteral("工单状态"), "wo_status");
    addDictItem("wo_status", QStringLiteral("待受理"), "0", 1);
    addDictItem("wo_status", QStringLiteral("已受理"), "1", 2);
    addDictItem("wo_status", QStringLiteral("已派单"), "2", 3);
    addDictItem("wo_status", QStringLiteral("处理中"), "3", 4);
    addDictItem("wo_status", QStringLiteral("已完成"), "4", 5);
    addDictItem("wo_status", QStringLiteral("已关闭"), "5", 6);
    addDictItem("wo_status", QStringLiteral("已评价"), "6", 7);
    addDictItem("wo_status", QStringLiteral("已退回"), "7", 8);

    addDictType(QStringLiteral("事件类别"), "ge_event_category");
    addDictItem("ge_event_category", QStringLiteral("民生服务"), "1", 1);
    addDictItem("ge_event_category", QStringLiteral("环境卫生"), "2", 2);
    addDictItem("ge_event_category", QStringLiteral("设施安全"), "3", 3);
    addDictItem("ge_event_category", QStringLiteral("邻里纠纷"), "4", 4);
    addDictItem("ge_event_category", QStringLiteral("特殊帮扶"), "5", 5);
    addDictItem("ge_event_category", QStringLiteral("市容秩序"), "6", 6);
    addDictItem("ge_event_category", QStringLiteral("突发预警"), "7", 7);

    addDictType(QStringLiteral("事件状态"), "ge_event_status");
    addDictItem("ge_event_status", QStringLiteral("待审核"), "0", 1);
    addDictItem("ge_event_status", QStringLiteral("已审核"), "1", 2);
    addDictItem("ge_event_status", QStringLiteral("已分派"), "2", 3);
    addDictItem("ge_event_status", QStringLiteral("处理中"), "3", 4);
    addDictItem("ge_event_status", QStringLiteral("已完成"), "4", 5);
    addDictItem("ge_event_status", QStringLiteral("已退回"), "5", 6);
    addDictItem("ge_event_status", QStringLiteral("已归档"), "6", 7);
    addDictItem("ge_event_status", QStringLiteral("已升级"), "7", 8);

    addDictType(QStringLiteral("公告类型"), "nt_announcement_type");
    addDictItem("nt_announcement_type", QStringLiteral("小区公告"), "1", 1);
    addDictItem("nt_announcement_type", QStringLiteral("社区公告"), "2", 2);
    addDictItem("nt_announcement_type", QStringLiteral("物业公告"), "3", 3);
    addDictItem("nt_announcement_type", QStringLiteral("系统公告"), "4", 4);
}

void DemoDataService::initArchiveData(DatabaseManager &db)
{
    // 获取物业公司ID
    QSqlQuery pq("SELECT id FROM sys_org WHERE org_type = 4 LIMIT 1");
    qint64 propOrgId = 0;
    if (pq.next())
        propOrgId = pq.value(0).toLongLong();

    // 物业公司档案
    qint64 propCompanyId = db.insert("cm_property_company", {{"org_id", propOrgId}, {"company_name", QStringLiteral("恒达物业管理有限公司")}, {"legal_person", QStringLiteral("陈明")}, {"contact_phone", "0551-88881234"}, {"service_scope", QStringLiteral("物业管理、维修、保洁、安保")}});

    // 获取社区ID
    QSqlQuery cq("SELECT id FROM sys_org WHERE org_type = 3 ORDER BY sort_order");
    qint64 comm1Id = 0, comm2Id = 0;
    if (cq.next())
        comm1Id = cq.value(0).toLongLong();
    if (cq.next())
        comm2Id = cq.value(0).toLongLong();

    // 小区
    qint64 estate1Id = db.insert("cm_estate", {{"org_id", comm1Id}, {"property_company_id", propCompanyId}, {"estate_name", QStringLiteral("翠苑小区")}, {"estate_code", "EST-001"}, {"address", QStringLiteral("翠苑路88号")}, {"total_area", 50000.0}, {"total_buildings", 6}, {"total_houses", 360}, {"built_year", 2010}, {"green_rate", 35.0}, {"parking_spaces", 200}});

    qint64 estate2Id = db.insert("cm_estate", {{"org_id", comm2Id}, {"property_company_id", propCompanyId}, {"estate_name", QStringLiteral("金都花园")}, {"estate_code", "EST-002"}, {"address", QStringLiteral("金都大道200号")}, {"total_area", 80000.0}, {"total_buildings", 10}, {"total_houses", 600}, {"built_year", 2015}, {"green_rate", 40.0}, {"parking_spaces", 400}});

    // 楼栋
    for (int i = 1; i <= 3; i++)
    {
        qint64 bldId = db.insert("cm_building", {{"estate_id", estate1Id}, {"building_name", QString::number(i) + QStringLiteral("号楼")}, {"building_code", QString("BLD-001-%1").arg(i, 2, 10, QChar('0'))}, {"total_units", 2}, {"total_floors", 18}, {"has_elevator", 1}, {"elevator_count", 2}});

        // 单元
        for (int u = 1; u <= 2; u++)
        {
            qint64 unitId = db.insert("cm_unit", {{"building_id", bldId}, {"unit_name", QString::number(u) + QStringLiteral("单元")}, {"unit_code", QString("UNIT-001-%1-%2").arg(i).arg(u)}});

            // 房屋(每单元每层2户)
            for (int f = 1; f <= 3; f++)
            {
                for (int r = 1; r <= 2; r++)
                {
                    QString code = QString("%1-%2-%3%4").arg(i).arg(u).arg(f).arg(r);
                    int hs = (f + r) % 4;
                    db.insert("cm_house", {{"unit_id", unitId}, {"estate_id", estate1Id}, {"house_code", code}, {"floor", f}, {"room_number", QString::number(f * 100 + r)}, {"area", 80.0 + (f + r) * 5}, {"house_type", QStringLiteral("两室一厅")}, {"house_status", hs}});
                }
            }
        }
    }

    // 金都花园简化楼栋
    for (int i = 1; i <= 2; i++)
    {
        qint64 bldId = db.insert("cm_building", {{"estate_id", estate2Id}, {"building_name", QString::number(i) + QStringLiteral("号楼")}, {"building_code", QString("BLD-002-%1").arg(i, 2, 10, QChar('0'))}, {"total_units", 1}, {"total_floors", 24}, {"has_elevator", 1}, {"elevator_count", 2}});
        qint64 unitId = db.insert("cm_unit", {{"building_id", bldId}, {"unit_name", QStringLiteral("1单元")}, {"unit_code", QString("UNIT-002-%1-1").arg(i)}});
        for (int f = 1; f <= 3; f++)
        {
            for (int r = 1; r <= 2; r++)
            {
                db.insert("cm_house", {{"unit_id", unitId}, {"estate_id", estate2Id}, {"house_code", QString("B%1-%2%3").arg(i).arg(f).arg(r)}, {"floor", f}, {"room_number", QString::number(f * 100 + r)}, {"area", 100.0}, {"house_type", QStringLiteral("三室两厅")}, {"house_status", 1}});
            }
        }
    }

    // 网格
    QSqlQuery gq("SELECT id FROM sys_user WHERE username = 'wangge_zhao'");
    qint64 gridWorkerId = 0;
    if (gq.next())
        gridWorkerId = gq.value(0).toLongLong();

    db.insert("cm_grid", {{"community_org_id", comm1Id}, {"grid_name", QStringLiteral("翠苑第一网格")}, {"grid_code", "GRID-001"}, {"description", QStringLiteral("翠苑小区1-3号楼范围")}, {"grid_worker_id", gridWorkerId}, {"cover_estates", QString("[%1]").arg(estate1Id)}});
    db.insert("cm_grid", {{"community_org_id", comm2Id}, {"grid_name", QStringLiteral("金都第一网格")}, {"grid_code", "GRID-002"}, {"description", QStringLiteral("金都花园1-5号楼范围")}, {"grid_worker_id", gridWorkerId}, {"cover_estates", QString("[%1]").arg(estate2Id)}});

    // 业委会
    QSqlQuery ocq("SELECT id FROM sys_org WHERE org_type = 5 LIMIT 1");
    qint64 ocOrgId = 0;
    if (ocq.next())
        ocOrgId = ocq.value(0).toLongLong();

    db.insert("cm_owner_committee", {{"org_id", ocOrgId}, {"estate_id", estate1Id}, {"committee_name", QStringLiteral("翠苑小区业主委员会")}, {"term_start", QDate(2025, 1, 1)}, {"term_end", QDate(2028, 1, 1)}, {"member_count", 5}});

    // 设施设备
    db.insert("cm_facility", {{"estate_id", estate1Id}, {"facility_name", QStringLiteral("1号楼电梯A")}, {"facility_type", 1}, {"facility_code", "FAC-E-001"}, {"location", QStringLiteral("1号楼")}, {"status", 0}});
    db.insert("cm_facility", {{"estate_id", estate1Id}, {"facility_name", QStringLiteral("1号楼电梯B")}, {"facility_type", 1}, {"facility_code", "FAC-E-002"}, {"location", QStringLiteral("1号楼")}, {"status", 0}});
    db.insert("cm_facility", {{"estate_id", estate1Id}, {"facility_name", QStringLiteral("消防栓-001")}, {"facility_type", 2}, {"facility_code", "FAC-F-001"}, {"location", QStringLiteral("1号楼大厅")}, {"status", 0}});
    db.insert("cm_facility", {{"estate_id", estate1Id}, {"facility_name", QStringLiteral("门禁-A01")}, {"facility_type", 3}, {"facility_code", "FAC-D-001"}, {"location", QStringLiteral("小区南门")}, {"status", 0}});

    // 车位
    for (int i = 1; i <= 10; i++)
    {
        db.insert("cm_parking_space", {{"estate_id", estate1Id}, {"space_code", QString("P-A-%1").arg(i, 3, 10, QChar('0'))}, {"area_name", QStringLiteral("A区")}, {"space_type", 1}, {"status", i <= 7 ? 1 : 0}});
    }

    // 特殊群体
    QSqlQuery rq("SELECT id FROM cm_resident LIMIT 1");
    if (rq.next())
    {
        qint64 rid = rq.value(0).toLongLong();
        db.insert("cm_special_group", {{"resident_id", rid}, {"group_type", 1}, {"detail", QStringLiteral("独居老人，子女在外地")}, {"care_level", 2}, {"care_frequency", "weekly"}});
    }

    // 新增8条居民档案
    struct ResidentInfo {
        const char *name; const char *phone; const char *phoneDisplay;
        int gender; const char *nationality; const char *occupation;
        const char *birthday; const char *education; const char *idCard;
        const char *politicalStatus;
    };
    ResidentInfo residents[] = {
        {"李四", "13800001002", "138****1002", 1, "汉族", "销售经理", "1985-07-22", "本科", "340104198507220000", "群众"},
        {"王五", "13800001003", "138****1003", 1, "汉族", "建筑工人", "1992-11-08", "高中", "340104199211080000", "群众"},
        {"赵六", "13800001005", "138****1005", 0, "汉族", "小学教师", "1988-04-12", "本科", "340104198804120000", "中共党员"},
        {"孙七", "13800001006", "138****1006", 1, "回族", "医生", "1975-09-30", "硕士", "340104197509300000", "群众"},
        {"周八", "13800001007", "138****1007", 0, "汉族", "退休会计", "1958-02-18", "大专", "340104195802180000", "中共党员"},
        {"吴九", "13800001008", "138****1008", 1, "汉族", "快递员", "1995-06-25", "大专", "340104199506250000", "群众"},
        {"郑十", "13800001009", "138****1009", 0, "汉族", "家庭主妇", "1990-12-03", "高中", "340104199012030000", "群众"},
        {"陈十一", "13800001010", "138****1010", 1, "汉族", "大学生", "2002-08-15", "在读本科", "340104200208150000", "共青团员"},
    };
    QList<qint64> residentIds;
    // 张三的ID
    QSqlQuery zsRq("SELECT id FROM cm_resident WHERE name = '张三' LIMIT 1");
    if (zsRq.next()) residentIds.append(zsRq.value(0).toLongLong());

    for (int i = 0; i < 8; i++)
    {
        QVariantMap data;
        data["name"] = QString::fromUtf8(residents[i].name);
        data["phone"] = QString::fromUtf8(residents[i].phone);
        data["phone_display"] = QString::fromUtf8(residents[i].phoneDisplay);
        data["gender"] = residents[i].gender;
        data["nationality"] = QString::fromUtf8(residents[i].nationality);
        data["occupation"] = QString::fromUtf8(residents[i].occupation);
        data["birthday"] = QString::fromUtf8(residents[i].birthday);
        data["education"] = QString::fromUtf8(residents[i].education);
        data["id_card"] = QString::fromUtf8(residents[i].idCard);
        data["political_status"] = QString::fromUtf8(residents[i].politicalStatus);
        data["status"] = 0;
        qint64 rid = db.insert("cm_resident", data);
        residentIds.append(rid);
    }

    Q_UNUSED(estate1Id)
    Q_UNUSED(estate2Id)
}

void DemoDataService::initFamilyData(DatabaseManager &db)
{
    // 获取房屋ID
    QSqlQuery hq("SELECT id FROM cm_house WHERE del_flag = 0 LIMIT 6");
    QList<qint64> houseIds;
    while (hq.next()) houseIds.append(hq.value(0).toLongLong());
    if (houseIds.size() < 3) return;

    // 获取居民ID
    QSqlQuery rq("SELECT id, name FROM cm_resident WHERE del_flag = 0 ORDER BY id LIMIT 9");
    QList<QPair<qint64, QString>> residentList;
    while (rq.next()) residentList.append({rq.value(0).toLongLong(), rq.value(1).toString()});
    if (residentList.size() < 9) return;

    // 家庭1: 张家 (residentList[0]=张三, [6]=郑十, [7]=陈十一)
    qint64 fam1Id = db.insert("cm_family", {
        {"family_name", QStringLiteral("张三家庭")}, {"house_id", houseIds[0]},
        {"head_resident_id", residentList[0].first}, {"member_count", 3}
    });
    db.insert("cm_family_member", {{"family_id", fam1Id}, {"resident_id", residentList[0].first}, {"relation", QStringLiteral("户主")}, {"is_head", 1}});
    db.insert("cm_family_member", {{"family_id", fam1Id}, {"resident_id", residentList[6].first}, {"relation", QStringLiteral("配偶")}, {"is_head", 0}});
    db.insert("cm_family_member", {{"family_id", fam1Id}, {"resident_id", residentList[7].first}, {"relation", QStringLiteral("子女")}, {"is_head", 0}});
    // 关联房屋
    db.insert("cm_house_resident", {{"house_id", houseIds[0]}, {"resident_id", residentList[0].first}, {"relation_type", 1}});
    db.insert("cm_house_resident", {{"house_id", houseIds[0]}, {"resident_id", residentList[6].first}, {"relation_type", 2}});
    db.insert("cm_house_resident", {{"house_id", houseIds[0]}, {"resident_id", residentList[7].first}, {"relation_type", 3}});

    // 家庭2: 李家 (residentList[1]=李四, [3]=赵六, [4]=孙七, [5]=周八)
    qint64 fam2Id = db.insert("cm_family", {
        {"family_name", QStringLiteral("李四家庭")}, {"house_id", houseIds[1]},
        {"head_resident_id", residentList[1].first}, {"member_count", 4}
    });
    db.insert("cm_family_member", {{"family_id", fam2Id}, {"resident_id", residentList[1].first}, {"relation", QStringLiteral("户主")}, {"is_head", 1}});
    db.insert("cm_family_member", {{"family_id", fam2Id}, {"resident_id", residentList[3].first}, {"relation", QStringLiteral("配偶")}, {"is_head", 0}});
    db.insert("cm_family_member", {{"family_id", fam2Id}, {"resident_id", residentList[4].first}, {"relation", QStringLiteral("父母")}, {"is_head", 0}});
    db.insert("cm_family_member", {{"family_id", fam2Id}, {"resident_id", residentList[5].first}, {"relation", QStringLiteral("父母")}, {"is_head", 0}});
    db.insert("cm_house_resident", {{"house_id", houseIds[1]}, {"resident_id", residentList[1].first}, {"relation_type", 1}});
    db.insert("cm_house_resident", {{"house_id", houseIds[1]}, {"resident_id", residentList[3].first}, {"relation_type", 2}});
    db.insert("cm_house_resident", {{"house_id", houseIds[1]}, {"resident_id", residentList[4].first}, {"relation_type", 3}});

    // 家庭3: 王家 (residentList[2]=王五, [8]=吴九)
    qint64 fam3Id = db.insert("cm_family", {
        {"family_name", QStringLiteral("王五家庭")}, {"house_id", houseIds[2]},
        {"head_resident_id", residentList[2].first}, {"member_count", 3}
    });
    db.insert("cm_family_member", {{"family_id", fam3Id}, {"resident_id", residentList[2].first}, {"relation", QStringLiteral("户主")}, {"is_head", 1}});
    db.insert("cm_family_member", {{"family_id", fam3Id}, {"resident_id", residentList[8].first}, {"relation", QStringLiteral("兄弟")}, {"is_head", 0}});
    db.insert("cm_house_resident", {{"house_id", houseIds[2]}, {"resident_id", residentList[2].first}, {"relation_type", 1}});
    db.insert("cm_house_resident", {{"house_id", houseIds[2]}, {"resident_id", residentList[8].first}, {"relation_type", 4}});
}

void DemoDataService::initWorkOrders(DatabaseManager &db)
{
    QSqlQuery rq("SELECT id FROM sys_user WHERE username = 'zhangsan'");
    qint64 reporterId = 0;
    QString reporterName;
    if (rq.next())
    {
        reporterId = rq.value(0).toLongLong();
        reporterName = QStringLiteral("张三");
    }

    QSqlQuery aq("SELECT id FROM sys_user WHERE username = 'wuye_kefu'");
    qint64 acceptId = 0;
    if (aq.next())
        acceptId = aq.value(0).toLongLong();

    QSqlQuery wq("SELECT id FROM sys_user WHERE username = 'wuye_weixiu'");
    qint64 workerId = 0;
    if (wq.next())
        workerId = wq.value(0).toLongLong();

    QSqlQuery eq("SELECT id FROM cm_estate LIMIT 1");
    qint64 estateId = 0;
    if (eq.next())
        estateId = eq.value(0).toLongLong();

    QSqlQuery hq("SELECT id FROM cm_house LIMIT 1");
    qint64 houseId = 0;
    if (hq.next())
        houseId = hq.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();

    // 已完成工单
    db.insert("wo_work_order", {{"order_no", Utils::generateOrderNo()}, {"estate_id", estateId}, {"house_id", houseId}, {"reporter_id", reporterId}, {"reporter_name", reporterName}, {"reporter_phone", "138****1001"}, {"order_type", 1}, {"priority", 1}, {"title", QStringLiteral("厨房水管漏水")}, {"description", QStringLiteral("厨房水龙头下方管道有渗水现象")}, {"status", 6}, {"accept_by", acceptId}, {"accept_time", now.addDays(-3)}, {"assign_to", workerId}, {"assign_time", now.addDays(-3)}, {"finish_time", now.addDays(-2)}, {"sla_deadline", now.addDays(-1)}, {"source", 0}});

    // 处理中工单
    db.insert("wo_work_order", {{"order_no", Utils::generateOrderNo()}, {"estate_id", estateId}, {"house_id", houseId}, {"reporter_id", reporterId}, {"reporter_name", reporterName}, {"reporter_phone", "138****1001"}, {"order_type", 2}, {"priority", 2}, {"title", QStringLiteral("楼道灯不亮")}, {"description", QStringLiteral("2楼到3楼之间的楼道灯已坏，晚上出行不便")}, {"status", 3}, {"accept_by", acceptId}, {"accept_time", now.addDays(-1)}, {"assign_to", workerId}, {"assign_time", now.addDays(-1)}, {"sla_deadline", now.addSecs(4 * 3600)}, {"source", 0}});

    // 待受理工单
    db.insert("wo_work_order", {{"order_no", Utils::generateOrderNo()}, {"estate_id", estateId}, {"house_id", houseId}, {"reporter_id", reporterId}, {"reporter_name", reporterName}, {"reporter_phone", "138****1001"}, {"order_type", 3}, {"priority", 1}, {"title", QStringLiteral("小区绿化带垃圾堆积")}, {"description", QStringLiteral("3号楼前绿化带内有人丢弃生活垃圾，影响环境")}, {"status", 0}, {"sla_deadline", now.addDays(2)}, {"source", 0}});

    // 更多已完成工单(统计数据用)
    for (int i = 0; i < 5; i++)
    {
        int type = (i % 5) + 1;
        db.insert("wo_work_order", {{"order_no", Utils::generateOrderNo()}, {"estate_id", estateId}, {"house_id", houseId}, {"reporter_id", reporterId}, {"reporter_name", reporterName}, {"order_type", type}, {"priority", 1}, {"title", QStringLiteral("历史工单-") + QString::number(i + 1)}, {"description", QStringLiteral("测试历史工单数据")}, {"status", 4}, {"accept_by", acceptId}, {"accept_time", now.addDays(-10 - i)}, {"assign_to", workerId}, {"assign_time", now.addDays(-10 - i)}, {"finish_time", now.addDays(-8 - i)}, {"sla_deadline", now.addDays(-8 - i)}, {"source", 0}});
    }

    // 投诉工单(source=2) - 投诉建议页面使用 WHERE source = 2
    db.insert("wo_work_order", {{"order_no", Utils::generateOrderNo()}, {"estate_id", estateId}, {"house_id", houseId}, {"reporter_id", reporterId}, {"reporter_name", reporterName}, {"reporter_phone", "138****1001"}, {"order_type", 0}, {"priority", 2}, {"title", QStringLiteral("物业态度恶劣，保安辱骂业主")}, {"description", QStringLiteral("小区南门保安对进出业主态度恶劣，多次发生言语冲突")}, {"status", 3}, {"accept_by", acceptId}, {"accept_time", now.addDays(-2)}, {"assign_to", workerId}, {"assign_time", now.addDays(-2)}, {"sla_deadline", now.addDays(1)}, {"source", 2}});

    db.insert("wo_work_order", {{"order_no", Utils::generateOrderNo()}, {"estate_id", estateId}, {"house_id", houseId}, {"reporter_id", reporterId}, {"reporter_name", reporterName}, {"reporter_phone", "138****1001"}, {"order_type", 1}, {"priority", 1}, {"title", QStringLiteral("建议在小区增设垃圾分类指导站")}, {"description", QStringLiteral("目前垃圾分类投放点标识不清，建议增设指导站并安排专人指导")}, {"status", 1}, {"accept_by", acceptId}, {"accept_time", now.addDays(-1)}, {"sla_deadline", now.addDays(3)}, {"source", 2}});

    db.insert("wo_work_order", {{"order_no", Utils::generateOrderNo()}, {"estate_id", estateId}, {"house_id", houseId}, {"reporter_id", reporterId}, {"reporter_name", reporterName}, {"reporter_phone", "138****1001"}, {"order_type", 0}, {"priority", 1}, {"title", QStringLiteral("楼上住户深夜扰民")}, {"description", QStringLiteral("2号楼501住户经常深夜大声喧哗，严重影响楼下居民休息")}, {"status", 4}, {"accept_by", acceptId}, {"accept_time", now.addDays(-5)}, {"assign_to", workerId}, {"assign_time", now.addDays(-5)}, {"finish_time", now.addDays(-3)}, {"sla_deadline", now.addDays(-2)}, {"source", 2}});
}

void DemoDataService::initEvents(DatabaseManager &db)
{
    QSqlQuery gq("SELECT id FROM sys_user WHERE username = 'wangge_zhao'");
    qint64 reporterId = 0;
    QString reporterName;
    if (gq.next())
    {
        reporterId = gq.value(0).toLongLong();
        reporterName = QStringLiteral("赵网格");
    }

    QSqlQuery sq("SELECT id FROM sys_user WHERE username = 'shequ_lifang'");
    qint64 reviewerId = 0;
    if (sq.next())
        reviewerId = sq.value(0).toLongLong();

    QSqlQuery gridQ("SELECT id FROM cm_grid LIMIT 1");
    qint64 gridId = 0;
    if (gridQ.next())
        gridId = gridQ.value(0).toLongLong();

    QSqlQuery cq("SELECT id FROM sys_org WHERE org_type = 3 LIMIT 1");
    qint64 commOrgId = 0;
    if (cq.next())
        commOrgId = cq.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();

    // 已归档事件
    qint64 evId = db.insert("ge_event", {{"event_no", Utils::generateEventNo()}, {"grid_id", gridId}, {"community_org_id", commOrgId}, {"reporter_id", reporterId}, {"reporter_name", reporterName}, {"event_category", 2}, {"priority", 1}, {"title", QStringLiteral("小区门口道路破损")}, {"description", QStringLiteral("翠苑小区南门入口处路面有破损，存在安全隐患")}, {"location", QStringLiteral("翠苑小区南门")}, {"status", 6}, {"reviewer_id", reviewerId}, {"review_time", now.addDays(-7)}, {"assign_to", reviewerId}, {"assign_time", now.addDays(-7)}, {"finish_time", now.addDays(-5)}, {"archive_time", now.addDays(-4)}, {"result_desc", QStringLiteral("已联系市政部门修复路面")}, {"sla_deadline", now.addDays(-4)}, {"source", 0}});

    // 事件流转记录
    db.insert("ge_event_flow", {{"event_id", evId}, {"action", QStringLiteral("上报")}, {"operator_id", reporterId}, {"operator_name", reporterName}, {"from_status", -1}, {"to_status", 0}, {"action_time", now.addDays(-8)}});
    db.insert("ge_event_flow", {{"event_id", evId}, {"action", QStringLiteral("审核")}, {"operator_id", reviewerId}, {"operator_name", QStringLiteral("李芳")}, {"from_status", 0}, {"to_status", 1}, {"action_time", now.addDays(-7)}});
    db.insert("ge_event_flow", {{"event_id", evId}, {"action", QStringLiteral("分派")}, {"operator_id", reviewerId}, {"operator_name", QStringLiteral("李芳")}, {"from_status", 1}, {"to_status", 2}, {"action_time", now.addDays(-7)}});
    db.insert("ge_event_flow", {{"event_id", evId}, {"action", QStringLiteral("处理")}, {"operator_id", reviewerId}, {"operator_name", QStringLiteral("李芳")}, {"from_status", 3}, {"to_status", 4}, {"action_time", now.addDays(-5)}});
    db.insert("ge_event_flow", {{"event_id", evId}, {"action", QStringLiteral("归档")}, {"operator_id", reviewerId}, {"operator_name", QStringLiteral("李芳")}, {"from_status", 4}, {"to_status", 6}, {"action_time", now.addDays(-4)}});

    // 处理中事件
    qint64 ev2Id = db.insert("ge_event", {{"event_no", Utils::generateEventNo()}, {"grid_id", gridId}, {"community_org_id", commOrgId}, {"reporter_id", reporterId}, {"reporter_name", reporterName}, {"event_category", 4}, {"priority", 2}, {"title", QStringLiteral("邻里噪音纠纷")}, {"description", QStringLiteral("2号楼301住户反映楼上住户深夜噪音扰民")}, {"location", QStringLiteral("翠苑小区2号楼")}, {"status", 3}, {"reviewer_id", reviewerId}, {"review_time", now.addDays(-2)}, {"assign_to", reporterId}, {"assign_time", now.addDays(-2)}, {"sla_deadline", now.addDays(1)}, {"source", 0}});

    db.insert("ge_event_flow", {{"event_id", ev2Id}, {"action", QStringLiteral("上报")}, {"operator_id", reporterId}, {"operator_name", reporterName}, {"from_status", -1}, {"to_status", 0}, {"action_time", now.addDays(-3)}});
    db.insert("ge_event_flow", {{"event_id", ev2Id}, {"action", QStringLiteral("审核分派")}, {"operator_id", reviewerId}, {"operator_name", QStringLiteral("李芳")}, {"from_status", 0}, {"to_status", 2}, {"action_time", now.addDays(-2)}});

    // 待审核事件
    db.insert("ge_event", {{"event_no", Utils::generateEventNo()}, {"grid_id", gridId}, {"community_org_id", commOrgId}, {"reporter_id", reporterId}, {"reporter_name", reporterName}, {"event_category", 1}, {"priority", 1}, {"title", QStringLiteral("老人需要居家养老服务")}, {"description", QStringLiteral("巡查发现1号楼独居老人刘奶奶需要居家养老服务")}, {"location", QStringLiteral("翠苑小区1号楼")}, {"status", 0}, {"sla_deadline", now.addDays(3)}, {"source", 3}});

    Q_UNUSED(evId)
    Q_UNUSED(ev2Id)
}

void DemoDataService::initAnnouncements(DatabaseManager &db)
{
    QSqlQuery pq("SELECT id FROM sys_user WHERE username = 'wuye_jingli'");
    qint64 pubId = 0;
    if (pq.next())
        pubId = pq.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();

    db.insert("nt_announcement", {{"title", QStringLiteral("关于小区供水管网检修的通知")},
                                  {"content", QStringLiteral("<p>尊敬的各位业主：</p><p>因小区供水管网老化，物业将于本周六(6月20日)上午8:00-12:00对供水管道进行检修，届时将暂停供水4小时，请各位业主提前做好储水准备。</p><p>给您带来的不便，敬请谅解！</p><p>恒达物业</p>")},
                                  {"announcement_type", 3},
                                  {"target_scope", "estate"},
                                  {"publisher_id", pubId},
                                  {"publish_time", now.addDays(-1)},
                                  {"is_top", 1},
                                  {"read_count", 156},
                                  {"status", 1}});

    db.insert("nt_announcement", {{"title", QStringLiteral("端午节社区活动通知")},
                                  {"content", QStringLiteral("<p>翠苑社区将于端午节期间举办包粽子、送温暖活动，欢迎广大居民踊跃报名参加！</p><p>活动时间：6月25日 上午9:00</p><p>活动地点：翠苑小区活动中心</p>")},
                                  {"announcement_type", 2},
                                  {"target_scope", "community"},
                                  {"publisher_id", pubId},
                                  {"publish_time", now.addDays(-3)},
                                  {"is_top", 0},
                                  {"read_count", 89},
                                  {"status", 1}});

    db.insert("nt_announcement", {{"title", QStringLiteral("2026年Q1公共收益公示")},
                                  {"content", QStringLiteral("<p>翠苑小区2026年第一季度公共区域收益情况如下：</p><p>广告位收入：15,000元</p><p>停车位收入：48,000元</p><p>场地租赁收入：8,000元</p><p>合计：71,000元</p><p>支出：绿化维护20,000元，设施维修15,000元</p><p>结余：36,000元</p>")},
                                  {"announcement_type", 1},
                                  {"target_scope", "estate"},
                                  {"publisher_id", pubId},
                                  {"publish_time", now.addDays(-5)},
                                  {"is_top", 0},
                                  {"read_count", 203},
                                  {"status", 1}});
}

void DemoDataService::initVolunteerData(DatabaseManager &db)
{
    QSqlQuery uq("SELECT id FROM sys_user WHERE username = 'zhangsan'");
    qint64 userId = 0;
    if (uq.next())
        userId = uq.value(0).toLongLong();

    qint64 volId = db.insert("sv_volunteer", {{"user_id", userId}, {"skills", QStringLiteral("教学,翻译")}, {"available_time", QStringLiteral("周末")}, {"total_hours", 12.5}, {"status", 0}});

    QSqlQuery cq("SELECT id FROM sys_org WHERE org_type = 3 LIMIT 1");
    qint64 commId = 0;
    if (cq.next())
        commId = cq.value(0).toLongLong();

    QSqlQuery pq("SELECT id FROM sys_user WHERE username = 'shequ_lifang'");
    qint64 pubId = 0;
    if (pq.next())
        pubId = pq.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();

    qint64 actId = db.insert("sv_volunteer_activity", {{"community_org_id", commId}, {"title", QStringLiteral("端午节关爱独居老人活动")}, {"description", QStringLiteral("走访慰问小区独居老人，送去节日祝福和粽子")}, {"activity_type", 2}, {"location", QStringLiteral("翠苑小区")}, {"start_time", now.addDays(5)}, {"end_time", now.addDays(5).addSecs(4 * 3600)}, {"need_count", 10}, {"enrolled_count", 3}, {"publisher_id", pubId}, {"status", 1}});

    db.insert("sv_volunteer_signup", {{"activity_id", actId}, {"volunteer_id", volId}, {"status", 1}});

    Q_UNUSED(actId)

    // More activities
    qint64 act2 = db.insert("sv_volunteer_activity", {{"community_org_id", commId}, {"title", QStringLiteral("社区环境清洁日")}, {"description", QStringLiteral("每月一次的社区环境清洁志愿活动")}, {"activity_type", 1}, {"location", QStringLiteral("翠苑社区广场")}, {"start_time", now.addDays(-2)}, {"end_time", now.addDays(-2).addSecs(3 * 3600)}, {"need_count", 20}, {"enrolled_count", 15}, {"publisher_id", pubId}, {"status", 3}});
    qint64 act3 = db.insert("sv_volunteer_activity", {{"community_org_id", commId}, {"title", QStringLiteral("暑期少儿阅读辅导")}, {"description", QStringLiteral("为社区少儿提供免费阅读指导和绘本分享")}, {"activity_type", 3}, {"location", QStringLiteral("社区图书室")}, {"start_time", now.addDays(10)}, {"end_time", now.addDays(10).addSecs(2 * 3600)}, {"need_count", 8}, {"enrolled_count", 5}, {"publisher_id", pubId}, {"status", 1}});
    qint64 act4 = db.insert("sv_volunteer_activity", {{"community_org_id", commId}, {"title", QStringLiteral("社区安全巡逻")}, {"description", QStringLiteral("夜间社区安全巡逻志愿活动")}, {"activity_type", 4}, {"location", QStringLiteral("翠苑全区域")}, {"start_time", now.addDays(3)}, {"end_time", now.addDays(3).addSecs(3 * 3600)}, {"need_count", 6}, {"enrolled_count", 4}, {"publisher_id", pubId}, {"status", 1}});
    db.insert("sv_volunteer_activity", {{"community_org_id", commId}, {"title", QStringLiteral("重阳节文艺汇演")}, {"description", QStringLiteral("组织社区老人文艺表演和互动游戏")}, {"activity_type", 5}, {"location", QStringLiteral("社区活动中心")}, {"start_time", now.addDays(-30)}, {"end_time", now.addDays(-30).addSecs(4 * 3600)}, {"need_count", 12}, {"enrolled_count", 12}, {"publisher_id", pubId}, {"status", 3}});

    // More signups
    db.insert("sv_volunteer_signup", {{"activity_id", act2}, {"volunteer_id", volId}, {"status", 3}, {"hours", 3.0}});
    db.insert("sv_volunteer_signup", {{"activity_id", act3}, {"volunteer_id", volId}, {"status", 1}});
    db.insert("sv_volunteer_signup", {{"activity_id", act4}, {"volunteer_id", volId}, {"status", 0}});
}

void DemoDataService::initServiceData(DatabaseManager &db)
{
    QSqlQuery sq("SELECT id FROM sys_org WHERE org_type = 6 LIMIT 1");
    qint64 orgId = 0;
    if (sq.next())
        orgId = sq.value(0).toLongLong();

    QSqlQuery uq("SELECT id FROM sys_user WHERE username = 'fuwu_sun'");
    qint64 userId = 0;
    if (uq.next())
        userId = uq.value(0).toLongLong();

    // Service providers
    db.insert("sv_service_provider", {{"org_id", orgId}, {"user_id", userId}, {"provider_name", QStringLiteral("好帮手家政服务")}, {"service_types", QStringLiteral("家政,保洁,维修")}, {"contact_person", QStringLiteral("孙丽")}, {"contact_phone", "13800005001"}, {"rating", 4.8}, {"total_orders", 32}, {"status", 0}});
    db.insert("sv_service_provider", {{"org_id", orgId},
                                      {"provider_name", QStringLiteral("快修家电服务")},
                                      {"service_types", QStringLiteral("家电维修,水电安装")},
                                      {"contact_person", QStringLiteral("周师傅")},
                                      {"contact_phone", "13800005002"},
                                      {"rating", 4.6},
                                      {"total_orders", 58},
                                      {"status", 0}});
    db.insert("sv_service_provider", {{"org_id", orgId},
                                      {"provider_name", QStringLiteral("爱心养老护理")},
                                      {"service_types", QStringLiteral("老人陪护,日间照料")},
                                      {"contact_person", QStringLiteral("李护理")},
                                      {"contact_phone", "13800005003"},
                                      {"rating", 4.9},
                                      {"total_orders", 21},
                                      {"status", 0}});

    // Service orders
    QSqlQuery rq("SELECT id FROM sys_user WHERE username = 'zhangsan'");
    qint64 applicantId = 0;
    if (rq.next())
        applicantId = rq.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();
    db.insert("sv_service_order", {{"order_no", "SV20260601001"}, {"applicant_id", applicantId}, {"service_type", 2}, {"title", QStringLiteral("家庭保洁服务")}, {"description", QStringLiteral("需要一次全面家庭保洁")}, {"appointment_time", now.addDays(2)}, {"address", QStringLiteral("翠苑小区1号楼")}, {"status", 0}});
    db.insert("sv_service_order", {{"order_no", "SV20260602001"}, {"applicant_id", applicantId}, {"service_type", 1}, {"title", QStringLiteral("水龙头维修")}, {"description", QStringLiteral("厨房水龙头漏水")}, {"appointment_time", now.addDays(-3)}, {"address", QStringLiteral("翠苑小区1号楼")}, {"status", 3}});
    db.insert("sv_service_order", {{"order_no", "SV20260603001"}, {"applicant_id", applicantId}, {"service_type", 4}, {"title", QStringLiteral("老人日间照料")}, {"description", QStringLiteral("需要每周3次的日间照料服务")}, {"appointment_time", now.addDays(5)}, {"address", QStringLiteral("翠苑小区2号楼")}, {"status", 1}});

    // Job postings
    QSqlQuery cq("SELECT id FROM sys_org WHERE org_type = 3 LIMIT 1");
    qint64 commOrgId = 0;
    if (cq.next())
        commOrgId = cq.value(0).toLongLong();

    db.insert("sv_job_posting", {{"publisher_id", userId}, {"community_org_id", commOrgId}, {"title", QStringLiteral("物业保安招聘")}, {"company", QStringLiteral("恒达物业")}, {"salary_range", QStringLiteral("4000-6000元/月")}, {"requirements", QStringLiteral("年龄25-50岁，身体健康，有责任心")}, {"description", QStringLiteral("负责小区安全巡逻、门岗值守等工作")}, {"headcount", 3}, {"deadline", QDate::currentDate().addDays(30)}, {"status", 0}});
    db.insert("sv_job_posting", {{"publisher_id", userId}, {"community_org_id", commOrgId}, {"title", QStringLiteral("社区保洁员")}, {"company", QStringLiteral("恒达物业")}, {"salary_range", QStringLiteral("3500-4500元/月")}, {"requirements", QStringLiteral("年龄30-55岁，吃苦耐劳")}, {"description", QStringLiteral("负责小区公共区域保洁工作")}, {"headcount", 2}, {"deadline", QDate::currentDate().addDays(20)}, {"status", 0}});
    db.insert("sv_job_posting", {{"publisher_id", userId}, {"community_org_id", commOrgId}, {"title", QStringLiteral("养老护理员")}, {"company", QStringLiteral("爱心养老护理")}, {"salary_range", QStringLiteral("5000-8000元/月")}, {"requirements", QStringLiteral("有护理经验优先，有耐心和爱心")}, {"description", QStringLiteral("为社区独居老人提供日常照料服务")}, {"headcount", 5}, {"deadline", QDate::currentDate().addDays(45)}, {"status", 0}});
}

void DemoDataService::initKnowledgeBase(DatabaseManager &db)
{
    auto add = [&](const QString &cat, const QString &q, const QString &a, const QString &kw)
    {
        db.insert("ai_knowledge", {{"category", cat}, {"question", q}, {"answer", a}, {"keywords", kw}, {"status", 0}});
    };

    add(QStringLiteral("报修"), QStringLiteral("怎么报修？"), QStringLiteral("您可以在首页点击「报事报修」，填写问题类型、描述和照片后提交即可。物业会在48小时内受理。"), QStringLiteral("报修,报事,维修"));
    add(QStringLiteral("报修"), QStringLiteral("报修多久能处理？"), QStringLiteral("普通问题48小时内处理，紧急问题4小时内，特急问题（如电梯困人）2小时内响应。"), QStringLiteral("处理,时间,多久"));
    add(QStringLiteral("物业"), QStringLiteral("物业费怎么交？"), QStringLiteral("目前物业费缴纳请到物业管理处现场办理，线上缴费功能即将上线。"), QStringLiteral("物业费,缴费,缴纳"));
    add(QStringLiteral("停车"), QStringLiteral("怎么办理停车月卡？"), QStringLiteral("请携带房产证或租赁合同、行驶证到物业管理处办理停车月卡。"), QStringLiteral("停车,月卡,车位"));
    add(QStringLiteral("社区"), QStringLiteral("社区在哪里？"), QStringLiteral("翠苑社区居委会位于翠苑路88号，工作时间为周一至周五 8:30-17:30。"), QStringLiteral("社区,地址,位置"));
    add(QStringLiteral("志愿"), QStringLiteral("怎么成为志愿者？"), QStringLiteral("在「社区服务」-「志愿服务」中点击注册，填写您的特长和可用时间，审核通过后即可报名参加志愿活动。"), QStringLiteral("志愿者,注册,报名"));
    add(QStringLiteral("证件"), QStringLiteral("怎么办居住证？"), QStringLiteral("办理居住证需要携带身份证、房屋租赁合同或房产证到社区居委会登记申请。"), QStringLiteral("居住证,办理,证件"));
}

void DemoDataService::initParkingData(DatabaseManager &db)
{
    // 查询小区ID
    QSqlQuery eq("SELECT id FROM cm_estate WHERE del_flag = 0 LIMIT 1");
    qint64 estateId = 0;
    if (eq.next())
        estateId = eq.value(0).toLongLong();

    // 创建停车位
    struct ParkingSpace
    {
        const char *code;
        const char *area;
        int type;
        int status;
    };
    ParkingSpace spaces[] = {
        {"A-001", "A区地下", 1, 1},
        {"A-002", "A区地下", 1, 1},
        {"A-003", "A区地下", 1, 0},
        {"A-004", "A区地下", 1, 1},
        {"A-005", "A区地下", 2, 0},
        {"B-001", "B区地下", 1, 1},
        {"B-002", "B区地下", 1, 0},
        {"B-003", "B区地下", 2, 1},
        {"C-001", "C区地面", 2, 1},
        {"C-002", "C区地面", 2, 0},
        {"C-003", "C区地面", 2, 0},
        {"C-004", "C区地面", 2, 1},
    };
    for (const auto &s : spaces)
    {
        db.insert("cm_parking_space", {{"estate_id", estateId}, {"space_code", s.code}, {"area_name", s.area}, {"space_type", s.type}, {"status", s.status}});
    }

    // 将已有车辆关联到部分车位
    QSqlQuery vq("SELECT id FROM cm_vehicle WHERE del_flag = 0 LIMIT 6");
    QSqlQuery pq("SELECT id FROM cm_parking_space WHERE status = 1 AND del_flag = 0 LIMIT 6");
    while (vq.next() && pq.next())
    {
        db.update("cm_vehicle", vq.value(0).toLongLong(), {{"parking_space_id", pq.value(0).toLongLong()}});
    }
}

void DemoDataService::initVehicleData(DatabaseManager &db)
{
    QSqlQuery eq("SELECT id FROM cm_estate WHERE del_flag = 0 LIMIT 1");
    qint64 estateId = eq.next() ? eq.value(0).toLongLong() : 0;

    QSqlQuery psq("SELECT id FROM cm_parking_space WHERE del_flag = 0 AND status = 1 LIMIT 5");
    QList<qint64> parkingIds;
    while (psq.next()) parkingIds.append(psq.value(0).toLongLong());

    QSqlQuery rq("SELECT id FROM cm_resident WHERE del_flag = 0 LIMIT 5");
    QList<qint64> residentIds;
    while (rq.next()) residentIds.append(rq.value(0).toLongLong());

    struct VehicleInfo {
        const char *plate; const char *brand; const char *color;
        int type;
    };
    VehicleInfo vehicles[] = {
        {"皖A·12345", "比亚迪", "白色", 1},  // 轿车
        {"皖A·23456", "大众", "黑色", 2},     // SUV
        {"皖A·34567", "丰田", "银色", 1},     // 轿车
        {"皖A·45678", "宝马", "红色", 2},     // SUV
        {"皖A·56789", "本田", "白色", 4},     // MPV (其他)
    };

    for (int i = 0; i < 5; i++)
    {
        qint64 psId = (i < parkingIds.size()) ? parkingIds[i] : 0;
        QVariantMap vData;
        vData["plate_number"] = QString(vehicles[i].plate);
        vData["vehicle_brand"] = QString(vehicles[i].brand);
        vData["vehicle_color"] = QString(vehicles[i].color);
        vData["vehicle_type"] = vehicles[i].type;
        vData["estate_id"] = estateId;
        if (psId > 0) vData["parking_space_id"] = psId;
        vData["status"] = 0;
        qint64 vid = db.insert("cm_vehicle", vData);

        // 关联车辆和居民
        if (i < residentIds.size() && vid > 0) {
            db.insert("cm_vehicle_owner", {{"vehicle_id", vid}, {"resident_id", residentIds[i]}, {"relation", QStringLiteral("车主")}});
        }
    }
}

void DemoDataService::initPublicIncomeData(DatabaseManager &db)
{
    QSqlQuery eq("SELECT id FROM cm_estate WHERE del_flag = 0 LIMIT 1");
    qint64 estateId = 0;
    if (eq.next())
        estateId = eq.value(0).toLongLong();

    QSqlQuery pq("SELECT id FROM sys_user WHERE username = 'admin'");
    qint64 pubId = 0;
    if (pq.next())
        pubId = pq.value(0).toLongLong();

    struct IncomeData
    {
        const char *period;
        double income;
        double expense;
        double balance;
        int daysAgo;
        int status;
    };
    IncomeData data[] = {
        {"2026年6月", 28500.00, 18200.00, 10300.00, 0, 0},
        {"2026年5月", 31200.00, 22800.00, 8400.00, 30, 0},
        {"2026年4月", 27800.00, 19500.00, 8300.00, 60, 0},
        {"2026年3月", 29600.00, 25100.00, 4500.00, 90, 0},
        {"2026年2月", 26400.00, 21000.00, 5400.00, 120, 0},
        {"2026年1月", 30100.00, 20500.00, 9600.00, 150, 0},
    };

    QDateTime now = QDateTime::currentDateTime();
    for (const auto &d : data)
    {
        db.insert("oc_public_income", {{"estate_id", estateId}, {"period", d.period}, {"income_amount", d.income}, {"expense_amount", d.expense}, {"balance", d.balance}, {"detail", QStringLiteral("停车费收入、广告位收入、场地租赁收入等")}, {"publisher_id", pubId}, {"publish_time", now.addDays(-d.daysAgo)}, {"status", d.status}});
    }
}

void DemoDataService::initTopicData(DatabaseManager &db)
{
    QSqlQuery eq("SELECT id FROM cm_estate WHERE del_flag = 0 LIMIT 1");
    qint64 estateId = 0;
    if (eq.next())
        estateId = eq.value(0).toLongLong();

    QSqlQuery cq("SELECT id FROM cm_owner_committee WHERE del_flag = 0 LIMIT 1");
    qint64 committeeId = 0;
    if (cq.next())
        committeeId = cq.value(0).toLongLong();

    QSqlQuery pq("SELECT id FROM sys_user WHERE username = 'yewei_zhao'");
    qint64 pubId = 0;
    if (pq.next())
        pubId = pq.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();

    db.insert("oc_topic", {{"committee_id", committeeId}, {"estate_id", estateId}, {"title", QStringLiteral("关于小区门禁系统升级的议题")}, {"content", QStringLiteral("当前门禁系统老化严重，建议更换为人脸识别+刷卡双模式门禁系统，预算约15万元。")}, {"topic_type", 3}, {"publisher_id", pubId}, {"publish_time", now.addDays(-10)}, {"need_vote", 1}, {"vote_start", now.addDays(-8)}, {"vote_end", now.addDays(-2)}, {"vote_result", 1}, {"status", 1}});

    db.insert("oc_topic", {{"committee_id", committeeId}, {"estate_id", estateId}, {"title", QStringLiteral("2026年Q2公共收益使用方案")}, {"content", QStringLiteral("提议将Q2公共收益结余用于小区绿化改造和儿童游乐设施更新。")}, {"topic_type", 1}, {"publisher_id", pubId}, {"publish_time", now.addDays(-5)}, {"need_vote", 1}, {"vote_start", now.addDays(-3)}, {"vote_end", now.addDays(3)}, {"status", 1}});

    db.insert("oc_topic", {{"committee_id", committeeId}, {"estate_id", estateId}, {"title", QStringLiteral("物业服务质量评议")}, {"content", QStringLiteral("对恒达物业本季度服务质量进行评议，包括安保、保洁、维修响应等方面。")}, {"topic_type", 2}, {"publisher_id", pubId}, {"publish_time", now.addDays(-1)}, {"need_vote", 0}, {"status", 0}});
}

void DemoDataService::initEvaluationData(DatabaseManager &db)
{
    // Work order evaluations
    QSqlQuery woQ("SELECT id FROM wo_work_order WHERE del_flag = 0 LIMIT 5");
    QSqlQuery uQ("SELECT id FROM sys_user WHERE username = 'zhangsan'");
    qint64 evalId = 0;
    if (uQ.next())
        evalId = uQ.value(0).toLongLong();

    struct EvalItem
    {
        const char *content;
        const char *reply;
        int rating;
    };
    EvalItem evals[] = {
        {"维修很快，师傅态度很好", "感谢您的好评！", 5},
        {"处理速度还可以，希望更快一些", "", 4},
        {"问题已解决，满意", "感谢反馈，我们会继续努力", 5},
        {"等待时间较长，但结果还不错", "感谢理解，会优化派单流程", 3},
        {"服务很专业，推荐", "", 5},
    };
    int ei = 0;
    while (woQ.next() && ei < 5)
    {
        QVariantMap evalData;
        evalData["biz_type"] = "work_order";
        evalData["biz_id"] = woQ.value(0).toLongLong();
        evalData["evaluator_id"] = evalId;
        evalData["rating"] = evals[ei].rating;
        evalData["content"] = QString(evals[ei].content);
        evalData["reply"] = QString(evals[ei].reply).isEmpty() ? QVariant() : QVariant(QString(evals[ei].reply));
        db.insert("ev_evaluation", evalData);
        ei++;
    }

    // Event evaluations
    QSqlQuery evQ("SELECT id FROM ge_event WHERE del_flag = 0 LIMIT 3");
    const char *evEvals[] = {"事件处理及时，感谢社区", "响应速度快，非常满意", "处理结果满意"};
    int evi = 0;
    while (evQ.next() && evi < 3)
    {
        QVariantMap evEvalData;
        evEvalData["biz_type"] = "event";
        evEvalData["biz_id"] = evQ.value(0).toLongLong();
        evEvalData["evaluator_id"] = evalId;
        evEvalData["rating"] = 5;
        evEvalData["content"] = QString(evEvals[evi]);
        db.insert("ev_evaluation", evEvalData);
        evi++;
    }

    // Chat logs
    QSqlQuery chatU("SELECT id FROM sys_user WHERE username = 'zhangsan' LIMIT 1");
    qint64 chatUserId = 0;
    if (chatU.next())
        chatUserId = chatU.value(0).toLongLong();
    struct ChatMsg
    {
        const char *role;
        const char *content;
        const char *intent;
    };
    ChatMsg chats[] = {
        {"user", "怎么报修", "报修咨询"},
        {"assistant", "您可以在首页点击「报事报修」，填写问题类型、描述和照片后提交即可。物业会在48小时内受理。", "报修咨询"},
        {"user", "物业费怎么交", "物业咨询"},
        {"assistant", "目前物业费缴纳请到物业管理处现场办理，线上缴费功能即将上线。", "物业咨询"},
        {"user", "怎么办停车月卡", "停车咨询"},
        {"assistant", "请携带房产证或租赁合同、行驶证到物业管理处办理停车月卡。", "停车咨询"},
    };
    for (const auto &c : chats)
    {
        QVariantMap chatData;
        chatData["user_id"] = chatUserId;
        chatData["session_id"] = "session_001";
        chatData["role"] = QString(c.role);
        chatData["content"] = QString(c.content);
        chatData["intent"] = QString(c.intent);
        chatData["is_useful"] = 1;
        db.insert("ai_chat_log", chatData);
    }
}

void DemoDataService::initAssessmentData(DatabaseManager &db)
{
    // Assessment config
    struct Indicator
    {
        const char *name;
        int type;
        double target;
        double weight;
        const char *period;
    };
    Indicator indicators[] = {
        {"工单处理及时率", 1, 95.0, 0.25, "monthly"},
        {"居民满意度", 2, 90.0, 0.25, "monthly"},
        {"事件响应时间(分钟)", 3, 30.0, 0.20, "monthly"},
        {"巡查完成率", 1, 100.0, 0.15, "monthly"},
        {"投诉处理率", 1, 98.0, 0.15, "monthly"},
    };
    qint64 cfgIds[5];
    for (int i = 0; i < 5; i++)
    {
        QVariantMap cfgData;
        cfgData["indicator_name"] = QString(indicators[i].name);
        cfgData["indicator_type"] = indicators[i].type;
        cfgData["target_value"] = indicators[i].target;
        cfgData["weight"] = indicators[i].weight;
        cfgData["assessment_period"] = QString(indicators[i].period);
        cfgData["status"] = 0;
        cfgIds[i] = db.insert("kf_assessment_config", cfgData);
    }

    // Assessment results for recent months
    QSqlQuery gridU("SELECT id FROM sys_user WHERE id IN (SELECT user_id FROM sys_user_role WHERE role_id IN (SELECT id FROM sys_role WHERE role_domain = 'property')) LIMIT 4");
    QStringList months = {"2026-06", "2026-05", "2026-04"};
    int userIdx = 0;
    while (gridU.next())
    {
        qint64 uid = gridU.value(0).toLongLong();
        for (const auto &m : months)
        {
            double base = 85.0 + (userIdx * 3.5);
            for (int i = 0; i < 5; i++)
            {
                double actual = base + (i * 2.0) - 5.0 + (QRandomGenerator::global()->bounded(10) - 5);
                if (actual > 100)
                    actual = 100;
                double score = actual >= indicators[i].target ? 100.0 : (actual / indicators[i].target * 100.0);
                db.insert("kf_assessment_result", {{"config_id", cfgIds[i]}, {"target_user_id", uid}, {"period", m}, {"actual_value", actual}, {"score", score}, {"rank", userIdx + 1}});
            }
        }
        userIdx++;
    }
}

void DemoDataService::initVisitorData(DatabaseManager &db)
{
    // Get estate and resident info for realistic data
    QSqlQuery estateQ("SELECT id FROM cm_estate LIMIT 1");
    qint64 estateId = estateQ.next() ? estateQ.value(0).toLongLong() : 1;

    QSqlQuery houseQ("SELECT id, house_code FROM cm_house LIMIT 6");
    QList<QPair<qint64, QString>> houses;
    while (houseQ.next())
    {
        houses.append({houseQ.value(0).toLongLong(),
                       houseQ.value(1).toString()});
    }

    QSqlQuery residentQ("SELECT id, name, phone FROM cm_resident LIMIT 6");
    QList<QPair<qint64, QPair<QString, QString>>> residents;
    while (residentQ.next())
    {
        residents.append({residentQ.value(0).toLongLong(),
                          {residentQ.value(1).toString(), residentQ.value(2).toString()}});
    }

    struct VData
    {
        const char *name;
        const char *phone;
        const char *purpose;
        int sts;
        int dayOffset;
    };
    VData vList[] = {
        {"王建国", "13812345678", "亲友拜访", 1, 0},
        {"李美华", "13987654321", "快递配送", 1, 0},
        {"赵志远", "13756781234", "维修服务", 0, 0},
        {"刘晓东", "13643219876", "商务拜访", 1, -1},
        {"陈思雨", "13598761234", "亲友拜访", 1, -1},
        {"张伟", "13456789876", "外卖配送", 1, -2},
        {"孙丽", "13345678901", "家政服务", 1, -3},
        {"周明", "13234567890", "亲友拜访", 1, -4},
        {"吴芳", "13123456789", "房屋看房", 1, -5},
        {"郑强", "13012345678", "维修服务", 1, -6},
    };

    for (int i = 0; i < 10; i++)
    {
        int rIdx = i % residents.size();
        int hIdx = i % houses.size();
        QVariantMap vData;
        vData["estate_id"] = estateId;
        vData["visitor_name"] = QString(vList[i].name);
        vData["phone"] = QString(vList[i].phone);
        vData["host_name"] = residents[rIdx].second.first;
        vData["host_phone"] = residents[rIdx].second.second;
        vData["host_house_id"] = houses[hIdx].first;
        vData["purpose"] = QString(vList[i].purpose);
        vData["visitor_count"] = 1;
        vData["status"] = vList[i].sts;

        // Generate arrive/leave times
        QString arriveDate = QDateTime::currentDateTime().addDays(vList[i].dayOffset).addSecs(9 * 3600 + i * 1800).toString("yyyy-MM-dd HH:mm:ss");
        vData["arrive_time"] = arriveDate;
        if (vList[i].sts == 1)
        {
            QString leaveDate = QDateTime::currentDateTime().addDays(vList[i].dayOffset).addSecs(11 * 3600 + i * 1800 + 3600).toString("yyyy-MM-dd HH:mm:ss");
            vData["leave_time"] = leaveDate;
        }
        db.insert("cm_visitor", vData);
    }
}

void DemoDataService::initBillData(DatabaseManager &db)
{
    QSqlQuery estateQ("SELECT id FROM cm_estate LIMIT 1");
    qint64 estateId = estateQ.next() ? estateQ.value(0).toLongLong() : 1;

    QSqlQuery houseQ("SELECT id FROM cm_house LIMIT 10");
    QList<qint64> houseIds;
    while (houseQ.next())
        houseIds.append(houseQ.value(0).toLongLong());

    QSqlQuery residentQ("SELECT id FROM cm_resident LIMIT 10");
    QList<qint64> residentIds;
    while (residentQ.next())
        residentIds.append(residentQ.value(0).toLongLong());

    // Bill types: 1=物业费, 2=水费, 3=电费, 4=停车费, 5=综合
    struct BData
    {
        int type;
        double amount;
        int sts;
        const char *period;
        const char *remark;
    };
    BData bList[] = {
        {1, 380.00, 1, "2026-06", "6月物业费"},
        {1, 420.00, 1, "2026-06", "6月物业费"},
        {5, 530.00, 0, "2026-06", "物业费+水费"},
        {5, 680.00, 0, "2026-06", "物业费+停车费"},
        {1, 350.00, 1, "2026-06", "6月物业费"},
        {5, 720.00, 1, "2026-06", "物业费+电费"},
        {1, 380.00, 1, "2026-05", "5月物业费"},
        {2, 45.50, 1, "2026-05", "5月水费"},
        {3, 128.30, 0, "2026-05", "5月电费"},
        {4, 200.00, 1, "2026-05", "5月停车费"},
        {1, 380.00, 2, "2026-04", "4月物业费(逾期)"},
        {1, 420.00, 1, "2026-04", "4月物业费"},
        {5, 560.00, 1, "2026-04", "物业费+水费"},
        {1, 350.00, 1, "2026-03", "3月物业费"},
        {4, 200.00, 1, "2026-03", "3月停车费"},
    };

    for (int i = 0; i < 15; i++)
    {
        int hIdx = i % houseIds.size();
        int rIdx = i % residentIds.size();
        QVariantMap bData;
        bData["bill_no"] = QString("FY%1-%2").arg(bList[i].period).arg(i + 1, 3, 10, QChar('0'));
        bData["estate_id"] = estateId;
        bData["house_id"] = houseIds[hIdx];
        bData["resident_id"] = residentIds[rIdx];
        bData["bill_type"] = bList[i].type;
        bData["amount"] = bList[i].amount;
        bData["period"] = QString(bList[i].period);
        bData["status"] = bList[i].sts;
        bData["remark"] = QString(bList[i].remark);
        if (bList[i].sts == 1)
        {
            bData["pay_time"] = QDateTime::currentDateTime().addDays(-i * 3).toString("yyyy-MM-dd HH:mm:ss");
            bData["pay_method"] = (i % 3) + 1;
        }
        db.insert("pm_bill", bData);
    }
}

void DemoDataService::initMonthlyCardData(DatabaseManager &db)
{
    QSqlQuery estateQ("SELECT id FROM cm_estate LIMIT 1");
    qint64 estateId = estateQ.next() ? estateQ.value(0).toLongLong() : 1;

    QSqlQuery spaceQ("SELECT id, space_code FROM cm_parking_space LIMIT 6");
    QList<QPair<qint64, QString>> spaces;
    while (spaceQ.next())
    {
        spaces.append({spaceQ.value(0).toLongLong(), spaceQ.value(1).toString()});
    }

    struct MCData
    {
        const char *plate;
        const char *owner;
        const char *phone;
        int cardType;
        int sts;
        int monthOffset;
    };
    MCData mcList[] = {
        {"京A88888", "张三", "13800001001", 2, 1, 0},  // 年卡, 有效
        {"京B66666", "李四", "13800001002", 1, 1, 0},  // 季卡, 有效
        {"京C12345", "王五", "13800001003", 0, 1, 0},  // 月卡, 有效
        {"京D99999", "赵六", "13800001005", 0, 0, -1}, // 月卡, 已过期
        {"京E55555", "钱七", "13800001011", 2, 1, 0},  // 年卡, 有效
        {"京F77777", "孙八", "13800001012", 1, 2, 0},  // 季卡, 待续费
    };

    double fees[] = {150.0, 400.0, 1500.0}; // 月卡/季卡/年卡
    for (int i = 0; i < 6; i++)
    {
        int sIdx = i % spaces.size();
        QVariantMap mcData;
        mcData["estate_id"] = estateId;
        mcData["plate_no"] = QString(mcList[i].plate);
        mcData["owner_name"] = QString(mcList[i].owner);
        mcData["owner_phone"] = QString(mcList[i].phone);
        mcData["space_id"] = spaces[sIdx].first;
        mcData["card_type"] = mcList[i].cardType;
        mcData["fee"] = fees[mcList[i].cardType];

        QDate startDate = QDate::currentDate().addMonths(mcList[i].monthOffset);
        QDate endDate;
        switch (mcList[i].cardType)
        {
        case 0:
            endDate = startDate.addMonths(1);
            break;
        case 1:
            endDate = startDate.addMonths(3);
            break;
        case 2:
            endDate = startDate.addYears(1);
            break;
        }
        if (mcList[i].sts == 0)
        {
            // 已过期 - 结束日期在过去
            startDate = startDate.addMonths(-2);
            endDate = endDate.addMonths(-2);
        }
        mcData["start_date"] = startDate.toString("yyyy-MM-dd");
        mcData["end_date"] = endDate.toString("yyyy-MM-dd");
        mcData["status"] = mcList[i].sts;
        db.insert("pm_monthly_card", mcData);
    }
}

void DemoDataService::initOpinionData(DatabaseManager &db)
{
    QSqlQuery estateQ("SELECT id FROM cm_estate LIMIT 1");
    qint64 estateId = estateQ.next() ? estateQ.value(0).toLongLong() : 1;

    QSqlQuery residentQ("SELECT id, name FROM cm_resident LIMIT 8");
    QList<QPair<qint64, QString>> residents;
    while (residentQ.next())
    {
        residents.append({residentQ.value(0).toLongLong(), residentQ.value(1).toString()});
    }

    struct OData
    {
        const char *cat;
        const char *title;
        const char *content;
        int sts;
        int dayOffset;
        const char *reply;
    };
    OData oList[] = {
        {"环境", "小区花园杂草较多", "小区花园杂草较多，建议定期修剪，影响居住环境美观", 1, -5, "已安排物业本周内完成修剪"},
        {"设施", "3号楼电梯运行噪音较大", "3号楼电梯运行时噪音较大，尤其夜间影响休息，建议尽快检修", 0, -6, ""},
        {"安全", "地下车库照明不足", "地下车库B区照明不足，存在安全隐患，建议增加照明设施", 1, -7, "已增加LED灯6盏，改善照明条件"},
        {"服务", "建议增加快递代收柜", "小区快递代收柜数量不足，经常爆满，建议增加一组代收柜", 2, -8, "已采纳，已联系供应商安装新柜"},
        {"环境", "垃圾分类投放点需要增加标识", "垃圾分类投放点标识不清，居民经常分错，建议增加清晰标识", 1, -9, "已制作新标识牌，本周安装"},
        {"设施", "儿童游乐设施需要维护", "小区儿童滑梯有螺丝松动，存在安全隐患", 1, -10, "已安排维修人员加固处理"},
        {"安全", "小区门禁系统经常故障", "东门门禁系统近期经常无法正常识别，影响通行", 0, -3, ""},
        {"服务", "建议延长物业客服工作时间", "物业客服下班后无法联系，建议延长至晚9点", 2, -4, "已采纳，下月起延长至21:00"},
    };

    for (int i = 0; i < 8; i++)
    {
        int rIdx = i % residents.size();
        QVariantMap oData;
        oData["estate_id"] = estateId;
        oData["resident_id"] = residents[rIdx].first;
        oData["category"] = QString(oList[i].cat);
        oData["title"] = QString(oList[i].title);
        oData["content"] = QString(oList[i].content);
        oData["status"] = oList[i].sts;
        oData["is_anonymous"] = 0;
        if (oList[i].sts >= 1 && strlen(oList[i].reply) > 0)
        {
            oData["reply_content"] = QString(oList[i].reply);
            oData["reply_by"] = 1;
            oData["reply_time"] = QDateTime::currentDateTime().addDays(oList[i].dayOffset + 1).toString("yyyy-MM-dd HH:mm:ss");
        }
        db.insert("ge_opinion", oData);
    }
}

void DemoDataService::initInspectionData(DatabaseManager &db)
{
    QSqlQuery gridQ("SELECT id FROM cm_grid LIMIT 1");
    qint64 gridId = 0;
    if (gridQ.next()) gridId = gridQ.value(0).toLongLong();

    QSqlQuery uq("SELECT id FROM sys_user WHERE username = 'wangge_zhao'");
    qint64 inspectorId = 0;
    if (uq.next()) inspectorId = uq.value(0).toLongLong();

    QSqlQuery uq2("SELECT id FROM sys_user WHERE username = 'wuye_baoan'");
    qint64 baoanId = 0;
    if (uq2.next()) baoanId = uq2.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();

    // 3 inspection plans
    qint64 p1 = db.insert("ge_inspection_plan", {
        {"grid_id", gridId}, {"plan_name", QStringLiteral("翠苑日常巡查计划")},
        {"frequency", "daily"}, {"inspector_id", inspectorId},
        {"start_date", now.addMonths(-3).toString("yyyy-MM-dd")},
        {"end_date", now.addMonths(6).toString("yyyy-MM-dd")}, {"status", 1}
    });
    qint64 p2 = db.insert("ge_inspection_plan", {
        {"grid_id", gridId}, {"plan_name", QStringLiteral("安全巡查周计划")},
        {"frequency", "weekly"}, {"inspector_id", baoanId},
        {"start_date", now.addMonths(-1).toString("yyyy-MM-dd")},
        {"end_date", now.addMonths(3).toString("yyyy-MM-dd")}, {"status", 0}
    });
    db.insert("ge_inspection_plan", {
        {"grid_id", gridId}, {"plan_name", QStringLiteral("夜间巡逻计划")},
        {"frequency", "daily"}, {"inspector_id", baoanId},
        {"start_date", now.addMonths(-2).toString("yyyy-MM-dd")},
        {"end_date", now.addMonths(1).toString("yyyy-MM-dd")}, {"status", 1}
    });

    // 5 inspection records
    struct InspRec { int dayOff; qint64 planId; int dur; int issues; int sts; const char* summary; };
    InspRec recs[] = {
        {-1, p1, 60, 2, 2, "发现2号楼外墙脱落和3号楼绿化枯死"},
        {-2, p1, 45, 0, 2, "巡查正常，未发现异常"},
        {-3, p2, 90, 1, 2, "发现南门门禁故障"},
        {-4, p1, 55, 3, 2, "发现垃圾堆积、路灯故障、消防栓漏水"},
        {-5, p2, 40, 0, 2, "夜间巡查正常，未发现异常"},
    };
    for (auto& r : recs) {
        db.insert("ge_inspection", {
            {"grid_id", gridId}, {"inspector_id", r.planId == p1 ? inspectorId : baoanId},
            {"plan_id", r.planId},
            {"start_time", now.addDays(r.dayOff)},
            {"end_time", now.addDays(r.dayOff).addSecs(r.dur * 60)},
            {"duration", r.dur}, {"issue_count", r.issues},
            {"summary", QString::fromUtf8(r.summary)}, {"status", r.sts}
        });
    }
}

void DemoDataService::initVisitRecordData(DatabaseManager &db)
{
    QSqlQuery sgQ("SELECT id FROM cm_special_group LIMIT 2");
    QList<qint64> sgIds;
    while (sgQ.next()) sgIds.append(sgQ.value(0).toLongLong());

    QSqlQuery uq("SELECT id FROM sys_user WHERE username = 'shequ_lifang'");
    qint64 lifangId = 0;
    if (uq.next()) lifangId = uq.value(0).toLongLong();

    QSqlQuery uq2("SELECT id FROM sys_user WHERE username = 'wangge_zhao'");
    qint64 wanggeId = 0;
    if (uq2.next()) wanggeId = uq2.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();

    struct VisitItem { int dayOff; qint64 visitorId; int type; const char* content; const char* issues; const char* follow; };
    VisitItem visits[] = {
        {-2, lifangId, 1, "定期走访独居老人刘奶奶，精神状态良好，生活物资充足", "", ""},
        {-5, wanggeId, 1, "定期走访残疾家庭王叔叔，反映需要康复训练指导", "建议联系社区卫生服务中心", "已联系社区卫生中心安排康复师"},
        {-8, lifangId, 3, "电话慰问张奶奶，询问近期身体状况", "", ""},
        {-12, wanggeId, 2, "临时走访低保户李大哥，了解就业意向", "希望找一份保安工作", "已登记就业需求，推荐社区保安岗位"},
        {-15, lifangId, 1, "定期走访空巢老人赵奶奶，帮助清理家务", "家中灯泡损坏", "已联系物业更换灯泡"},
    };
    for (auto& v : visits) {
        QVariantMap data;
        data["visitor_id"] = v.visitorId;
        data["visit_time"] = now.addDays(v.dayOff);
        data["visit_type"] = v.type;
        data["content"] = QString::fromUtf8(v.content);
        if (strlen(v.issues) > 0) data["found_issues"] = QString::fromUtf8(v.issues);
        if (strlen(v.follow) > 0) data["follow_up"] = QString::fromUtf8(v.follow);
        if (!sgIds.isEmpty()) data["special_group_id"] = sgIds.first();
        db.insert("ge_visit_record", data);
    }
}

void DemoDataService::initSupervisionData(DatabaseManager &db)
{
    QSqlQuery evQ("SELECT id FROM ge_event WHERE del_flag = 0 LIMIT 3");
    QList<qint64> evIds;
    while (evQ.next()) evIds.append(evQ.value(0).toLongLong());
    if (evIds.isEmpty()) return;

    QSqlQuery uq("SELECT id FROM sys_user WHERE username = 'shequ_shuji'");
    qint64 supervisorId = 0;
    if (uq.next()) supervisorId = uq.value(0).toLongLong();

    QSqlQuery uq2("SELECT id FROM sys_user WHERE username = 'wangge_zhao'");
    qint64 wanggeId = 0;
    if (uq2.next()) wanggeId = uq2.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();

    struct SupItem { int evIdx; int dayOff; int sts; const char* reason; const char* feedback; };
    SupItem items[] = {
        {0, 5, 2, "处理超时，需加快进度", "已完成修复，经检查验收合格"},
        {1, 3, 1, "需协调城管部门参与处理", "已联系城管，预计3天内联合处理"},
        {0, 7, 0, "涉及多部门协调，需上级支持", ""},
        {evIds.size() > 2 ? 2 : 0, 10, 0, "居民反映强烈，需优先处理", ""},
    };
    for (auto& s : items) {
        int idx = s.evIdx < evIds.size() ? s.evIdx : 0;
        QVariantMap data;
        data["event_id"] = evIds[idx];
        data["supervisor_id"] = supervisorId;
        data["supervise_to"] = wanggeId;
        data["deadline"] = now.addDays(s.dayOff);
        data["reason"] = QString::fromUtf8(s.reason);
        data["status"] = s.sts;
        if (strlen(s.feedback) > 0) {
            data["feedback"] = QString::fromUtf8(s.feedback);
            data["feedback_time"] = now.addDays(-1);
        }
        db.insert("ge_supervision", data);
    }
}

void DemoDataService::initOperationLogData(DatabaseManager &db)
{
    QDateTime now = QDateTime::currentDateTime();

    struct LogItem { const char* user; const char* module; const char* op; const char* ip; int dayOff; int hourOff; };
    LogItem logs[] = {
        {"admin", "用户管理", "新增用户", "192.168.1.100", 0, -2},
        {"wuye_kefu", "工单管理", "受理工单 #WO202606-003", "192.168.1.101", -1, -3},
        {"shequ_lifang", "事件管理", "审核事件 #EV202606-001", "192.168.1.102", -1, -5},
        {"zhangsan", "报事报修", "提交报修工单", "192.168.1.103", -2, -1},
        {"admin", "系统设置", "修改数据字典", "192.168.1.100", -2, -4},
        {"wuye_jingli", "公告管理", "发布社区公告", "192.168.1.105", -3, -2},
        {"wangge_zhao", "事件管理", "上报网格事件", "192.168.1.106", -3, -6},
        {"shequ_shuji", "督办管理", "创建督办任务", "192.168.1.107", -4, -1},
        {"admin", "角色管理", "修改角色权限", "192.168.1.100", -5, -3},
        {"wuye_weixiu", "工单管理", "完成维修并反馈", "192.168.1.108", -5, -7},
    };
    for (auto& l : logs) {
        QVariantMap data;
        data["username"] = QString::fromUtf8(l.user);
        data["module"] = QString::fromUtf8(l.module);
        data["operation"] = QString::fromUtf8(l.op);
        data["method"] = "POST";
        data["ip"] = QString::fromUtf8(l.ip);
        data["status"] = 0;
        int inner = l.dayOff * 7 + l.hourOff * 13 + 42;
        data["cost_time"] = 50 + (std::abs(inner) % 200);
        data["operation_time"] = now.addDays(l.dayOff).addSecs(l.hourOff * 3600);
        db.insert("sys_operation_log", data);
    }
}

void DemoDataService::initVoteData(DatabaseManager &db)
{
    QSqlQuery tQ("SELECT id FROM oc_topic WHERE need_vote = 1 ORDER BY id LIMIT 2");
    QList<qint64> topicIds;
    while (tQ.next()) topicIds.append(tQ.value(0).toLongLong());
    if (topicIds.isEmpty()) return;

    // Get voter IDs
    QStringList voters = {"yewei_zhao", "zhangsan", "admin", "wuye_jingli", "shequ_lifang", "wangge_zhao", "liulou"};
    QList<qint64> voterIds;
    for (auto& v : voters) {
        QSqlQuery vq(QString("SELECT id FROM sys_user WHERE username = '%1'").arg(v));
        if (vq.next()) voterIds.append(vq.value(0).toLongLong());
    }

    // Topic 1: votes (4 approve, 1 reject, 1 abstain = passed)
    if (topicIds.size() >= 1) {
        int choices1[] = {1, 1, 1, 2, 1, 3, 1}; // approve/reject/abstain
        for (int i = 0; i < voterIds.size() && i < 7; i++) {
            db.insert("oc_vote", {
                {"topic_id", topicIds[0]}, {"voter_id", voterIds[i]},
                {"choice", choices1[i]}
            });
        }
    }

    // Topic 2: votes (in progress, mixed)
    if (topicIds.size() >= 2) {
        int choices2[] = {1, 3, 1, 1, 2, 1, 2};
        for (int i = 0; i < voterIds.size() && i < 7; i++) {
            db.insert("oc_vote", {
                {"topic_id", topicIds[1]}, {"voter_id", voterIds[i]},
                {"choice", choices2[i]}
            });
        }
    }
}

void DemoDataService::initNotificationData(DatabaseManager &db)
{
    QSqlQuery uq("SELECT id FROM sys_user WHERE username = 'zhangsan'");
    qint64 zsId = 0;
    if (uq.next()) zsId = uq.value(0).toLongLong();

    QSqlQuery uq2("SELECT id FROM sys_user WHERE username = 'admin'");
    qint64 adminId = 0;
    if (uq2.next()) adminId = uq2.value(0).toLongLong();

    QSqlQuery uq3("SELECT id FROM sys_user WHERE username = 'wuye_kefu'");
    qint64 kefuId = 0;
    if (uq3.next()) kefuId = uq3.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();

    struct NotiItem { qint64 uid; int type; const char* title; const char* content; int read; int dayOff; };
    NotiItem items[] = {
        {zsId, 1, "工单处理完成", "您提交的报修工单已完成，请对服务进行评价", 0, -1},
        {zsId, 2, "社区活动通知", "端午节社区包粽子活动将于本周六举行，欢迎参加！", 0, -2},
        {adminId, 3, "系统更新提醒", "系统将于今晚22:00进行例行维护，预计30分钟", 1, -3},
        {kefuId, 1, "新工单待处理", "居民张三提交了新的报修工单，请及时受理", 1, -1},
        {zsId, 4, "物业费催缴", "您6月份物业费尚未缴纳，请尽快办理", 0, 0},
        {kefuId, 5, "审批待办", "有一条新的投诉需要审批处理", 1, -4},
    };
    for (auto& n : items) {
        QVariantMap data;
        data["user_id"] = n.uid;
        data["title"] = QString::fromUtf8(n.title);
        data["content"] = QString::fromUtf8(n.content);
        data["notification_type"] = n.type;
        data["is_read"] = n.read;
        data["create_time"] = now.addDays(n.dayOff);
        if (n.read) data["read_time"] = now.addDays(n.dayOff).addSecs(3600);
        db.insert("nt_notification", data);
    }
}
