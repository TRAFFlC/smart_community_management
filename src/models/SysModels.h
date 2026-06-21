#ifndef SYSMODELS_H
#define SYSMODELS_H

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringList>

#include "BaseModels.h"

struct SysUser : BaseEntity {
    QString username;
    QString password;
    QString nickname;
    QString realName;
    QString avatar;
    QString phone;
    QString email;
    QString idCard;
    int gender = 0;
    QDate birthday;
    int status = 0;
    int userType = 0;
    QDateTime lastLoginTime;
    QString lastLoginIp;

    // 运行时关联(非数据库字段)
    QList<int> roleIds;
    QList<int> orgIds;
    QStringList roleNames;
    QStringList roleKeys;
    QStringList permissions;
    QString roleDomain;  // 当前用户的主角色域(resident/property/governance/service/supervision/platform)
};

struct SysRole : BaseEntity {
    QString roleName;
    QString roleKey;
    QString roleDomain;
    int dataScope = 6;
    int sortOrder = 0;
    int status = 0;
};

struct SysMenu {
    qint64 id = 0;
    qint64 parentId = 0;
    QString menuName;
    int menuType = 0;
    QString path;
    QString component;
    QString permission;
    QString icon;
    int sortOrder = 0;
    int visible = 1;
    int status = 0;
    QList<SysMenu> children;
};

struct SysOrg : BaseEntity {
    qint64 parentId = 0;
    QString orgName;
    int orgType = 0;
    QString orgCode;
    QString leader;
    QString phone;
    QString address;
    QString ancestors;
    int sortOrder = 0;
    int status = 0;
};

struct SysDictType {
    qint64 id = 0;
    QString dictName;
    QString dictType;
    int status = 0;
    QString remark;
};

struct SysDictData {
    qint64 id = 0;
    QString dictType;
    QString dictLabel;
    QString dictValue;
    int sortOrder = 0;
    QString cssClass;
    int status = 0;
};

struct SysOperationLog {
    qint64 id = 0;
    qint64 userId = 0;
    QString username;
    QString module;
    QString operation;
    QString method;
    QString requestUrl;
    QString requestMethod;
    QString requestParams;
    QString responseResult;
    QString ip;
    int status = 0;
    QString errorMsg;
    qint64 costTime = 0;
    QDateTime operationTime;
};

struct SysLoginLog {
    qint64 id = 0;
    qint64 userId = 0;
    QString username;
    QString ip;
    QString location;
    QString browser;
    QString os;
    int status = 0;
    QString message;
    QDateTime loginTime;
};

#endif // SYSMODELS_H
