#include "EventService.h"

#include <QSqlError>
#include <QSqlQuery>

#include "database/DatabaseManager.h"
#include "services/AuthService.h"
#include "ui_kit/AuthHelpers.h"
#include "ui_kit/DbHelpers.h"
#include "ui_kit/PagedQuery.h"
#include "utils/Utils.h"

EventService& EventService::instance() {
    static EventService inst;
    return inst;
}

namespace {

// 状态筛选下拉映射：全部(-1), 待审核(0), 已审核(1), 已分派(2), 处理中(3), 已完成(4), 已归档(6)
const int STATUS_FILTER_MAP[] = {-1, 0, 1, 2, 3, 4, 6};

const QString EVENT_COLUMNS =
    "id, event_no, grid_id, community_org_id, reporter_id, reporter_name, "
    "event_category, priority, title, description, location, longitude, latitude, "
    "images, status, reviewer_id, review_time, assign_to, assign_org_id, assign_time, "
    "finish_time, archive_time, result_desc, result_images, sla_deadline, "
    "is_supervised, is_coordinated, source, create_by, create_time, update_by, update_time, "
    "del_flag, remark";

QString operatorName() {
    const auto& user = AuthService::instance().currentUser();
    return user.nickname.isEmpty() ? user.username : user.nickname;
}

} // namespace

EventService::EventListResult EventService::queryEvents(const EventQueryParams& params) {
    EventListResult result;

    QString baseSql = QStringLiteral("SELECT %1 FROM ge_event WHERE del_flag = 0").arg(EVENT_COLUMNS);
    UiKit::PagedQuery pq(baseSql);

    // 数据权限过滤
    auto scopeFilter = UiKit::buildDataScopeFilter(QString(), QStringLiteral("community_org_id"));
    if (!scopeFilter.first.isEmpty())
    {
        QString scopeSql = scopeFilter.first.trimmed();
        if (scopeSql.startsWith("AND", Qt::CaseInsensitive))
            scopeSql = scopeSql.mid(3).trimmed();
        QVariantMap scopeBinds;
        for (int i = 0; i + 1 < scopeFilter.second.size(); i += 2)
        {
            QString key = scopeFilter.second[i].toString();
            if (key.startsWith(":")) key = key.mid(1);
            scopeBinds.insert(key, scopeFilter.second[i + 1]);
        }
        pq.where(scopeSql, scopeBinds);
    }

    if (!params.keyword.isEmpty()) {
        pq.where("(event_no LIKE :search OR title LIKE :search)", {{"search", "%" + params.keyword + "%"}});
    }
    if (params.statusFilter >= 0 && params.statusFilter < 7) {
        pq.where("status = :status", {{"status", STATUS_FILTER_MAP[params.statusFilter]}});
    }
    if (params.onlyMine) {
        pq.where("assign_to = :myId", {{"myId", AuthService::instance().currentUser().id}});
    }
    pq.orderBy("create_time DESC");

    auto res = pq.execute(params.pageSize, params.offset);
    result.totalCount = res.totalCount;
    for (const auto& rec : res.rows) {
        result.items.append(GeEvent::fromRecord(rec));
    }
    return result;
}

QList<GeEvent> EventService::queryEventsByReporter(qint64 reporterId, int limit) {
    QList<GeEvent> list;
    if (reporterId <= 0) return list;

    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(QStringLiteral("SELECT %1 FROM ge_event WHERE del_flag = 0 AND reporter_id = :reporter_id ORDER BY create_time DESC LIMIT :limit")
                  .arg(EVENT_COLUMNS));
    q.bindValue(":reporter_id", reporterId);
    q.bindValue(":limit", limit);
    if (!q.exec()) {
        qWarning() << "queryEventsByReporter failed:" << q.lastError().text();
        return list;
    }
    while (q.next()) {
        list.append(GeEvent::fromQuery(q));
    }
    return list;
}

int EventService::countPendingEvents(qint64 reporterId) {
    QSqlQuery q(DatabaseManager::instance().database());
    QString sql = QStringLiteral("SELECT COUNT(*) FROM ge_event WHERE status = 0 AND del_flag = 0");
    if (reporterId >= 0) {
        sql += " AND reporter_id = :reporter_id";
        q.prepare(sql);
        q.bindValue(":reporter_id", reporterId);
    } else {
        q.prepare(sql);
    }
    if (!q.exec() || !q.next()) return 0;
    return q.value(0).toInt();
}

GeEvent EventService::getEventById(qint64 id) {
    GeEvent ev;
    if (id <= 0) return ev;
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(QStringLiteral("SELECT %1 FROM ge_event WHERE id = :id AND del_flag = 0")
                  .arg(EVENT_COLUMNS));
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) return ev;
    return GeEvent::fromQuery(q);
}

bool EventService::updateEvent(qint64 id, const QVariantMap& data) {
    return DatabaseManager::instance().update("ge_event", id, data);
}

bool EventService::updateEventStatus(qint64 id, int newStatus, qint64 operatorId,
                                     const QDateTime& now) {
    QVariantMap data;
    data["status"] = newStatus;
    data["update_by"] = operatorId;
    data["update_time"] = now;

    switch (newStatus) {
    case 1: // 已审核
        data["reviewer_id"] = operatorId;
        data["review_time"] = now;
        break;
    case 4: // 已完成
        data["finish_time"] = now;
        break;
    case 6: // 已归档
        data["archive_time"] = now;
        break;
    default:
        break;
    }
    return updateEvent(id, data);
}

bool EventService::reviewEvent(qint64 id, qint64 reviewerId, const QDateTime& now) {
    if (!updateEventStatus(id, 1, reviewerId, now)) return false;
    return insertEventFlow(id, QStringLiteral("审核"), reviewerId, operatorName(),
                           0, 1, QString(), now);
}

bool EventService::assignEvent(qint64 id, qint64 assignTo, qint64 operatorId,
                               const QString& comment, const QDateTime& now) {
    QVariantMap data;
    data["status"] = 2;
    data["assign_to"] = assignTo;
    data["assign_time"] = now;
    data["update_by"] = operatorId;
    data["update_time"] = now;
    if (!updateEvent(id, data)) return false;
    return insertEventFlow(id, QStringLiteral("分派"), operatorId, operatorName(),
                           1, 2, comment, now);
}

bool EventService::processEvent(qint64 id, qint64 handlerId, const QDateTime& now) {
    if (!updateEventStatus(id, 3, handlerId, now)) return false;
    return insertEventFlow(id, QStringLiteral("开始处理"), handlerId, operatorName(),
                           2, 3, QString(), now);
}

EventService::CompleteResult EventService::completeEvent(qint64 id, const QString& resultDesc,
                                                         qint64 operatorId,
                                                         const QDateTime& now) {
    CompleteResult result;
    GeEvent ev = getEventById(id);
    if (ev.id <= 0) return result;

    QVariantMap data;
    data["status"] = 4;
    data["finish_time"] = now;
    data["result_desc"] = resultDesc;
    data["update_by"] = operatorId;
    data["update_time"] = now;
    if (!updateEvent(id, data)) return result;

    insertEventFlow(id, QStringLiteral("完成归档"), operatorId, operatorName(),
                    3, 4, resultDesc, now);

    result.success = true;
    result.reporterId = ev.reporterId;
    result.title = ev.title;
    return result;
}

qint64 EventService::reportEvent(const QString& title, int eventCategory, int priority,
                                 const QString& description, const QString& location,
                                 const QString& images, qint64 reporterId,
                                 const QString& reporterName, const QDateTime& now) {
    QVariantMap data;
    data["event_no"] = Utils::generateEventNo();
    data["title"] = title;
    data["event_category"] = eventCategory;
    data["priority"] = priority;
    data["description"] = description;
    data["location"] = location;
    data["images"] = images.isEmpty() ? QVariant() : images;
    data["reporter_id"] = reporterId;
    data["reporter_name"] = reporterName;
    data["status"] = 0;
    data["source"] = 1;
    data["create_by"] = reporterId;
    data["create_time"] = now;
    return DatabaseManager::instance().insert("ge_event", data);
}

QList<QPair<qint64, QString>> EventService::listGridWorkers() {
    QList<QPair<qint64, QString>> workers;
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT u.id, u.nickname, u.username FROM sys_user u "
              "JOIN sys_user_role ur ON u.id = ur.user_id "
              "JOIN sys_role r ON ur.role_id = r.id "
              "WHERE r.role_key IN ('community_worker', 'grid_worker') AND u.status = 0 AND u.del_flag = 0");
    if (!q.exec()) {
        qWarning() << "listGridWorkers failed:" << q.lastError().text();
        return workers;
    }
    while (q.next()) {
        QString name = q.value(1).toString();
        if (name.isEmpty()) name = q.value(2).toString();
        workers.append(qMakePair(q.value(0).toLongLong(), name));
    }
    return workers;
}

bool EventService::insertEventFlow(qint64 eventId, const QString& action, qint64 operatorId,
                                   const QString& operatorName, int fromStatus, int toStatus,
                                   const QString& comment, const QDateTime& actionTime) {
    QVariantMap data;
    data["event_id"] = eventId;
    data["action"] = action;
    data["operator_id"] = operatorId;
    data["operator_name"] = operatorName;
    data["from_status"] = fromStatus;
    data["to_status"] = toStatus;
    data["comment"] = comment.isEmpty() ? QVariant() : comment;
    data["action_time"] = actionTime;
    return DatabaseManager::instance().insert("ge_event_flow", data) > 0;
}
