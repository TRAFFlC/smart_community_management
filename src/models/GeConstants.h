#ifndef GE_CONSTANTS_H
#define GE_CONSTANTS_H

#include "EnumHelper.h"

namespace OpinionStatus {
    enum Type : int { Pending = 0, Replied = 1, Adopted = 2 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Pending, QStringLiteral("待处理") },
            { Replied, QStringLiteral("已回复") },
            { Adopted, QStringLiteral("已采纳") },
        };
        return map;
    }
    inline const QMap<Type, QString>& colors() {
        static const QMap<Type, QString> map {
            { Pending, QStringLiteral("#eff6ff") },
            { Replied, QStringLiteral("#f0fdf4") },
            { Adopted, QStringLiteral("#fffbeb") },
        };
        return map;
    }
    inline const QMap<Type, QString>& fgColors() {
        static const QMap<Type, QString> map {
            { Pending, QStringLiteral("#2563eb") },
            { Replied, QStringLiteral("#15803d") },
            { Adopted, QStringLiteral("#d97706") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
    inline QString color(int v) { return EnumHelper<Type>::color(v, colors()); }
    inline QString fgColor(int v) { return EnumHelper<Type>::color(v, fgColors()); }
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

// 投诉状态与事件状态语义、标签、颜色完全一致，直接复用
namespace ComplaintStatus = EventStatus;

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

#endif // GE_CONSTANTS_H
