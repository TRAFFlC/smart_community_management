#include "WorkOrderDetailDialog.h"

#include "../database/DatabaseManager.h"
#include "../models/Constants.h"
#include "../services/AuthService.h"
#include "../ui_kit/AuthHelpers.h"
#include "../ui_kit/UiKit.h"

#include <QComboBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QTextEdit>
#include <QVBoxLayout>

void WorkOrderDetailActions::setupActionButtons(QDialog& dlg, QHBoxLayout* btnRow,
                                                qint64 workOrderId, int status,
                                                qint64 reporterId, qint64 assignTo,
                                                const QString& title) {
    const auto& user = AuthService::instance().currentUser();
    auto sendWoNotification = [user](int targetUserId, const QString& title, const QString& content, int bizId) {
        if (targetUserId <= 0) return;
        DatabaseManager::instance().insert("nt_notification", QVariantMap{
            {"user_id", targetUserId}, {"title", title}, {"content", content},
            {"notification_type", 2}, {"biz_type", "work_order"}, {"biz_id", bizId},
            {"is_read", 0}, {"create_time", QDateTime::currentDateTime()}
        });
    };

    auto addActionBtn = [&](const QString& text, const QString& color, const std::function<void()>& action) {
        auto* btn = new QPushButton(text, &dlg);
        btn->setStyleSheet(QString("QPushButton{background:%1;color:#fff;border:none;border-radius:4px;padding:8px 18px;font-size:13px;font-weight:500;} QPushButton:hover{background:%2;}")
                           .arg(color).arg(QColor(color).darker(110).name()));
        btn->setCursor(Qt::PointingHandCursor);
        QObject::connect(btn, &QPushButton::clicked, &dlg, action);
        btnRow->addWidget(btn);
    };

    if (UiKit::canOperateWorkOrder(status, reporterId, assignTo)) {
        if (status == 0) {
            addActionBtn(QStringLiteral("受理"), "#b45309", [&dlg, workOrderId, user, sendWoNotification, reporterId, title]() {
                auto ret = QMessageBox::question(&dlg, QStringLiteral("确认操作"), QStringLiteral("确认受理此工单？"),
                                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (ret != QMessageBox::Yes) return;
                DatabaseManager::instance().update("wo_work_order", workOrderId, QVariantMap{
                    {"status", 1}, {"accept_time", QDateTime::currentDateTime()}, {"accept_by", user.id}
                });
                sendWoNotification((int)reporterId, QStringLiteral("工单已受理"),
                                   QStringLiteral("您的报修「%1」已受理").arg(title), (int)workOrderId);
                UiKit::showToast(QStringLiteral("工单已受理"), &dlg);
                dlg.accept();
            });
        }
        else if (status == 1) {
            addActionBtn(QStringLiteral("派单"), "#b45309", [&dlg, workOrderId, user, sendWoNotification, reporterId, title]() {
                QDialog assignDlg(&dlg);
                assignDlg.setWindowTitle(QStringLiteral("派单"));
                assignDlg.setMinimumWidth(400);
                auto* layout = new QVBoxLayout(&assignDlg);
                layout->setContentsMargins(24, 20, 24, 20);
                auto* combo = new QComboBox(&assignDlg);
                QSqlQuery workerQ("SELECT u.id, u.nickname, u.username FROM sys_user u "
                                  "JOIN sys_user_role ur ON u.id = ur.user_id "
                                  "JOIN sys_role r ON ur.role_id = r.id "
                                  "WHERE r.role_key = 'property_repair' AND u.status = 0 AND u.del_flag = 0");
                while (workerQ.next()) {
                    QString name = workerQ.value(1).toString();
                    if (name.isEmpty()) name = workerQ.value(2).toString();
                    combo->addItem(name, workerQ.value(0).toInt());
                }
                if (combo->count() == 0) {
                    QSqlQuery workerQ2("SELECT u.id, u.nickname, u.username FROM sys_user u "
                                       "JOIN sys_user_role ur ON u.id = ur.user_id "
                                       "JOIN sys_role r ON ur.role_id = r.id "
                                       "WHERE r.role_key IN ('property_cs', 'property_steward', 'property_manager') "
                                       "AND u.status = 0 AND u.del_flag = 0");
                    while (workerQ2.next()) {
                        QString name = workerQ2.value(1).toString();
                        if (name.isEmpty()) name = workerQ2.value(2).toString();
                        combo->addItem(name, workerQ2.value(0).toInt());
                    }
                }
                layout->addWidget(combo);
                auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &assignDlg);
                buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认派单"));
                buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
                QObject::connect(buttons, &QDialogButtonBox::rejected, &assignDlg, &QDialog::reject);
                QObject::connect(buttons, &QDialogButtonBox::accepted, &assignDlg, [&]() {
                    if (combo->count() == 0) {
                        QMessageBox::warning(&assignDlg, QStringLiteral("提示"), QStringLiteral("没有可选的维修人员"));
                        return;
                    }
                    int workerId = combo->currentData().toInt();
                    QString workerName = combo->currentText();
                    auto ret = QMessageBox::question(&assignDlg, QStringLiteral("确认操作"),
                                                     QStringLiteral("确认将工单派单给 %1？").arg(workerName),
                                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (ret != QMessageBox::Yes) return;
                    DatabaseManager::instance().update("wo_work_order", workOrderId, QVariantMap{
                        {"status", 2}, {"assign_to", workerId}, {"assign_time", QDateTime::currentDateTime()}
                    });
                    sendWoNotification(workerId, QStringLiteral("新工单已派单"),
                                       QStringLiteral("您有一个新的维修工单待处理"), (int)workOrderId);
                    sendWoNotification((int)reporterId, QStringLiteral("工单已派单"),
                                       QStringLiteral("您的报修「%1」已派单给 %2").arg(title).arg(workerName), (int)workOrderId);
                    UiKit::showToast(QStringLiteral("派单成功"), &assignDlg);
                    assignDlg.accept();
                    dlg.accept();
                });
                layout->addWidget(buttons);
                assignDlg.exec();
            });
        }
        else if (status == 2) {
            addActionBtn(QStringLiteral("开始处理"), "#a16207", [&dlg, workOrderId]() {
                auto ret = QMessageBox::question(&dlg, QStringLiteral("确认操作"), QStringLiteral("确认开始处理此工单？"),
                                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (ret != QMessageBox::Yes) return;
                DatabaseManager::instance().update("wo_work_order", workOrderId, QVariantMap{{"status", 3}});
                UiKit::showToast(QStringLiteral("已开始处理"), &dlg);
                dlg.accept();
            });
        }
        else if (status == 3) {
            addActionBtn(QStringLiteral("完成"), "#15803d", [&dlg, workOrderId, user, sendWoNotification, reporterId, title]() {
                QDialog finishDlg(&dlg);
                finishDlg.setWindowTitle(QStringLiteral("填写处理结果"));
                finishDlg.setMinimumWidth(450);
                auto* layout = new QVBoxLayout(&finishDlg);
                layout->setContentsMargins(24, 20, 24, 20);
                auto* form = new QFormLayout(&finishDlg);
                form->setSpacing(12);
                auto* resultEdit = new QTextEdit(&finishDlg);
                resultEdit->setPlaceholderText(QStringLiteral("请描述维修处理结果..."));
                resultEdit->setFixedHeight(80);
                form->addRow(QStringLiteral("处理结果:"), resultEdit);
                layout->addLayout(form);
                auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &finishDlg);
                buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认完成"));
                buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
                QObject::connect(buttons, &QDialogButtonBox::rejected, &finishDlg, &QDialog::reject);
                QObject::connect(buttons, &QDialogButtonBox::accepted, &finishDlg, [&]() {
                    if (resultEdit->toPlainText().trimmed().isEmpty()) {
                        QMessageBox::warning(&finishDlg, QStringLiteral("提示"), QStringLiteral("请填写处理结果"));
                        return;
                    }
                    auto ret = QMessageBox::question(&finishDlg, QStringLiteral("确认操作"), QStringLiteral("确认完成此工单？"),
                                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (ret != QMessageBox::Yes) return;
                    DatabaseManager::instance().update("wo_work_order", workOrderId, QVariantMap{
                        {"status", 4}, {"finish_time", QDateTime::currentDateTime()},
                        {"result_desc", resultEdit->toPlainText().trimmed()}
                    });
                    sendWoNotification((int)reporterId, QStringLiteral("工单已完成"),
                                       QStringLiteral("您的报修「%1」已处理完成，请评价").arg(title), (int)workOrderId);
                    UiKit::showToast(QStringLiteral("工单已完成"), &finishDlg);
                    finishDlg.accept();
                    dlg.accept();
                });
                layout->addWidget(buttons);
                finishDlg.exec();
            });
        }
        else if (status == 4) {
            addActionBtn(QStringLiteral("评价"), "#15803d", [&dlg, workOrderId, user]() {
                QDialog evalDlg(&dlg);
                evalDlg.setWindowTitle(QStringLiteral("评价工单"));
                evalDlg.setMinimumWidth(450);
                auto* layout = new QVBoxLayout(&evalDlg);
                layout->setContentsMargins(24, 20, 24, 20);
                auto* form = new QFormLayout(&evalDlg);
                form->setSpacing(12);
                auto* ratingCombo = new QComboBox(&evalDlg);
                ratingCombo->addItems({QStringLiteral("1星 - 很差"), QStringLiteral("2星 - 一般"),
                                       QStringLiteral("3星 - 还行"), QStringLiteral("4星 - 满意"),
                                       QStringLiteral("5星 - 非常满意")});
                ratingCombo->setCurrentIndex(4);
                form->addRow(QStringLiteral("评分:"), ratingCombo);
                auto* contentEdit = new QTextEdit(&evalDlg);
                contentEdit->setPlaceholderText(QStringLiteral("请输入评价内容..."));
                contentEdit->setFixedHeight(80);
                form->addRow(QStringLiteral("评价:"), contentEdit);
                layout->addLayout(form);
                auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &evalDlg);
                buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交评价"));
                buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
                QObject::connect(buttons, &QDialogButtonBox::rejected, &evalDlg, &QDialog::reject);
                QObject::connect(buttons, &QDialogButtonBox::accepted, &evalDlg, [&]() {
                    int rating = ratingCombo->currentIndex() + 1;
                    auto ret = QMessageBox::question(&evalDlg, QStringLiteral("确认操作"), QStringLiteral("确认提交评价？"),
                                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (ret != QMessageBox::Yes) return;
                    DatabaseManager::instance().insert("ev_evaluation", QVariantMap{
                        {"biz_type", "work_order"}, {"biz_id", (int)workOrderId},
                        {"evaluator_id", user.id}, {"rating", rating},
                        {"content", contentEdit->toPlainText().trimmed()},
                        {"eval_time", QDateTime::currentDateTime()},
                        {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                    });
                    DatabaseManager::instance().update("wo_work_order", workOrderId, QVariantMap{{"status", 6}});
                    UiKit::showToast(QStringLiteral("评价提交成功"), &evalDlg);
                    evalDlg.accept();
                    dlg.accept();
                });
                layout->addWidget(buttons);
                evalDlg.exec();
            });
        }
    }
}
