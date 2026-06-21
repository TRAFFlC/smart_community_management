#ifndef EVMODELS_H
#define EVMODELS_H

#include <QDateTime>
#include <QString>

#include "BaseModels.h"

struct EvEvaluation : BaseEntity {
    QString bizType;
    qint64 bizId = 0;
    qint64 evaluatorId = 0;
    int rating = 0;
    QString content;
    QString reply;
    qint64 replyBy = 0;
    QDateTime replyTime;
    QDateTime evalTime;
};

#endif // EVMODELS_H
