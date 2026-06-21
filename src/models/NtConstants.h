#ifndef NT_CONSTANTS_H
#define NT_CONSTANTS_H

#include "EnumHelper.h"

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

#endif // NT_CONSTANTS_H
