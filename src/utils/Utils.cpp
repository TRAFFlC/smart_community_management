#include "Utils.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <limits>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <dpapi.h>
#else
#include <QDir>
#include <QFile>
#endif

namespace Utils {

static QAtomicInt g_sequenceCounter(0);

int nextSequenceNumber() {
    int prev = g_sequenceCounter.fetchAndAddRelaxed(1);
    // 溢出保护：接近 INT_MAX 时重置计数器，避免产生负数序列号
    if (prev >= std::numeric_limits<int>::max() - 1) {
        g_sequenceCounter.storeRelaxed(0);
    }
    return prev + 1;
}

QString hashPassword(const QString& password) {
    // 生成 16 字节随机盐
    QByteArray salt(16, '\0');
    for (int i = 0; i < 16; ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    // PBKDF2-SHA512，10000 轮迭代，输出 64 字节
    QByteArray hash = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha512,
        password.toUtf8(),
        salt,
        10000,
        64
    );
    return salt.toHex() + ":" + hash.toHex();
}

bool verifyPassword(const QString& password, const QString& hashed) {
    // 兼容旧格式（无冒号 = 旧 SHA256 + 固定盐）
    if (!hashed.contains(':')) {
        QByteArray oldHash = QCryptographicHash::hash(
            (password + QStringLiteral("smart_community_salt")).toUtf8(),
            QCryptographicHash::Sha256
        );
        return oldHash.toHex() == hashed;
    }
    // 新格式：salt:hash
    int sep = hashed.indexOf(':');
    QByteArray salt = QByteArray::fromHex(hashed.left(sep).toUtf8());
    QByteArray storedHash = QByteArray::fromHex(hashed.mid(sep + 1).toUtf8());
    QByteArray newHash = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha512,
        password.toUtf8(),
        salt,
        10000,
        64
    );
    return newHash == storedHash;
}

QString generateOrderNo() {
    QString prefix = QStringLiteral("WO");
    QString date = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd"));
    int seq = nextSequenceNumber() % 1000000;
    return prefix + date + QStringLiteral("%1").arg(seq, 6, 10, QChar('0'));
}

QString generateEventNo() {
    QString prefix = QStringLiteral("GE");
    QString date = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd"));
    int seq = nextSequenceNumber() % 1000000;
    return prefix + date + QStringLiteral("%1").arg(seq, 6, 10, QChar('0'));
}

QString generateServiceOrderNo() {
    QString prefix = QStringLiteral("SV");
    QString date = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd"));
    int seq = nextSequenceNumber() % 1000000;
    return prefix + date + QStringLiteral("%1").arg(seq, 6, 10, QChar('0'));
}

static const char* kEncPrefix = "encv2:";

#ifdef Q_OS_WIN

// Windows DPAPI：使用当前用户上下文加密，安全地保护本地敏感数据
QString secureEncrypt(const QString& plaintext) {
    if (plaintext.isEmpty()) return QString();

    QByteArray utf8 = plaintext.toUtf8();
    DATA_BLOB inBlob;
    inBlob.pbData = reinterpret_cast<BYTE*>(utf8.data());
    inBlob.cbData = static_cast<DWORD>(utf8.size());

    DATA_BLOB outBlob;
    if (!CryptProtectData(&inBlob, L"SmartCommunitySensitiveData", nullptr, nullptr,
                          nullptr, CRYPTPROTECT_UI_FORBIDDEN, &outBlob)) {
        qWarning() << "secureEncrypt failed:" << GetLastError();
        return QString();
    }

    QByteArray encrypted(reinterpret_cast<const char*>(outBlob.pbData), static_cast<int>(outBlob.cbData));
    LocalFree(outBlob.pbData);
    return QString::fromLatin1(kEncPrefix) + QString::fromLatin1(encrypted.toBase64());
}

QString secureDecrypt(const QString& ciphertext) {
    if (!ciphertext.startsWith(QString::fromLatin1(kEncPrefix)))
        return QString(); // 非新版本密文，由调用方自行按明文旧数据兼容

    QByteArray encrypted = QByteArray::fromBase64(ciphertext.mid(qstrlen(kEncPrefix)).toLatin1());
    if (encrypted.isEmpty()) return QString();

    DATA_BLOB inBlob;
    inBlob.pbData = reinterpret_cast<BYTE*>(encrypted.data());
    inBlob.cbData = static_cast<DWORD>(encrypted.size());

    DATA_BLOB outBlob;
    if (!CryptUnprotectData(&inBlob, nullptr, nullptr, nullptr, nullptr,
                            CRYPTPROTECT_UI_FORBIDDEN, &outBlob)) {
        qWarning() << "secureDecrypt failed:" << GetLastError();
        return QString();
    }

    QByteArray decrypted(reinterpret_cast<const char*>(outBlob.pbData), static_cast<int>(outBlob.cbData));
    LocalFree(outBlob.pbData);
    return QString::fromUtf8(decrypted);
}

#else // Non-Windows fallback

// 非 Windows 平台：没有系统级密钥环时，使用 per-installation 随机密钥做 XOR 混淆。
// 安全性低于 DPAPI，但优于硬编码固定种子，且不需要额外依赖。
static QByteArray loadOrCreateFallbackKey() {
    const QString keyDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString keyFile = keyDir + QStringLiteral("/.sc_key");
    QDir().mkpath(keyDir);

    QFile f(keyFile);
    if (f.exists() && f.open(QIODevice::ReadOnly)) {
        QByteArray key = QByteArray::fromBase64(f.readAll());
        if (key.size() == 32) return key;
    }

    QByteArray key(32, '\0');
    for (int i = 0; i < 32; ++i) {
        key[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(key.toBase64());
    }
    return key;
}

QString secureEncrypt(const QString& plaintext) {
    if (plaintext.isEmpty()) return QString();

    static const QByteArray key = loadOrCreateFallbackKey();
    QByteArray data = plaintext.toUtf8();
    const int keyLen = key.size();
    for (int i = 0; i < data.size(); ++i) {
        data[i] = static_cast<char>(static_cast<quint8>(data[i]) ^
                                    static_cast<quint8>(key[i % keyLen]));
    }
    return QString::fromLatin1(kEncPrefix) + QString::fromLatin1(data.toBase64());
}

QString secureDecrypt(const QString& ciphertext) {
    if (!ciphertext.startsWith(QString::fromLatin1(kEncPrefix)))
        return QString();

    static const QByteArray key = loadOrCreateFallbackKey();
    QByteArray data = QByteArray::fromBase64(ciphertext.mid(qstrlen(kEncPrefix)).toLatin1());
    if (data.isEmpty()) return QString();

    const int keyLen = key.size();
    for (int i = 0; i < data.size(); ++i) {
        data[i] = static_cast<char>(static_cast<quint8>(data[i]) ^
                                    static_cast<quint8>(key[i % keyLen]));
    }
    return QString::fromUtf8(data);
}

#endif

} // namespace Utils
