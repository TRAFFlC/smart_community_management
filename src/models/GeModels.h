#ifndef GEMODELS_H
#define GEMODELS_H

#include <QDateTime>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QVariantMap>

#include "BaseModels.h"

struct GeEvent : BaseEntity {
    QString eventNo;
    qint64 gridId = 0;
    qint64 communityOrgId = 0;
    qint64 reporterId = 0;
    QString reporterName;
    int eventCategory = 0;
    int priority = 1;
    QString title;
    QString description;
    QString location;
    double longitude = 0;
    double latitude = 0;
    QString images;
    int status = 0;
    qint64 reviewerId = 0;
    QDateTime reviewTime;
    qint64 assignTo = 0;
    qint64 assignOrgId = 0;
    QDateTime assignTime;
    QDateTime finishTime;
    QDateTime archiveTime;
    QString resultDesc;
    QString resultImages;
    QDateTime slaDeadline;
    int isSupervised = 0;
    int isCoordinated = 0;
    int source = 0;

    static GeEvent fromQuery(QSqlQuery& q) {
        GeEvent e;
        e.id = q.value("id").toLongLong();
        e.eventNo = q.value("event_no").toString();
        e.gridId = q.value("grid_id").toLongLong();
        e.communityOrgId = q.value("community_org_id").toLongLong();
        e.reporterId = q.value("reporter_id").toLongLong();
        e.reporterName = q.value("reporter_name").toString();
        e.eventCategory = q.value("event_category").toInt();
        e.priority = q.value("priority").toInt();
        e.title = q.value("title").toString();
        e.description = q.value("description").toString();
        e.location = q.value("location").toString();
        e.longitude = q.value("longitude").toDouble();
        e.latitude = q.value("latitude").toDouble();
        e.images = q.value("images").toString();
        e.status = q.value("status").toInt();
        e.reviewerId = q.value("reviewer_id").toLongLong();
        e.reviewTime = q.value("review_time").toDateTime();
        e.assignTo = q.value("assign_to").toLongLong();
        e.assignOrgId = q.value("assign_org_id").toLongLong();
        e.assignTime = q.value("assign_time").toDateTime();
        e.finishTime = q.value("finish_time").toDateTime();
        e.archiveTime = q.value("archive_time").toDateTime();
        e.resultDesc = q.value("result_desc").toString();
        e.resultImages = q.value("result_images").toString();
        e.slaDeadline = q.value("sla_deadline").toDateTime();
        e.isSupervised = q.value("is_supervised").toInt();
        e.isCoordinated = q.value("is_coordinated").toInt();
        e.source = q.value("source").toInt();
        e.createBy = q.value("create_by").toLongLong();
        e.createTime = q.value("create_time").toDateTime();
        e.updateBy = q.value("update_by").toLongLong();
        e.updateTime = q.value("update_time").toDateTime();
        e.delFlag = q.value("del_flag").toInt();
        e.remark = q.value("remark").toString();
        return e;
    }

    static GeEvent fromRecord(const QSqlRecord& r) {
        GeEvent e;
        e.id = r.value("id").toLongLong();
        e.eventNo = r.value("event_no").toString();
        e.gridId = r.value("grid_id").toLongLong();
        e.communityOrgId = r.value("community_org_id").toLongLong();
        e.reporterId = r.value("reporter_id").toLongLong();
        e.reporterName = r.value("reporter_name").toString();
        e.eventCategory = r.value("event_category").toInt();
        e.priority = r.value("priority").toInt();
        e.title = r.value("title").toString();
        e.description = r.value("description").toString();
        e.location = r.value("location").toString();
        e.longitude = r.value("longitude").toDouble();
        e.latitude = r.value("latitude").toDouble();
        e.images = r.value("images").toString();
        e.status = r.value("status").toInt();
        e.reviewerId = r.value("reviewer_id").toLongLong();
        e.reviewTime = r.value("review_time").toDateTime();
        e.assignTo = r.value("assign_to").toLongLong();
        e.assignOrgId = r.value("assign_org_id").toLongLong();
        e.assignTime = r.value("assign_time").toDateTime();
        e.finishTime = r.value("finish_time").toDateTime();
        e.archiveTime = r.value("archive_time").toDateTime();
        e.resultDesc = r.value("result_desc").toString();
        e.resultImages = r.value("result_images").toString();
        e.slaDeadline = r.value("sla_deadline").toDateTime();
        e.isSupervised = r.value("is_supervised").toInt();
        e.isCoordinated = r.value("is_coordinated").toInt();
        e.source = r.value("source").toInt();
        e.createBy = r.value("create_by").toLongLong();
        e.createTime = r.value("create_time").toDateTime();
        e.updateBy = r.value("update_by").toLongLong();
        e.updateTime = r.value("update_time").toDateTime();
        e.delFlag = r.value("del_flag").toInt();
        e.remark = r.value("remark").toString();
        return e;
    }

    QVariantMap toVariantMap() const {
        return {{"id", id},
                {"event_no", eventNo},
                {"grid_id", gridId},
                {"community_org_id", communityOrgId},
                {"reporter_id", reporterId},
                {"reporter_name", reporterName},
                {"event_category", eventCategory},
                {"priority", priority},
                {"title", title},
                {"description", description},
                {"location", location},
                {"longitude", longitude},
                {"latitude", latitude},
                {"images", images},
                {"status", status},
                {"reviewer_id", reviewerId},
                {"review_time", reviewTime},
                {"assign_to", assignTo},
                {"assign_org_id", assignOrgId},
                {"assign_time", assignTime},
                {"finish_time", finishTime},
                {"archive_time", archiveTime},
                {"result_desc", resultDesc},
                {"result_images", resultImages},
                {"sla_deadline", slaDeadline},
                {"is_supervised", isSupervised},
                {"is_coordinated", isCoordinated},
                {"source", source},
                {"create_by", createBy},
                {"create_time", createTime},
                {"update_by", updateBy},
                {"update_time", updateTime},
                {"del_flag", delFlag},
                {"remark", remark}};
    }
};

struct GeEventFlow {
    qint64 id = 0;
    qint64 eventId = 0;
    QString action;
    qint64 operatorId = 0;
    QString operatorName;
    int fromStatus = 0;
    int toStatus = 0;
    QString comment;
    QString images;
    QDateTime actionTime;
};

struct GeInspection : BaseEntity {
    qint64 gridId = 0;
    qint64 inspectorId = 0;
    qint64 planId = 0;
    QDateTime startTime;
    QDateTime endTime;
    int duration = 0;
    QString routePoints;
    int issueCount = 0;
    QString summary;
    int status = 0;
};

struct GeVisitRecord : BaseEntity {
    qint64 specialGroupId = 0;
    qint64 residentId = 0;
    qint64 visitorId = 0;
    QDateTime visitTime;
    int visitType = 1;
    QString content;
    QString images;
    QString foundIssues;
    QString followUp;
};

struct GeSupervision : BaseEntity {
    qint64 eventId = 0;
    qint64 supervisorId = 0;
    qint64 superviseTo = 0;
    QDateTime deadline;
    QString reason;
    int status = 0;
    QString feedback;
    QDateTime feedbackTime;
};

#endif // GEMODELS_H
