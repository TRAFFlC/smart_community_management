#include "pages/PageFactory.h"
#include "PagesCommon.h"

namespace PageFactory {

void buildSystemLog(BasePage *page, QVBoxLayout *layout, DatabaseManager &db, QLabel *emptyHint)
{
    (void)db; // 日志查询使用 DatabaseManager::instance()

    auto *table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    UiKit::configureTable(table);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(UiKit::TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索用户/模块/操作..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *moduleCombo = new QComboBox(toolbar);
    moduleCombo->addItem(QStringLiteral("全部模块"), "");
    moduleCombo->addItem(QStringLiteral("用户管理"), QStringLiteral("用户管理"));
    moduleCombo->addItem(QStringLiteral("工单管理"), QStringLiteral("工单管理"));
    moduleCombo->addItem(QStringLiteral("事件管理"), QStringLiteral("事件管理"));
    moduleCombo->addItem(QStringLiteral("报事报修"), QStringLiteral("报事报修"));
    moduleCombo->addItem(QStringLiteral("公告管理"), QStringLiteral("公告管理"));
    moduleCombo->addItem(QStringLiteral("督办管理"), QStringLiteral("督办管理"));
    moduleCombo->addItem(QStringLiteral("系统设置"), QStringLiteral("系统设置"));
    moduleCombo->addItem(QStringLiteral("角色管理"), QStringLiteral("角色管理"));
    moduleCombo->addItem(QStringLiteral("菜单管理"), QStringLiteral("菜单管理"));
    moduleCombo->addItem(QStringLiteral("字典管理"), QStringLiteral("字典管理"));
    moduleCombo->setMinimumWidth(120);
    tbLayout->addWidget(moduleCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("用户"), QStringLiteral("模块"),
                                      QStringLiteral("操作"), QStringLiteral("IP"), QStringLiteral("时间")});

    // 分页状态（使用 shared_ptr 以便 lambda 共享）
    struct PageState
    {
        int currentPage;
        int totalPages;
        int totalCount;
        int pageSize;
    };
    auto pageState = std::make_shared<PageState>();
    pageState->currentPage = 1;
    pageState->totalPages = 1;
    pageState->totalCount = 0;
    pageState->pageSize = 20;

    // 分页控件
    auto *pageBar = new QWidget(page);
    pageBar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
    auto *pbLayout = new QHBoxLayout(pageBar);
    pbLayout->setContentsMargins(8, 4, 8, 4);
    pbLayout->setSpacing(8);
    auto *prevBtn = new QPushButton(QStringLiteral("上一页"), pageBar);
    prevBtn->setStyleSheet("QPushButton { background: #ffffff; color: #64748b; border: 1px solid #D4D0C8; border-radius: 4px; padding: 5px 14px; min-height: 32px; }"
                           "QPushButton:hover { border-color: #d97706; color: #d97706; }"
                           "QPushButton:disabled { color: #94a3b8; border-color: #D4D0C8; }");
    auto *nextBtn = new QPushButton(QStringLiteral("下一页"), pageBar);
    nextBtn->setStyleSheet(prevBtn->styleSheet());
    auto *pageLabel = new QLabel(QStringLiteral("第 1/1 页"), pageBar);
    pageLabel->setStyleSheet("color: #64748b; font-size: 13px; padding: 0 8px;");
    auto *pageSizeLabel = new QLabel(QStringLiteral("每页"), pageBar);
    pageSizeLabel->setStyleSheet("color: #64748b; font-size: 13px;");
    auto *pageSizeCombo = new QComboBox(pageBar);
    pageSizeCombo->addItem(QStringLiteral("20"), 20);
    pageSizeCombo->addItem(QStringLiteral("50"), 50);
    pageSizeCombo->addItem(QStringLiteral("100"), 100);
    pageSizeCombo->setFixedWidth(70);
    pageSizeCombo->setStyleSheet("QComboBox { min-height: 32px; padding: 2px 6px; border: 1px solid #D4D0C8; border-radius: 4px; }");
    auto *jumpLabel = new QLabel(QStringLiteral("跳转到"), pageBar);
    jumpLabel->setStyleSheet("color: #64748b; font-size: 13px;");
    auto *jumpEdit = new QLineEdit(pageBar);
    jumpEdit->setFixedWidth(60);
    jumpEdit->setStyleSheet("QLineEdit { min-height: 32px; padding: 2px 6px; border: 1px solid #D4D0C8; border-radius: 4px; }");
    auto *jumpBtn = new QPushButton(QStringLiteral("Go"), pageBar);
    jumpBtn->setStyleSheet("QPushButton { background: #b45309; color: #fff; border: none; border-radius: 4px; padding: 5px 12px; min-height: 32px; }"
                           "QPushButton:hover { background: #d97706; }");
    auto *totalLabel = new QLabel(QStringLiteral("共 0 条"), pageBar);
    totalLabel->setStyleSheet("color: #64748b; font-size: 13px;");

    pbLayout->addWidget(totalLabel);
    pbLayout->addStretch();
    pbLayout->addWidget(prevBtn);
    pbLayout->addWidget(pageLabel);
    pbLayout->addWidget(nextBtn);
    pbLayout->addSpacing(12);
    pbLayout->addWidget(pageSizeLabel);
    pbLayout->addWidget(pageSizeCombo);
    pbLayout->addSpacing(8);
    pbLayout->addWidget(jumpLabel);
    pbLayout->addWidget(jumpEdit);
    pbLayout->addWidget(jumpBtn);

    std::function<void()> loadLogs;
    loadLogs = [table, searchEdit, moduleCombo, emptyHint, pageState, pageLabel, totalLabel, prevBtn, nextBtn]()
    {
        table->setSortingEnabled(false);
        table->setRowCount(0);
        // 构建 WHERE 条件
        QString where = "1=1";
        QString searchText = searchEdit->text().trimmed();
        QString moduleFilter = moduleCombo->currentData().toString();
        if (!searchText.isEmpty())
            where += " AND (username LIKE :search OR module LIKE :search OR operation LIKE :search)";
        if (!moduleFilter.isEmpty())
            where += " AND module = :module";

        // 查询总数
        QSqlQuery cntQ(DatabaseManager::instance().database());
        cntQ.prepare("SELECT COUNT(*) FROM sys_operation_log WHERE " + where);
        if (!searchText.isEmpty())
            cntQ.bindValue(":search", "%" + searchText + "%");
        if (!moduleFilter.isEmpty())
            cntQ.bindValue(":module", moduleFilter);
        cntQ.exec();
        int total = 0;
        if (cntQ.next())
            total = cntQ.value(0).toInt();
        pageState->totalCount = total;
        pageState->totalPages = (total + pageState->pageSize - 1) / pageState->pageSize;
        if (pageState->totalPages < 1)
            pageState->totalPages = 1;
        if (pageState->currentPage > pageState->totalPages)
            pageState->currentPage = pageState->totalPages;
        if (pageState->currentPage < 1)
            pageState->currentPage = 1;

        // 查询当前页
        int offset = (pageState->currentPage - 1) * pageState->pageSize;
        QString sql = "SELECT username, module, operation, ip, operation_time FROM sys_operation_log WHERE " + where;
        sql += " ORDER BY operation_time DESC LIMIT :pageSize OFFSET :offset";
        QSqlQuery q(DatabaseManager::instance().database());
        q.prepare(sql);
        if (!searchText.isEmpty())
            q.bindValue(":search", "%" + searchText + "%");
        if (!moduleFilter.isEmpty())
            q.bindValue(":module", moduleFilter);
        q.bindValue(":pageSize", pageState->pageSize);
        q.bindValue(":offset", offset);
        q.exec();
        int row = 0;
        while (q.next())
        {
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
            table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
            table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
            table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
            table->setItem(row, 4, new QTableWidgetItem(q.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
            row++;
        }
        table->setSortingEnabled(true);
        UiKit::syncEmptyHint(table, emptyHint);

        // 更新分页控件
        pageLabel->setText(QStringLiteral("第 %1/%2 页").arg(pageState->currentPage).arg(pageState->totalPages));
        totalLabel->setText(QStringLiteral("共 %1 条").arg(total));
        prevBtn->setEnabled(pageState->currentPage > 1);
        nextBtn->setEnabled(pageState->currentPage < pageState->totalPages);
    };
    loadLogs();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { pageState->currentPage = 1; loadLogs(); });
    QObject::connect(moduleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { pageState->currentPage = 1; loadLogs(); });
    QObject::connect(prevBtn, &QPushButton::clicked, page, [=]()
            {
            if (pageState->currentPage > 1) { pageState->currentPage--; loadLogs(); } });
    QObject::connect(nextBtn, &QPushButton::clicked, page, [=]()
            {
            if (pageState->currentPage < pageState->totalPages) { pageState->currentPage++; loadLogs(); } });
    QObject::connect(pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            {
            pageState->pageSize = pageSizeCombo->currentData().toInt();
            pageState->currentPage = 1;
            loadLogs(); });
    QObject::connect(jumpBtn, &QPushButton::clicked, page, [=]()
            {
            bool ok = false;
            int p = jumpEdit->text().toInt(&ok);
            if (ok && p >= 1 && p <= pageState->totalPages) {
                pageState->currentPage = p;
                loadLogs();
            } else {
                QMessageBox::warning(page, QStringLiteral("提示"),
                    QStringLiteral("请输入 1-%1 之间的页码").arg(pageState->totalPages));
            }
            jumpEdit->clear(); });
    QObject::connect(jumpEdit, &QLineEdit::returnPressed, page, [=]()
            {
            bool ok = false;
            int p = jumpEdit->text().toInt(&ok);
            if (ok && p >= 1 && p <= pageState->totalPages) {
                pageState->currentPage = p;
                loadLogs();
            }
            jumpEdit->clear(); });

    layout->addWidget(table);
    layout->addWidget(pageBar);
    layout->addWidget(emptyHint);
}

} // namespace PageFactory
