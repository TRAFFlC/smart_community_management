#ifndef KFMODELS_H
#define KFMODELS_H

#include <QDateTime>
#include <QString>

#include "BaseModels.h"

struct KfAssessmentConfig : BaseEntity {
    QString configName;
    int assessmentType = 0;
    QString targetOrg;
    QString targetRole;
    QString periodType;
    QString metric;
    double weight = 0;
    double targetValue = 0;
    int status = 0;
};

struct KfAssessmentResult : BaseEntity {
    qint64 configId = 0;
    qint64 targetOrgId = 0;
    qint64 targetUserId = 0;
    QString period;
    double actualValue = 0;
    double score = 0;
    int rank = 0;
    QDateTime assessTime;
};

#endif // KFMODELS_H
