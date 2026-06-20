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
    QString roleDomain;  // 当前用户的主角色域(resident/property/governance/service/supervision/platform)
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
        e.id = q.value("id").toLongLong();
        e.orgId = q.value("org_id").toLongLong();
        e.propertyCompanyId = q.value("property_company_id").toLongLong();
        e.estateName = q.value("estate_name").toString();
        e.estateCode = q.value("estate_code").toString();
        e.address = q.value("address").toString();
        e.totalArea = q.value("total_area").toDouble();
        e.totalBuildings = q.value("total_buildings").toInt();
        e.totalHouses = q.value("total_houses").toInt();
        e.builtYear = q.value("built_year").toInt();
        e.greenRate = q.value("green_rate").toDouble();
        e.parkingSpaces = q.value("parking_spaces").toInt();
        e.longitude = q.value("longitude").toDouble();
        e.latitude = q.value("latitude").toDouble();
        e.status = q.value("status").toInt();
        e.createBy = q.value("create_by").toLongLong();
        e.createTime = q.value("create_time").toDateTime();
        e.updateBy = q.value("update_by").toLongLong();
        e.updateTime = q.value("update_time").toDateTime();
        e.delFlag = q.value("del_flag").toInt();
        e.remark = q.value("remark").toString();
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
        h.id = q.value("id").toLongLong();
        h.unitId = q.value("unit_id").toLongLong();
        h.estateId = q.value("estate_id").toLongLong();
        h.houseCode = q.value("house_code").toString();
        h.floor = q.value("floor").toInt();
        h.roomNumber = q.value("room_number").toString();
        h.area = q.value("area").toDouble();
        h.houseType = q.value("house_type").toString();
        h.houseStatus = q.value("house_status").toInt();
        h.ownerName = q.value("owner_name").toString();
        h.ownerPhone = q.value("owner_phone").toString();
        h.createBy = q.value("create_by").toLongLong();
        h.createTime = q.value("create_time").toDateTime();
        h.updateBy = q.value("update_by").toLongLong();
        h.updateTime = q.value("update_time").toDateTime();
        h.delFlag = q.value("del_flag").toInt();
        h.remark = q.value("remark").toString();
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
        r.id = q.value("id").toLongLong();
        r.userId = q.value("user_id").toLongLong();
        r.name = q.value("name").toString();
        r.idCard = q.value("id_card").toString();
        r.gender = q.value("gender").toInt();
        r.phone = q.value("phone").toString();
        r.phoneDisplay = q.value("phone_display").toString();
        r.birthday = q.value("birthday").toDate();
        r.photo = q.value("photo").toString();
        r.nationality = q.value("nationality").toString();
        r.education = q.value("education").toString();
        r.occupation = q.value("occupation").toString();
        r.politicalStatus = q.value("political_status").toString();
        r.isSpecial = q.value("is_special").toInt();
        r.specialType = q.value("special_type").toString();
        r.emergencyContact = q.value("emergency_contact").toString();
        r.emergencyPhone = q.value("emergency_phone").toString();
        r.status = q.value("status").toInt();
        r.createBy = q.value("create_by").toLongLong();
        r.createTime = q.value("create_time").toDateTime();
        r.updateBy = q.value("update_by").toLongLong();
        r.updateTime = q.value("update_time").toDateTime();
        r.delFlag = q.value("del_flag").toInt();
        r.remark = q.value("remark").toString();
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
