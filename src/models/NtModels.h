#ifndef NTMODELS_H
#define NTMODELS_H

#include <QDateTime>
#include <QString>

#include "BaseModels.h"

struct NtAnnouncement : BaseEntity {
    QString title;
    QString content;
    int announcementType = 0;
    QString targetScope;
    QString targetIds;
    qint64 publisherId = 0;
    QDateTime publishTime;
    int isTop = 0;
    int readCount = 0;
    int status = 0;
};

struct NtNotification {
    qint64 id = 0;
    qint64 userId = 0;
    QString title;
    QString content;
    int notificationType = 0;
    QString bizType;
    qint64 bizId = 0;
    int isRead = 0;
    QDateTime readTime;
    QDateTime createTime;
};

#endif // NTMODELS_H
