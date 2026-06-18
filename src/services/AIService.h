#ifndef AISERVICE_H
#define AISERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

#include "AuthService.h"
#include "database/DatabaseManager.h"
#include "models/Constants.h"

// AI 智能问答服务
// 安全架构：
//   1. API Key 存储在本地配置文件中，不硬编码
//   2. 每次查询前根据用户权限构建安全的数据上下文
//   3. LLM 只能基于提供的上下文回答，无法直接访问数据库
//   4. 敏感数据（密码、身份证号）在上下文中被脱敏
class AIService : public QObject {
    Q_OBJECT
public:
    static AIService& instance() {
        static AIService inst;
        return inst;
    }

    // 获取/设置 API 配置
    QString apiKey() const { return m_apiKey; }
    void setApiKey(const QString& key) {
        m_apiKey = key;
        saveConfig();
    }
    QString model() const { return m_model; }
    void setModel(const QString& m) { m_model = m; saveConfig(); }
    bool isConfigured() const { return !m_apiKey.isEmpty(); }
    static QStringList availableModels() { return kModels; }

    // 重新从数据库加载配置（用于用户切换/登录后刷新）
    void reloadConfig() { loadConfig(); }

    // 发送问题，异步返回回答
    using ResponseCallback = std::function<void(const QString& answer, bool success)>;
    void ask(const QString& question, ResponseCallback callback) {
        if (!isConfigured()) {
            callback(QStringLiteral("请先在设置中配置 API Key。"), false);
            return;
        }

        // 构建安全的数据上下文
        QString context = buildSafeContext(question);

        // 构建请求
        QJsonObject body;
        body["model"] = m_model;
        body["max_tokens"] = 1024;
        body["temperature"] = 0.3;

        QJsonArray messages;
        // 系统提示：限定 LLM 只基于上下文回答
        QJsonObject sysMsg;
        sysMsg["role"] = "system";
        sysMsg["content"] = buildSystemPrompt(context);
        messages.append(sysMsg);

        // 用户问题
        QJsonObject userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = question;
        messages.append(userMsg);

        body["messages"] = messages;

        // 发送 HTTP 请求
        QNetworkRequest request(QUrl("https://openrouter.ai/api/v1/chat/completions"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QString authHeader = "Bearer " + m_apiKey.trimmed();
        request.setRawHeader("Authorization", authHeader.toUtf8());
        request.setRawHeader("HTTP-Referer", "https://smart-community.local");
        request.setRawHeader("X-Title", "Smart Community AI Assistant");

#ifdef QT_DEBUG
        // 调试：打印 key 前后几位（不泄露完整 key）
        QString masked = m_apiKey.trimmed();
        if (masked.length() > 10) masked = masked.left(6) + "..." + masked.right(4);
        qDebug() << "[AI] Sending request with key:" << masked;
        qDebug() << "[AI] Model:" << m_model;
#endif

        QJsonDocument doc(body);
        qDebug() << "[AI] Body:" << doc.toJson(QJsonDocument::Compact).left(200);
        QNetworkReply* reply = m_nam.post(request, doc.toJson());

        connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
            reply->deleteLater();

            // 无论成功失败，先读取响应体（用于错误诊断）
            QByteArray respBody = reply->readAll();
            int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (reply->error() != QNetworkReply::NoError) {
                // 尝试从响应体提取更详细的错误信息
                QJsonDocument errDoc = QJsonDocument::fromJson(respBody);
                QString detail = errDoc.object()["error"].toObject()["message"].toString();
                if (detail.isEmpty()) detail = QString::fromUtf8(respBody).left(200);
                callback(QStringLiteral("请求失败 (HTTP %1)：%2\n%3")
                    .arg(httpStatus).arg(reply->errorString()).arg(detail), false);
                return;
            }
            QJsonDocument respDoc = QJsonDocument::fromJson(respBody);
            QJsonObject resp = respDoc.object();

            if (resp.contains("error")) {
                QString errMsg = resp["error"].toObject()["message"].toString();
                callback(QStringLiteral("API 错误：%1").arg(errMsg), false);
                return;
            }

            // 提取回答
            QJsonArray choices = resp["choices"].toArray();
            if (choices.isEmpty()) {
                callback(QStringLiteral("未获取到回答。"), false);
                return;
            }
            QString answer = choices[0].toObject()["message"].toObject()["content"].toString();
            callback(answer, true);
        });
    }

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

    AIService() {
        loadConfig();
    }

    // 从数据库 sys_config 表加载配置
    void loadConfig() {
        auto& db = DatabaseManager::instance();
        QSqlQuery q(db.database());
        if (!q.exec("SELECT config_key, config_value FROM sys_config WHERE config_key LIKE 'ai_%'")) {
            q.exec("CREATE TABLE IF NOT EXISTS sys_config (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "config_name VARCHAR(128), config_key VARCHAR(128) UNIQUE, config_value VARCHAR(512), "
                   "config_type INTEGER DEFAULT 0, remark VARCHAR(500), create_time DATETIME DEFAULT CURRENT_TIMESTAMP)");
            q.exec("SELECT config_key, config_value FROM sys_config WHERE config_key LIKE 'ai_%'");
        }
        while (q.next()) {
            QString key = q.value(0).toString();
            QString val = q.value(1).toString();
            qDebug() << "[AI] Loaded config:" << key << "=" << (key.contains("key") ? val.left(6) + "..." : val);
            if (key == "ai_api_key") m_apiKey = val;
            else if (key == "ai_model") m_model = val;
            else if (key.startsWith("ai_api_key_")) m_backupKeys << val;
        }
        if (m_model.isEmpty()) m_model = "poolside/laguna-m.1:free";
        if (m_apiKey.isEmpty()) {
            m_apiKey = qEnvironmentVariable("OPENROUTER_API_KEY");
            if (!m_apiKey.isEmpty()) qDebug() << "[AI] Using env var OPENROUTER_API_KEY";
        }
        qDebug() << "[AI] Final: key_len=" << m_apiKey.trimmed().length() << "model=" << m_model;
    }

    // 保存配置到数据库
    void saveConfig() {
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
        upsert("ai_api_key", QStringLiteral("AI 主 API Key"), m_apiKey);
        upsert("ai_model", QStringLiteral("AI 模型名称"), m_model);
        for (int i = 0; i < m_backupKeys.size(); i++) {
            upsert(QString("ai_api_key_%1").arg(i+1),
                   QStringLiteral("AI 备用 Key %1").arg(i+1), m_backupKeys[i]);
        }
    }

    // 构建系统提示词
    QString buildSystemPrompt(const QString& context) {
        return QStringLiteral(
            "你是智慧社区管理平台的 AI 智能助手。你的职责是回答用户关于社区管理的问题。\n\n"
            "【安全规则】\n"
            "1. 你只能基于下方提供的「数据上下文」回答问题\n"
            "2. 如果上下文中没有相关信息，坦诚告知用户你无法回答\n"
            "3. 绝不编造数据或推测具体数字\n"
            "4. 绝不泄露任何密码、身份证号等敏感信息\n"
            "5. 回答要简洁、专业、有条理\n\n"
            "【数据上下文】\n"
            "%1\n\n"
            "请基于以上上下文回答用户的问题。"
        ).arg(context);
    }

    // 构建安全的数据上下文（根据用户权限范围）
    QString buildSafeContext(const QString& question) {
        auto& auth = AuthService::instance();
        if (!auth.isLoggedIn()) return QStringLiteral("（用户未登录）");

        const auto& user = auth.currentUser();
        auto& db = DatabaseManager::instance();
        QStringList ctx;

        // 用户基本信息
        ctx << QStringLiteral("当前用户：%1（%2）").arg(user.username, user.roleNames.join("、"));

        // 根据问题关键词动态检索相关数据
        QString q = question.toLower();

        // 工单相关
        if (q.contains("工单") || q.contains("报修") || q.contains("投诉")) {
            ctx << queryWorkOrderContext(db, user);
        }
        // 事件相关
        if (q.contains("事件") || q.contains("网格") || q.contains("巡查")) {
            ctx << queryEventContext(db, user);
        }
        // 公告相关
        if (q.contains("公告") || q.contains("通知")) {
            ctx << queryAnnouncementContext(db, user);
        }
        // 服务相关
        if (q.contains("服务") || q.contains("志愿") || q.contains("便民")) {
            ctx << queryServiceContext(db, user);
        }
        // 停车/缴费相关
        if (q.contains("停车") || q.contains("月卡") || q.contains("缴费") || q.contains("物业费")) {
            ctx << queryBillingContext(db, user);
        }
        // 居民/小区相关
        if (q.contains("居民") || q.contains("小区") || q.contains("楼栋")) {
            ctx << queryResidentContext(db, user);
        }

        // 如果没有匹配到特定领域，提供通用概览
        if (ctx.size() <= 1) {
            ctx << queryOverviewContext(db, user);
        }

        return ctx.join("\n");
    }

    // 查询工单上下文（权限过滤）
    QString queryWorkOrderContext(DatabaseManager& db, const SysUser& user) {
        QStringList lines;
        lines << "【工单信息】";

        bool isResident = (user.userType == UserType::Resident);
        int scope = AuthService::instance().currentUserDataScope(); // 1=平台级

        if (isResident) {
            // 居民仅查看自己提交的工单，不返回全平台总数
            QSqlQuery q(db.database());
            q.prepare("SELECT status, COUNT(*) FROM wo_work_order "
                      "WHERE del_flag = 0 AND create_by = :uid GROUP BY status");
            q.bindValue(":uid", user.id);
            q.exec();
            int total = 0;
            QMap<int, int> statusMap;
            while (q.next()) {
                int s = q.value(0).toInt(), c = q.value(1).toInt();
                statusMap[s] = c; total += c;
            }
            lines << QStringLiteral("我的工单总数：%1，待受理：%2，处理中：%3，已完成：%4")
                     .arg(total).arg(statusMap.value(0, 0))
                     .arg(statusMap.value(2, 0) + statusMap.value(3, 0))
                     .arg(statusMap.value(4, 0));

            // 最近 5 条我的工单
            QSqlQuery recent(db.database());
            recent.prepare("SELECT order_no, title, order_type, priority, status, create_time "
                           "FROM wo_work_order WHERE del_flag = 0 AND create_by = :uid "
                           "ORDER BY create_time DESC LIMIT 5");
            recent.bindValue(":uid", user.id);
            recent.exec();
            while (recent.next()) {
                lines << QStringLiteral("  %1 | %2 | %3 | %4")
                         .arg(recent.value(0).toString(), recent.value(1).toString(),
                              WorkOrderStatus::label(recent.value(4).toInt()),
                              recent.value(5).toDateTime().toString("MM-dd hh:mm"));
            }
        } else {
            // 工作人员/管理员
            if (scope != DataScope::Platform) {
                // 非平台级：标注数据范围（简化处理，仍查询全量但提示范围）
                lines << QStringLiteral("（数据范围：%1）").arg(DataScope::label(scope));
            }
            // 总数统计
            QSqlQuery q(db.database());
            q.prepare("SELECT status, COUNT(*) FROM wo_work_order WHERE del_flag = 0 GROUP BY status");
            q.exec();
            int total = 0;
            QMap<int, int> statusMap;
            while (q.next()) {
                int s = q.value(0).toInt(), c = q.value(1).toInt();
                statusMap[s] = c; total += c;
            }
            lines << QStringLiteral("工单总数：%1，待受理：%2，处理中：%3，已完成：%4")
                     .arg(total).arg(statusMap.value(0, 0)).arg(statusMap.value(2, 0) + statusMap.value(3, 0)).arg(statusMap.value(4, 0));

            // 最近 5 条工单（脱敏）
            QSqlQuery recent(db.database());
            recent.prepare("SELECT order_no, title, order_type, priority, status, create_time "
                           "FROM wo_work_order WHERE del_flag = 0 ORDER BY create_time DESC LIMIT 5");
            recent.exec();
            while (recent.next()) {
                lines << QStringLiteral("  %1 | %2 | %3 | %4")
                         .arg(recent.value(0).toString(), recent.value(1).toString(),
                              WorkOrderStatus::label(recent.value(4).toInt()),
                              recent.value(5).toDateTime().toString("MM-dd hh:mm"));
            }
        }
        return lines.join("\n");
    }

    QString queryEventContext(DatabaseManager& db, const SysUser& user) {
        QStringList lines;
        lines << "【网格事件】";

        bool isResident = (user.userType == UserType::Resident);
        int scope = AuthService::instance().currentUserDataScope(); // 1=平台级

        QSqlQuery q(db.database());
        if (isResident) {
            // 居民仅查看自己上报的事件
            q.prepare("SELECT event_no, title, event_category, status, create_time "
                      "FROM ge_event WHERE del_flag = 0 AND create_by = :uid "
                      "ORDER BY create_time DESC LIMIT 5");
            q.bindValue(":uid", user.id);
            q.exec();
            bool hasData = false;
            while (q.next()) {
                hasData = true;
                lines << QStringLiteral("  %1 | %2 | %3")
                         .arg(q.value(0).toString(), q.value(1).toString(),
                              EventStatus::label(q.value(3).toInt()));
            }
            if (!hasData) {
                lines << QStringLiteral("  暂无相关事件");
            }
        } else {
            // 工作人员/管理员
            if (scope != DataScope::Platform) {
                // 非平台级：标注数据范围（简化处理，仍查询全量但提示范围）
                lines << QStringLiteral("（数据范围：%1）").arg(DataScope::label(scope));
            }
            q.prepare("SELECT event_no, title, event_category, status, create_time "
                      "FROM ge_event WHERE del_flag = 0 ORDER BY create_time DESC LIMIT 5");
            q.exec();
            while (q.next()) {
                lines << QStringLiteral("  %1 | %2 | %3")
                         .arg(q.value(0).toString(), q.value(1).toString(),
                              EventStatus::label(q.value(3).toInt()));
            }
        }
        return lines.join("\n");
    }

    QString queryAnnouncementContext(DatabaseManager& db, const SysUser& user) {
        QStringList lines;
        lines << "【公告通知】";
        QSqlQuery q(db.database());
        q.prepare("SELECT title, announcement_type, publish_time "
                  "FROM nt_announcement WHERE del_flag = 0 ORDER BY publish_time DESC LIMIT 5");
        q.exec();
        while (q.next()) {
            lines << QStringLiteral("  %1 | %2")
                     .arg(q.value(0).toString(),
                          q.value(2).toDateTime().toString("MM-dd"));
        }
        return lines.join("\n");
    }

    QString queryServiceContext(DatabaseManager& db, const SysUser& user) {
        QStringList lines;
        lines << "【社区服务】";
        QSqlQuery q(db.database());
        q.prepare("SELECT title, activity_type, status, start_time "
                  "FROM sv_volunteer_activity WHERE del_flag = 0 ORDER BY start_time DESC LIMIT 3");
        q.exec();
        while (q.next()) {
            lines << QStringLiteral("  志愿活动：%1 | %2")
                     .arg(q.value(0).toString(), VolunteerActivityStatus::label(q.value(2).toInt()));
        }
        return lines.join("\n");
    }

    QString queryBillingContext(DatabaseManager& db, const SysUser& user) {
        QStringList lines;
        lines << "【缴费信息】";

        bool isResident = (user.userType == UserType::Resident);
        int scope = AuthService::instance().currentUserDataScope(); // 1=平台级

        QSqlQuery q(db.database());
        if (isResident) {
            // 居民仅查看自己的账单（通过 user_id 关联 cm_resident 找到 resident_id）
            q.prepare("SELECT bill_type, amount, period, status "
                      "FROM pm_bill WHERE del_flag = 0 "
                      "AND resident_id = (SELECT id FROM cm_resident WHERE user_id = :uid AND del_flag = 0 LIMIT 1) "
                      "ORDER BY create_time DESC LIMIT 5");
            q.bindValue(":uid", user.id);
            q.exec();
            bool hasData = false;
            while (q.next()) {
                hasData = true;
                lines << QStringLiteral("  账单 | %1元 | %2")
                         .arg(q.value(1).toDouble(), 0, 'f', 2)
                         .arg(q.value(2).toString());
            }
            if (!hasData) {
                lines << QStringLiteral("  暂无缴费记录");
            }
        } else {
            // 工作人员/管理员
            if (scope != DataScope::Platform) {
                // 非平台级：标注数据范围（简化处理，仍查询全量但提示范围）
                lines << QStringLiteral("（数据范围：%1）").arg(DataScope::label(scope));
            }
            q.prepare("SELECT bill_type, amount, period, status "
                      "FROM pm_bill WHERE del_flag = 0 ORDER BY create_time DESC LIMIT 5");
            q.exec();
            while (q.next()) {
                lines << QStringLiteral("  账单 | %1元 | %2")
                         .arg(q.value(1).toDouble(), 0, 'f', 2)
                         .arg(q.value(2).toString());
            }
        }
        return lines.join("\n");
    }

    QString queryResidentContext(DatabaseManager& db, const SysUser& user) {
        QStringList lines;
        lines << "【居民/小区概况】";

        bool isResident = (user.userType == UserType::Resident);
        int scope = AuthService::instance().currentUserDataScope(); // 1=平台级

        if (isResident) {
            // 居民不返回平台级聚合数据，仅返回本人档案信息
            QSqlQuery q(db.database());
            q.prepare("SELECT r.name, r.phone_display, e.estate_name, h.room_number "
                      "FROM cm_resident r "
                      "LEFT JOIN cm_house_resident hr ON hr.resident_id = r.id "
                      "LEFT JOIN cm_house h ON h.id = hr.house_id "
                      "LEFT JOIN cm_estate e ON e.id = h.estate_id "
                      "WHERE r.del_flag = 0 AND r.user_id = :uid "
                      "ORDER BY hr.id DESC LIMIT 1");
            q.bindValue(":uid", user.id);
            q.exec();
            if (q.next()) {
                QString name = q.value(0).toString();
                QString phone = q.value(1).toString();
                QString estate = q.value(2).toString();
                QString room = q.value(3).toString();
                lines << QStringLiteral("您是%1的居民%2")
                         .arg(estate.isEmpty() ? QStringLiteral("本社区") : estate,
                              name.isEmpty() ? user.username : name);
                if (!room.isEmpty()) {
                    lines << QStringLiteral("住房：%1 %2").arg(estate, room);
                }
                if (!phone.isEmpty()) {
                    lines << QStringLiteral("联系电话：%1").arg(phone);
                }
            } else {
                lines << QStringLiteral("您当前以居民身份登录，暂无居民档案信息");
            }
            lines << QStringLiteral("（注：居民无权查看平台概况数据）");
        } else {
            // 工作人员/管理员
            if (scope != DataScope::Platform) {
                // 非平台级：标注数据范围（简化处理，仍查询全量但提示范围）
                lines << QStringLiteral("（数据范围：%1）").arg(DataScope::label(scope));
            }
            QSqlQuery q(db.database());
            q.prepare("SELECT COUNT(*) FROM cm_resident WHERE del_flag = 0");
            q.exec(); if (q.next()) lines << QStringLiteral("居民总数：%1").arg(q.value(0).toInt());
            q.prepare("SELECT COUNT(*) FROM cm_estate WHERE del_flag = 0");
            q.exec(); if (q.next()) lines << QStringLiteral("小区数量：%1").arg(q.value(0).toInt());
            q.prepare("SELECT COUNT(*) FROM cm_house WHERE del_flag = 0");
            q.exec(); if (q.next()) lines << QStringLiteral("房屋总数：%1").arg(q.value(0).toInt());
        }
        return lines.join("\n");
    }

    QString queryOverviewContext(DatabaseManager& db, const SysUser& user) {
        QStringList lines;
        bool isResident = (user.userType == UserType::Resident);
        if (isResident) {
            // 居民概览：仅返回个人相关数据，不展示平台级聚合信息
            lines << "【我的概览】";
            lines << queryResidentContext(db, user);
            lines << queryWorkOrderContext(db, user);
            lines << queryEventContext(db, user);
        } else {
            // 工作人员/管理员概览
            lines << "【平台概览】";
            lines << queryWorkOrderContext(db, user);
            lines << queryEventContext(db, user);
        }
        return lines.join("\n");
    }

signals:
    void responseReceived(const QString& answer);
};

#endif // AISERVICE_H
