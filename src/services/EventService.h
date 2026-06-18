#ifndef EVENTSERVICE_H
#define EVENTSERVICE_H

#include <QDateTime>
#include <QList>
#include <QPair>
#include <QString>
#include <QVariantMap>

#include "models/Models.h"

class EventService {
public:
    struct EventQueryParams {
        QString keyword;
        int statusFilter = -1; // -1 表示全部
        bool onlyMine = false;
        int pageSize = 20;
        int offset = 0;
    };

    struct EventListResult {
        QList<GeEvent> items;
        int totalCount = 0;
    };

    struct CompleteResult {
        bool success = false;
        qint64 reporterId = 0;
        QString title;
    };

    static EventService& instance();

    // 列表查询（带数据权限、关键词、状态、只看我的、分页）
    EventListResult queryEvents(const EventQueryParams& params);

    // 某上报人最近事件
    QList<GeEvent> queryEventsByReporter(qint64 reporterId, int limit = 5);

    // 待审核事件数；reporterId >= 0 则只统计该上报人
    int countPendingEvents(qint64 reporterId = -1);

    // 根据 ID 查询单个事件
    GeEvent getEventById(qint64 id);

    // 通用字段更新
    bool updateEvent(qint64 id, const QVariantMap& data);

    // 状态更新（含相关时间/操作人字段）
    bool updateEventStatus(qint64 id, int newStatus, qint64 operatorId,
                           const QDateTime& now = QDateTime::currentDateTime());

    // 明确的业务流转方法
    bool reviewEvent(qint64 id, qint64 reviewerId,
                     const QDateTime& now = QDateTime::currentDateTime());
    bool assignEvent(qint64 id, qint64 assignTo, qint64 operatorId,
                     const QString& comment = QString(),
                     const QDateTime& now = QDateTime::currentDateTime());
    bool processEvent(qint64 id, qint64 handlerId,
                      const QDateTime& now = QDateTime::currentDateTime());
    CompleteResult completeEvent(qint64 id, const QString& resultDesc, qint64 operatorId,
                                 const QDateTime& now = QDateTime::currentDateTime());

    // 新增事件上报
    qint64 reportEvent(const QString& title, int eventCategory, int priority,
                       const QString& description, const QString& location,
                       const QString& images, qint64 reporterId,
                       const QString& reporterName,
                       const QDateTime& now = QDateTime::currentDateTime());

    // 可分派的网格/社区工作人员列表
    QList<QPair<qint64, QString>> listGridWorkers();

private:
    EventService() = default;
    ~EventService() = default;
    EventService(const EventService&) = delete;
    EventService& operator=(const EventService&) = delete;

    // 插入事件流转日志
    bool insertEventFlow(qint64 eventId, const QString& action, qint64 operatorId,
                         const QString& operatorName, int fromStatus, int toStatus,
                         const QString& comment = QString(),
                         const QDateTime& actionTime = QDateTime::currentDateTime());
};

#endif // EVENTSERVICE_H
