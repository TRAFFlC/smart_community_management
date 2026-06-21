#ifndef OCMODELS_H
#define OCMODELS_H

#include <QDateTime>
#include <QString>

#include "BaseModels.h"

struct OcTopic : BaseEntity {
    qint64 committeeId = 0;
    qint64 estateId = 0;
    QString title;
    QString content;
    int topicType = 0;
    qint64 publisherId = 0;
    QDateTime publishTime;
    int needVote = 0;
    QDateTime voteStart;
    QDateTime voteEnd;
    int voteResult = 0;
    int status = 0;
};

struct OcVote {
    qint64 id = 0;
    qint64 topicId = 0;
    qint64 voterId = 0;
    int choice = 0;
    QDateTime voteTime;
};

struct OcPublicIncome : BaseEntity {
    qint64 committeeId = 0;
    qint64 estateId = 0;
    QString itemName;
    int incomeType = 0;
    double amount = 0;
    QDateTime occurTime;
    QString payer;
    QString receiptNo;
    QString remark;
    int status = 0;
};

#endif // OCMODELS_H
