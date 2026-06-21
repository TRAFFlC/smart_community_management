#include "pages/EventPage.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "services/EventService.h"
#include "ui_kit/AuthHelpers.h"
#include <QFileDialog>
#include <QFileInfo>

void EventPage::onActionEvent(qint64 eventId, int status)
{
    GeEvent ev = EventService::instance().getEventById(eventId);
    if (ev.id <= 0) return;

    if (!UiKit::canOperateEvent(status, ev.reporterId, ev.assignTo)) {
        QMessageBox::warning(this, QStringLiteral("无权限"),
                             QStringLiteral("您没有权限执行此操作，请联系相关负责人。"));
        return;
    }

    const auto& user = AuthService::instance().currentUser();

    if (status == 0) {
        auto ret = QMessageBox::question(this, QStringLiteral("确认操作"),
                                         QStringLiteral("确认审核通过此事件？"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
        EventService::instance().reviewEvent(eventId, user.id);
        UiKit::showToast(QStringLiteral("事件已审核"), this);
        loadData();
    }
    else if (status == 1) {
        QDialog dlg(this);
        dlg.setWindowTitle(QStringLiteral("分派事件"));
        dlg.setMinimumWidth(450);
        auto* dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto* titleLabel = new QLabel(QStringLiteral("选择处理人员"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(8);

        auto* form = new QFormLayout(&dlg);
        form->setSpacing(12);
        auto* workerCombo = new QComboBox(&dlg);
        auto workers = EventService::instance().listGridWorkers();
        for (const auto& w : workers) {
            workerCombo->addItem(w.second, (int)w.first);
        }
        form->addRow(QStringLiteral("处理人:"), workerCombo);

        auto* assignRemarkEdit = new QTextEdit(&dlg);
        assignRemarkEdit->setPlaceholderText(QStringLiteral("分派备注(可选)"));
        assignRemarkEdit->setFixedHeight(70);
        form->addRow(QStringLiteral("备注:"), assignRemarkEdit);
        dlgLayout->addLayout(form);
        dlgLayout->addSpacing(12);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认分派"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, [&dlg, workerCombo, assignRemarkEdit, eventId, user, this]() {
            if (workerCombo->count() == 0) {
                QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("没有可选的处理人员"));
                return;
            }
            int workerId = workerCombo->currentData().toInt();
            QString workerName = workerCombo->currentText();
            auto ret = QMessageBox::question(this, QStringLiteral("确认操作"),
                                             QStringLiteral("确认将事件分派给 %1？").arg(workerName),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (ret != QMessageBox::Yes) return;
            EventService::instance().assignEvent(eventId, workerId, user.id,
                                                  assignRemarkEdit->toPlainText().trimmed());
            requestNotification(workerId, QStringLiteral("新事件已分派"),
                                QStringLiteral("您有一个新的社区事件待处理"), 2, "event", (int)eventId);
            UiKit::showToast(QStringLiteral("事件分派成功"), this);
            dlg.accept();
            loadData();
        });
        dlgLayout->addWidget(buttons);
        dlg.exec();
    }
    else if (status == 2) {
        auto ret = QMessageBox::question(this, QStringLiteral("确认操作"),
                                         QStringLiteral("确认开始处理此事件？"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
        EventService::instance().processEvent(eventId, user.id);
        UiKit::showToast(QStringLiteral("事件已开始处理"), this);
        loadData();
    }
    else if (status == 3) {
        QDialog dlg(this);
        dlg.setWindowTitle(QStringLiteral("填写处理结果"));
        dlg.setMinimumWidth(450);
        auto* dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto* titleLabel = new QLabel(QStringLiteral("填写事件处理结果"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(8);

        auto* form = new QFormLayout(&dlg);
        form->setSpacing(12);
        auto* resultEdit = new QTextEdit(&dlg);
        resultEdit->setPlaceholderText(QStringLiteral("请描述处理过程和结果..."));
        resultEdit->setFixedHeight(100);
        form->addRow(QStringLiteral("处理结果:"), resultEdit);
        dlgLayout->addLayout(form);
        dlgLayout->addSpacing(12);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认完成"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, [&dlg, resultEdit, eventId, user, this]() {
            if (resultEdit->toPlainText().trimmed().isEmpty()) {
                QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写处理结果"));
                return;
            }
            auto ret = QMessageBox::question(this, QStringLiteral("确认操作"),
                                             QStringLiteral("确认完成此事件？"),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (ret != QMessageBox::Yes) return;
            auto cr = EventService::instance().completeEvent(eventId, resultEdit->toPlainText().trimmed(), user.id);
            if (cr.success && cr.reporterId > 0) {
                requestNotification((int)cr.reporterId,
                                    QStringLiteral("事件已处理: %1").arg(cr.title),
                                    QStringLiteral("您上报的事件已处理完成"), 1, "event", (int)eventId);
            }
            UiKit::showToast(QStringLiteral("事件已完成"), this);
            dlg.accept();
            loadData();
        });
        dlgLayout->addWidget(buttons);
        dlg.exec();
    }
}
