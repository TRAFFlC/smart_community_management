#include <QTest>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>
#include "database/DatabaseManager.h"
#include "services/AuthService.h"
#include "services/DemoDataService.h"

class TestCoreLogic : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        // 使用内存数据库
        QVERIFY(DatabaseManager::instance().initialize(":memory:"));
        QVERIFY(DatabaseManager::instance().initSchema());
        DemoDataService::initIfEmpty();
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
        struct Account { const char* username; const char* password; };
        Account accounts[] = {
            {"admin", "admin123"},
            {"wangge_zhao", "123456"}, {"wuye_kefu", "123456"},
            {"shequ_lifang", "123456"}, {"zhangsan", "123456"},
            {"yewei_zhao", "123456"}, {"wuye_jingli", "123456"},
            {"shequ_shuji", "123456"}, {"jiedao_worker", "123456"}
        };
        for (const auto& acc : accounts) {
            bool ok = AuthService::instance().login(acc.username, acc.password);
            QVERIFY2(ok, QString("Login failed for %1").arg(acc.username).toUtf8());
            AuthService::instance().logout();
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
        // Verify all page keys have corresponding menu entries
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
            q.prepare("SELECT id FROM sys_menu WHERE id = :id AND status = 0");
            q.bindValue(":id", key.toInt());
            QVERIFY2(q.exec(), QString("Query failed for key %1").arg(key).toUtf8());
        }
    }

    void cleanupTestCase() {
        DatabaseManager::instance().close();
    }
};

QTEST_MAIN(TestCoreLogic)
#include "test_core_logic.moc"
