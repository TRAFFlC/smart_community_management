#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QMap>
#include <QString>

// ============================================================================
// 类型安全改进说明：
// 所有命名空间内的裸 enum 已改为带类型名和底层类型的 enum Type : int { ... }。
// - 枚举值仍然与 int 兼容（可隐式转换），保持向后兼容，无需修改引用点
// - 有了明确的类型名 Type，便于文档化与理解
// - 底层类型固定为 int，保证跨平台一致性
// 注意：不同命名空间的同名枚举值（如 UserStatus::Normal 与 WorkOrderPriority::Normal）
// 仍为 int 类型，使用时需注意区分命名空间。
// ============================================================================

// ----------------------------------------------------------------------------
// 通用枚举助手：根据静态映射表返回标签/颜色，未命中时返回默认值
// ----------------------------------------------------------------------------
template<typename T>
struct EnumHelper {
    static QString label(T value, const QMap<T, QString>& labels) {
        auto it = labels.find(value);
        return it != labels.end() ? it.value() : QStringLiteral("未知");
    }
    static QString label(int value, const QMap<T, QString>& labels) {
        return label(static_cast<T>(value), labels);
    }
    static QString color(T value, const QMap<T, QString>& colors) {
        auto it = colors.find(value);
        return it != colors.end() ? it.value() : QStringLiteral("#64748b");
    }
    static QString color(int value, const QMap<T, QString>& colors) {
        return color(static_cast<T>(value), colors);
    }
};

namespace UserStatus {
    enum Type : int { Normal = 0, Disabled = 1, Locked = 2 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Normal, QStringLiteral("正常") },
            { Disabled, QStringLiteral("禁用") },
            { Locked, QStringLiteral("锁定") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace UserType {
    enum Type : int { Resident = 0, Staff = 1, ServiceProvider = 2, Admin = 3 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Resident, QStringLiteral("居民") },
            { Staff, QStringLiteral("工作人员") },
            { ServiceProvider, QStringLiteral("服务商") },
            { Admin, QStringLiteral("管理员") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace OrgType {
    enum Type : int { Platform = 1, Street = 2, Community = 3, PropertyCompany = 4,
           OwnerCommittee = 5, ServiceOrg = 6, SocialOrg = 7 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Platform, QStringLiteral("平台") },
            { Street, QStringLiteral("街道") },
            { Community, QStringLiteral("社区") },
            { PropertyCompany, QStringLiteral("物业公司") },
            { OwnerCommittee, QStringLiteral("业委会") },
            { ServiceOrg, QStringLiteral("服务商") },
            { SocialOrg, QStringLiteral("社会组织") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace DataScope {
    enum Type : int { Platform = 1, Street = 2, Community = 3, Estate = 4,
           Building = 5, Personal = 6, Collaborative = 7 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Platform, QStringLiteral("平台级") },
            { Street, QStringLiteral("街道级") },
            { Community, QStringLiteral("社区级") },
            { Estate, QStringLiteral("小区级") },
            { Building, QStringLiteral("楼栋级") },
            { Personal, QStringLiteral("个人级") },
            { Collaborative, QStringLiteral("协同级") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace HouseStatus {
    enum Type : int { Vacant = 0, OwnerOccupied = 1, Rented = 2, Sold = 3 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Vacant, QStringLiteral("空置") },
            { OwnerOccupied, QStringLiteral("自住") },
            { Rented, QStringLiteral("出租") },
            { Sold, QStringLiteral("已售") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace HouseRelation {
    enum Type : int { Owner = 1, Resident = 2, Tenant = 3, FamilyMember = 4 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Owner, QStringLiteral("产权人") },
            { Resident, QStringLiteral("居住人") },
            { Tenant, QStringLiteral("租户") },
            { FamilyMember, QStringLiteral("家庭成员") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace VehicleType {
    enum Type : int { Sedan = 1, SUV = 2, Electric = 3, Other = 4 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Sedan, QStringLiteral("轿车") },
            { SUV, QStringLiteral("SUV") },
            { Electric, QStringLiteral("电动车") },
            { Other, QStringLiteral("其他") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace FacilityType {
    enum Type : int { Elevator = 1, FireFight = 2, AccessControl = 3, Camera = 4,
           Fitness = 5, Lighting = 6, Other = 7 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Elevator, QStringLiteral("电梯") },
            { FireFight, QStringLiteral("消防") },
            { AccessControl, QStringLiteral("门禁") },
            { Camera, QStringLiteral("摄像头") },
            { Fitness, QStringLiteral("健身器材") },
            { Lighting, QStringLiteral("照明") },
            { Other, QStringLiteral("其他") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace SpecialGroupType {
    enum Type : int { ElderlyAlone = 1, Disabled = 2, LowIncome = 3, PriorityHelp = 4, Other = 5 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { ElderlyAlone, QStringLiteral("独居老人") },
            { Disabled, QStringLiteral("残障人士") },
            { LowIncome, QStringLiteral("低保户") },
            { PriorityHelp, QStringLiteral("重点帮扶") },
            { Other, QStringLiteral("其他") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace WorkOrderType {
    enum Type : int { WaterElectric = 1, PublicFacility = 2, Environment = 3,
           SecurityOrder = 4, Other = 5 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { WaterElectric, QStringLiteral("水电维修") },
            { PublicFacility, QStringLiteral("公共设施") },
            { Environment, QStringLiteral("环境卫生") },
            { SecurityOrder, QStringLiteral("安全秩序") },
            { Other, QStringLiteral("其他") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace WorkOrderPriority {
    enum Type : int { Normal = 1, Urgent = 2, Critical = 3 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Normal, QStringLiteral("普通") },
            { Urgent, QStringLiteral("紧急") },
            { Critical, QStringLiteral("特急") },
        };
        return map;
    }
    inline const QMap<Type, QString>& colors() {
        static const QMap<Type, QString> map {
            { Normal, QStringLiteral("#64748b") },
            { Urgent, QStringLiteral("#d97706") },
            { Critical, QStringLiteral("#b91c1c") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
    inline QString color(int v) { return EnumHelper<Type>::color(v, colors()); }
}

namespace WorkOrderStatus {
    enum Type : int { Pending = 0, Accepted = 1, Assigned = 2, Processing = 3,
           Completed = 4, Closed = 5, Evaluated = 6, Rejected = 7 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Pending, QStringLiteral("待受理") },
            { Accepted, QStringLiteral("已受理") },
            { Assigned, QStringLiteral("已派单") },
            { Processing, QStringLiteral("处理中") },
            { Completed, QStringLiteral("已完成") },
            { Closed, QStringLiteral("已关闭") },
            { Evaluated, QStringLiteral("已评价") },
            { Rejected, QStringLiteral("已退回") },
        };
        return map;
    }
    inline const QMap<Type, QString>& colors() {
        static const QMap<Type, QString> map {
            { Pending, QStringLiteral("#d97706") },
            { Accepted, QStringLiteral("#2563eb") },
            { Assigned, QStringLiteral("#2563eb") },
            { Processing, QStringLiteral("#2563eb") },
            { Completed, QStringLiteral("#15803d") },
            { Closed, QStringLiteral("#64748b") },
            { Evaluated, QStringLiteral("#15803d") },
            { Rejected, QStringLiteral("#b91c1c") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
    inline QString color(int v) { return EnumHelper<Type>::color(v, colors()); }
}

namespace EventCategory {
    enum Type : int { Livelihood = 1, Environment = 2, FacilitySafety = 3,
           NeighborDispute = 4, SpecialCare = 5, CityOrder = 6, Emergency = 7 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Livelihood, QStringLiteral("民生服务") },
            { Environment, QStringLiteral("环境卫生") },
            { FacilitySafety, QStringLiteral("设施安全") },
            { NeighborDispute, QStringLiteral("邻里纠纷") },
            { SpecialCare, QStringLiteral("特殊帮扶") },
            { CityOrder, QStringLiteral("市容秩序") },
            { Emergency, QStringLiteral("突发预警") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace EventPriority {
    enum Type : int { Normal = 1, Important = 2, Urgent = 3, Critical = 4 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Normal, QStringLiteral("一般") },
            { Important, QStringLiteral("重要") },
            { Urgent, QStringLiteral("紧急") },
            { Critical, QStringLiteral("特急") },
        };
        return map;
    }
    inline const QMap<Type, QString>& colors() {
        static const QMap<Type, QString> map {
            { Normal, QStringLiteral("#64748b") },
            { Important, QStringLiteral("#2563eb") },
            { Urgent, QStringLiteral("#d97706") },
            { Critical, QStringLiteral("#b91c1c") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
    inline QString color(int v) { return EnumHelper<Type>::color(v, colors()); }
}

namespace EventStatus {
    enum Type : int { PendingReview = 0, Reviewed = 1, Assigned = 2, Processing = 3,
           Completed = 4, Rejected = 5, Archived = 6, Escalated = 7 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { PendingReview, QStringLiteral("待审核") },
            { Reviewed, QStringLiteral("已审核") },
            { Assigned, QStringLiteral("已分派") },
            { Processing, QStringLiteral("处理中") },
            { Completed, QStringLiteral("已完成") },
            { Rejected, QStringLiteral("已退回") },
            { Archived, QStringLiteral("已归档") },
            { Escalated, QStringLiteral("已升级") },
        };
        return map;
    }
    inline const QMap<Type, QString>& colors() {
        static const QMap<Type, QString> map {
            { PendingReview, QStringLiteral("#d97706") },
            { Reviewed, QStringLiteral("#2563eb") },
            { Assigned, QStringLiteral("#2563eb") },
            { Processing, QStringLiteral("#2563eb") },
            { Completed, QStringLiteral("#15803d") },
            { Rejected, QStringLiteral("#b91c1c") },
            { Archived, QStringLiteral("#64748b") },
            { Escalated, QStringLiteral("#b91c1c") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
    inline QString color(int v) { return EnumHelper<Type>::color(v, colors()); }
}

namespace EventSource {
    enum Type : int { GridWorker = 0, Resident = 1, Property = 2, Inspection = 3 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { GridWorker, QStringLiteral("网格员上报") },
            { Resident, QStringLiteral("居民上报") },
            { Property, QStringLiteral("物业上报") },
            { Inspection, QStringLiteral("巡查发现") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace VolunteerActivityType {
    enum Type : int { Environmental = 1, ElderlyCare = 2, Education = 3, Security = 4, Other = 5 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Environmental, QStringLiteral("环保") },
            { ElderlyCare, QStringLiteral("助老") },
            { Education, QStringLiteral("文教") },
            { Security, QStringLiteral("治安") },
            { Other, QStringLiteral("其他") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace VolunteerActivityStatus {
    enum Type : int { Draft = 0, Recruiting = 1, InProgress = 2, Ended = 3, Cancelled = 4 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Draft, QStringLiteral("草稿") },
            { Recruiting, QStringLiteral("招募中") },
            { InProgress, QStringLiteral("进行中") },
            { Ended, QStringLiteral("已结束") },
            { Cancelled, QStringLiteral("已取消") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace ServiceType {
    enum Type : int { Repair = 1, Housekeeping = 2, Delivery = 3, ElderCare = 4, Legal = 5, Other = 6 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Repair, QStringLiteral("维修") },
            { Housekeeping, QStringLiteral("家政") },
            { Delivery, QStringLiteral("配送") },
            { ElderCare, QStringLiteral("养老") },
            { Legal, QStringLiteral("法律") },
            { Other, QStringLiteral("其他") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace ServiceOrderStatus {
    enum Type : int { Pending = 0, Accepted = 1, Appointed = 2, InService = 3,
           Completed = 4, Evaluated = 5, Cancelled = 6 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Pending, QStringLiteral("待接单") },
            { Accepted, QStringLiteral("已接单") },
            { Appointed, QStringLiteral("已预约") },
            { InService, QStringLiteral("服务中") },
            { Completed, QStringLiteral("已完成") },
            { Evaluated, QStringLiteral("已评价") },
            { Cancelled, QStringLiteral("已取消") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace AnnouncementType {
    enum Type : int { Estate = 1, Community = 2, Property = 3, System = 4 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Estate, QStringLiteral("小区公告") },
            { Community, QStringLiteral("社区公告") },
            { Property, QStringLiteral("物业公告") },
            { System, QStringLiteral("系统公告") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace NotificationType {
    enum Type : int { SystemMsg = 1, WorkOrderReminder = 2, ApprovalReminder = 3,
           TimeoutWarning = 4, ActivityNotice = 5, AnnouncementPush = 6 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { SystemMsg, QStringLiteral("系统消息") },
            { WorkOrderReminder, QStringLiteral("工单提醒") },
            { ApprovalReminder, QStringLiteral("审批待办") },
            { TimeoutWarning, QStringLiteral("超时预警") },
            { ActivityNotice, QStringLiteral("活动通知") },
            { AnnouncementPush, QStringLiteral("公告推送") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace TopicType {
    enum Type : int { PublicIncome = 1, PropertySupervision = 2, FacilityReno = 3, Other = 4 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { PublicIncome, QStringLiteral("公共收益") },
            { PropertySupervision, QStringLiteral("物业监督") },
            { FacilityReno, QStringLiteral("设施改造") },
            { Other, QStringLiteral("其他") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace VoteChoice {
    enum Type : int { Approve = 1, Oppose = 2, Abstain = 3 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Approve, QStringLiteral("赞成") },
            { Oppose, QStringLiteral("反对") },
            { Abstain, QStringLiteral("弃权") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
}

namespace MenuType {
    enum Type : int { Directory = 1, Menu = 2, Button = 3, Api = 4 };
}

namespace RoleDomain {
    inline QString label(const QString& domain) {
        if (domain == "resident") return QStringLiteral("居民域");
        if (domain == "property") return QStringLiteral("物业域");
        if (domain == "governance") return QStringLiteral("治理域");
        if (domain == "service") return QStringLiteral("服务域");
        if (domain == "supervision") return QStringLiteral("监督域");
        if (domain == "platform") return QStringLiteral("平台域");
        return QStringLiteral("未知");
    }
}

#endif // CONSTANTS_H
