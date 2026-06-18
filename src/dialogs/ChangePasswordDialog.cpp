#include "ChangePasswordDialog.h"

#include "../database/DatabaseManager.h"
#include "../services/AuthService.h"
#include "../ui_kit/UiKit.h"
#include "../utils/Utils.h"

#include <QDateTime>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QVBoxLayout>

using UiKit::showToast;

void ChangePasswordDialog::show(QWidget* parent) {
    const auto& user = AuthService::instance().currentUser();
    if (user.id <= 0) return;

    QDialog dlg(parent);
    dlg.setWindowTitle(QStringLiteral("修改密码"));
    dlg.setFixedWidth(420);
    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(12);

    auto* titleLabel = new QLabel(QStringLiteral("修改密码"), &dlg);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
    layout->addWidget(titleLabel);
    layout->addSpacing(4);

    auto* form = new QFormLayout();
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto* oldPwdEdit = new QLineEdit(&dlg);
    oldPwdEdit->setEchoMode(QLineEdit::Password);
    oldPwdEdit->setPlaceholderText(QStringLiteral("请输入旧密码"));
    form->addRow(QStringLiteral("旧密码:"), oldPwdEdit);

    auto* newPwdEdit = new QLineEdit(&dlg);
    newPwdEdit->setEchoMode(QLineEdit::Password);
    newPwdEdit->setPlaceholderText(QStringLiteral("至少6位"));
    form->addRow(QStringLiteral("新密码:"), newPwdEdit);

    auto* confirmPwdEdit = new QLineEdit(&dlg);
    confirmPwdEdit->setEchoMode(QLineEdit::Password);
    confirmPwdEdit->setPlaceholderText(QStringLiteral("再次输入新密码"));
    form->addRow(QStringLiteral("确认密码:"), confirmPwdEdit);

    QString inputStyle = "QLineEdit { min-height: 36px; padding: 4px 10px; border: 1px solid #e2e8f0;"
                         " border-radius: 4px; font-size: 13px; background: #ffffff; }"
                         "QLineEdit:focus { border-color: #b45309; }";
    oldPwdEdit->setStyleSheet(inputStyle);
    newPwdEdit->setStyleSheet(inputStyle);
    confirmPwdEdit->setStyleSheet(inputStyle);

    layout->addLayout(form);
    layout->addSpacing(8);

    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    auto* cancelBtn = new QPushButton(QStringLiteral("取消"), &dlg);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    auto* okBtn = new QPushButton(QStringLiteral("确认修改"), &dlg);
    okBtn->setProperty("cssClass", "primary");
    okBtn->setCursor(Qt::PointingHandCursor);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);
    layout->addLayout(btnRow);

    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    QObject::connect(okBtn, &QPushButton::clicked, &dlg, [&]() {
        QString oldPwd = oldPwdEdit->text();
        QString newPwd = newPwdEdit->text();
        QString confirmPwd = confirmPwdEdit->text();

        if (oldPwd.isEmpty()) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请输入旧密码"));
            return;
        }
        if (newPwd.length() < 6) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("新密码至少6位"));
            return;
        }
        if (newPwd != confirmPwd) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("两次输入的新密码不一致"));
            return;
        }

        QSqlQuery verifyQ(DatabaseManager::instance().database());
        verifyQ.prepare("SELECT password FROM sys_user WHERE id = :id AND del_flag = 0");
        verifyQ.bindValue(":id", user.id);
        if (!verifyQ.exec() || !verifyQ.next()) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("用户不存在"));
            return;
        }
        QString storedHash = verifyQ.value(0).toString();
        if (!Utils::verifyPassword(oldPwd, storedHash)) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("旧密码错误"));
            return;
        }

        QString newHash = Utils::hashPassword(newPwd);
        if (DatabaseManager::instance().update("sys_user", user.id, {
            {"password", newHash},
            {"update_by", user.id},
            {"update_time", QDateTime::currentDateTime()}
        })) {
            showToast(QStringLiteral("密码修改成功"), parent);
            dlg.accept();
        } else {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("密码修改失败，请重试"));
        }
    });

    dlg.exec();
}
