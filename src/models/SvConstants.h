#ifndef SV_CONSTANTS_H
#define SV_CONSTANTS_H

#include "EnumHelper.h"

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

#endif // SV_CONSTANTS_H
