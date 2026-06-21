#include "AIService.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

void AIService::ask(const QString& question, ResponseCallback callback) {
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

QString AIService::buildSystemPrompt(const QString& context) {
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
