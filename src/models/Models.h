#ifndef MODELS_H
#define MODELS_H

#include <QString>
#include <QDateTime>
#include <QVariant>
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
