#ifndef AIMODELS_H
#define AIMODELS_H

#include <QDateTime>
#include <QString>

#include "BaseModels.h"

struct AiKnowledge : BaseEntity {
    QString category;
    QString question;
    QString answer;
    QString keywords;
    int priority = 0;
    int hitCount = 0;
    int status = 0;
};

struct AiChatLog {
    qint64 id = 0;
    qint64 userId = 0;
    QString sessionId;
    QString role;
    QString content;
    QString intent;
    qint64 matchedKnowledgeId = 0;
    int isUseful = -1;
    QDateTime createTime;
};

#endif // AIMODELS_H
