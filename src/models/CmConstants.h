#ifndef CM_CONSTANTS_H
#define CM_CONSTANTS_H

#include "EnumHelper.h"

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

namespace SpecialGroupCareLevel {
    enum Type : int { General = 1, Key = 2, Special = 3 };
    inline const QMap<Type, QString>& labels() {
        static const QMap<Type, QString> map {
            { General, QStringLiteral("一般") },
            { Key, QStringLiteral("重点") },
            { Special, QStringLiteral("特殊") },
        };
        return map;
    }
    inline const QMap<Type, QString>& colors() {
        static const QMap<Type, QString> map {
            { General, QStringLiteral("#e6f4ff") },
            { Key, QStringLiteral("#fff7e6") },
            { Special, QStringLiteral("#fff1f0") },
        };
        return map;
    }
    inline const QMap<Type, QString>& fgColors() {
        static const QMap<Type, QString> map {
            { General, QStringLiteral("#b45309") },
            { Key, QStringLiteral("#d97706") },
            { Special, QStringLiteral("#b91c1c") },
        };
        return map;
    }
    inline QString label(int v) { return EnumHelper<Type>::label(v, labels()); }
    inline QString color(int v) { return EnumHelper<Type>::color(v, colors()); }
    inline QString fgColor(int v) { return EnumHelper<Type>::color(v, fgColors()); }
}

#endif // CM_CONSTANTS_H
