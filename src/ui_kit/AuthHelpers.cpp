#include "AuthHelpers.h"

#include "../database/DatabaseManager.h"
#include "../services/AuthService.h"

#include <QList>
#include <QSet>
#include <QSqlQuery>
#include <QStringList>

namespace UiKit {

bool isResidentUser() {
    return AuthService::instance().isLoggedIn() && AuthService::instance().currentUser().userType == 0;
}

bool currentUserHasRoleKey(const QStringList& roleKeys) {
    auto& auth = AuthService::instance();
    if (!auth.isLoggedIn()) return false;
    const auto& user = auth.currentUser();
    if (user.permissions.contains("*:*:*")) return true; // 超级管理员
    if (roleKeys.isEmpty()) return true;

    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT r.role_key FROM sys_role r "
              "JOIN sys_user_role ur ON r.id = ur.role_id "
              "WHERE ur.user_id = :uid AND r.status = 0 AND r.del_flag = 0");
    q.bindValue(":uid", user.id);
    q.exec();
    QSet<QString> userRoleKeys;
    while (q.next()) userRoleKeys.insert(q.value(0).toString());
    for (const auto& key : roleKeys) {
        if (userRoleKeys.contains(key)) return true;
    }
    return false;
}

bool canOperateWorkOrder(int sts, qint64 reporterId, qint64 assigneeId) {
    auto& auth = AuthService::instance();
    if (!auth.isLoggedIn()) return false;
    const auto& user = auth.currentUser();
    if (user.permissions.contains("*:*:*")) return true; // 超级管理员

    switch (sts) {
        case 0: // 受理：物业客服/管家/经理
            return currentUserHasRoleKey({"property_cs", "property_steward", "property_manager"});
        case 1: // 派单：物业客服/经理
            return currentUserHasRoleKey({"property_cs", "property_manager"});
        case 2: // 开始处理：维修人员（限本人）
            return assigneeId == user.id || currentUserHasRoleKey({"property_manager"});
        case 3: // 完成：维修人员（限本人）
            return assigneeId == user.id || currentUserHasRoleKey({"property_manager"});
        case 4: // 评价：报修人（限本人）
            return reporterId == user.id;
        default:
            return false;
    }
}

bool canOperateEvent(int sts, qint64 reporterId, qint64 assigneeId) {
    auto& auth = AuthService::instance();
    if (!auth.isLoggedIn()) return false;
    const auto& user = auth.currentUser();
    if (user.permissions.contains("*:*:*")) return true; // 超级管理员

    switch (sts) {
        case 0: // 审核：社区工作人员/社区书记
            return currentUserHasRoleKey({"community_worker", "community_secretary"});
        case 1: // 分派：社区工作人员/社区书记
            return currentUserHasRoleKey({"community_worker", "community_secretary"});
        case 2: // 处理：社区工作人员/网格员（限本人）
            return assigneeId == user.id || currentUserHasRoleKey({"community_secretary"});
        case 3: // 完成：社区工作人员/网格员（限本人）
            return assigneeId == user.id || currentUserHasRoleKey({"community_secretary"});
        default:
            return false;
    }
}

QPair<QString, QVariantList> buildDataScopeFilter(const QString& tableAlias, const QString& orgColumn) {
    auto& auth = AuthService::instance();
    if (!auth.isLoggedIn()) return {"", {}};

    const auto& user = auth.currentUser();

    // 平台管理员和超级管理员：不做过滤
    if (user.permissions.contains("*:*:*") || user.userType == 3) return {"", {}};

    int scope = auth.currentUserDataScope();
    QString p = tableAlias.isEmpty() ? "" : tableAlias + ".";

    // scope 1 = 平台级，不做过滤
    if (scope <= 1) return {"", {}};

    // scope >= 6 = 个人级/服务商级，只看自己上报的数据
    // 居民(userType==0)也只看自己的数据
    if (scope >= 6 || user.userType == 0) {
        return {QString(" AND %1reporter_id = :scope_uid").arg(p), {":scope_uid", user.id}};
    }

    // scope 2-5: 基于组织的数据范围过滤
    QSqlQuery orgQ(DatabaseManager::instance().database());
    orgQ.prepare("SELECT o.id, o.org_type FROM sys_org o "
                 "JOIN sys_user_org uo ON o.id = uo.org_id "
                 "WHERE uo.user_id = :uid AND o.del_flag = 0");
    orgQ.bindValue(":uid", user.id);
    orgQ.exec();

    QList<qint64> orgIds;
    while (orgQ.next()) {
        orgIds.append(orgQ.value(0).toLongLong());
    }

    if (orgIds.isEmpty()) {
        return {QString(" AND %1reporter_id = :scope_uid").arg(p), {":scope_uid", user.id}};
    }

    QVariantList binds;
    if (orgColumn == QStringLiteral("estate_id")) {
        QSqlQuery estateQ(DatabaseManager::instance().database());
        QString placeholders = QStringList(orgIds.size(), QStringLiteral("?")).join(QStringLiteral(","));
        estateQ.prepare(QStringLiteral("SELECT DISTINCT e.id FROM cm_estate e "
                                       "LEFT JOIN sys_org o ON e.org_id = o.id "
                                       "WHERE o.id IN (%1) AND e.del_flag = 0").arg(placeholders));
        for (int i = 0; i < orgIds.size(); ++i) estateQ.bindValue(i, orgIds[i]);
        estateQ.exec();

        QList<qint64> estateIds;
        while (estateQ.next()) estateIds.append(estateQ.value(0).toLongLong());

        if (estateIds.isEmpty()) {
            return {QString(" AND %1reporter_id = :scope_uid").arg(p), {":scope_uid", user.id}};
        }

        QStringList phList;
        for (int i = 0; i < estateIds.size(); ++i) {
            phList << QString(QStringLiteral(":scope_eid_%1")).arg(i);
            binds << phList.last() << estateIds[i];
        }
        return {QString(" AND %1estate_id IN (%2)").arg(p).arg(phList.join(QStringLiteral(","))), binds};
    } else if (orgColumn == QStringLiteral("community_org_id")) {
        QStringList phList;
        for (int i = 0; i < orgIds.size(); ++i) {
            phList << QString(QStringLiteral(":scope_oid_%1")).arg(i);
            binds << phList.last() << orgIds[i];
        }
        return {QString(" AND %1community_org_id IN (%2)").arg(p).arg(phList.join(QStringLiteral(","))), binds};
    }

    return {QString(" AND %1reporter_id = :scope_uid").arg(p), {":scope_uid", user.id}};
}

} // namespace UiKit
