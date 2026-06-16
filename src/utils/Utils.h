#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QCryptographicHash>
#include <QRandomGenerator>

namespace Utils {

// 简单密码哈希（演示用，生产环境应用 BCrypt）
inline QString hashPassword(const QString& password) {
    QByteArray hash = QCryptographicHash::hash(
        (password + "smart_community_salt").toUtf8(),
        QCryptographicHash::Sha256
    );
    return hash.toHex();
}

inline bool verifyPassword(const QString& password, const QString& hashed) {
    return hashPassword(password) == hashed;
}

// 生成工单编号: WO + yyyyMMddHHmmss + 4位随机数
inline QString generateOrderNo() {
    QString prefix = "WO";
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    int rand = QRandomGenerator::global()->bounded(1000, 9999);
    return prefix + timestamp + QString::number(rand);
}

// 生成事件编号: GE + yyyyMMddHHmmss + 4位随机数
inline QString generateEventNo() {
    QString prefix = "GE";
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    int rand = QRandomGenerator::global()->bounded(1000, 9999);
    return prefix + timestamp + QString::number(rand);
}

// 生成服务订单编号: SV + yyyyMMddHHmmss + 4位随机数
inline QString generateServiceOrderNo() {
    QString prefix = "SV";
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    int rand = QRandomGenerator::global()->bounded(1000, 9999);
    return prefix + timestamp + QString::number(rand);
}

// 手机号脱敏: 138****5678
inline QString maskPhone(const QString& phone) {
    if (phone.length() >= 7) {
        return phone.left(3) + "****" + phone.right(4);
    }
    return phone;
}

// 身份证脱敏: 110***********1234
inline QString maskIdCard(const QString& idCard) {
    if (idCard.length() >= 7) {
        return idCard.left(3) + QString(idCard.length() - 7, QChar('*')) + idCard.right(4);
    }
    return idCard;
}

// 计算SLA截止时间
inline QDateTime calculateSLA(int priority, const QString& bizType) {
    QDateTime now = QDateTime::currentDateTime();
    int hours = 48; // 默认

    if (bizType == "work_order") {
        switch(priority) {
            case 1: hours = 48; break;  // 普通
            case 2: hours = 4; break;   // 紧急
            case 3: hours = 2; break;   // 特急
        }
    } else if (bizType == "event") {
        switch(priority) {
            case 1: hours = 72; break;  // 一般
            case 2: hours = 24; break;  // 重要
            case 3: hours = 8; break;   // 紧急
            case 4: hours = 2; break;   // 特急
        }
    } else if (bizType == "complaint") {
        hours = 48;
    }

    return now.addSecs(hours * 3600);
}

// 格式化文件大小
inline QString formatFileSize(qint64 bytes) {
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024 * 1024 * 1024) return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GB";
}

// 生成会话ID
inline QString generateSessionId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace Utils

#endif // UTILS_H
