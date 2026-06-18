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
    static AuthService& instance() {
        static AuthService inst;
        return inst;
    }

    // 登录
    bool login(const QString& username, const QString& password) {
        auto& db = DatabaseManager::instance();
        QSqlQuery q = db.query(
            "SELECT * FROM sys_user WHERE username = :u AND del_flag = 0",
            {{":u", username}}
        );
        if (!q.next()) {
            m_lastError = QStringLiteral("用户不存在");
            return false;
        }

        QString storedHash = q.value("password").toString();
        if (!Utils::verifyPassword(password, storedHash)) {
            m_lastError = QStringLiteral("密码错误");
            // 记录登录失败日志
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

        // 加载当前用户完整信息
        loadCurrentUser(q.value("id").toLongLong());

        // 更新最后登录时间
        db.update("sys_user", m_currentUser.id, {
            {"last_login_time", QDateTime::currentDateTime()},
            {"last_login_ip", "127.0.0.1"}
        });

        // 记录登录成功日志
        db.insert("sys_login_log", {
            {"user_id", m_currentUser.id}, {"username", username},
            {"ip", "127.0.0.1"}, {"status", 0},
            {"message", QStringLiteral("登录成功")},
            {"login_time", QDateTime::currentDateTime()}
        });

        // 记录操作日志
        db.insert("sys_operation_log", {
            {"user_id", m_currentUser.id}, {"username", username},
            {"module", QStringLiteral("系统")}, {"operation", QStringLiteral("用户登录")},
            {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
        });

        return true;
    }

    void logout() {
        m_currentUser = SysUser();
        m_currentUser.id = 0;
    }

    bool isLoggedIn() const { return m_currentUser.id > 0; }

    const SysUser& currentUser() const { return m_currentUser; }

    QString lastError() const { return m_lastError; }

    // 获取当前用户菜单树
    QList<SysMenu> currentUserMenus() {
        if (!isLoggedIn()) return {};
        auto& db = DatabaseManager::instance();

        // 获取用户所有角色关联的菜单(去重)
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

    // 检查当前用户是否有指定权限
    bool hasPermission(const QString& permission) {
        if (!isLoggedIn()) return false;
        if (m_currentUser.permissions.contains("*:*:*")) return true; // 超级管理员
        return m_currentUser.permissions.contains(permission);
    }

    // 获取当前用户数据权限范围
    int currentUserDataScope() {
        if (!isLoggedIn()) return 6;
        // 取最高权限(最小数值 = 最大权限)
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

    // 重新加载当前用户信息（用于个人中心保存后刷新）
    void loadCurrentUserPublic(qint64 userId) {
        loadCurrentUser(userId);
    }

private:
    AuthService() = default;
    SysUser m_currentUser;
    QString m_lastError;

    void loadCurrentUser(qint64 userId) {
        auto& db = DatabaseManager::instance();
        QSqlQuery q = db.query(
            "SELECT * FROM sys_user WHERE id = :id AND del_flag = 0",
            {{":id", userId}}
        );
        if (!q.next()) return;

        m_currentUser.id = q.value("id").toLongLong();
        m_currentUser.username = q.value("username").toString();
        m_currentUser.password = q.value("password").toString();
        m_currentUser.nickname = q.value("nickname").toString();
        m_currentUser.realName = q.value("real_name").toString();
        m_currentUser.avatar = q.value("avatar").toString();
        m_currentUser.phone = q.value("phone").toString();
        m_currentUser.email = q.value("email").toString();
        m_currentUser.gender = q.value("gender").toInt();
        m_currentUser.status = q.value("status").toInt();
        m_currentUser.userType = q.value("user_type").toInt();

        // 加载角色
        m_currentUser.roleIds.clear();
        m_currentUser.roleNames.clear();
        QSqlQuery rq = db.query(
            "SELECT r.id, r.role_name, r.role_key, r.data_scope FROM sys_role r "
            "INNER JOIN sys_user_role ur ON r.id = ur.role_id "
            "WHERE ur.user_id = :uid AND r.status = 0",
            {{":uid", userId}}
        );
        while (rq.next()) {
            m_currentUser.roleIds.append(rq.value("id").toInt());
            m_currentUser.roleNames.append(rq.value("role_name").toString());
        }

        // 加载权限
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

        // 加载组织
        m_currentUser.orgIds.clear();
        QSqlQuery oq = db.query(
            "SELECT org_id FROM sys_user_org WHERE user_id = :uid",
            {{":uid", userId}}
        );
        while (oq.next()) {
            m_currentUser.orgIds.append(oq.value(0).toInt());
        }
    }

    QList<SysMenu> buildMenuTree(QList<SysMenu>& allMenus, qint64 parentId) {
        QList<SysMenu> tree;
        for (auto& menu : allMenus) {
            if (menu.parentId == parentId) {
                menu.children = buildMenuTree(allMenus, menu.id);
                tree.append(menu);
            }
        }
        return tree;
    }
};

#endif // AUTHSERVICE_H
