#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QPasswordDigestor>
#include <QByteArray>
#include <QAtomicInt>

namespace Utils {

// 进程内自增序列号，保证同一毫秒内生成的编号不会重复
int nextSequenceNumber();

// 密码哈希：PBKDF2-SHA512 + 16字节随机盐，10000轮迭代
// 返回格式: salt:hash (均为十六进制字符串)
// 旧版使用裸 SHA256 + 固定盐，verifyPassword 保留向后兼容
QString hashPassword(const QString& password);

// 密码校验：兼容新旧两种格式
// - 新格式 (含 ':'): salt:hash，使用 PBKDF2-SHA512 重新计算比对
// - 旧格式 (不含 ':'): 裸 SHA256 + 固定盐，确保已有数据库用户可登录
bool verifyPassword(const QString& password, const QString& hashed);

// 生成工单编号: WO + yyyyMMdd + 6位自增序列号（简洁、可排序、UNIQUE 约束兜底）
QString generateOrderNo();

// 生成事件编号: GE + yyyyMMdd + 6位自增序列号
QString generateEventNo();

// 生成服务订单编号: SV + yyyyMMdd + 6位自增序列号
QString generateServiceOrderNo();

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

// 本地敏感数据加密（如 API Key）。
// Windows 使用 DPAPI 绑定当前用户会话；其他平台回退到基于随机安装密钥的混淆。
// 返回格式: "encv2:" + Base64。
QString secureEncrypt(const QString& plaintext);

// 解密 secureEncrypt 生成的密文。
// 若密文格式不合法或解密失败，返回空字符串便于调用方识别。
// 注意：非 "encv2:" 前缀的密文不在这里处理，请调用方按明文旧数据兼容。
QString secureDecrypt(const QString& ciphertext);

// 【已废弃，仅用于迁移旧版 "enc:" XOR 密文】
// 读取旧版 simpleEncrypt 生成的密文；非 "enc:" 前缀返回原字符串。
[[deprecated("仅用于兼容旧版加密数据，新代码请使用 secureDecrypt")]]
inline QString simpleDecrypt(const QString& ciphertext) {
    if (!ciphertext.startsWith(QStringLiteral("enc:")) || ciphertext.startsWith(QStringLiteral("encv2:")))
        return ciphertext;

    QByteArray data = QByteArray::fromBase64(ciphertext.mid(4).toLatin1());
    if (data.isEmpty()) return QString();

    static const QByteArray key = QCryptographicHash::hash(
        QByteArrayLiteral("SmartCommunity_EncSeed_v1"),
        QCryptographicHash::Sha256);

    const int keyLen = key.size();
    for (int i = 0; i < data.size(); ++i) {
        data[i] = static_cast<char>(static_cast<quint8>(data[i]) ^
                                    static_cast<quint8>(key[i % keyLen]));
    }
    return QString::fromUtf8(data);
}

} // namespace Utils

#include <QTableWidgetItem>

// 自定义表格项，支持数值和日期的智能排序
class NumericSortTableWidgetItem : public QTableWidgetItem {
public:
    using QTableWidgetItem::QTableWidgetItem;

    bool operator<(const QTableWidgetItem& other) const override {
        // 尝试去除货币符号和逗号后按数值比较
        QString thisText = text().remove(QChar(0x00A5))
                               .remove(QChar::fromLatin1(','))
                               .remove(QChar::fromLatin1('%'))
                               .trimmed();
        QString otherText = other.text().remove(QChar(0x00A5))
                                .remove(QChar::fromLatin1(','))
                                .remove(QChar::fromLatin1('%'))
                                .trimmed();

        bool thisOk, otherOk;
        double thisVal = thisText.toDouble(&thisOk);
        double otherVal = otherText.toDouble(&otherOk);

        if (thisOk && otherOk)
            return thisVal < otherVal;

        // 尝试日期比较 (yyyy-MM-dd hh:mm 格式)
        QDateTime thisDate = QDateTime::fromString(text(), "yyyy-MM-dd hh:mm");
        QDateTime otherDate = QDateTime::fromString(other.text(), "yyyy-MM-dd hh:mm");
        if (thisDate.isValid() && otherDate.isValid())
            return thisDate < otherDate;

        // 回退到字符串比较
        return QTableWidgetItem::operator<(other);
    }
};

#endif // UTILS_H
