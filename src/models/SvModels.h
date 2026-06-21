#ifndef SVMODELS_H
#define SVMODELS_H

#include <QDateTime>
#include <QString>

#include "BaseModels.h"

struct SvVolunteer : BaseEntity {
    qint64 userId = 0;
    qint64 residentId = 0;
    QString skills;
    QString availableTime;
    double totalHours = 0;
    int status = 0;
    QDateTime registerTime;
};

struct SvVolunteerActivity : BaseEntity {
    qint64 communityOrgId = 0;
    QString title;
    QString description;
    int activityType = 0;
    QString location;
    QDateTime startTime;
    QDateTime endTime;
    int needCount = 0;
    int enrolledCount = 0;
    qint64 publisherId = 0;
    int status = 0;
};

struct SvVolunteerSignup {
    qint64 id = 0;
    qint64 activityId = 0;
    qint64 volunteerId = 0;
    QDateTime signupTime;
    int status = 0;
    QDateTime checkinTime;
    QDateTime checkoutTime;
    double hours = 0;
};

struct SvServiceProvider : BaseEntity {
    qint64 orgId = 0;
    qint64 userId = 0;
    QString providerName;
    QString serviceTypes;
    QString contactPerson;
    QString contactPhone;
    QString serviceArea;
    double rating = 5.0;
    int totalOrders = 0;
    int status = 0;
};

struct SvServiceOrder : BaseEntity {
    QString orderNo;
    qint64 applicantId = 0;
    qint64 providerId = 0;
    int serviceType = 0;
    QString title;
    QString description;
    QDateTime appointmentTime;
    QString address;
    int status = 0;
    QDateTime acceptTime;
    QDateTime finishTime;
    QString proofImages;
    int rating = 0;
    QString comment;
};

#endif // SVMODELS_H
