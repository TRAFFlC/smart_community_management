#ifndef SYS_CONSTANTS_H
#define SYS_CONSTANTS_H

#include "EnumHelper.h"

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

#endif // SYS_CONSTANTS_H
