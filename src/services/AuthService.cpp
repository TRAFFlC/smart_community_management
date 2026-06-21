#include "AuthService.h"

#include <QDateTime>
#include <QHostInfo>
#include <QSqlQuery>

// 桌面端登录 IP：优先使用本地主机名，获取失败时回退到 localhost
static QString localClientIp() {
    static QString ip = QHostInfo::localHostName();
    return ip.isEmpty() ? QStringLiteral("localhost") : ip;
}

AuthService& AuthService::instance() {
    static AuthService inst;
    return inst;
}

bool AuthService::login(const QString& username, const QString& password) {
    auto& db = DatabaseManager::instance();
    QSqlQuery q = db.query(
        "SELECT * FROM sys_user WHERE username = :u AND del_flag = 0",
        {{":u", username}}
    );
    if (!q.next()) {
        m_lastError = QStringLiteral("用户名或密码错误");
        return false;
    }

    QString storedHash = q.value("password").toString();
    if (!Utils::verifyPassword(password, storedHash)) {
        m_lastError = QStringLiteral("用户名或密码错误");
        db.insert("sys_login_log", {
            {"username", username}, {"status", 1},
            {"message", QStringLiteral("密码错误")},
            {"login_time", QDateTime::currentDateTime()}
        });
        return false;
    }

    int userSts = q.value("status").toInt();
    if (userSts == 1) {
        m_lastError = QStringLiteral("账号已被禁用");
        return false;
    }
    if (userSts == 2) {
        m_lastError = QStringLiteral("账号已被锁定");
        return false;
    }

    loadCurrentUser(q.value("id").toLongLong());

    db.update("sys_user", m_currentUser.id, {
        {"last_login_time", QDateTime::currentDateTime()},
        {"last_login_ip", localClientIp()}
    });

    db.insert("sys_login_log", {
        {"user_id", m_currentUser.id}, {"username", username},
        {"ip", localClientIp()}, {"status", 0},
        {"message", QStringLiteral("登录成功")},
        {"login_time", QDateTime::currentDateTime()}
    });

    db.insert("sys_operation_log", {
        {"user_id", m_currentUser.id}, {"username", username},
        {"module", QStringLiteral("系统")}, {"operation", QStringLiteral("用户登录")},
        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
    });

    return true;
}

void AuthService::logout() {
    m_currentUser = SysUser();
    m_currentUser.id = 0;
}

bool AuthService::isLoggedIn() const { return m_currentUser.id > 0; }

const SysUser& AuthService::currentUser() const { return m_currentUser; }

QString AuthService::lastError() const { return m_lastError; }

QList<SysMenu> AuthService::currentUserMenus() {
    if (!isLoggedIn()) return {};
    auto& db = DatabaseManager::instance();

    QString sql = R"(
        SELECT DISTINCT m.* FROM sys_menu m
        INNER JOIN sys_role_menu rm ON m.id = rm.menu_id
        INNER JOIN sys_user_role ur ON rm.role_id = ur.role_id
        WHERE ur.user_id = :uid AND m.status = 0 AND m.menu_type IN (1, 2)
        ORDER BY m.parent_id, m.sort_order
    )";

    QSqlQuery q = db.query(sql, {{":uid", m_currentUser.id}});
    QList<SysMenu> menus;
    while (q.next()) {
        SysMenu menu;
        menu.id = q.value("id").toLongLong();
        menu.parentId = q.value("parent_id").toLongLong();
        menu.menuName = q.value("menu_name").toString();
        menu.menuType = q.value("menu_type").toInt();
        menu.path = q.value("path").toString();
        menu.component = q.value("component").toString();
        menu.permission = q.value("permission").toString();
        menu.icon = q.value("icon").toString();
        menu.sortOrder = q.value("sort_order").toInt();
        menu.visible = q.value("visible").toInt();
        menus.append(menu);
    }

    return buildMenuTree(menus, 0);
}

bool AuthService::hasPermission(const QString& permission) {
    if (!isLoggedIn()) return false;
    if (m_currentUser.permissions.contains("*:*:*")) return true;
    return m_currentUser.permissions.contains(permission);
}

int AuthService::currentUserDataScope() {
    if (!isLoggedIn()) return 6;
    int minScope = 7;
    auto& db = DatabaseManager::instance();
    QSqlQuery q = db.query(
        "SELECT MIN(r.data_scope) FROM sys_role r "
        "INNER JOIN sys_user_role ur ON r.id = ur.role_id "
        "WHERE ur.user_id = :uid AND r.status = 0",
        {{":uid", m_currentUser.id}}
    );
    if (q.next() && !q.value(0).isNull()) {
        minScope = q.value(0).toInt();
    }
    return minScope;
}

void AuthService::loadCurrentUserPublic(qint64 userId) {
    loadCurrentUser(userId);
}

void AuthService::loadCurrentUser(qint64 userId) {
    auto& db = DatabaseManager::instance();
    // 明确列名，避免将密码哈希加载到内存中的 SysUser 结构体
    QSqlQuery q = db.query(
        "SELECT id, username, nickname, real_name, avatar, phone, email, gender, status, user_type "
        "FROM sys_user WHERE id = :id AND del_flag = 0",
        {{":id", userId}}
    );
    if (!q.next()) return;

    m_currentUser.id = q.value("id").toLongLong();
    m_currentUser.username = q.value("username").toString();
    m_currentUser.password.clear();
    m_currentUser.nickname = q.value("nickname").toString();
    m_currentUser.realName = q.value("real_name").toString();
    m_currentUser.avatar = q.value("avatar").toString();
    m_currentUser.phone = q.value("phone").toString();
    m_currentUser.email = q.value("email").toString();
    m_currentUser.gender = q.value("gender").toInt();
    m_currentUser.status = q.value("status").toInt();
    m_currentUser.userType = q.value("user_type").toInt();

    m_currentUser.roleIds.clear();
    m_currentUser.roleNames.clear();
    m_currentUser.roleKeys.clear();
    m_currentUser.roleDomain.clear();
    QSqlQuery rq = db.query(
        "SELECT r.id, r.role_name, r.role_key, r.role_domain, r.data_scope FROM sys_role r "
        "INNER JOIN sys_user_role ur ON r.id = ur.role_id "
        "WHERE ur.user_id = :uid AND r.status = 0 ORDER BY r.sort_order",
        {{":uid", userId}}
    );
    while (rq.next()) {
        m_currentUser.roleIds.append(rq.value("id").toInt());
        m_currentUser.roleNames.append(rq.value("role_name").toString());
        m_currentUser.roleKeys.append(rq.value("role_key").toString());
        if (m_currentUser.roleDomain.isEmpty()) {
            m_currentUser.roleDomain = rq.value("role_domain").toString();
        }
    }

    m_currentUser.permissions.clear();
    QSqlQuery pq = db.query(
        "SELECT DISTINCT m.permission FROM sys_menu m "
        "INNER JOIN sys_role_menu rm ON m.id = rm.menu_id "
        "INNER JOIN sys_user_role ur ON rm.role_id = ur.role_id "
        "WHERE ur.user_id = :uid AND m.permission IS NOT NULL AND m.permission != ''",
        {{":uid", userId}}
    );
    while (pq.next()) {
        m_currentUser.permissions.append(pq.value(0).toString());
    }

    m_currentUser.orgIds.clear();
    QSqlQuery oq = db.query(
        "SELECT org_id FROM sys_user_org WHERE user_id = :uid",
        {{":uid", userId}}
    );
    while (oq.next()) {
        m_currentUser.orgIds.append(oq.value(0).toInt());
    }
}

QList<SysMenu> AuthService::buildMenuTree(QList<SysMenu>& allMenus, qint64 parentId) {
    QList<SysMenu> tree;
    for (auto& menu : allMenus) {
        if (menu.parentId == parentId) {
            menu.children = buildMenuTree(allMenus, menu.id);
            tree.append(menu);
        }
    }
    return tree;
}
