#ifndef WOMODELS_H
#define WOMODELS_H

#include <QDateTime>
#include <QSqlQuery>
#include <QString>
#include <QVariantMap>

#include "BaseModels.h"

struct WoWorkOrder : BaseEntity {
    QString orderNo;
    qint64 estateId = 0;
    qint64 houseId = 0;
    qint64 reporterId = 0;
    QString reporterName;
    QString reporterPhone;
    int orderType = 0;
    int priority = 1;
    QString title;
    QString description;
    QString locationDesc;
    QString images;
    int status = 0;
    qint64 acceptBy = 0;
    QDateTime acceptTime;
    qint64 assignTo = 0;
    QDateTime assignTime;
    QDateTime finishTime;
    QDateTime closeTime;
    QString resultDesc;
    QString resultImages;
    QDateTime slaDeadline;
    int isEscalated = 0;
    qint64 escalateToEventId = 0;
    int source = 0;

    static WoWorkOrder fromQuery(QSqlQuery& q) {
        WoWorkOrder o;
        o.id = q.value("id").toLongLong();
        o.orderNo = q.value("order_no").toString();
        o.estateId = q.value("estate_id").toLongLong();
        o.houseId = q.value("house_id").toLongLong();
        o.reporterId = q.value("reporter_id").toLongLong();
        o.reporterName = q.value("reporter_name").toString();
        o.reporterPhone = q.value("reporter_phone").toString();
        o.orderType = q.value("order_type").toInt();
        o.priority = q.value("priority").toInt();
        o.title = q.value("title").toString();
        o.description = q.value("description").toString();
        o.locationDesc = q.value("location_desc").toString();
        o.images = q.value("images").toString();
        o.status = q.value("status").toInt();
        o.acceptBy = q.value("accept_by").toLongLong();
        o.acceptTime = q.value("accept_time").toDateTime();
        o.assignTo = q.value("assign_to").toLongLong();
        o.assignTime = q.value("assign_time").toDateTime();
        o.finishTime = q.value("finish_time").toDateTime();
        o.closeTime = q.value("close_time").toDateTime();
        o.resultDesc = q.value("result_desc").toString();
        o.resultImages = q.value("result_images").toString();
        o.slaDeadline = q.value("sla_deadline").toDateTime();
        o.isEscalated = q.value("is_escalated").toInt();
        o.escalateToEventId = q.value("escalate_to_event_id").toLongLong();
        o.source = q.value("source").toInt();
        o.createBy = q.value("create_by").toLongLong();
        o.createTime = q.value("create_time").toDateTime();
        o.updateBy = q.value("update_by").toLongLong();
        o.updateTime = q.value("update_time").toDateTime();
        o.delFlag = q.value("del_flag").toInt();
        o.remark = q.value("remark").toString();
        return o;
    }

    QVariantMap toVariantMap() const {
        return {{"id", id},
                {"order_no", orderNo},
                {"estate_id", estateId},
                {"house_id", houseId},
                {"reporter_id", reporterId},
                {"reporter_name", reporterName},
                {"reporter_phone", reporterPhone},
                {"order_type", orderType},
                {"priority", priority},
                {"title", title},
                {"description", description},
                {"location_desc", locationDesc},
                {"images", images},
                {"status", status},
                {"accept_by", acceptBy},
                {"accept_time", acceptTime},
                {"assign_to", assignTo},
                {"assign_time", assignTime},
                {"finish_time", finishTime},
                {"close_time", closeTime},
                {"result_desc", resultDesc},
                {"result_images", resultImages},
                {"sla_deadline", slaDeadline},
                {"is_escalated", isEscalated},
                {"escalate_to_event_id", escalateToEventId},
                {"source", source},
                {"create_by", createBy},
                {"create_time", createTime},
                {"update_by", updateBy},
                {"update_time", updateTime},
                {"del_flag", delFlag},
                {"remark", remark}};
    }
};

#endif // WOMODELS_H
