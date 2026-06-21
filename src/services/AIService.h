#ifndef AISERVICE_H
#define AISERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariantMap>
#include <functional>

#include "AuthService.h"
#include "database/DatabaseManager.h"
#include "models/Constants.h"
#include "ui_kit/AuthHelpers.h"
#include "utils/Utils.h"

// AI 智能问答服务
// 安全架构：
//   1. API Key 存储在本地配置文件中，不硬编码
//   2. 每次查询前根据用户权限构建安全的数据上下文
//   3. LLM 只能基于提供的上下文回答，无法直接访问数据库
//   4. 敏感数据（密码、身份证号）在上下文中被脱敏
class AIService : public QObject {
    Q_OBJECT
public:
    static AIService& instance();

    // 获取/设置 API 配置
    QString apiKey() const { return m_apiKey; }
    void setApiKey(const QString& key);
    QString model() const { return m_model; }
    void setModel(const QString& m);
    bool isConfigured() const { return !m_apiKey.isEmpty(); }
    static QStringList availableModels() { return kModels; }

    // 重新从数据库加载配置（用于用户切换/登录后刷新）
    void reloadConfig();

    // 发送问题，异步返回回答
    using ResponseCallback = std::function<void(const QString& answer, bool success)>;
    void ask(const QString& question, ResponseCallback callback);

private:
    QNetworkAccessManager m_nam;
    QString m_apiKey;
    QString m_model;
    QStringList m_backupKeys;
    int m_currentKeyIndex = 0;

    // 可选模型列表
    static inline const QStringList kModels = {
        "poolside/laguna-m.1:free",
        "nvidia/nemotron-3-super-120b-a12b:free",
        "nvidia/nemotron-3-ultra-550b-a55b:free",
        "openai/gpt-oss-120b:free",
    };

    AIService();

    // 从数据库 sys_config 表加载配置
    void loadConfig();

    // 保存配置到数据库
    void saveConfig();

    // 绑定数据范围过滤参数（扁平列表：名、值交替）
    static void bindScopeParams(QSqlQuery& q, const QVariantList& params);

    // 构建系统提示词
    QString buildSystemPrompt(const QString& context);

    // 构建安全的数据上下文（根据用户权限范围）
    QString buildSafeContext(const QString& question);

    // 各领域上下文查询
    QString queryWorkOrderContext(DatabaseManager& db, const SysUser& user);
    QString queryEventContext(DatabaseManager& db, const SysUser& user);
    QString queryAnnouncementContext(DatabaseManager& db, const SysUser& user);
    QString queryServiceContext(DatabaseManager& db, const SysUser& user);
    QString queryBillingContext(DatabaseManager& db, const SysUser& user);
    QString queryResidentContext(DatabaseManager& db, const SysUser& user);
    QString queryOverviewContext(DatabaseManager& db, const SysUser& user);

signals:
    void responseReceived(const QString& answer);
};

#endif // AISERVICE_H
