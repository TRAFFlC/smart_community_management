#include "pages/EventPage.h"

#include "PagesCommon.h"
#include "pages/PageFactory.h"
#include "services/EventService.h"
#include "ui_kit/AuthHelpers.h"

#include <QFileInfo>

EventPage::EventPage(QWidget* parent)
    : BasePage(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(20, 20, 20, 20);

    // Page header
    m_layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_route"), QStringLiteral("网格事件管理"),
                                                 QStringLiteral("管理社区事件上报、审核分派、处理反馈、督办归档全流程"),
                                                 UiKit::moduleColor("event"), this));
    m_layout->addSpacing(12);

    // Toolbar
    buildToolbar();

    // Pagination (placed above table, same as original layout order)
    m_pagination = new PaginationBar(this);
    m_layout->addWidget(m_pagination);

    // Table
    m_table = new QTableWidget(this);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setStyleSheet(UiKit::TABLE_STYLE);
    m_table->setShowGrid(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSortingEnabled(true);
    buildTable();
    m_layout->addWidget(m_table);

    // Empty hint
    m_emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无事件记录"), this);
    m_layout->addWidget(m_emptyHint);

    // Connections
    connect(m_searchEdit, &QLineEdit::textChanged, this, &EventPage::loadData);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EventPage::loadData);
    connect(m_onlyMineCheck, &QCheckBox::toggled, this, &EventPage::loadData);
    connect(m_pagination, &PaginationBar::pageChanged, this, &EventPage::loadData);

    connect(m_table, &QTableWidget::cellClicked, this, [this](int row, int col) {
        if (col != 7) return;
        auto* item = m_table->item(row, col);
        if (!item) return;
        qint64 id = item->data(Qt::UserRole).toLongLong();
        int sts = item->data(Qt::UserRole + 2).toInt();
        onActionEvent(id, sts);
    });
    connect(m_table, &QTableWidget::cellEntered, this, [this](int, int c) {
        m_table->viewport()->setCursor(c == 7 ? Qt::PointingHandCursor : Qt::ArrowCursor);
    });

    loadData();
}

void EventPage::buildToolbar()
{
    auto* toolbar = new QWidget(this);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto* tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);

    m_searchEdit = new QLineEdit(toolbar);
    m_searchEdit->setPlaceholderText(QStringLiteral("搜索事件号/标题..."));
    m_searchEdit->setFixedWidth(220);
    tbLayout->addWidget(m_searchEdit);

    m_filterCombo = new QComboBox(toolbar);
    m_filterCombo->addItems({QStringLiteral("全部状态"), QStringLiteral("待审核"), QStringLiteral("已审核"),
                             QStringLiteral("已分派"), QStringLiteral("处理中"), QStringLiteral("已完成"),
                             QStringLiteral("已归档")});
    m_filterCombo->setFixedWidth(130);
    tbLayout->addWidget(m_filterCombo);

    auto* exportBtn = new QPushButton(QStringLiteral("导出"), toolbar);
    exportBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(exportBtn);
    connect(exportBtn, &QPushButton::clicked, this, [this]() {
        UiKit::exportTableToCsv(m_table, QStringLiteral("事件列表.csv"), this);
    });

    m_onlyMineCheck = new QCheckBox(QStringLiteral("只看我的"), toolbar);
    tbLayout->addWidget(m_onlyMineCheck);

    tbLayout->addStretch();

    auto* newBtn = new QPushButton(QStringLiteral("+ 上报事件"), toolbar);
    newBtn->setProperty("cssClass", "primary");
    newBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(newBtn);
    connect(newBtn, &QPushButton::clicked, this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle(QStringLiteral("上报事件"));
        dlg.setMinimumWidth(500);
        auto* dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto* formTitle = new QLabel(QStringLiteral("填写事件信息"), &dlg);
        formTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
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
        QStringList selectedImages;
        auto* imgLabel = new QLabel(QStringLiteral("附件图片（可选，最多5张）"), &dlg);
        imgLabel->setStyleSheet("font-size: 13px; color: #64748b; background: transparent;");
        dlgLayout->addWidget(imgLabel);
        auto* imgListWidget = new QWidget(&dlg);
        auto* imgListLayout = new QVBoxLayout(imgListWidget);
        imgListLayout->setContentsMargins(0, 0, 0, 0);
        imgListLayout->setSpacing(4);
        dlgLayout->addWidget(imgListWidget);

        std::function<void()> refreshImageList;
        refreshImageList = [&]() {
            QLayoutItem* child;
            while ((child = imgListLayout->takeAt(0)) != nullptr) {
                if (child->widget()) child->widget()->deleteLater();
                delete child;
            }
            for (int i = 0; i < selectedImages.size(); ++i) {
                auto* row = new QWidget(imgListWidget);
                auto* rowLay = new QHBoxLayout(row);
                rowLay->setContentsMargins(0, 0, 0, 0);
                auto* nameLabel = new QLabel(QFileInfo(selectedImages[i]).fileName(), row);
                nameLabel->setStyleSheet("font-size: 12px; color: #334155; background: transparent;");
                rowLay->addWidget(nameLabel);
                rowLay->addStretch();
                auto* rmBtn = new QPushButton(QStringLiteral("×"), row);
                rmBtn->setFixedSize(20, 20);
                rmBtn->setStyleSheet("QPushButton{border:none; color:#b91c1c; font-size:14px; font-weight:bold; background:transparent;} QPushButton:hover{background:#fee2e2; border-radius:3px;}");
                rmBtn->setCursor(Qt::PointingHandCursor);
                int idx = i;
                connect(rmBtn, &QPushButton::clicked, [&selectedImages, idx, &refreshImageList]() {
                    selectedImages.removeAt(idx);
                    refreshImageList();
                });
                rowLay->addWidget(rmBtn);
                imgListLayout->addWidget(row);
            }
        };

        auto* addImgBtn = new QPushButton(QStringLiteral("+ 添加图片"), &dlg);
        addImgBtn->setCursor(Qt::PointingHandCursor);
        addImgBtn->setStyleSheet("QPushButton{border:1px dashed #cbd5e1; border-radius:4px; padding:6px 16px; color:#64748b; background:#f8fafc; font-size:13px;} QPushButton:hover{border-color:#b45309; color:#b45309;}");
        connect(addImgBtn, &QPushButton::clicked, [&dlg, &selectedImages, &refreshImageList]() {
            if (selectedImages.size() >= 5) {
                QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("最多上传5张图片"));
                return;
            }
            QStringList files = QFileDialog::getOpenFileNames(&dlg, QStringLiteral("选择图片"), QString(),
                QStringLiteral("图片文件 (*.jpg *.jpeg *.png *.bmp *.gif);;所有文件 (*)"));
            for (const QString& f : files) {
                if (selectedImages.size() >= 5) break;
                if (!selectedImages.contains(f)) selectedImages.append(f);
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
                                                        &selectedImages, this]() {
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
                selectedImages.join(","),
                user.id,
                reporterName);
            UiKit::showToast(QStringLiteral("事件上报成功"), this);
            dlg.accept();
            loadData();
        });
        dlgLayout->addWidget(buttons);
        dlg.exec();
    });

    m_layout->addWidget(toolbar);
}

void EventPage::buildTable()
{
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({QStringLiteral("事件号"), QStringLiteral("标题"), QStringLiteral("类别"),
                                        QStringLiteral("优先级"), QStringLiteral("状态"), QStringLiteral("上报人"),
                                        QStringLiteral("创建时间"), QStringLiteral("操作")});
    m_table->setColumnWidth(1, 180);
    m_table->setColumnWidth(7, 100);
    m_table->horizontalHeader()->setStretchLastSection(true);
}

void EventPage::loadData()
{
    m_table->setRowCount(0);

    EventService::EventQueryParams params;
    params.keyword = m_searchEdit->text().trimmed();
    params.statusFilter = m_filterCombo->currentIndex();
    params.onlyMine = m_onlyMineCheck->isChecked();
    params.pageSize = m_pagination->pageSize();
    params.offset = m_pagination->offset();

    auto result = EventService::instance().queryEvents(params);

    m_pagination->setTotalCount(result.totalCount);

    int row = 0;
    for (const auto& ev : result.items) {
        m_table->insertRow(row);
        updateRow(row, ev);
        ++row;
    }

    UiKit::syncEmptyHint(m_table, m_emptyHint);
    m_pagination->refreshData();
}

void EventPage::updateRow(int row, const GeEvent& ev)
{
    m_table->setItem(row, 0, new QTableWidgetItem(ev.eventNo));
    m_table->setItem(row, 1, new QTableWidgetItem(ev.title));
    m_table->setItem(row, 2, new QTableWidgetItem(EventCategory::label(ev.eventCategory)));

    QColor priColor(EventPriority::color(ev.priority));
    auto* priItem = UiKit::createTagTableItem(EventPriority::label(ev.priority),
                                               QColor(priColor.red(), priColor.green(), priColor.blue(), 30),
                                               priColor);
    m_table->setItem(row, 3, priItem);

    QColor evColor(EventStatus::color(ev.status));
    auto* statusItem = UiKit::createTagTableItem(EventStatus::label(ev.status),
                                                  QColor(evColor.red(), evColor.green(), evColor.blue(), 30),
                                                  evColor);
    m_table->setItem(row, 4, statusItem);

    m_table->setItem(row, 5, new QTableWidgetItem(ev.reporterName));
    m_table->setItem(row, 6, new QTableWidgetItem(ev.createTime.toString("yyyy-MM-dd hh:mm")));

    const auto& curUser = AuthService::instance().currentUser();
    bool canEdit = (ev.status <= 1) && (ev.reporterId == curUser.id);

    QList<QPushButton*> actionBtns;
    if (canEdit) {
        auto* editBtn = new QPushButton(QStringLiteral("编辑"));
        editBtn->setProperty("cssClass", "text");
        editBtn->setCursor(Qt::PointingHandCursor);
        connect(editBtn, &QPushButton::clicked, this, [this, ev]() {
            onEditEvent(ev.id);
        });
        actionBtns.append(editBtn);
    }

    if (UiKit::canOperateEvent(ev.status, ev.reporterId, ev.assignTo)) {
        QString actionText;
        QString actionColor;
        if (ev.status == 0) { actionText = QStringLiteral("审核"); actionColor = "#b45309"; }
        else if (ev.status == 1) { actionText = QStringLiteral("分派"); actionColor = "#b45309"; }
        else if (ev.status == 2) { actionText = QStringLiteral("处理"); actionColor = "#a16207"; }
        else if (ev.status == 3) { actionText = QStringLiteral("完成"); actionColor = "#15803d"; }
        else { actionText = QStringLiteral("--"); actionColor = "#64748b"; }

        if (actionText != QStringLiteral("--")) {
            auto* mainBtn = new QPushButton(actionText);
            mainBtn->setStyleSheet(QString("QPushButton{border:none; color:%1; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;} QPushButton:hover{text-decoration:underline;}")
                                   .arg(actionColor));
            mainBtn->setCursor(Qt::PointingHandCursor);
            connect(mainBtn, &QPushButton::clicked, this, [this, ev]() {
                onActionEvent(ev.id, ev.status);
            });
            actionBtns.append(mainBtn);
        }
    }

    if (actionBtns.isEmpty()) {
        m_table->setItem(row, 7, UiKit::createActionItem(QStringLiteral("--"), "#64748b", ev.id, ev.status));
    } else {
        m_table->setCellWidget(row, 7, UiKit::createActionCell(actionBtns, m_table));
    }
}

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
    formTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
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
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
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
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
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

// 工厂方法实现
BasePage* PageFactory::createEventPage() { return new EventPage(); }
