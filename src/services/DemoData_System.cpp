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
