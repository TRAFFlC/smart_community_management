#include "LoginWidget.h"
#include "services/AuthService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QPainter>
#include <QFont>

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(960, 600);
    setupUI();
}

void LoginWidget::setupUI() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ====== 左侧品牌区 ======
    auto* brandPanel = new QWidget(this);
    brandPanel->setFixedWidth(400);
    brandPanel->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:0.6, y2:1,
                stop:0 #1677ff, stop:0.5 #0958d9, stop:1 #003eb3);
        }
    )");
    auto* brandLayout = new QVBoxLayout(brandPanel);
    brandLayout->setContentsMargins(40, 60, 40, 60);

    brandLayout->addStretch();

    // Logo icon area
    auto* iconLabel = new QLabel(QStringLiteral("\xE2\x9C\xA6"), brandPanel);
    iconLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 48px; background: transparent;");
    iconLabel->setAlignment(Qt::AlignCenter);
    brandLayout->addWidget(iconLabel);
    brandLayout->addSpacing(16);

    auto* titleLabel = new QLabel(QStringLiteral("智慧社区管理平台"), brandPanel);
    titleLabel->setStyleSheet("color: #ffffff; font-size: 26px; font-weight: bold; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    brandLayout->addWidget(titleLabel);

    brandLayout->addSpacing(8);

    auto* subtitleLabel = new QLabel(QStringLiteral("Smart Community Management Platform"), brandPanel);
    subtitleLabel->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 12px; background: transparent; letter-spacing: 1px;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    brandLayout->addWidget(subtitleLabel);

    brandLayout->addSpacing(32);

    // Feature list with better spacing
    auto createFeatureItem = [&](const QString& icon, const QString& text) -> QWidget* {
        auto* row = new QWidget(brandPanel);
        row->setStyleSheet("background: transparent;");
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(24, 0, 0, 0);
        rowLayout->setSpacing(12);

        auto* iconLbl = new QLabel(icon, row);
        iconLbl->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 16px; background: transparent;");
        iconLbl->setFixedWidth(24);
        rowLayout->addWidget(iconLbl);

        auto* textLbl = new QLabel(text, row);
        textLbl->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 14px; background: transparent;");
        rowLayout->addWidget(textLbl);
        rowLayout->addStretch();
        return row;
    };

    brandLayout->addWidget(createFeatureItem(QStringLiteral("\u25CB"), QStringLiteral("小区管理与社区治理一体化")));
    brandLayout->addWidget(createFeatureItem(QStringLiteral("\u25CB"), QStringLiteral("居民服务与物业协同")));
    brandLayout->addWidget(createFeatureItem(QStringLiteral("\u25CB"), QStringLiteral("流程闭环与数据驱动")));
    brandLayout->addWidget(createFeatureItem(QStringLiteral("\u25CB"), QStringLiteral("多角色分层权限体系")));

    brandLayout->addStretch();

    // Version info
    auto* versionLabel = new QLabel(QStringLiteral("v1.0.0"), brandPanel);
    versionLabel->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 11px; background: transparent;");
    versionLabel->setAlignment(Qt::AlignCenter);
    brandLayout->addWidget(versionLabel);

    mainLayout->addWidget(brandPanel);

    // ====== 右侧登录表单 ======
    auto* formPanel = new QWidget(this);
    formPanel->setStyleSheet("background: #ffffff;");
    auto* formLayout = new QVBoxLayout(formPanel);
    formLayout->setContentsMargins(80, 0, 80, 0);

    // Center the form vertically
    formLayout->addStretch();

    auto* welcomeLabel = new QLabel(QStringLiteral("欢迎登录"), formPanel);
    welcomeLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #1f1f1f; background: transparent;");
    formLayout->addWidget(welcomeLabel);

    formLayout->addSpacing(4);

    auto* descLabel = new QLabel(QStringLiteral("请输入您的账号和密码以访问系统"), formPanel);
    descLabel->setStyleSheet("font-size: 14px; color: #8c8c8c; background: transparent;");
    formLayout->addWidget(descLabel);

    formLayout->addSpacing(36);

    // 用户名
    auto* userLabel = new QLabel(QStringLiteral("账号"), formPanel);
    userLabel->setStyleSheet("font-size: 14px; color: #595959; font-weight: 500; background: transparent;");
    formLayout->addWidget(userLabel);
    formLayout->addSpacing(6);

    m_usernameEdit = new QLineEdit(formPanel);
    m_usernameEdit->setPlaceholderText(QStringLiteral("请输入登录账号"));
    m_usernameEdit->setFixedHeight(44);
    m_usernameEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #d9d9d9; border-radius: 8px; padding: 0 14px;
            font-size: 14px; background: #ffffff; color: #1f1f1f;
        }
        QLineEdit:hover { border-color: #4096ff; }
        QLineEdit:focus { border-color: #1677ff; }
    )");
    formLayout->addWidget(m_usernameEdit);
    formLayout->addSpacing(16);

    // 密码
    auto* passLabel = new QLabel(QStringLiteral("密码"), formPanel);
    passLabel->setStyleSheet("font-size: 14px; color: #595959; font-weight: 500; background: transparent;");
    formLayout->addWidget(passLabel);
    formLayout->addSpacing(6);

    m_passwordEdit = new QLineEdit(formPanel);
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setFixedHeight(44);
    m_passwordEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #d9d9d9; border-radius: 8px; padding: 0 14px;
            font-size: 14px; background: #ffffff; color: #1f1f1f;
        }
        QLineEdit:hover { border-color: #4096ff; }
        QLineEdit:focus { border-color: #1677ff; }
    )");
    formLayout->addWidget(m_passwordEdit);
    formLayout->addSpacing(28);

    // 登录按钮
    m_loginBtn = new QPushButton(QStringLiteral("登  录"), formPanel);
    m_loginBtn->setFixedHeight(46);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(R"(
        QPushButton {
            background: #1677ff; color: #ffffff; border: none; border-radius: 8px;
            font-size: 16px; font-weight: 600;
        }
        QPushButton:hover { background: #4096ff; }
        QPushButton:pressed { background: #0958d9; }
        QPushButton:disabled { background: #91caff; }
    )");
    formLayout->addWidget(m_loginBtn);
    formLayout->addSpacing(20);

    // 演示账号提示 - card style
    auto* hintCard = new QFrame(formPanel);
    hintCard->setStyleSheet(R"(
        QFrame {
            background: #f6ffed; border: 1px solid #b7eb8f; border-radius: 8px;
            padding: 12px;
        }
        QLabel { background: transparent; }
    )");
    auto* hintLayout = new QVBoxLayout(hintCard);
    hintLayout->setContentsMargins(14, 10, 14, 10);
    hintLayout->setSpacing(4);

    auto* hintTitle = new QLabel(QStringLiteral("演示账号"), hintCard);
    hintTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #389e0d;");
    hintLayout->addWidget(hintTitle);

    auto* hint1 = new QLabel(QStringLiteral("管理员: admin / admin123"), hintCard);
    hint1->setStyleSheet("font-size: 12px; color: #52c41a;");
    hintLayout->addWidget(hint1);

    auto* hint2 = new QLabel(QStringLiteral("居    民: zhangsan / 123456"), hintCard);
    hint2->setStyleSheet("font-size: 12px; color: #52c41a;");
    hintLayout->addWidget(hint2);

    formLayout->addWidget(hintCard);

    formLayout->addStretch();

    mainLayout->addWidget(formPanel);

    // 连接信号
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWidget::onLogin);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLogin);
    connect(m_usernameEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLogin);

    m_usernameEdit->setText("admin");
    m_passwordEdit->setText("admin123");
}

void LoginWidget::onLogin() {
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请输入账号和密码"));
        return;
    }

    auto& auth = AuthService::instance();
    if (auth.login(username, password)) {
        emit loginSuccess();
    } else {
        QMessageBox::warning(this, QStringLiteral("登录失败"), auth.lastError());
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
    }
}
