#include "pages/SystemPageHelpers.h"
#include "PagesCommon.h"

using namespace SystemPageHelpers;

QVBoxLayout *SystemPageHelpers::createFormDialog(QWidget *parent, const QString &title,
                                                 QFormLayout *&outForm, QDialog *&outDlg)
{
    outDlg = new QDialog(parent);
    outDlg->setWindowTitle(title);
    outDlg->setFixedWidth(480);
    auto *dlgLayout = new QVBoxLayout(outDlg);
    dlgLayout->setContentsMargins(24, 20, 24, 20);
    dlgLayout->setSpacing(12);

    auto *titleLabel = new QLabel(title, outDlg);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
    dlgLayout->addWidget(titleLabel);
    dlgLayout->addSpacing(4);

    outForm = new QFormLayout(outDlg);
    outForm->setSpacing(12);
    outForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    dlgLayout->addLayout(outForm);
    return dlgLayout;
}

void SystemPageHelpers::styleInput(QWidget *w)
{
    w->setStyleSheet("QLineEdit, QComboBox, QSpinBox, QTextEdit {"
                     "  min-height: 36px; padding: 4px 8px; border: 1px solid #D4D0C8;"
                     "  border-radius: 4px; font-size: 13px; background: #ffffff;"
                     "}"
                     "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QTextEdit:focus {"
                     "  border-color: #b45309;"
                     "}");
}

void SystemPageHelpers::addDialogButtons(QVBoxLayout *dlgLayout, QDialog *dlg, const QString &okText,
                                         std::function<bool()> onOk)
{
    dlgLayout->addSpacing(8);
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    auto *cancelBtn = new QPushButton(QStringLiteral("取消"), dlg);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    auto *okBtn = new QPushButton(okText, dlg);
    okBtn->setProperty("cssClass", "primary");
    okBtn->setCursor(Qt::PointingHandCursor);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);
    dlgLayout->addLayout(btnRow);
    QObject::connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    QObject::connect(okBtn, &QPushButton::clicked, dlg, [dlg, onOk]() {
        if (onOk()) dlg->accept();
    });
}

QString SystemPageHelpers::primaryBtnStyle()
{
    return QStringLiteral(
        "QPushButton { background: #b45309; color: #fff; border: none; border-radius: 4px;"
        " padding: 7px 16px; font-size: 14px; min-height: 36px; }"
        "QPushButton:hover { background: #d97706; }"
        "QPushButton:pressed { background: #92400e; }");
}
