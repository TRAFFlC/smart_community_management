#include "pages/EventPage.h"

#include "PagesCommon.h"
#include "pages/PageFactory.h"
#include "services/AuthService.h"
#include "services/EventService.h"
#include "ui_kit/AuthHelpers.h"

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
    UiKit::configureTable(m_table);
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
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
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
    connect(newBtn, &QPushButton::clicked, this, &EventPage::onNewEvent);

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
    m_table->setSortingEnabled(false);
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
    m_table->setSortingEnabled(true);
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

// 工厂方法实现
BasePage* PageFactory::createEventPage() { return new EventPage(); }
