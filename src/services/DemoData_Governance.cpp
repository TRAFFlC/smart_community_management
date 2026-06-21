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
#include <cstring>

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
