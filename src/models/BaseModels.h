#ifndef BASEMODELS_H
#define BASEMODELS_H

#include <QDateTime>
#include <QString>

// 通用基类
struct BaseEntity {
    qint64 id = 0;
    qint64 createBy = 0;
    QDateTime createTime;
    qint64 updateBy = 0;
    QDateTime updateTime;
    int delFlag = 0;
    QString remark;
};

#endif // BASEMODELS_H
