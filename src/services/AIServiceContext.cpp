#include "AIService.h"

#include <QMap>
#include <QSqlQuery>

void AIService::bindScopeParams(QSqlQuery& q, const QVariantList& params) {
    for (int i = 0; i + 1 < params.size(); i += 2) {
        q.bindValue(params[i].toString(), params[i + 1]);
    }
}

QString AIService::buildSafeContext(const QString& question) {
    auto& auth = AuthService::instance();
    if (!auth.isLoggedIn()) return QStringLiteral("（用户未登录）");

    const auto& user = auth.currentUser();
    auto& db = DatabaseManager::instance();
    QStringList ctx;

    // 用户基本信息
    ctx << QStringLiteral("当前用户：%1（%2）").arg(user.username, user.roleNames.join("、"));

    // 根据问题关键词动态检索相关数据
    QString q = question.toLower();

    // 工单相关
    if (q.contains("工单") || q.contains("报修") || q.contains("投诉")) {
        ctx << queryWorkOrderContext(db, user);
    }
    // 事件相关
    if (q.contains("事件") || q.contains("网格") || q.contains("巡查")) {
        ctx << queryEventContext(db, user);
    }
    // 公告相关
    if (q.contains("公告") || q.contains("通知")) {
        ctx << queryAnnouncementContext(db, user);
    }
    // 服务相关
    if (q.contains("服务") || q.contains("志愿") || q.contains("便民")) {
        ctx << queryServiceContext(db, user);
    }
    // 停车/缴费相关
    if (q.contains("停车") || q.contains("月卡") || q.contains("缴费") || q.contains("物业费")) {
        ctx << queryBillingContext(db, user);
    }
    // 居民/小区相关
    if (q.contains("居民") || q.contains("小区") || q.contains("楼栋")) {
        ctx << queryResidentContext(db, user);
    }

    // 如果没有匹配到特定领域，提供通用概览
    if (ctx.size() <= 1) {
        ctx << queryOverviewContext(db, user);
    }

    return ctx.join("\n");
}

QString AIService::queryWorkOrderContext(DatabaseManager& db, const SysUser& user) {
    QStringList lines;
    lines << "【工单信息】";

    bool isResident = (user.userType == UserType::Resident);
    int scope = AuthService::instance().currentUserDataScope(); // 1=平台级

    if (isResident) {
        // 居民仅查看自己提交的工单，不返回全平台总数
        QSqlQuery q(db.database());
        q.prepare("SELECT status, COUNT(*) FROM wo_work_order "
                  "WHERE del_flag = 0 AND create_by = :uid GROUP BY status");
        q.bindValue(":uid", user.id);
        q.exec();
        int total = 0;
        QMap<int, int> statusMap;
        while (q.next()) {
            int s = q.value(0).toInt(), c = q.value(1).toInt();
            statusMap[s] = c; total += c;
        }
        lines << QStringLiteral("我的工单总数：%1，待受理：%2，处理中：%3，已完成：%4")
                 .arg(total).arg(statusMap.value(0, 0))
                 .arg(statusMap.value(2, 0) + statusMap.value(3, 0))
                 .arg(statusMap.value(4, 0));

        // 最近 5 条我的工单
        QSqlQuery recent(db.database());
        recent.prepare("SELECT order_no, title, order_type, priority, status, create_time "
                       "FROM wo_work_order WHERE del_flag = 0 AND create_by = :uid "
                       "ORDER BY create_time DESC LIMIT 5");
        recent.bindValue(":uid", user.id);
        recent.exec();
        while (recent.next()) {
            lines << QStringLiteral("  %1 | %2 | %3 | %4")
                     .arg(recent.value(0).toString(), recent.value(1).toString(),
                          WorkOrderStatus::label(recent.value(4).toInt()),
                          recent.value(5).toDateTime().toString("MM-dd hh:mm"));
        }
    } else {
        // 工作人员/管理员：按数据权限过滤
        auto scopeFilter = UiKit::buildDataScopeFilter(QString(), QStringLiteral("estate_id"));
        if (!scopeFilter.first.isEmpty()) {
            lines << QStringLiteral("（数据范围：%1）").arg(DataScope::label(scope));
        }
        // 总数统计
        QSqlQuery q(db.database());
        QString sql = QStringLiteral("SELECT status, COUNT(*) FROM wo_work_order WHERE del_flag = 0");
        sql += scopeFilter.first;
        sql += QStringLiteral(" GROUP BY status");
        q.prepare(sql);
        bindScopeParams(q, scopeFilter.second);
        q.exec();
        int total = 0;
        QMap<int, int> statusMap;
        while (q.next()) {
            int s = q.value(0).toInt(), c = q.value(1).toInt();
            statusMap[s] = c; total += c;
        }
        lines << QStringLiteral("工单总数：%1，待受理：%2，处理中：%3，已完成：%4")
                 .arg(total).arg(statusMap.value(0, 0)).arg(statusMap.value(2, 0) + statusMap.value(3, 0)).arg(statusMap.value(4, 0));

        // 最近 5 条工单（脱敏）
        QSqlQuery recent(db.database());
        QString recentSql = QStringLiteral("SELECT order_no, title, order_type, priority, status, create_time "
                                           "FROM wo_work_order WHERE del_flag = 0");
        recentSql += scopeFilter.first;
        recentSql += QStringLiteral(" ORDER BY create_time DESC LIMIT 5");
        recent.prepare(recentSql);
        bindScopeParams(recent, scopeFilter.second);
        recent.exec();
        while (recent.next()) {
            lines << QStringLiteral("  %1 | %2 | %3 | %4")
                     .arg(recent.value(0).toString(), recent.value(1).toString(),
                          WorkOrderStatus::label(recent.value(4).toInt()),
                          recent.value(5).toDateTime().toString("MM-dd hh:mm"));
        }
    }
    return lines.join("\n");
}

QString AIService::queryEventContext(DatabaseManager& db, const SysUser& user) {
    QStringList lines;
    lines << "【网格事件】";

    bool isResident = (user.userType == UserType::Resident);
    int scope = AuthService::instance().currentUserDataScope(); // 1=平台级

    QSqlQuery q(db.database());
    if (isResident) {
        // 居民仅查看自己上报的事件
        q.prepare("SELECT event_no, title, event_category, status, create_time "
                  "FROM ge_event WHERE del_flag = 0 AND create_by = :uid "
                  "ORDER BY create_time DESC LIMIT 5");
        q.bindValue(":uid", user.id);
        q.exec();
        bool hasData = false;
        while (q.next()) {
            hasData = true;
            lines << QStringLiteral("  %1 | %2 | %3")
                     .arg(q.value(0).toString(), q.value(1).toString(),
                          EventStatus::label(q.value(3).toInt()));
        }
        if (!hasData) {
            lines << QStringLiteral("  暂无相关事件");
        }
    } else {
        // 工作人员/管理员：按数据权限过滤
        auto scopeFilter = UiKit::buildDataScopeFilter(QString(), QStringLiteral("community_org_id"));
        if (!scopeFilter.first.isEmpty()) {
            lines << QStringLiteral("（数据范围：%1）").arg(DataScope::label(scope));
        }
        QString sql = QStringLiteral("SELECT event_no, title, event_category, status, create_time "
                                     "FROM ge_event WHERE del_flag = 0");
        sql += scopeFilter.first;
        sql += QStringLiteral(" ORDER BY create_time DESC LIMIT 5");
        q.prepare(sql);
        bindScopeParams(q, scopeFilter.second);
        q.exec();
        while (q.next()) {
            lines << QStringLiteral("  %1 | %2 | %3")
                     .arg(q.value(0).toString(), q.value(1).toString(),
                          EventStatus::label(q.value(3).toInt()));
        }
    }
    return lines.join("\n");
}

QString AIService::queryAnnouncementContext(DatabaseManager& db, const SysUser& user) {
    Q_UNUSED(user)
    QStringList lines;
    lines << "【公告通知】";
    QSqlQuery q(db.database());
    q.prepare("SELECT title, announcement_type, publish_time "
              "FROM nt_announcement WHERE del_flag = 0 ORDER BY publish_time DESC LIMIT 5");
    q.exec();
    while (q.next()) {
        lines << QStringLiteral("  %1 | %2")
                 .arg(q.value(0).toString(),
                      q.value(2).toDateTime().toString("MM-dd"));
    }
    return lines.join("\n");
}

QString AIService::queryServiceContext(DatabaseManager& db, const SysUser& user) {
    Q_UNUSED(user)
    QStringList lines;
    lines << "【社区服务】";
    QSqlQuery q(db.database());
    q.prepare("SELECT title, activity_type, status, start_time "
              "FROM sv_volunteer_activity WHERE del_flag = 0 ORDER BY start_time DESC LIMIT 3");
    q.exec();
    while (q.next()) {
        lines << QStringLiteral("  志愿活动：%1 | %2")
                 .arg(q.value(0).toString(), VolunteerActivityStatus::label(q.value(2).toInt()));
    }
    return lines.join("\n");
}

QString AIService::queryBillingContext(DatabaseManager& db, const SysUser& user) {
    QStringList lines;
    lines << "【缴费信息】";

    bool isResident = (user.userType == UserType::Resident);
    int scope = AuthService::instance().currentUserDataScope(); // 1=平台级

    QSqlQuery q(db.database());
    if (isResident) {
        // 居民仅查看自己的账单（通过 user_id 关联 cm_resident 找到 resident_id）
        q.prepare("SELECT bill_type, amount, period, status "
                  "FROM pm_bill WHERE del_flag = 0 "
                  "AND resident_id = (SELECT id FROM cm_resident WHERE user_id = :uid AND del_flag = 0 LIMIT 1) "
                  "ORDER BY create_time DESC LIMIT 5");
        q.bindValue(":uid", user.id);
        q.exec();
        bool hasData = false;
        while (q.next()) {
            hasData = true;
            lines << QStringLiteral("  账单 | %1元 | %2")
                     .arg(q.value(1).toDouble(), 0, 'f', 2)
                     .arg(q.value(2).toString());
        }
        if (!hasData) {
            lines << QStringLiteral("  暂无缴费记录");
        }
    } else {
        // 工作人员/管理员：按数据权限过滤
        auto scopeFilter = UiKit::buildDataScopeFilter(QString(), QStringLiteral("estate_id"));
        if (!scopeFilter.first.isEmpty()) {
            lines << QStringLiteral("（数据范围：%1）").arg(DataScope::label(scope));
        }
        QString sql = QStringLiteral("SELECT bill_type, amount, period, status "
                                     "FROM pm_bill WHERE del_flag = 0");
        sql += scopeFilter.first;
        sql += QStringLiteral(" ORDER BY create_time DESC LIMIT 5");
        q.prepare(sql);
        bindScopeParams(q, scopeFilter.second);
        q.exec();
        while (q.next()) {
            lines << QStringLiteral("  账单 | %1元 | %2")
                     .arg(q.value(1).toDouble(), 0, 'f', 2)
                     .arg(q.value(2).toString());
        }
    }
    return lines.join("\n");
}

QString AIService::queryResidentContext(DatabaseManager& db, const SysUser& user) {
    QStringList lines;
    lines << "【居民/小区概况】";

    bool isResident = (user.userType == UserType::Resident);
    int scope = AuthService::instance().currentUserDataScope(); // 1=平台级

    if (isResident) {
        // 居民不返回平台级聚合数据，仅返回本人档案信息
        QSqlQuery q(db.database());
        q.prepare("SELECT r.name, r.phone_display, e.estate_name, h.room_number "
                  "FROM cm_resident r "
                  "LEFT JOIN cm_house_resident hr ON hr.resident_id = r.id "
                  "LEFT JOIN cm_house h ON h.id = hr.house_id "
                  "LEFT JOIN cm_estate e ON e.id = h.estate_id "
                  "WHERE r.del_flag = 0 AND r.user_id = :uid "
                  "ORDER BY hr.id DESC LIMIT 1");
        q.bindValue(":uid", user.id);
        q.exec();
        if (q.next()) {
            QString name = q.value(0).toString();
            QString phone = q.value(1).toString();
            QString estate = q.value(2).toString();
            QString room = q.value(3).toString();
            lines << QStringLiteral("您是%1的居民%2")
                     .arg(estate.isEmpty() ? QStringLiteral("本社区") : estate,
                          name.isEmpty() ? user.username : name);
            if (!room.isEmpty()) {
                lines << QStringLiteral("住房：%1 %2").arg(estate, room);
            }
            if (!phone.isEmpty()) {
                lines << QStringLiteral("联系电话：%1").arg(phone);
            }
        } else {
            lines << QStringLiteral("您当前以居民身份登录，暂无居民档案信息");
        }
        lines << QStringLiteral("（注：居民无权查看平台概况数据）");
    } else {
        // 工作人员/管理员：按数据权限过滤
        auto scopeFilter = UiKit::buildDataScopeFilter(QString(), QStringLiteral("estate_id"));
        if (!scopeFilter.first.isEmpty()) {
            lines << QStringLiteral("（数据范围：%1）").arg(DataScope::label(scope));
        }
        QStringList phList;
        for (int i = 0; i + 1 < scopeFilter.second.size(); i += 2) {
            phList << scopeFilter.second[i].toString();
        }
        QString estatePh = phList.join(QStringLiteral(","));

        QSqlQuery q(db.database());
        if (estatePh.isEmpty()) {
            q.prepare("SELECT COUNT(*) FROM cm_resident WHERE del_flag = 0");
            q.exec(); if (q.next()) lines << QStringLiteral("居民总数：%1").arg(q.value(0).toInt());
            q.prepare("SELECT COUNT(*) FROM cm_estate WHERE del_flag = 0");
            q.exec(); if (q.next()) lines << QStringLiteral("小区数量：%1").arg(q.value(0).toInt());
            q.prepare("SELECT COUNT(*) FROM cm_house WHERE del_flag = 0");
            q.exec(); if (q.next()) lines << QStringLiteral("房屋总数：%1").arg(q.value(0).toInt());
        } else {
            q.prepare(QStringLiteral("SELECT COUNT(DISTINCT r.id) FROM cm_resident r "
                                     "JOIN cm_house_resident hr ON hr.resident_id = r.id "
                                     "JOIN cm_house h ON h.id = hr.house_id "
                                     "WHERE r.del_flag = 0 AND h.estate_id IN (%1)").arg(estatePh));
            bindScopeParams(q, scopeFilter.second);
            q.exec(); if (q.next()) lines << QStringLiteral("居民总数：%1").arg(q.value(0).toInt());
            q.prepare(QStringLiteral("SELECT COUNT(*) FROM cm_estate WHERE del_flag = 0 AND id IN (%1)").arg(estatePh));
            bindScopeParams(q, scopeFilter.second);
            q.exec(); if (q.next()) lines << QStringLiteral("小区数量：%1").arg(q.value(0).toInt());
            q.prepare(QStringLiteral("SELECT COUNT(*) FROM cm_house WHERE del_flag = 0 AND estate_id IN (%1)").arg(estatePh));
            bindScopeParams(q, scopeFilter.second);
            q.exec(); if (q.next()) lines << QStringLiteral("房屋总数：%1").arg(q.value(0).toInt());
        }
    }
    return lines.join("\n");
}

QString AIService::queryOverviewContext(DatabaseManager& db, const SysUser& user) {
    QStringList lines;
    bool isResident = (user.userType == UserType::Resident);
    if (isResident) {
        // 居民概览：仅返回个人相关数据，不展示平台级聚合信息
        lines << "【我的概览】";
        lines << queryResidentContext(db, user);
        lines << queryWorkOrderContext(db, user);
        lines << queryEventContext(db, user);
    } else {
        // 工作人员/管理员概览
        lines << "【平台概览】";
        lines << queryWorkOrderContext(db, user);
        lines << queryEventContext(db, user);
    }
    return lines.join("\n");
}
