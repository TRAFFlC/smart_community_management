#include "ProfileDialog.h"

#include "ChangePasswordDialog.h"

#include "../database/DatabaseManager.h"
#include "../services/AuthService.h"
#include "../ui_kit/UiKit.h"

#include <QComboBox>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSqlQuery>
#include <QVBoxLayout>

using UiKit::showToast;

void ProfileDialog::show(QWidget* parent) {
    const auto& user = AuthService::instance().currentUser();
    if (user.id <= 0) return;

    QDialog dlg(parent);
    dlg.setWindowTitle(QStringLiteral("个人中心"));
    dlg.setMinimumWidth(560);
    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ===== 顶部用户信息区 =====
    auto* headerWidget = new QWidget(&dlg);
    headerWidget->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                                "stop:0 #b45309, stop:1 #d97706);");
    headerWidget->setFixedHeight(140);
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(32, 24, 32, 24);
    headerLayout->setSpacing(20);

    auto* avatarLabel = new QLabel(headerWidget);
    avatarLabel->setFixedSize(72, 72);
    avatarLabel->setAlignment(Qt::AlignCenter);
    QString initial = user.nickname.isEmpty() ? user.username.left(1).toUpper() : user.nickname.left(1);
    avatarLabel->setText(initial);
    avatarLabel->setStyleSheet("QLabel { background: #ffffff; color: #b45309; border-radius: 36px; "
                               "font-size: 32px; font-weight: bold; border: 3px solid rgba(255,255,255,0.4); }");
    headerLayout->addWidget(avatarLabel, 0, Qt::AlignVCenter);

    auto* nameInfoLayout = new QVBoxLayout();
    nameInfoLayout->setSpacing(6);
    auto* nameLabel = new QLabel(user.nickname.isEmpty() ? user.username : user.nickname, headerWidget);
    nameLabel->setStyleSheet("color: #ffffff; font-size: 20px; font-weight: 600; background: transparent; border: none;");
    nameInfoLayout->addWidget(nameLabel);

    QString userTypeLabel;
    switch (user.userType) {
        case 0: userTypeLabel = QStringLiteral("居民"); break;
        case 1: userTypeLabel = QStringLiteral("工作人员"); break;
        case 2: userTypeLabel = QStringLiteral("服务商"); break;
        case 3: userTypeLabel = QStringLiteral("管理员"); break;
        default: userTypeLabel = QStringLiteral("未知"); break;
    }
    auto* typeLabel = new QLabel(QStringLiteral("@%1 · %2").arg(user.username).arg(userTypeLabel), headerWidget);
    typeLabel->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 13px; background: transparent; border: none;");
    nameInfoLayout->addWidget(typeLabel);

    if (!user.roleNames.isEmpty()) {
        auto* roleLabel = new QLabel(QStringLiteral("角色: %1").arg(user.roleNames.join(QStringLiteral("、"))), headerWidget);
        roleLabel->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 12px; background: transparent; border: none;");
        nameInfoLayout->addWidget(roleLabel);
    }
    headerLayout->addLayout(nameInfoLayout, 1);
    layout->addWidget(headerWidget);

    // ===== 内容区（可滚动） =====
    auto* scrollArea = new QScrollArea(&dlg);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: #ffffff; }");

    auto* contentWidget = new QWidget();
    contentWidget->setObjectName("profileContent");
    contentWidget->setStyleSheet("#profileContent { background: #ffffff; }");
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(32, 24, 32, 24);
    contentLayout->setSpacing(16);

    // ===== 基本信息编辑区 =====
    auto* infoTitle = new QLabel(QStringLiteral("基本信息"), contentWidget);
    infoTitle->setStyleSheet("font-size: 15px; font-weight: 600; color: #0f172a; background: transparent; border: none;");
    contentLayout->addWidget(infoTitle);

    auto* infoGrid = new QGridLayout();
    infoGrid->setSpacing(12);
    infoGrid->setColumnStretch(1, 1);
    infoGrid->setColumnStretch(3, 1);

    auto createInfoLabel = [](const QString& text) {
        auto* lbl = new QLabel(text);
        lbl->setStyleSheet("color: #64748b; font-size: 13px; background: transparent; border: none;");
        return lbl;
    };
    auto createValueLabel = [](const QString& text) {
        auto* lbl = new QLabel(text);
        lbl->setStyleSheet("color: #0f172a; font-size: 13px; background: transparent; border: none;");
        return lbl;
    };
    auto createEditField = [](const QString& text, const QString& placeholder = QString()) {
        auto* edit = new QLineEdit(text);
        edit->setStyleSheet("QLineEdit { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 4px; "
                            "padding: 6px 10px; font-size: 13px; color: #0f172a; }"
                            "QLineEdit:focus { border-color: #b45309; background: #ffffff; }");
        if (!placeholder.isEmpty()) edit->setPlaceholderText(placeholder);
        return edit;
    };

    infoGrid->addWidget(createInfoLabel(QStringLiteral("用户名")), 0, 0);
    infoGrid->addWidget(createValueLabel(user.username), 0, 1);

    infoGrid->addWidget(createInfoLabel(QStringLiteral("真实姓名")), 0, 2);
    auto* realNameEdit = createEditField(user.realName, QStringLiteral("请输入真实姓名"));
    infoGrid->addWidget(realNameEdit, 0, 3);

    infoGrid->addWidget(createInfoLabel(QStringLiteral("昵称")), 1, 0);
    auto* nicknameEdit = createEditField(user.nickname, QStringLiteral("请输入昵称"));
    infoGrid->addWidget(nicknameEdit, 1, 1);

    infoGrid->addWidget(createInfoLabel(QStringLiteral("性别")), 1, 2);
    auto* genderCombo = new QComboBox(contentWidget);
    genderCombo->addItem(QStringLiteral("未知"), 0);
    genderCombo->addItem(QStringLiteral("男"), 1);
    genderCombo->addItem(QStringLiteral("女"), 2);
    genderCombo->setCurrentIndex(user.gender);
    genderCombo->setStyleSheet("QComboBox { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 4px; "
                                "padding: 6px 10px; font-size: 13px; color: #0f172a; }"
                                "QComboBox:focus { border-color: #b45309; }");
    infoGrid->addWidget(genderCombo, 1, 3);

    infoGrid->addWidget(createInfoLabel(QStringLiteral("手机号")), 2, 0);
    auto* phoneEdit = createEditField(user.phone, QStringLiteral("请输入手机号"));
    infoGrid->addWidget(phoneEdit, 2, 1);

    infoGrid->addWidget(createInfoLabel(QStringLiteral("邮箱")), 2, 2);
    auto* emailEdit = createEditField(user.email, QStringLiteral("请输入邮箱"));
    infoGrid->addWidget(emailEdit, 2, 3);

    // 查询用户所属组织（只读）
    QString orgNames;
    {
        QSqlQuery orgQ(DatabaseManager::instance().database());
        orgQ.prepare("SELECT o.org_name FROM sys_org o "
                     "JOIN sys_user_org uo ON o.id = uo.org_id "
                     "WHERE uo.user_id = :uid AND o.del_flag = 0");
        orgQ.bindValue(":uid", user.id);
        if (orgQ.exec()) {
            QStringList names;
            while (orgQ.next()) names << orgQ.value(0).toString();
            orgNames = names.join(QStringLiteral("、"));
        }
    }
    infoGrid->addWidget(createInfoLabel(QStringLiteral("所属组织")), 3, 0);
    infoGrid->addWidget(createValueLabel(orgNames.isEmpty() ? QStringLiteral("未分配") : orgNames), 3, 1);

    int dataScope = AuthService::instance().currentUserDataScope();
    QString scopeText;
    switch (dataScope) {
        case 1: scopeText = QStringLiteral("平台级（全部数据）"); break;
        case 2: scopeText = QStringLiteral("街道级"); break;
        case 3: scopeText = QStringLiteral("社区级"); break;
        case 4: scopeText = QStringLiteral("小区级"); break;
        case 5: scopeText = QStringLiteral("楼栋级"); break;
        case 6: scopeText = QStringLiteral("个人级（仅本人数据）"); break;
        case 7: scopeText = QStringLiteral("服务商级"); break;
        default: scopeText = QStringLiteral("未知"); break;
    }
    infoGrid->addWidget(createInfoLabel(QStringLiteral("数据范围")), 3, 2);
    infoGrid->addWidget(createValueLabel(scopeText), 3, 3);

    contentLayout->addLayout(infoGrid);

    auto* separator = new QFrame(contentWidget);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #f1f5f9; background: #f1f5f9; max-height: 1px; border: none;");
    contentLayout->addWidget(separator);

    // 账号统计卡片
    auto* statTitle = new QLabel(QStringLiteral("账号统计"), contentWidget);
    statTitle->setStyleSheet("font-size: 15px; font-weight: 600; color: #0f172a; background: transparent; border: none;");
    contentLayout->addWidget(statTitle);

    auto* statRow = new QHBoxLayout();
    statRow->setSpacing(12);

    auto createStatCard = [&contentWidget](const QString& title, int count, const QString& color) {
        auto* card = new QFrame(contentWidget);
        card->setStyleSheet(QString("QFrame { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 6px; }"));
        card->setFixedHeight(72);
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(16, 12, 16, 12);
        cardLayout->setSpacing(4);
        auto* numLabel = new QLabel(QString::number(count), card);
        numLabel->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: bold; background: transparent; border: none;").arg(color));
        numLabel->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(numLabel);
        auto* titleLabel = new QLabel(title, card);
        titleLabel->setStyleSheet("color: #64748b; font-size: 12px; background: transparent; border: none;");
        titleLabel->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(titleLabel);
        return card;
    };

    int woCount = 0, evCount = 0, unreadNoti = 0;
    {
        QSqlQuery woQ(DatabaseManager::instance().database());
        woQ.prepare("SELECT COUNT(*) FROM wo_work_order WHERE reporter_id = :uid AND del_flag = 0");
        woQ.bindValue(":uid", user.id);
        if (woQ.exec() && woQ.next()) woCount = woQ.value(0).toInt();

        QSqlQuery evQ(DatabaseManager::instance().database());
        evQ.prepare("SELECT COUNT(*) FROM ge_event WHERE reporter_id = :uid AND del_flag = 0");
        evQ.bindValue(":uid", user.id);
        if (evQ.exec() && evQ.next()) evCount = evQ.value(0).toInt();

        QSqlQuery notiQ(DatabaseManager::instance().database());
        notiQ.prepare("SELECT COUNT(*) FROM nt_notification WHERE user_id = :uid AND is_read = 0");
        notiQ.bindValue(":uid", user.id);
        if (notiQ.exec() && notiQ.next()) unreadNoti = notiQ.value(0).toInt();
    }

    statRow->addWidget(createStatCard(QStringLiteral("我的报修"), woCount, "#b45309"));
    statRow->addWidget(createStatCard(QStringLiteral("我的上报"), evCount, "#15803d"));
    statRow->addWidget(createStatCard(QStringLiteral("未读消息"), unreadNoti, "#dc2626"));
    contentLayout->addLayout(statRow);

    contentLayout->addStretch();

    // 底部按钮区
    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    auto* changePwdBtn = new QPushButton(QStringLiteral("修改密码"), contentWidget);
    changePwdBtn->setCursor(Qt::PointingHandCursor);
    changePwdBtn->setStyleSheet("QPushButton { padding: 8px 20px; border: 1px solid #e2e8f0; border-radius: 4px; "
                                "background: #ffffff; color: #475569; font-size: 13px; }"
                                "QPushButton:hover { border-color: #cbd5e1; background: #f8fafc; }");
    auto* saveBtn = new QPushButton(QStringLiteral("保存修改"), contentWidget);
    saveBtn->setProperty("cssClass", "primary");
    saveBtn->setCursor(Qt::PointingHandCursor);
    auto* closeBtn = new QPushButton(QStringLiteral("关闭"), contentWidget);
    closeBtn->setCursor(Qt::PointingHandCursor);
    btnRow->addWidget(changePwdBtn);
    btnRow->addWidget(saveBtn);
    btnRow->addWidget(closeBtn);
    contentLayout->addLayout(btnRow);

    scrollArea->setWidget(contentWidget);
    layout->addWidget(scrollArea, 1);

    QObject::connect(saveBtn, &QPushButton::clicked, &dlg, [&dlg, user, realNameEdit, nicknameEdit, phoneEdit, emailEdit, genderCombo]() {
        QString realName = realNameEdit->text().trimmed();
        QString nickname = nicknameEdit->text().trimmed();
        QString phone = phoneEdit->text().trimmed();
        QString email = emailEdit->text().trimmed();
        int gender = genderCombo->currentData().toInt();

        if (!phone.isEmpty() && phone.length() != 11) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("手机号格式不正确"));
            return;
        }
        if (!email.isEmpty() && !email.contains('@')) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("邮箱格式不正确"));
            return;
        }

        if (!DatabaseManager::instance().update("sys_user", user.id, {
            {"real_name", realName},
            {"nickname", nickname},
            {"phone", phone},
            {"email", email},
            {"gender", gender},
            {"update_time", QDateTime::currentDateTime()}
        })) {
            QMessageBox::warning(&dlg, QStringLiteral("错误"), QStringLiteral("保存失败，请重试"));
            return;
        }

        DatabaseManager::instance().insert("sys_operation_log", {
            {"user_id", user.id}, {"username", user.username},
            {"module", QStringLiteral("个人中心")}, {"operation", QStringLiteral("修改个人信息")},
            {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
        });

        AuthService::instance().loadCurrentUserPublic(user.id);

        QMessageBox::information(&dlg, QStringLiteral("成功"), QStringLiteral("个人信息保存成功"));
        dlg.accept();
    });

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(changePwdBtn, &QPushButton::clicked, &dlg, [&dlg, parent]() {
        dlg.accept();
        ChangePasswordDialog::show(parent);
    });

    dlg.exec();
}
