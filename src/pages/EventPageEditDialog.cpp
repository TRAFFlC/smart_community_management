#include "pages/EventPage.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "services/EventService.h"
#include "ui_kit/AuthHelpers.h"
#include <QFileDialog>
#include <QFileInfo>

void EventPage::onEditEvent(qint64 eventId)
{
    GeEvent ev = EventService::instance().getEventById(eventId);
    if (ev.id <= 0) return;

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("编辑事件"));
    dlg.setMinimumWidth(500);
    auto* dlgLayout = new QVBoxLayout(&dlg);
    dlgLayout->setContentsMargins(24, 20, 24, 20);

    auto* formTitle = new QLabel(QStringLiteral("修改事件信息"), &dlg);
    formTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
    dlgLayout->addWidget(formTitle);
    dlgLayout->addSpacing(8);

    auto* form = new QFormLayout(&dlg);
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);

    auto* titleEdit = new QLineEdit(ev.title, &dlg);
    form->addRow(QStringLiteral("标题:"), titleEdit);

    auto* catCombo = new QComboBox(&dlg);
    catCombo->addItems({QStringLiteral("民生服务"), QStringLiteral("环境卫生"), QStringLiteral("设施安全"),
                        QStringLiteral("邻里纠纷"), QStringLiteral("特殊帮扶"), QStringLiteral("市容秩序"),
                        QStringLiteral("突发预警")});
    if (ev.eventCategory >= 1 && ev.eventCategory <= 7) catCombo->setCurrentIndex(ev.eventCategory - 1);
    form->addRow(QStringLiteral("类别:"), catCombo);

    auto* prioCombo = new QComboBox(&dlg);
    prioCombo->addItems({QStringLiteral("一般"), QStringLiteral("重要"), QStringLiteral("紧急"), QStringLiteral("特急")});
    if (ev.priority >= 1 && ev.priority <= 4) prioCombo->setCurrentIndex(ev.priority - 1);
    form->addRow(QStringLiteral("优先级:"), prioCombo);

    auto* descEdit = new QTextEdit(&dlg);
    descEdit->setPlainText(ev.description);
    descEdit->setFixedHeight(80);
    form->addRow(QStringLiteral("描述:"), descEdit);

    auto* locEdit = new QLineEdit(ev.location, &dlg);
    form->addRow(QStringLiteral("地点:"), locEdit);

    dlgLayout->addLayout(form);
    dlgLayout->addSpacing(12);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("保存"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, [&dlg, titleEdit, catCombo, prioCombo, descEdit, locEdit, eventId, this]() {
        if (titleEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题"));
            return;
        }
        const auto& editUser = AuthService::instance().currentUser();
        EventService::instance().updateEvent(eventId, {
            {"title", titleEdit->text().trimmed()},
            {"event_category", catCombo->currentIndex() + 1},
            {"priority", prioCombo->currentIndex() + 1},
            {"description", descEdit->toPlainText().trimmed()},
            {"location", locEdit->text().trimmed()},
            {"update_by", editUser.id},
            {"update_time", QDateTime::currentDateTime()}
        });
        UiKit::showToast(QStringLiteral("事件已更新"), this);
        dlg.accept();
        loadData();
    });
    dlgLayout->addWidget(buttons);
    dlg.exec();
}
