#ifndef UIKIT_AUTH_HELPERS_H
#define UIKIT_AUTH_HELPERS_H

#include <QPair>
#include <QString>
#include <QStringList>
#include <QVariantList>

namespace UiKit {

// 判断当前用户是否为居民（userType==0）
bool isResidentUser();

// 检查当前用户是否拥有指定 role_key 中的任意一个
// 超级管理员自动通过
bool currentUserHasRoleKey(const QStringList& roleKeys);

// 工单操作权限校验：根据状态和角色判断当前用户是否可执行该操作
// sts: 0=待受理, 1=已受理, 2=已派单, 3=处理中, 4=已完成
bool canOperateWorkOrder(int sts, qint64 reporterId, qint64 assigneeId);

// 事件操作权限校验：根据状态和角色判断当前用户是否可执行该操作
// sts: 0=待审核, 1=已审核, 2=已分派, 3=处理中
bool canOperateEvent(int sts, qint64 reporterId, qint64 assigneeId);

// 构建数据范围过滤 WHERE 子句片段
// 返回 (SQL 片段，以 " AND " 开头；绑定值扁平列表)
// tableAlias: 主表别名（如 "wo"）
// orgColumn: 组织列名（"estate_id" 用于工单，"community_org_id" 用于事件）
QPair<QString, QVariantList> buildDataScopeFilter(const QString& tableAlias = QString(),
                                                  const QString& orgColumn = QStringLiteral("estate_id"));

} // namespace UiKit

#endif // UIKIT_AUTH_HELPERS_H
