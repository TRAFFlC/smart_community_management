#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace UserStatus {
    enum { Normal = 0, Disabled = 1, Locked = 2 };
    inline QString label(int v) {
        switch(v) {
            case Normal: return QStringLiteral("正常");
            case Disabled: return QStringLiteral("禁用");
            case Locked: return QStringLiteral("锁定");
            default: return QStringLiteral("未知");
        }
    }
}

namespace UserType {
    enum { Resident = 0, Staff = 1, ServiceProvider = 2, Admin = 3 };
    inline QString label(int v) {
        switch(v) {
            case Resident: return QStringLiteral("居民");
            case Staff: return QStringLiteral("工作人员");
            case ServiceProvider: return QStringLiteral("服务商");
            case Admin: return QStringLiteral("管理员");
            default: return QStringLiteral("未知");
        }
    }
}

namespace OrgType {
    enum { Platform = 1, Street = 2, Community = 3, PropertyCompany = 4,
           OwnerCommittee = 5, ServiceOrg = 6, SocialOrg = 7 };
    inline QString label(int v) {
        switch(v) {
            case Platform: return QStringLiteral("平台");
            case Street: return QStringLiteral("街道");
            case Community: return QStringLiteral("社区");
            case PropertyCompany: return QStringLiteral("物业公司");
            case OwnerCommittee: return QStringLiteral("业委会");
            case ServiceOrg: return QStringLiteral("服务商");
            case SocialOrg: return QStringLiteral("社会组织");
            default: return QStringLiteral("未知");
        }
    }
}

namespace DataScope {
    enum { Platform = 1, Street = 2, Community = 3, Estate = 4,
           Building = 5, Personal = 6, Collaborative = 7 };
    inline QString label(int v) {
        switch(v) {
            case Platform: return QStringLiteral("平台级");
            case Street: return QStringLiteral("街道级");
            case Community: return QStringLiteral("社区级");
            case Estate: return QStringLiteral("小区级");
            case Building: return QStringLiteral("楼栋级");
            case Personal: return QStringLiteral("个人级");
            case Collaborative: return QStringLiteral("协同级");
            default: return QStringLiteral("未知");
        }
    }
}

namespace HouseStatus {
    enum { Vacant = 0, OwnerOccupied = 1, Rented = 2, Sold = 3 };
    inline QString label(int v) {
        switch(v) {
            case Vacant: return QStringLiteral("空置");
            case OwnerOccupied: return QStringLiteral("自住");
            case Rented: return QStringLiteral("出租");
            case Sold: return QStringLiteral("已售");
            default: return QStringLiteral("未知");
        }
    }
}

namespace HouseRelation {
    enum { Owner = 1, Resident = 2, Tenant = 3, FamilyMember = 4 };
    inline QString label(int v) {
        switch(v) {
            case Owner: return QStringLiteral("产权人");
            case Resident: return QStringLiteral("居住人");
            case Tenant: return QStringLiteral("租户");
            case FamilyMember: return QStringLiteral("家庭成员");
            default: return QStringLiteral("未知");
        }
    }
}

namespace VehicleType {
    enum { Sedan = 1, SUV = 2, Electric = 3, Other = 4 };
    inline QString label(int v) {
        switch(v) {
            case Sedan: return QStringLiteral("轿车");
            case SUV: return QStringLiteral("SUV");
            case Electric: return QStringLiteral("电动车");
            case Other: return QStringLiteral("其他");
            default: return QStringLiteral("未知");
        }
    }
}

namespace FacilityType {
    enum { Elevator = 1, FireFight = 2, AccessControl = 3, Camera = 4,
           Fitness = 5, Lighting = 6, Other = 7 };
    inline QString label(int v) {
        switch(v) {
            case Elevator: return QStringLiteral("电梯");
            case FireFight: return QStringLiteral("消防");
            case AccessControl: return QStringLiteral("门禁");
            case Camera: return QStringLiteral("摄像头");
            case Fitness: return QStringLiteral("健身器材");
            case Lighting: return QStringLiteral("照明");
            case Other: return QStringLiteral("其他");
            default: return QStringLiteral("未知");
        }
    }
}

namespace SpecialGroupType {
    enum { ElderlyAlone = 1, Disabled = 2, LowIncome = 3, PriorityHelp = 4, Other = 5 };
    inline QString label(int v) {
        switch(v) {
            case ElderlyAlone: return QStringLiteral("独居老人");
            case Disabled: return QStringLiteral("残障人士");
            case LowIncome: return QStringLiteral("低保户");
            case PriorityHelp: return QStringLiteral("重点帮扶");
            case Other: return QStringLiteral("其他");
            default: return QStringLiteral("未知");
        }
    }
}

namespace WorkOrderType {
    enum { WaterElectric = 1, PublicFacility = 2, Environment = 3,
           SecurityOrder = 4, Other = 5 };
    inline QString label(int v) {
        switch(v) {
            case WaterElectric: return QStringLiteral("水电维修");
            case PublicFacility: return QStringLiteral("公共设施");
            case Environment: return QStringLiteral("环境卫生");
            case SecurityOrder: return QStringLiteral("安全秩序");
            case Other: return QStringLiteral("其他");
            default: return QStringLiteral("未知");
        }
    }
}

namespace WorkOrderPriority {
    enum { Normal = 1, Urgent = 2, Critical = 3 };
    inline QString label(int v) {
        switch(v) {
            case Normal: return QStringLiteral("普通");
            case Urgent: return QStringLiteral("紧急");
            case Critical: return QStringLiteral("特急");
            default: return QStringLiteral("未知");
        }
    }
}

namespace WorkOrderStatus {
    enum { Pending = 0, Accepted = 1, Assigned = 2, Processing = 3,
           Completed = 4, Closed = 5, Evaluated = 6, Rejected = 7 };
    inline QString label(int v) {
        switch(v) {
            case Pending: return QStringLiteral("待受理");
            case Accepted: return QStringLiteral("已受理");
            case Assigned: return QStringLiteral("已派单");
            case Processing: return QStringLiteral("处理中");
            case Completed: return QStringLiteral("已完成");
            case Closed: return QStringLiteral("已关闭");
            case Evaluated: return QStringLiteral("已评价");
            case Rejected: return QStringLiteral("已退回");
            default: return QStringLiteral("未知");
        }
    }
    inline QString color(int v) {
        switch(v) {
            case Pending: return "#E6A23C";
            case Accepted: return "#409EFF";
            case Assigned: return "#409EFF";
            case Processing: return "#409EFF";
            case Completed: return "#67C23A";
            case Closed: return "#909399";
            case Evaluated: return "#67C23A";
            case Rejected: return "#F56C6C";
            default: return "#909399";
        }
    }
}

namespace EventCategory {
    enum { Livelihood = 1, Environment = 2, FacilitySafety = 3,
           NeighborDispute = 4, SpecialCare = 5, CityOrder = 6, Emergency = 7 };
    inline QString label(int v) {
        switch(v) {
            case Livelihood: return QStringLiteral("民生服务");
            case Environment: return QStringLiteral("环境卫生");
            case FacilitySafety: return QStringLiteral("设施安全");
            case NeighborDispute: return QStringLiteral("邻里纠纷");
            case SpecialCare: return QStringLiteral("特殊帮扶");
            case CityOrder: return QStringLiteral("市容秩序");
            case Emergency: return QStringLiteral("突发预警");
            default: return QStringLiteral("未知");
        }
    }
}

namespace EventPriority {
    enum { Normal = 1, Important = 2, Urgent = 3, Critical = 4 };
    inline QString label(int v) {
        switch(v) {
            case Normal: return QStringLiteral("一般");
            case Important: return QStringLiteral("重要");
            case Urgent: return QStringLiteral("紧急");
            case Critical: return QStringLiteral("特急");
            default: return QStringLiteral("未知");
        }
    }
}

namespace EventStatus {
    enum { PendingReview = 0, Reviewed = 1, Assigned = 2, Processing = 3,
           Completed = 4, Rejected = 5, Archived = 6, Escalated = 7 };
    inline QString label(int v) {
        switch(v) {
            case PendingReview: return QStringLiteral("待审核");
            case Reviewed: return QStringLiteral("已审核");
            case Assigned: return QStringLiteral("已分派");
            case Processing: return QStringLiteral("处理中");
            case Completed: return QStringLiteral("已完成");
            case Rejected: return QStringLiteral("已退回");
            case Archived: return QStringLiteral("已归档");
            case Escalated: return QStringLiteral("已升级");
            default: return QStringLiteral("未知");
        }
    }
    inline QString color(int v) {
        switch(v) {
            case PendingReview: return "#E6A23C";
            case Reviewed: return "#409EFF";
            case Assigned: return "#409EFF";
            case Processing: return "#409EFF";
            case Completed: return "#67C23A";
            case Rejected: return "#F56C6C";
            case Archived: return "#909399";
            case Escalated: return "#F56C6C";
            default: return "#909399";
        }
    }
}

namespace EventSource {
    enum { GridWorker = 0, Resident = 1, Property = 2, Inspection = 3 };
    inline QString label(int v) {
        switch(v) {
            case GridWorker: return QStringLiteral("网格员上报");
            case Resident: return QStringLiteral("居民上报");
            case Property: return QStringLiteral("物业上报");
            case Inspection: return QStringLiteral("巡查发现");
            default: return QStringLiteral("未知");
        }
    }
}

namespace VolunteerActivityType {
    enum { Environmental = 1, ElderlyCare = 2, Education = 3, Security = 4, Other = 5 };
    inline QString label(int v) {
        switch(v) {
            case Environmental: return QStringLiteral("环保");
            case ElderlyCare: return QStringLiteral("助老");
            case Education: return QStringLiteral("文教");
            case Security: return QStringLiteral("治安");
            case Other: return QStringLiteral("其他");
            default: return QStringLiteral("未知");
        }
    }
}

namespace VolunteerActivityStatus {
    enum { Draft = 0, Recruiting = 1, InProgress = 2, Ended = 3, Cancelled = 4 };
    inline QString label(int v) {
        switch(v) {
            case Draft: return QStringLiteral("草稿");
            case Recruiting: return QStringLiteral("招募中");
            case InProgress: return QStringLiteral("进行中");
            case Ended: return QStringLiteral("已结束");
            case Cancelled: return QStringLiteral("已取消");
            default: return QStringLiteral("未知");
        }
    }
}

namespace ServiceType {
    enum { Repair = 1, Housekeeping = 2, Delivery = 3, ElderCare = 4, Legal = 5, Other = 6 };
    inline QString label(int v) {
        switch(v) {
            case Repair: return QStringLiteral("维修");
            case Housekeeping: return QStringLiteral("家政");
            case Delivery: return QStringLiteral("配送");
            case ElderCare: return QStringLiteral("养老");
            case Legal: return QStringLiteral("法律");
            case Other: return QStringLiteral("其他");
            default: return QStringLiteral("未知");
        }
    }
}

namespace ServiceOrderStatus {
    enum { Pending = 0, Accepted = 1, Appointed = 2, InService = 3,
           Completed = 4, Evaluated = 5, Cancelled = 6 };
    inline QString label(int v) {
        switch(v) {
            case Pending: return QStringLiteral("待接单");
            case Accepted: return QStringLiteral("已接单");
            case Appointed: return QStringLiteral("已预约");
            case InService: return QStringLiteral("服务中");
            case Completed: return QStringLiteral("已完成");
            case Evaluated: return QStringLiteral("已评价");
            case Cancelled: return QStringLiteral("已取消");
            default: return QStringLiteral("未知");
        }
    }
}

namespace AnnouncementType {
    enum { Estate = 1, Community = 2, Property = 3, System = 4 };
    inline QString label(int v) {
        switch(v) {
            case Estate: return QStringLiteral("小区公告");
            case Community: return QStringLiteral("社区公告");
            case Property: return QStringLiteral("物业公告");
            case System: return QStringLiteral("系统公告");
            default: return QStringLiteral("未知");
        }
    }
}

namespace NotificationType {
    enum { SystemMsg = 1, WorkOrderReminder = 2, ApprovalReminder = 3,
           TimeoutWarning = 4, ActivityNotice = 5, AnnouncementPush = 6 };
    inline QString label(int v) {
        switch(v) {
            case SystemMsg: return QStringLiteral("系统消息");
            case WorkOrderReminder: return QStringLiteral("工单提醒");
            case ApprovalReminder: return QStringLiteral("审批待办");
            case TimeoutWarning: return QStringLiteral("超时预警");
            case ActivityNotice: return QStringLiteral("活动通知");
            case AnnouncementPush: return QStringLiteral("公告推送");
            default: return QStringLiteral("未知");
        }
    }
}

namespace TopicType {
    enum { PublicIncome = 1, PropertySupervision = 2, FacilityReno = 3, Other = 4 };
    inline QString label(int v) {
        switch(v) {
            case PublicIncome: return QStringLiteral("公共收益");
            case PropertySupervision: return QStringLiteral("物业监督");
            case FacilityReno: return QStringLiteral("设施改造");
            case Other: return QStringLiteral("其他");
            default: return QStringLiteral("未知");
        }
    }
}

namespace VoteChoice {
    enum { Approve = 1, Oppose = 2, Abstain = 3 };
    inline QString label(int v) {
        switch(v) {
            case Approve: return QStringLiteral("赞成");
            case Oppose: return QStringLiteral("反对");
            case Abstain: return QStringLiteral("弃权");
            default: return QStringLiteral("未知");
        }
    }
}

namespace MenuType {
    enum { Directory = 1, Menu = 2, Button = 3, Api = 4 };
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
