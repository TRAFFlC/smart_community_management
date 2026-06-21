#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AIService.h"
#include "services/AuthService.h"

namespace PageFactory {

void buildSystemAI(BasePage *page, QVBoxLayout *layout, DatabaseManager &db, QLabel *emptyHint)
{
    (void)db;
    (void)emptyHint;

    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_robot"), QStringLiteral("智能问答助手"),
                                       QStringLiteral("输入您的问题，智能助手将为您提供解答"), UiKit::moduleColor("ai"), page));
    // API 状态指示（每次进入页面重新加载配置，确保管理员配置后所有用户可见）
    auto &aiSvc = AIService::instance();
    aiSvc.reloadConfig();
    auto *statusLabel = new QLabel(page);
    if (aiSvc.isConfigured())
    {
        QString keyMask = aiSvc.apiKey().trimmed();
        if (keyMask.length() > 10)
            keyMask = keyMask.left(6) + "..." + keyMask.right(4);
        statusLabel->setText(QStringLiteral("API 已配置 | 模型：%1 | Key：%2").arg(aiSvc.model(), keyMask));
        statusLabel->setStyleSheet("color:#15803d;font-size:12px;background:transparent;");
    }
    else
    {
        statusLabel->setText(QStringLiteral("注意：API 未配置，仅使用本地知识库"));
        statusLabel->setStyleSheet("color:#a16207;font-size:12px;background:transparent;");
    }
    layout->addWidget(statusLabel);
    layout->addSpacing(12);

    auto *chatDisplay = new QTextEdit(page);
    chatDisplay->setReadOnly(true);
    chatDisplay->setStyleSheet(R"(
            QTextEdit {
                background: #FAF9F6; border: 1px solid #D4D0C8; border-radius: 4px;
                padding: 12px; font-size: 14px; color: #141413;
            }
        )");
    {
        QString initHtml = QString::fromUtf8(
            "<div style='margin-bottom:12px;'>"
            "<div style='background:#b45309;color:#fff;padding:10px 14px;border-radius:2px;display:inline-block;max-width:80%;'>"
            "\xe6\x82\xa8\xe5\xa5\xbd\xef\xbc\x81\xe6\x88\x91\xe6\x98\xaf\xe6\x99\xba\xe6\x85\xa7\xe7\xa4\xbe\xe5\x8c\xba\xe6\x99\xba\xe8\x83\xbd\xe5\x8a\xa9\xe6\x89\x8b\xef\xbc\x8c\xe6\x9c\x89\xe4\xbb\x80\xe4\xb9\x88\xe5\x8f\xaf\xe4\xbb\xa5\xe5\xb8\xae\xe6\x82\xa8\xe7\x9a\x84\xe5\x90\x97\xef\xbc\x9f</div></div>"
            "<div style='color:#64748b;font-size:12px;'>\xe5\xb0\x9d\xe8\xaf\x95\xe8\xbe\x93\xe5\x85\xa5\xef\xbc\x9a\xe6\x80\x8e\xe4\xb9\x88\xe6\x8a\xa5\xe4\xbf\xae\xe3\x80\x81\xe7\x89\xa9\xe4\xb8\x9a\xe8\xb4\xb9\xe6\x80\x8e\xe4\xb9\x88\xe4\xba\xa4\xe3\x80\x81\xe5\x81\x9c\xe8\xbd\xa6\xe6\x9c\x88\xe5\x8d\xa1\xe3\x80\x81\xe5\xbf\x97\xe6\x84\xbf\xe8\x80\x85\xe3\x80\x81\xe7\xa4\xbe\xe5\x8c\xba\xe5\x9c\xa8\xe5\x93\xaa\xe9\x87\x8c\xe3\x80\x81\xe5\x8a\x9e\xe5\xb1\x85\xe4\xbd\x8f\xe8\xaf\x81</div>");
        chatDisplay->setHtml(initHtml);
    }
    layout->addWidget(chatDisplay, 1);

    auto *inputLayout = new QHBoxLayout();
    auto *inputEdit = new QLineEdit(page);
    inputEdit->setPlaceholderText(QStringLiteral("请输入您的问题..."));
    inputEdit->setStyleSheet("QLineEdit { padding: 10px 14px; border: 1px solid #D4D0C8; border-radius: 4px; font-size: 14px; }"
                             "QLineEdit:focus { border-color: #b45309; }");
    auto *sendBtn = new QPushButton(QStringLiteral("发送"), page);
    sendBtn->setProperty("cssClass", "primary");
    sendBtn->setFixedSize(80, 40);
    sendBtn->setCursor(Qt::PointingHandCursor);
    inputLayout->addWidget(inputEdit, 1);
    inputLayout->addWidget(sendBtn);
    layout->addLayout(inputLayout);

    auto *quickLayout = new QHBoxLayout();
    quickLayout->setSpacing(8);
    auto addQuickQ = [&](const QString &text)
    {
        auto *btn = new QPushButton(text, page);
        btn->setStyleSheet("QPushButton{background:rgba(146, 64, 14, 0.06);color:#b45309;border:1px solid #D4D0C8;border-radius:4px;padding:4px 12px;font-size:12px;}"
                           "QPushButton:hover{background:#b45309;color:#fff;}");
        btn->setCursor(Qt::PointingHandCursor);
        // 点击快捷问题后直接填充并发送
        QObject::connect(btn, &QPushButton::clicked, page, [inputEdit, sendBtn, text]()
        {
            inputEdit->setText(text);
            sendBtn->click(); // 直接触发发送
        });
        quickLayout->addWidget(btn);
    };
    addQuickQ(QStringLiteral("怎么报修？"));
    addQuickQ(QStringLiteral("物业费怎么交？"));
    addQuickQ(QStringLiteral("怎么办停车月卡？"));
    addQuickQ(QStringLiteral("怎么成为志愿者？"));
    addQuickQ(QStringLiteral("社区在哪里？"));

    // 管理员可配置系统级 API Key
    const auto &curUser = AuthService::instance().currentUser();
    bool isAdmin = curUser.permissions.contains("*:*:*") || curUser.userType == 3;
    if (isAdmin)
    {
        auto *settingsBtn = new QPushButton(QStringLiteral("API 设置"), page);
        settingsBtn->setStyleSheet("QPushButton{background:#D4D0C8;color:#64748b;border:1px solid #D4D0C8;border-radius:4px;padding:4px 12px;font-size:12px;}"
                                   "QPushButton:hover{border-color:#b45309;color:#b45309;}");
        settingsBtn->setCursor(Qt::PointingHandCursor);
        quickLayout->addWidget(settingsBtn);
        QObject::connect(settingsBtn, &QPushButton::clicked, page, [page]()
        {
            auto& ai = AIService::instance();
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("AI 服务配置（系统级）"));
            dlg.setMinimumWidth(500);
            auto* dl = new QVBoxLayout(&dlg);
            auto* tipLabel = new QLabel(QStringLiteral("配置 OpenRouter API Key，所有用户共享此配置。\n未配置时仅使用本地知识库回答。"), &dlg);
            tipLabel->setStyleSheet("color:#64748b;font-size:13px;"); tipLabel->setWordWrap(true);
            dl->addWidget(tipLabel); dl->addSpacing(8);
            auto* formLayout = new QFormLayout();
            auto* keyEdit = new QLineEdit(&dlg);
            keyEdit->setText(ai.apiKey());
            keyEdit->setPlaceholderText("sk-or-v1-xxxxx（以 sk-or- 开头）");
            keyEdit->setEchoMode(QLineEdit::Password);
            auto* modelCombo = new QComboBox(&dlg);
            for (const auto& m : AIService::availableModels()) modelCombo->addItem(m);
            modelCombo->setEditable(true);
            modelCombo->setCurrentText(ai.model());
            formLayout->addRow(QStringLiteral("API Key:"), keyEdit);
            formLayout->addRow(QStringLiteral("模型:"), modelCombo);
            dl->addLayout(formLayout);
            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("保存"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            dl->addWidget(buttons);
            if (dlg.exec() == QDialog::Accepted) {
                QString newKey = keyEdit->text().trimmed();
                if (!newKey.isEmpty() && !newKey.startsWith("sk-or-")) {
                    QMessageBox::warning(page, QStringLiteral("格式错误"),
                        QStringLiteral("API Key 应以 sk-or- 开头。\n请确认粘贴的是 OpenRouter 的 API Key，不是模型名称。"));
                    return;
                }
                ai.setApiKey(newKey);
                ai.setModel(modelCombo->currentText().trimmed());
                UiKit::showToast(QStringLiteral("AI 配置已保存到数据库"), page);
            }
        });
    }
    quickLayout->addStretch();
    layout->addLayout(quickLayout);

    // 发送消息：本地知识库 → LLM API 降级策略
    auto sendMessage = [=]()
    {
        QString question = inputEdit->text().trimmed();
        if (question.isEmpty())
            return;
        chatDisplay->append(QString("<div style='text-align:right;margin:8px 0;'><div style='background:#b45309;color:#fff;padding:8px 12px;border-radius:2px;display:inline-block;max-width:80%;'>%1</div></div>").arg(question));
        inputEdit->clear();
        sendBtn->setEnabled(false);

        // 1. 优先本地知识库
        QSqlQuery searchQ(DatabaseManager::instance().database());
        searchQ.prepare("SELECT id, answer FROM ai_knowledge WHERE status = 0 AND (question LIKE :q1 OR keywords LIKE :q2) ORDER BY hit_count DESC LIMIT 1");
        searchQ.bindValue(":q1", "%" + question + "%");
        searchQ.bindValue(":q2", "%" + question + "%");
        QString localAnswer;
        if (searchQ.exec() && searchQ.next())
        {
            localAnswer = searchQ.value(1).toString();
            qint64 kid = searchQ.value(0).toLongLong();
            QSqlQuery updQ(DatabaseManager::instance().database());
            updQ.prepare("UPDATE ai_knowledge SET hit_count = hit_count + 1 WHERE id = :id");
            updQ.bindValue(":id", kid);
            updQ.exec();
        }
        else
        {
            QStringList keywords = {QStringLiteral("报修"), QStringLiteral("物业"), QStringLiteral("停车"), QStringLiteral("志愿"), QStringLiteral("社区"), QStringLiteral("证件"), QStringLiteral("居住")};
            for (const auto &kw : keywords)
            {
                if (question.contains(kw))
                {
                    QSqlQuery fuzzyQ(DatabaseManager::instance().database());
                    fuzzyQ.prepare("SELECT answer FROM ai_knowledge WHERE status = 0 AND keywords LIKE :kw ORDER BY hit_count DESC LIMIT 1");
                    fuzzyQ.bindValue(":kw", "%" + kw + "%");
                    if (fuzzyQ.exec() && fuzzyQ.next())
                    {
                        localAnswer = fuzzyQ.value(0).toString();
                        break;
                    }
                }
            }
        }

        if (!localAnswer.isEmpty())
        {
            chatDisplay->append(QString("<div style='margin:8px 0;'><div style='background:#fff;border:1px solid #D4D0C8;padding:8px 12px;border-radius:2px;display:inline-block;max-width:80%;'>%1</div></div>").arg(localAnswer));
            sendBtn->setEnabled(true);
        }
        else if (AIService::instance().isConfigured())
        {
            // 2. 调用 LLM API
            chatDisplay->append(QStringLiteral("<div style='color:#64748b;font-size:12px;margin:4px 0;'>正在思考...</div>"));
            AIService::instance().ask(question, [chatDisplay, sendBtn](const QString &answer, bool success)
            {
                if (success) {
                    chatDisplay->append(QString("<div style='margin:8px 0;'><div style='background:#fff;border:1px solid #D4D0C8;padding:8px 12px;border-radius:2px;display:inline-block;max-width:80%;'>%1</div></div>").arg(answer));
                } else {
                    chatDisplay->append(QString("<div style='margin:8px 0;'><div style='background:#fff2f0;border:1px solid #ffccc7;padding:8px 12px;border-radius:2px;display:inline-block;max-width:80%;color:#b91c1c;'>%1</div></div>").arg(answer));
                }
                sendBtn->setEnabled(true);
            });
        }
        else
        {
            chatDisplay->append(QString("<div style='margin:8px 0;'><div style='background:#fff;border:1px solid #D4D0C8;padding:8px 12px;border-radius:2px;display:inline-block;max-width:80%;'>%1</div></div>")
                                .arg(QStringLiteral("抱歉，本地知识库暂无匹配答案。<br>如需 LLM 智能问答，请联系管理员在「系统管理 → 智能问答」中配置 API Key。")));
            sendBtn->setEnabled(true);
        }
    };

    QObject::connect(sendBtn, &QPushButton::clicked, page, sendMessage);
    QObject::connect(inputEdit, &QLineEdit::returnPressed, page, sendMessage);
}

} // namespace PageFactory
