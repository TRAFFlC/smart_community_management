#ifndef CMMODELS_H
#define CMMODELS_H

#include <QDate>
#include <QDateTime>
#include <QSqlQuery>
#include <QString>
#include <QVariantMap>

#include "BaseModels.h"

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

#endif // CMMODELS_H
