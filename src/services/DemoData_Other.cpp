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
