#ifndef OC_CONSTANTS_H
#define OC_CONSTANTS_H

#include "EnumHelper.h"

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

#endif // OC_CONSTANTS_H
