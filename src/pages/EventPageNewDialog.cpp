#include "pages/EventPage.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "services/EventService.h"
#include "ui_kit/AuthHelpers.h"
#include <QFileDialog>
#include <QFileInfo>

void EventPage::onNewEvent()
{
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("上报事件"));
    dlg.setMinimumWidth(500);
    auto* dlgLayout = new QVBoxLayout(&dlg);
    dlgLayout->setContentsMargins(24, 20, 24, 20);

    auto* formTitle = new QLabel(QStringLiteral("填写事件信息"), &dlg);
    formTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
    dlgLayout->addWidget(formTitle);
    dlgLayout->addSpacing(8);

    auto* form = new QFormLayout(&dlg);
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);

    auto* titleEdit = new QLineEdit(&dlg);
    titleEdit->setPlaceholderText(QStringLiteral("请简要描述事件"));
    form->addRow(QStringLiteral("标题:"), titleEdit);

    auto* catCombo = new QComboBox(&dlg);
    catCombo->addItems({QStringLiteral("民生服务"), QStringLiteral("环境卫生"), QStringLiteral("设施安全"),
                        QStringLiteral("邻里纠纷"), QStringLiteral("特殊帮扶"), QStringLiteral("市容秩序"),
                        QStringLiteral("突发预警")});
    form->addRow(QStringLiteral("类别:"), catCombo);

    auto* prioCombo = new QComboBox(&dlg);
    prioCombo->addItems({QStringLiteral("一般"), QStringLiteral("重要"), QStringLiteral("紧急"), QStringLiteral("特急")});
    form->addRow(QStringLiteral("优先级:"), prioCombo);

    auto* descEdit = new QTextEdit(&dlg);
    descEdit->setPlaceholderText(QStringLiteral("详细描述事件情况..."));
    descEdit->setFixedHeight(80);
    form->addRow(QStringLiteral("描述:"), descEdit);

    auto* locEdit = new QLineEdit(&dlg);
    locEdit->setPlaceholderText(QStringLiteral("事件发生地点"));
    form->addRow(QStringLiteral("地点:"), locEdit);
    dlgLayout->addLayout(form);
    dlgLayout->addSpacing(8);

    // 图片上传区域
    auto selectedImages = std::make_shared<QStringList>();
    auto* imgLabel = new QLabel(QStringLiteral("附件图片（可选，最多5张）"), &dlg);
    imgLabel->setStyleSheet("font-size: 13px; color: #64748b; background: transparent;");
    dlgLayout->addWidget(imgLabel);
    auto* imgListWidget = new QWidget(&dlg);
    auto* imgListLayout = new QVBoxLayout(imgListWidget);
    imgListLayout->setContentsMargins(0, 0, 0, 0);
    imgListLayout->setSpacing(4);
    dlgLayout->addWidget(imgListWidget);

    std::function<void()> refreshImageList;
    refreshImageList = [selectedImages, imgListWidget, imgListLayout, &refreshImageList]() {
        QLayoutItem* child;
        while ((child = imgListLayout->takeAt(0)) != nullptr) {
            if (child->widget()) child->widget()->deleteLater();
            delete child;
        }
        for (int i = 0; i < selectedImages->size(); ++i) {
            auto* row = new QWidget(imgListWidget);
            auto* rowLay = new QHBoxLayout(row);
            rowLay->setContentsMargins(0, 0, 0, 0);
            auto* nameLabel = new QLabel(QFileInfo(selectedImages->at(i)).fileName(), row);
            nameLabel->setStyleSheet("font-size: 12px; color: #141413; background: transparent;");
            rowLay->addWidget(nameLabel);
            rowLay->addStretch();
            auto* rmBtn = new QPushButton(QStringLiteral("×"), row);
            rmBtn->setFixedSize(20, 20);
            rmBtn->setStyleSheet("QPushButton{border:none; color:#b91c1c; font-size:14px; font-weight:bold; background:transparent;} QPushButton:hover{background:#fee2e2; border-radius:3px;}");
            rmBtn->setCursor(Qt::PointingHandCursor);
            int idx = i;
            connect(rmBtn, &QPushButton::clicked, [selectedImages, idx, refreshImageList]() {
                selectedImages->removeAt(idx);
                refreshImageList();
            });
            rowLay->addWidget(rmBtn);
            imgListLayout->addWidget(row);
        }
    };

    auto* addImgBtn = new QPushButton(QStringLiteral("+ 添加图片"), &dlg);
    addImgBtn->setCursor(Qt::PointingHandCursor);
    addImgBtn->setStyleSheet("QPushButton{border:1px dashed #cbd5e1; border-radius:4px; padding:6px 16px; color:#64748b; background:#f8fafc; font-size:13px;} QPushButton:hover{border-color:#b45309; color:#b45309;}");
    connect(addImgBtn, &QPushButton::clicked, [&dlg, selectedImages, refreshImageList]() {
        if (selectedImages->size() >= 5) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("最多上传5张图片"));
            return;
        }
        QStringList files = QFileDialog::getOpenFileNames(&dlg, QStringLiteral("选择图片"), QString(),
            QStringLiteral("图片文件 (*.jpg *.jpeg *.png *.bmp *.gif);;所有文件 (*)"));
        for (const QString& f : files) {
            if (selectedImages->size() >= 5) break;
            if (!selectedImages->contains(f)) selectedImages->append(f);
        }
        refreshImageList();
    });
    dlgLayout->addWidget(addImgBtn);

    dlgLayout->addSpacing(12);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, [&dlg, titleEdit, catCombo, prioCombo, descEdit, locEdit,
                                                    selectedImages, this]() {
        if (titleEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题"));
            return;
        }
        const auto& user = AuthService::instance().currentUser();
        QString reporterName = user.nickname.isEmpty() ? user.username : user.nickname;
        EventService::instance().reportEvent(
            titleEdit->text().trimmed(),
            catCombo->currentIndex() + 1,
            prioCombo->currentIndex() + 1,
            descEdit->toPlainText().trimmed(),
            locEdit->text().trimmed(),
            selectedImages->join(","),
            user.id,
            reporterName);
        UiKit::showToast(QStringLiteral("事件上报成功"), this);
        dlg.accept();
        loadData();
    });
    dlgLayout->addWidget(buttons);
    dlg.exec();
}
