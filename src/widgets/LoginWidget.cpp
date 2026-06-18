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
#include <QPaintEvent>
#include <QLinearGradient>

// 左侧品牌区的自定义绘制 — 几何线条建筑感
class BrandPanel : public QWidget {
public:
    explicit BrandPanel(QWidget* parent = nullptr) : QWidget(parent) {
        setFixedWidth(440);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // 深色渐变背景
        QLinearGradient bg(0, 0, width(), height());
        bg.setColorAt(0, QColor("#0A1628"));
        bg.setColorAt(0.6, QColor("#0f172a"));
        bg.setColorAt(1, QColor("#001529"));
        p.fillRect(rect(), bg);

        // 几何线条 — 模拟城市天际线/建筑轮廓
        p.setPen(QPen(QColor(180, 83, 9, 25), 1));
        // 水平网格线
        for (int y = 60; y < height(); y += 40) {
            p.drawLine(0, y, width(), y);
        }
        // 垂直网格线
        for (int x = 40; x < width(); x += 40) {
            p.drawLine(x, 0, x, height());
        }

        // 建筑块 — 半透明矩形组合
        p.setBrush(QColor(180, 83, 9, 15));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRectF(40, height() * 0.35, 60, height() * 0.3), 4, 4);
        p.drawRoundedRect(QRectF(110, height() * 0.25, 50, height() * 0.4), 4, 4);
        p.drawRoundedRect(QRectF(170, height() * 0.4, 70, height() * 0.25), 4, 4);
        p.drawRoundedRect(QRectF(250, height() * 0.2, 55, height() * 0.45), 4, 4);
        p.drawRoundedRect(QRectF(315, height() * 0.3, 65, height() * 0.35), 4, 4);

        // 高亮建筑 — 主色填充
        p.setBrush(QColor(180, 83, 9, 40));
        p.drawRoundedRect(QRectF(170, height() * 0.4, 70, height() * 0.25), 4, 4);

        // 顶部光点
        p.setBrush(QColor(21, 128, 61, 180));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(205, height() * 0.38), 3, 3);
        p.setBrush(QColor(21, 128, 61, 60));
        p.drawEllipse(QPointF(205, height() * 0.38), 8, 8);

        // 品牌文字区
        p.setPen(QColor(255, 255, 255, 230));
        QFont titleFont("Microsoft YaHei UI", 22, QFont::Bold);
        p.setFont(titleFont);
        p.drawText(QRect(40, height() - 180, width() - 80, 36),
                   Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("智慧社区管理平台"));

        p.setPen(QColor(255, 255, 255, 100));
        QFont subFont("Segoe UI", 10);
        subFont.setLetterSpacing(QFont::AbsoluteSpacing, 2);
        p.setFont(subFont);
        p.drawText(QRect(40, height() - 140, width() - 80, 20),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QStringLiteral("SMART  COMMUNITY  PLATFORM"));

        // 分割线
        p.setPen(QPen(QColor(180, 83, 9, 80), 2));
        p.drawLine(40, height() - 108, 80, height() - 108);

        // 特性文字
        p.setPen(QColor(255, 255, 255, 80));
        QFont featFont("Microsoft YaHei UI", 9);
        p.setFont(featFont);
        int fy = height() - 80;
        p.drawText(QRect(40, fy, width() - 80, 16), Qt::AlignLeft,
                   QStringLiteral("多角色协同  ·  流程闭环  ·  数据驱动"));
    }
};

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(1040, 640);
    setStyleSheet("background: #f8fafc;");
    setupUI();
}

void LoginWidget::setupUI() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ====== 左侧品牌区（自定义绘制） ======
    auto* brandPanel = new BrandPanel(this);
    mainLayout->addWidget(brandPanel);

    // ====== 右侧登录表单 ======
    auto* formPanel = new QWidget(this);
    formPanel->setStyleSheet("background: #ffffff; border-radius: 6px;");
    auto* formLayout = new QVBoxLayout(formPanel);
    formLayout->setContentsMargins(72, 0, 72, 0);
    formLayout->addStretch();

    // 标题区
    auto* welcomeLabel = new QLabel(QStringLiteral("欢迎回来"), formPanel);
    welcomeLabel->setStyleSheet("font-size: 30px; font-weight: 700; color: #0f172a; background: transparent; letter-spacing: 1px;");
    formLayout->addWidget(welcomeLabel);

    formLayout->addSpacing(6);

    auto* descLabel = new QLabel(QStringLiteral("登录以进入智慧社区管理平台"), formPanel);
    descLabel->setStyleSheet("font-size: 14px; color: #64748b; background: transparent;");
    formLayout->addWidget(descLabel);

    formLayout->addSpacing(40);

    // 用户名
    auto* userLabel = new QLabel(QStringLiteral("账号"), formPanel);
    userLabel->setStyleSheet("font-size: 13px; color: #334155; font-weight: 600; background: transparent;");
    formLayout->addWidget(userLabel);
    formLayout->addSpacing(8);

    m_usernameEdit = new QLineEdit(formPanel);
    m_usernameEdit->setPlaceholderText(QStringLiteral("请输入账号"));
    m_usernameEdit->setText(QStringLiteral("admin"));
    m_usernameEdit->setFixedHeight(46);
    m_usernameEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #e2e8f0; border-radius: 4px; padding: 0 16px;
            font-size: 14px; background: #ffffff; color: #0f172a;
        }
        QLineEdit:hover { border-color: #94a3b8; background: #ffffff; }
        QLineEdit:focus { border-color: #b45309; background: #ffffff; }
    )");
    formLayout->addWidget(m_usernameEdit);
    formLayout->addSpacing(20);

    // 密码
    auto* passLabel = new QLabel(QStringLiteral("密码"), formPanel);
    passLabel->setStyleSheet("font-size: 13px; color: #334155; font-weight: 600; background: transparent;");
    formLayout->addWidget(passLabel);
    formLayout->addSpacing(8);

    m_passwordEdit = new QLineEdit(formPanel);
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passwordEdit->setText(QStringLiteral("admin123"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setFixedHeight(46);
    m_passwordEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #e2e8f0; border-radius: 4px; padding: 0 16px;
            font-size: 14px; background: #ffffff; color: #0f172a;
        }
        QLineEdit:hover { border-color: #94a3b8; background: #ffffff; }
        QLineEdit:focus { border-color: #b45309; background: #ffffff; }
    )");
    formLayout->addWidget(m_passwordEdit);
    formLayout->addSpacing(32);

    // 登录按钮
    m_loginBtn = new QPushButton(QStringLiteral("登 录"), formPanel);
    m_loginBtn->setFixedHeight(48);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(R"(
        QPushButton {
            background: #b45309; color: #ffffff; border: none; border-radius: 4px;
            font-size: 15px; font-weight: 600; letter-spacing: 4px;
        }
        QPushButton:hover { background: #92400e; }
        QPushButton:pressed { background: #78350f; }
        QPushButton:disabled { background: #94a3b8; }
    )");
    formLayout->addWidget(m_loginBtn);
    formLayout->addSpacing(24);

    // 演示账号提示 — 极简横条
    auto* hintFrame = new QFrame(formPanel);
    hintFrame->setStyleSheet(R"(
        QFrame {
            background: #f1f5f9; border: 1px solid #e2e8f0; border-radius: 6px;
        }
        QLabel { background: transparent; border: none; }
    )");
    auto* hintLayout = new QHBoxLayout(hintFrame);
    hintLayout->setContentsMargins(16, 12, 16, 12);
    hintLayout->setSpacing(16);

    auto* hintTitle = new QLabel(QStringLiteral("演示账号"), hintFrame);
    hintTitle->setStyleSheet("font-size: 12px; font-weight: 600; color: #64748b;");
    hintLayout->addWidget(hintTitle);

    auto* sep1 = new QFrame(hintFrame);
    sep1->setFrameShape(QFrame::VLine);
    sep1->setStyleSheet("color: #e2e8f0; max-width: 1px;");
    hintLayout->addWidget(sep1);

    auto* hint1 = new QLabel(QStringLiteral("管理员  admin / admin123"), hintFrame);
    hint1->setStyleSheet("font-size: 12px; color: #64748b;");
    hintLayout->addWidget(hint1);

    auto* sep2 = new QFrame(hintFrame);
    sep2->setFrameShape(QFrame::VLine);
    sep2->setStyleSheet("color: #e2e8f0; max-width: 1px;");
    hintLayout->addWidget(sep2);

    auto* hint2 = new QLabel(QStringLiteral("居民  zhangsan / 123456"), hintFrame);
    hint2->setStyleSheet("font-size: 12px; color: #64748b;");
    hintLayout->addWidget(hint2);

    hintLayout->addStretch();
    formLayout->addWidget(hintFrame);

    formLayout->addStretch();

    // 底部版权
    auto* copyrightLabel = new QLabel(QStringLiteral("© 2026 智慧社区管理平台  ·  课程实训项目"), formPanel);
    copyrightLabel->setStyleSheet("font-size: 11px; color: #94a3b8; background: transparent;");
    copyrightLabel->setAlignment(Qt::AlignCenter);
    formLayout->addWidget(copyrightLabel);
    formLayout->addSpacing(20);

    mainLayout->addWidget(formPanel, 1);

    // 连接信号
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWidget::onLogin);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLogin);
    connect(m_usernameEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLogin);
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
