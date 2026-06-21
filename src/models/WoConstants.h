#ifndef WO_CONSTANTS_H
#define WO_CONSTANTS_H

#include "EnumHelper.h"

namespace InspectionPlanStatus {
    enum Type : int { Pending = 0, InProgress = 1, Completed = 2 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { Pending, QStringLiteral("待执行") },
            { InProgress, QStringLiteral("进行中") },
            { Completed, QStringLiteral("已完成") },
        };
        return map;
    }
    inline const QMap<Type, QString>& colors() {
        static const QMap<Type, QString> map {
            { Pending, QStringLiteral("#eff6ff") },
            { InProgress, QStringLiteral("#fffbeb") },
            { Completed, QStringLiteral("#f0fdf4") },
        };
        return map;
    }
    inline const QMap<Type, QString>& fgColors() {
        static const QMap<Type, QString> map {
            { Pending, QStringLiteral("#2563eb") },
            { InProgress, QStringLiteral("#d97706") },
            { Completed, QStringLiteral("#15803d") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
    inline QString color(int v) { return EnumHelper<Type>::color(v, colors()); }
    inline QString fgColor(int v) { return EnumHelper<Type>::color(v, fgColors()); }
}

namespace InspectionRecordStatus {
    enum Type : int { InProgress = 0, Completed = 1 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { InProgress, QStringLiteral("进行中") },
            { Completed, QStringLiteral("已完成") },
        };
        return map;
    }
    inline const QMap<Type, QString>& colors() {
        static const QMap<Type, QString> map {
            { InProgress, QStringLiteral("#e6f4ff") },
            { Completed, QStringLiteral("#f6ffed") },
        };
        return map;
    }
    inline const QMap<Type, QString>& fgColors() {
        static const QMap<Type, QString> map {
            { InProgress, QStringLiteral("#b45309") },
            { Completed, QStringLiteral("#15803d") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
    inline QString color(int v) { return EnumHelper<Type>::color(v, colors()); }
    inline QString fgColor(int v) { return EnumHelper<Type>::color(v, fgColors()); }
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

    // 工单状态筛选下拉框索引 -> 状态值映射（0=全部，占位）
    inline const int* statusFilterMap()
    {
        static const int map[] = {
            -1,        // 全部（不使用）
            Pending,
            Accepted,
            Assigned,
            Processing,
            Completed,
            Closed,
            Evaluated
        };
        return map;
    }
}

#endif // WO_CONSTANTS_H
