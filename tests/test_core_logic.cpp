#include <QTest>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include "database/DatabaseManager.h"
#include "services/AuthService.h"
#include "services/DemoDataService.h"

static void dbgLog(const QString& msg) {
    QFile f("test_debug.log");
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream s(&f);
        s << msg << "\n"; s.flush();
        f.close();
    }
}

class TestCoreLogic : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        dbgLog("[DBG] step1: initialize");
        QVERIFY(DatabaseManager::instance().initialize(":memory:"));
        dbgLog("[DBG] step2: initSchema");
        QVERIFY(DatabaseManager::instance().initSchema());
        dbgLog("[DBG] step3: initIfEmpty");
        DemoDataService::initIfEmpty();
        dbgLog("[DBG] step4: done");
    }

    void testAllTablesCreated() {
        QStringList expectedTables = {
            "sys_user", "sys_role", "sys_user_role", "sys_menu", "sys_role_menu",
            "sys_org", "sys_user_org", "sys_dict_type", "sys_dict_data", "sys_config",
            "sys_operation_log", "sys_login_log", "sys_file",
            "cm_estate", "cm_building", "cm_unit", "cm_house", "cm_resident",
            "cm_family", "cm_family_member", "cm_house_resident", "cm_vehicle",
            "cm_vehicle_owner", "cm_parking_space", "cm_facility", "cm_grid",
            "cm_property_company", "cm_owner_committee", "cm_owner_committee_member",
            "cm_special_group", "cm_social_org", "cm_visitor",
            "wo_work_order",
            "ge_event", "ge_event_flow", "ge_inspection", "ge_inspection_plan",
            "ge_visit_record", "ge_supervision", "ge_opinion",
            "sv_volunteer", "sv_volunteer_activity", "sv_volunteer_signup",
            "sv_service_provider", "sv_service_order", "sv_job_posting",
            "nt_announcement", "nt_notification",
            "oc_topic", "oc_vote", "oc_public_income",
            "ev_evaluation",
            "ai_knowledge", "ai_chat_log",
            "kf_assessment_config", "kf_assessment_result",
            "pm_bill", "pm_monthly_card"
        };
        QSqlQuery q("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name");
        QSet<QString> actualTables;
        while (q.next()) actualTables.insert(q.value(0).toString());
        for (const auto& t : expectedTables) {
            QVERIFY2(actualTables.contains(t), QString("Missing table: %1").arg(t).toUtf8());
        }
    }

    void testSeedDataNotEmpty() {
        QStringList businessTables = {
            "sys_user", "sys_role", "sys_menu", "sys_org",
            "cm_estate", "cm_building", "cm_house", "cm_resident",
            "wo_work_order", "ge_event", "nt_announcement",
            "sv_volunteer_activity", "sv_service_order", "sv_job_posting",
            "oc_topic", "oc_public_income", "ev_evaluation",
            "ai_knowledge", "kf_assessment_config", "kf_assessment_result",
            "cm_visitor", "pm_bill", "pm_monthly_card", "ge_opinion"
        };
        for (const auto& table : businessTables) {
            QSqlQuery q(QString("SELECT COUNT(*) FROM %1").arg(table));
            QVERIFY(q.exec());
            QVERIFY(q.next());
            QVERIFY2(q.value(0).toInt() > 0, QString("Table %1 has no data").arg(table).toUtf8());
        }
    }

    void testDemoAccountsLogin() {
        // 验证登录 + 角色绑定正确性（防止 role_id 漂移）
        struct Account { const char* username; const char* password; const char* expectedRoleKey; };
        Account accounts[] = {
            {"admin", "admin123", "super_admin"},
            {"zhangsan", "123456", "resident_owner"},
            {"wuye_kefu", "123456", "property_cs"},
            {"wuye_weixiu", "123456", "property_repair"},
            {"wuye_jingli", "123456", "property_manager"},
            {"wangge_zhao", "123456", "grid_worker"},
            {"shequ_lifang", "123456", "community_worker"},
            {"shequ_shuji", "123456", "community_secretary"},
            {"jiedao_worker", "123456", "street_worker"},
            {"jiedao_leader", "123456", "street_leader"},
            {"yewei_zhao", "123456", "oc_member"},
            {"fuwu_sun", "123456", "service_provider"}
        };
        for (const auto& acc : accounts) {
            bool ok = AuthService::instance().login(acc.username, acc.password);
            QVERIFY2(ok, QString("Login failed for %1").arg(acc.username).toUtf8());

            // 验证角色绑定：查询用户实际 role_key
            QSqlQuery roleQ;
            roleQ.prepare("SELECT r.role_key FROM sys_role r "
                          "JOIN sys_user_role ur ON r.id = ur.role_id "
                          "JOIN sys_user u ON ur.user_id = u.id "
                          "WHERE u.username = :username AND r.status = 0 AND r.del_flag = 0");
            roleQ.bindValue(":username", acc.username);
            QVERIFY2(roleQ.exec(), QString("Role query failed for %1: %2")
                .arg(acc.username).arg(roleQ.lastError().text()).toUtf8());
            QVERIFY2(roleQ.next(), QString("No role found for %1").arg(acc.username).toUtf8());
            QString actualRoleKey = roleQ.value(0).toString();
            QVERIFY2(actualRoleKey == acc.expectedRoleKey,
                QString("Role mismatch for %1: expected %2, got %3")
                    .arg(acc.username).arg(acc.expectedRoleKey).arg(actualRoleKey).toUtf8());

            // 验证超级管理员拥有所有菜单权限
            if (acc.expectedRoleKey == QStringLiteral("super_admin")) {
                QVERIFY2(AuthService::instance().currentUser().permissions.contains("*:*:*"),
                    "super_admin should have *:*:* permission");
            }

            AuthService::instance().logout();
        }
    }

    void testRoleMenuAssignment() {
        // 验证关键角色的菜单分配数量合理（防止角色错位导致菜单丢失）
        struct RoleMenuExpectation { const char* roleKey; int minMenuCount; };
        RoleMenuExpectation expectations[] = {
            {"super_admin", 30},    // 超级管理员应有所有菜单
            {"property_cs", 15},    // 物业客服
            {"property_manager", 20}, // 物业经理
            {"grid_worker", 10},    // 网格员
            {"resident_owner", 10}, // 居民
            {"oc_member", 5}        // 业委会成员
        };
        for (const auto& exp : expectations) {
            QSqlQuery q;
            q.prepare("SELECT COUNT(*) FROM sys_role_menu rm "
                      "JOIN sys_role r ON rm.role_id = r.id "
                      "WHERE r.role_key = :roleKey");
            q.bindValue(":roleKey", exp.roleKey);
            QVERIFY2(q.exec(), QString("Query failed for role %1").arg(exp.roleKey).toUtf8());
            QVERIFY2(q.next(), QString("No result for role %1").arg(exp.roleKey).toUtf8());
            int count = q.value(0).toInt();
            QVERIFY2(count >= exp.minMenuCount,
                QString("Role %1 has only %2 menus, expected at least %3")
                    .arg(exp.roleKey).arg(count).arg(exp.minMenuCount).toUtf8());
        }
    }

    void testWorkOrderDynamicQuery() {
        // Test search filter
        QSqlQuery q;
        q.prepare("SELECT id FROM wo_work_order WHERE del_flag = 0 AND (order_no LIKE :search OR title LIKE :search) LIMIT 1");
        q.bindValue(":search", "%报修%");
        QVERIFY(q.exec());

        // Test status filter
        QSqlQuery q2;
        q2.prepare("SELECT id FROM wo_work_order WHERE del_flag = 0 AND status = :status LIMIT 1");
        q2.bindValue(":status", 0);
        QVERIFY(q2.exec());
    }

    void testEventDynamicQuery() {
        QSqlQuery q;
        q.prepare("SELECT id FROM ge_event WHERE del_flag = 0 AND (event_no LIKE :search OR title LIKE :search) LIMIT 1");
        q.bindValue(":search", "%事件%");
        QVERIFY(q.exec());
    }

    void testPageKeyMapping() {
        // 验证所有页面 key 对应的菜单确实存在（调用 next() 验证，而非仅 exec()）
        QStringList pageKeys = {
            "1", "201", "202", "203", "204", "205", "206", "207", "208",
            "301", "302", "303", "304", "305", "306", "307", "308", "309",
            "401", "402", "403", "404", "405", "406",
            "501", "502", "503",
            "601", "602", "603", "604",
            "701", "702", "703", "704", "705", "706"
        };
        for (const auto& key : pageKeys) {
            QSqlQuery q;
            q.prepare("SELECT id, menu_name FROM sys_menu WHERE id = :id AND status = 0");
            q.bindValue(":id", key.toInt());
            QVERIFY2(q.exec(), QString("Query failed for key %1: %2")
                .arg(key).arg(q.lastError().text()).toUtf8());
            QVERIFY2(q.next(), QString("Menu not found for key %1 (menu_id=%2)")
                .arg(key).arg(key).toUtf8());
        }
    }

    void cleanupTestCase() {
        DatabaseManager::instance().close();
    }
};

// 自定义 main 函数，添加调试输出
int main(int argc, char *argv[]) {
    dbgLog("[DBG] main: before QCoreApplication");
    QCoreApplication app(argc, argv);
    dbgLog("[DBG] main: after QCoreApplication, before qExec");
    TestCoreLogic tc;
    int result = QTest::qExec(&tc, argc, argv);
    dbgLog(QString("[DBG] main: after qExec, result=%1").arg(result));
    return result;
}
#include "test_core_logic.moc"
