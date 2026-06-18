#ifndef MODELS_H
#define MODELS_H

#include <QString>
#include <QDateTime>
#include <QVariant>
#include <QVariantMap>
#include <QSqlQuery>
#include <QList>

// ========== 通用基类 ==========
struct BaseEntity {
    qint64 id = 0;
    qint64 createBy = 0;
    QDateTime createTime;
    qint64 updateBy = 0;
    QDateTime updateTime;
    int delFlag = 0;
    QString remark;
};

// ========== SYS 系统管理域 ==========

struct SysUser : BaseEntity {
    QString username;
    QString password;
    QString nickname;
    QString realName;
    QString avatar;
    QString phone;
    QString email;
    QString idCard;
    int gender = 0;
    QDate birthday;
    int status = 0;
    int userType = 0;
    QDateTime lastLoginTime;
    QString lastLoginIp;

    // 运行时关联(非数据库字段)
    QList<int> roleIds;
    QList<int> orgIds;
    QStringList roleNames;
    QStringList permissions;
};

struct SysRole : BaseEntity {
    QString roleName;
    QString roleKey;
    QString roleDomain;
    int dataScope = 6;
    int sortOrder = 0;
    int status = 0;
};

struct SysMenu {
    qint64 id = 0;
    qint64 parentId = 0;
    QString menuName;
    int menuType = 0;
    QString path;
    QString component;
    QString permission;
    QString icon;
    int sortOrder = 0;
    int visible = 1;
    int status = 0;
    QList<SysMenu> children;
};

struct SysOrg : BaseEntity {
    qint64 parentId = 0;
    QString orgName;
    int orgType = 0;
    QString orgCode;
    QString leader;
    QString phone;
    QString address;
    QString ancestors;
    int sortOrder = 0;
    int status = 0;
};

struct SysDictType {
    qint64 id = 0;
    QString dictName;
    QString dictType;
    int status = 0;
    QString remark;
};

struct SysDictData {
    qint64 id = 0;
    QString dictType;
    QString dictLabel;
    QString dictValue;
    int sortOrder = 0;
    QString cssClass;
    int status = 0;
};

struct SysOperationLog {
    qint64 id = 0;
    qint64 userId = 0;
    QString username;
    QString module;
    QString operation;
    QString method;
    QString requestUrl;
    QString requestMethod;
    QString requestParams;
    QString responseResult;
    QString ip;
    int status = 0;
    QString errorMsg;
    qint64 costTime = 0;
    QDateTime operationTime;
};

struct SysLoginLog {
    qint64 id = 0;
    qint64 userId = 0;
    QString username;
    QString ip;
    QString location;
    QString browser;
    QString os;
    int status = 0;
    QString message;
    QDateTime loginTime;
};

// ========== CM 基础档案域 ==========

struct CmEstate : BaseEntity {
    qint64 orgId = 0;
    qint64 propertyCompanyId = 0;
    QString estateName;
    QString estateCode;
    QString address;
    double totalArea = 0;
    int totalBuildings = 0;
    int totalHouses = 0;
    int builtYear = 0;
    double greenRate = 0;
    int parkingSpaces = 0;
    double longitude = 0;
    double latitude = 0;
    int status = 0;

    static CmEstate fromQuery(QSqlQuery& q) {
        CmEstate e;
        e.id = q.value(0).toLongLong();
        e.orgId = q.value(1).toLongLong();
        e.propertyCompanyId = q.value(2).toLongLong();
        e.estateName = q.value(3).toString();
        e.estateCode = q.value(4).toString();
        e.address = q.value(5).toString();
        e.totalArea = q.value(6).toDouble();
        e.totalBuildings = q.value(7).toInt();
        e.totalHouses = q.value(8).toInt();
        e.builtYear = q.value(9).toInt();
        e.greenRate = q.value(10).toDouble();
        e.parkingSpaces = q.value(11).toInt();
        e.longitude = q.value(12).toDouble();
        e.latitude = q.value(13).toDouble();
        e.status = q.value(14).toInt();
        e.createBy = q.value(15).toLongLong();
        e.createTime = q.value(16).toDateTime();
        e.updateBy = q.value(17).toLongLong();
        e.updateTime = q.value(18).toDateTime();
        e.delFlag = q.value(19).toInt();
        e.remark = q.value(20).toString();
        return e;
    }

    QVariantMap toVariantMap() const {
        return {{"id", id},
                {"org_id", orgId},
                {"property_company_id", propertyCompanyId},
                {"estate_name", estateName},
                {"estate_code", estateCode},
                {"address", address},
                {"total_area", totalArea},
                {"total_buildings", totalBuildings},
                {"total_houses", totalHouses},
                {"built_year", builtYear},
                {"green_rate", greenRate},
                {"parking_spaces", parkingSpaces},
                {"longitude", longitude},
                {"latitude", latitude},
                {"status", status},
                {"create_by", createBy},
                {"create_time", createTime},
                {"update_by", updateBy},
                {"update_time", updateTime},
                {"del_flag", delFlag},
                {"remark", remark}};
    }
};

struct CmBuilding : BaseEntity {
    qint64 estateId = 0;
    QString buildingName;
    QString buildingCode;
    int totalUnits = 0;
    int totalFloors = 0;
    int hasElevator = 0;
    int elevatorCount = 0;
};

struct CmUnit : BaseEntity {
    qint64 buildingId = 0;
    QString unitName;
    QString unitCode;
};

struct CmHouse : BaseEntity {
    qint64 unitId = 0;
    qint64 estateId = 0;
    QString houseCode;
    int floor = 0;
    QString roomNumber;
    double area = 0;
    QString houseType;
    int houseStatus = 0;
    QString ownerName;
    QString ownerPhone;

    static CmHouse fromQuery(QSqlQuery& q) {
        CmHouse h;
        h.id = q.value(0).toLongLong();
        h.unitId = q.value(1).toLongLong();
        h.estateId = q.value(2).toLongLong();
        h.houseCode = q.value(3).toString();
        h.floor = q.value(4).toInt();
        h.roomNumber = q.value(5).toString();
        h.area = q.value(6).toDouble();
        h.houseType = q.value(7).toString();
        h.houseStatus = q.value(8).toInt();
        h.ownerName = q.value(9).toString();
        h.ownerPhone = q.value(10).toString();
        h.createBy = q.value(11).toLongLong();
        h.createTime = q.value(12).toDateTime();
        h.updateBy = q.value(13).toLongLong();
        h.updateTime = q.value(14).toDateTime();
        h.delFlag = q.value(15).toInt();
        h.remark = q.value(16).toString();
        return h;
    }

    QVariantMap toVariantMap() const {
        return {{"id", id},
                {"unit_id", unitId},
                {"estate_id", estateId},
                {"house_code", houseCode},
                {"floor", floor},
                {"room_number", roomNumber},
                {"area", area},
                {"house_type", houseType},
                {"house_status", houseStatus},
                {"owner_name", ownerName},
                {"owner_phone", ownerPhone},
                {"create_by", createBy},
                {"create_time", createTime},
                {"update_by", updateBy},
                {"update_time", updateTime},
                {"del_flag", delFlag},
                {"remark", remark}};
    }
};

struct CmResident : BaseEntity {
    qint64 userId = 0;
    QString name;
    QString idCard;
    int gender = 0;
    QString phone;
    QString phoneDisplay;
    QDate birthday;
    QString photo;
    QString nationality;
    QString education;
    QString occupation;
    QString politicalStatus;
    int isSpecial = 0;
    QString specialType;
    QString emergencyContact;
    QString emergencyPhone;
    int status = 0;

    static CmResident fromQuery(QSqlQuery& q) {
        CmResident r;
        r.id = q.value(0).toLongLong();
        r.userId = q.value(1).toLongLong();
        r.name = q.value(2).toString();
        r.idCard = q.value(3).toString();
        r.gender = q.value(4).toInt();
        r.phone = q.value(5).toString();
        r.phoneDisplay = q.value(6).toString();
        r.birthday = q.value(7).toDate();
        r.photo = q.value(8).toString();
        r.nationality = q.value(9).toString();
        r.education = q.value(10).toString();
        r.occupation = q.value(11).toString();
        r.politicalStatus = q.value(12).toString();
        r.isSpecial = q.value(13).toInt();
        r.specialType = q.value(14).toString();
        r.emergencyContact = q.value(15).toString();
        r.emergencyPhone = q.value(16).toString();
        r.status = q.value(17).toInt();
        r.createBy = q.value(18).toLongLong();
        r.createTime = q.value(19).toDateTime();
        r.updateBy = q.value(20).toLongLong();
        r.updateTime = q.value(21).toDateTime();
        r.delFlag = q.value(22).toInt();
        r.remark = q.value(23).toString();
        return r;
    }

    QVariantMap toVariantMap() const {
        return {{"id", id},
                {"user_id", userId},
                {"name", name},
                {"id_card", idCard},
                {"gender", gender},
                {"phone", phone},
                {"phone_display", phoneDisplay},
                {"birthday", birthday},
                {"photo", photo},
                {"nationality", nationality},
                {"education", education},
                {"occupation", occupation},
                {"political_status", politicalStatus},
                {"is_special", isSpecial},
                {"special_type", specialType},
                {"emergency_contact", emergencyContact},
                {"emergency_phone", emergencyPhone},
                {"status", status},
                {"create_by", createBy},
                {"create_time", createTime},
                {"update_by", updateBy},
                {"update_time", updateTime},
                {"del_flag", delFlag},
                {"remark", remark}};
    }
};

struct CmFamily : BaseEntity {
    QString familyName;
    qint64 houseId = 0;
    qint64 headResidentId = 0;
    int memberCount = 0;
};

struct CmFamilyMember {
    qint64 id = 0;
    qint64 familyId = 0;
    qint64 residentId = 0;
    QString relation;
    int isHead = 0;
};

struct CmHouseResident {
    qint64 id = 0;
    qint64 houseId = 0;
    qint64 residentId = 0;
    int relationType = 1;
    QDate startDate;
    QDate endDate;
    int status = 0;
};

struct CmVehicle : BaseEntity {
    QString plateNumber;
    QString vehicleBrand;
    QString vehicleColor;
    int vehicleType = 1;
    qint64 estateId = 0;
    qint64 parkingSpaceId = 0;
    int status = 0;
};

struct CmParkingSpace : BaseEntity {
    qint64 estateId = 0;
    QString spaceCode;
    QString areaName;
    int spaceType = 1;
    int status = 0;
};

struct CmFacility : BaseEntity {
    qint64 estateId = 0;
    QString facilityName;
    int facilityType = 0;
    QString facilityCode;
    QString location;
    QDate installDate;
    QDate warrantyExpire;
    QDate lastMaintenance;
    QDate nextMaintenance;
    int status = 0;
};

struct CmGrid : BaseEntity {
    qint64 communityOrgId = 0;
    QString gridName;
    QString gridCode;
    QString description;
    qint64 gridWorkerId = 0;
    QString coverEstates;
    double longitude = 0;
    double latitude = 0;
    int status = 0;
};

struct CmPropertyCompany : BaseEntity {
    qint64 orgId = 0;
    QString companyName;
    QString legalPerson;
    QString licenseNo;
    QString serviceScope;
    QString contactPhone;
    QDate contractStart;
    QDate contractEnd;
    int status = 0;
};

struct CmOwnerCommittee : BaseEntity {
    qint64 orgId = 0;
    qint64 estateId = 0;
    QString committeeName;
    QDate establishedDate;
    QDate termStart;
    QDate termEnd;
    int memberCount = 0;
    int status = 0;
};

struct CmSpecialGroup : BaseEntity {
    qint64 residentId = 0;
    int groupType = 0;
    QString detail;
    int careLevel = 1;
    QString careFrequency;
    qint64 careWorkerId = 0;
    QDateTime lastVisitTime;
};

// ========== WO 工单域 ==========

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
        o.id = q.value(0).toLongLong();
        o.orderNo = q.value(1).toString();
        o.estateId = q.value(2).toLongLong();
        o.houseId = q.value(3).toLongLong();
        o.reporterId = q.value(4).toLongLong();
        o.reporterName = q.value(5).toString();
        o.reporterPhone = q.value(6).toString();
        o.orderType = q.value(7).toInt();
        o.priority = q.value(8).toInt();
        o.title = q.value(9).toString();
        o.description = q.value(10).toString();
        o.locationDesc = q.value(11).toString();
        o.images = q.value(12).toString();
        o.status = q.value(13).toInt();
        o.acceptBy = q.value(14).toLongLong();
        o.acceptTime = q.value(15).toDateTime();
        o.assignTo = q.value(16).toLongLong();
        o.assignTime = q.value(17).toDateTime();
        o.finishTime = q.value(18).toDateTime();
        o.closeTime = q.value(19).toDateTime();
        o.resultDesc = q.value(20).toString();
        o.resultImages = q.value(21).toString();
        o.slaDeadline = q.value(22).toDateTime();
        o.isEscalated = q.value(23).toInt();
        o.escalateToEventId = q.value(24).toLongLong();
        o.source = q.value(25).toInt();
        o.createBy = q.value(26).toLongLong();
        o.createTime = q.value(27).toDateTime();
        o.updateBy = q.value(28).toLongLong();
        o.updateTime = q.value(29).toDateTime();
        o.delFlag = q.value(30).toInt();
        o.remark = q.value(31).toString();
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

// ========== GE 社区治理域 ==========

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
        e.id = q.value(0).toLongLong();
        e.eventNo = q.value(1).toString();
        e.gridId = q.value(2).toLongLong();
        e.communityOrgId = q.value(3).toLongLong();
        e.reporterId = q.value(4).toLongLong();
        e.reporterName = q.value(5).toString();
        e.eventCategory = q.value(6).toInt();
        e.priority = q.value(7).toInt();
        e.title = q.value(8).toString();
        e.description = q.value(9).toString();
        e.location = q.value(10).toString();
        e.longitude = q.value(11).toDouble();
        e.latitude = q.value(12).toDouble();
        e.images = q.value(13).toString();
        e.status = q.value(14).toInt();
        e.reviewerId = q.value(15).toLongLong();
        e.reviewTime = q.value(16).toDateTime();
        e.assignTo = q.value(17).toLongLong();
        e.assignOrgId = q.value(18).toLongLong();
        e.assignTime = q.value(19).toDateTime();
        e.finishTime = q.value(20).toDateTime();
        e.archiveTime = q.value(21).toDateTime();
        e.resultDesc = q.value(22).toString();
        e.resultImages = q.value(23).toString();
        e.slaDeadline = q.value(24).toDateTime();
        e.isSupervised = q.value(25).toInt();
        e.isCoordinated = q.value(26).toInt();
        e.source = q.value(27).toInt();
        e.createBy = q.value(28).toLongLong();
        e.createTime = q.value(29).toDateTime();
        e.updateBy = q.value(30).toLongLong();
        e.updateTime = q.value(31).toDateTime();
        e.delFlag = q.value(32).toInt();
        e.remark = q.value(33).toString();
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

// ========== SV 社区服务域 ==========

struct SvVolunteer : BaseEntity {
    qint64 userId = 0;
    qint64 residentId = 0;
    QString skills;
    QString availableTime;
    double totalHours = 0;
    int status = 0;
    QDateTime registerTime;
};

struct SvVolunteerActivity : BaseEntity {
    qint64 communityOrgId = 0;
    QString title;
    QString description;
    int activityType = 0;
    QString location;
    QDateTime startTime;
    QDateTime endTime;
    int needCount = 0;
    int enrolledCount = 0;
    qint64 publisherId = 0;
    int status = 0;
};

struct SvVolunteerSignup {
    qint64 id = 0;
    qint64 activityId = 0;
    qint64 volunteerId = 0;
    QDateTime signupTime;
    int status = 0;
    QDateTime checkinTime;
    QDateTime checkoutTime;
    double hours = 0;
};

struct SvServiceProvider : BaseEntity {
    qint64 orgId = 0;
    qint64 userId = 0;
    QString providerName;
    QString serviceTypes;
    QString contactPerson;
    QString contactPhone;
    QString serviceArea;
    double rating = 5.0;
    int totalOrders = 0;
    int status = 0;
};

struct SvServiceOrder : BaseEntity {
    QString orderNo;
    qint64 applicantId = 0;
    qint64 providerId = 0;
    int serviceType = 0;
    QString title;
    QString description;
    QDateTime appointmentTime;
    QString address;
    int status = 0;
    QDateTime acceptTime;
    QDateTime finishTime;
    QString proofImages;
    int rating = 0;
    QString comment;
};

// ========== NT 消息通知域 ==========

struct NtAnnouncement : BaseEntity {
    QString title;
    QString content;
    int announcementType = 0;
    QString targetScope;
    QString targetIds;
    qint64 publisherId = 0;
    QDateTime publishTime;
    int isTop = 0;
    int readCount = 0;
    int status = 0;
};

struct NtNotification {
    qint64 id = 0;
    qint64 userId = 0;
    QString title;
    QString content;
    int notificationType = 0;
    QString bizType;
    qint64 bizId = 0;
    int isRead = 0;
    QDateTime readTime;
    QDateTime createTime;
};

// ========== OC 业委会域 ==========

struct OcTopic : BaseEntity {
    qint64 committeeId = 0;
    qint64 estateId = 0;
    QString title;
    QString content;
    int topicType = 0;
    qint64 publisherId = 0;
    QDateTime publishTime;
    int needVote = 0;
    QDateTime voteStart;
    QDateTime voteEnd;
    int voteResult = 0;
    int status = 0;
};

struct OcVote {
    qint64 id = 0;
    qint64 topicId = 0;
    qint64 voterId = 0;
    int choice = 0;
    QDateTime voteTime;
};

// ========== EV 评价域 ==========

struct EvEvaluation : BaseEntity {
    QString bizType;
    qint64 bizId = 0;
    qint64 evaluatorId = 0;
    int rating = 0;
    QString content;
    QString reply;
    qint64 replyBy = 0;
    QDateTime replyTime;
    QDateTime evalTime;
};

// ========== AI 智能助手域 ==========

struct AiKnowledge : BaseEntity {
    QString category;
    QString question;
    QString answer;
    QString keywords;
    int priority = 0;
    int hitCount = 0;
    int status = 0;
};

struct AiChatLog {
    qint64 id = 0;
    qint64 userId = 0;
    QString sessionId;
    QString role;
    QString content;
    QString intent;
    qint64 matchedKnowledgeId = 0;
    int isUseful = -1;
    QDateTime createTime;
};

// ========== KF 考核域 ==========

struct KfAssessmentResult : BaseEntity {
    qint64 configId = 0;
    qint64 targetOrgId = 0;
    qint64 targetUserId = 0;
    QString period;
    double actualValue = 0;
    double score = 0;
    int rank = 0;
    QDateTime assessTime;
};

#endif // MODELS_H
