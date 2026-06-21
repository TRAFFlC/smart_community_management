#include "AIService.h"

#include <QDebug>
#include <QUrl>
#include <QtGlobal>

AIService& AIService::instance() {
    static AIService inst;
    return inst;
}

AIService::AIService() {
    loadConfig();
}

void AIService::setApiKey(const QString& key) {
    m_apiKey = key;
    saveConfig();
}

void AIService::setModel(const QString& m) {
    m_model = m;
    saveConfig();
}

void AIService::reloadConfig() {
    loadConfig();
}

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED

void AIService::loadConfig() {
    auto& db = DatabaseManager::instance();
    QSqlQuery q(db.database());
    if (!q.exec("SELECT config_key, config_value FROM sys_config WHERE config_key LIKE 'ai_%'")) {
        qWarning() << "[AI] Failed to load config from sys_config:" << q.lastError().text();
        if (m_model.isEmpty()) m_model = "poolside/laguna-m.1:free";
        return;
    }
    bool needsEncrypt = false;
    while (q.next()) {
        QString key = q.value(0).toString();
        QString val = q.value(1).toString();
        qDebug() << "[AI] Loaded config:" << key << "=" << (key.contains("key") ? val.left(6) + "..." : val);
        if (key == "ai_api_key") {
            if (val.startsWith(QStringLiteral("encv2:"))) {
                m_apiKey = Utils::secureDecrypt(val);
            } else if (val.startsWith(QStringLiteral("enc:"))) {
                // 旧版 XOR 密文迁移到新版 DPAPI/安全加密
                m_apiKey = Utils::simpleDecrypt(val);
                needsEncrypt = !m_apiKey.isEmpty();
            } else {
                m_apiKey = val;
                needsEncrypt = !m_apiKey.isEmpty();
            }
        } else if (key == "ai_model") {
            m_model = val;
        } else if (key.startsWith("ai_api_key_")) {
            // 备用 Key 同样使用新版加密存储
            if (val.startsWith(QStringLiteral("encv2:"))) {
                m_backupKeys << Utils::secureDecrypt(val);
            } else if (val.startsWith(QStringLiteral("enc:"))) {
                m_backupKeys << Utils::simpleDecrypt(val);
                needsEncrypt = true;
            } else {
                m_backupKeys << val;
                needsEncrypt = true;
            }
        }
    }
    if (m_model.isEmpty()) m_model = "poolside/laguna-m.1:free";
    if (m_apiKey.isEmpty()) {
        m_apiKey = qEnvironmentVariable("OPENROUTER_API_KEY");
        if (!m_apiKey.isEmpty()) {
            qDebug() << "[AI] Using env var OPENROUTER_API_KEY";
            needsEncrypt = true;
        }
    }
    // 自动迁移：将旧版明文或环境变量中的明文加密写入数据库
    if (needsEncrypt) {
        saveConfig();
    }
    qDebug() << "[AI] Final: key_len=" << m_apiKey.trimmed().length() << "model=" << m_model;
}

QT_WARNING_POP

void AIService::saveConfig() {
    auto& db = DatabaseManager::instance();
    qDebug() << "[AI] saveConfig: saving key_len=" << m_apiKey.trimmed().length() << "model=" << m_model;
    auto upsert = [&db](const QString& key, const QString& name, const QString& value) {
        QSqlQuery q(db.database());
        q.prepare("INSERT OR REPLACE INTO sys_config (config_name, config_key, config_value) VALUES (:name, :key, :val)");
        q.bindValue(":name", name);
        q.bindValue(":key", key);
        q.bindValue(":val", value);
        if (!q.exec()) {
            qWarning() << "[AI] saveConfig FAILED:" << q.lastError().text() << "key:" << key;
        } else {
            qDebug() << "[AI] saveConfig OK:" << key << "=" << (key.contains("key") ? value.left(6) + "..." : value);
        }
    };
    upsert("ai_api_key", QStringLiteral("AI 主 API Key"),
           m_apiKey.isEmpty() ? QString() : Utils::secureEncrypt(m_apiKey));
    upsert("ai_model", QStringLiteral("AI 模型名称"), m_model);
    for (int i = 0; i < m_backupKeys.size(); i++) {
        upsert(QString("ai_api_key_%1").arg(i+1),
               QStringLiteral("AI 备用 Key %1").arg(i+1),
               m_backupKeys[i].isEmpty() ? QString() : Utils::secureEncrypt(m_backupKeys[i]));
    }
}
