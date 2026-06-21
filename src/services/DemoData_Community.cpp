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
