#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <QObject>
#include "database/DatabaseManager.h"
#include "models/Models.h"
#include "models/Constants.h"
#include "utils/Utils.h"

class AuthService : public QObject {
    Q_OBJECT
public:
    static AuthService& instance();

    bool login(const QString& username, const QString& password);
    void logout();
    bool isLoggedIn() const;
    const SysUser& currentUser() const;
    QString lastError() const;

    QList<SysMenu> currentUserMenus();
    bool hasPermission(const QString& permission);
    int currentUserDataScope();

    // 重新加载当前用户信息（用于个人中心保存后刷新）
    void loadCurrentUserPublic(qint64 userId);

private:
    AuthService() = default;
    SysUser m_currentUser;
    QString m_lastError;

    void loadCurrentUser(qint64 userId);
    QList<SysMenu> buildMenuTree(QList<SysMenu>& allMenus, qint64 parentId);
};

#endif // AUTHSERVICE_H
