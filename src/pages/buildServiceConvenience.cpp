#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "ui_kit/PagedQuery.h"

BasePage *PageFactory::buildServiceConvenience()
{
    auto *page = new BasePage();
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    auto *table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    UiKit::configureTable(table);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(UiKit::TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);
    table->setSortingEnabled(true);

    auto *emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无便民服务记录"), page);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索订单号/标题..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("待接单"), 0);
    statusCombo->addItem(QStringLiteral("已接单"), 1);
    statusCombo->addItem(QStringLiteral("服务中"), 2);
    statusCombo->addItem(QStringLiteral("已完成"), 3);
    statusCombo->addItem(QStringLiteral("已取消"), 4);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("订单号"), QStringLiteral("标题"), QStringLiteral("服务类型"), QStringLiteral("状态"), QStringLiteral("预约时间")});
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadConvenience = [table, searchEdit, statusCombo, emptyHint, pb]()
    {
        table->setSortingEnabled(false);
        table->setRowCount(0);
        QString searchText = searchEdit->text().trimmed();
        int statusFilter = statusCombo->currentData().toInt();
        UiKit::PagedQuery pq("SELECT order_no, title, service_type, status, appointment_time FROM sv_service_order WHERE del_flag = 0");
        if (!searchText.isEmpty())
            pq.where("(order_no LIKE :search OR title LIKE :search)", {{"search", "%" + searchText + "%"}});
        if (statusFilter >= 0)
            pq.where("status = :status", {{"status", statusFilter}});
        pq.orderBy("create_time DESC");

        auto result = pq.execute(pb->pageSize(), pb->offset());
        pb->setTotalCount(result.totalCount);
        int row = 0;
        for (const auto& rec : result.rows)
        {
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(rec.value(0).toString()));
            table->setItem(row, 1, new QTableWidgetItem(rec.value(1).toString()));
            table->setItem(row, 2, new QTableWidgetItem(ServiceType::label(rec.value(2).toInt())));
            int convSts = rec.value(3).toInt();
            QColor convBg, convFg;
            switch (convSts)
            {
            case 0:
                convBg = QColor("#eff6ff");
                convFg = QColor("#2563eb");
                break;
            case 1:
                convBg = QColor("#fffbeb");
                convFg = QColor("#d97706");
                break;
            case 2:
                convBg = QColor("#f0fdf4");
                convFg = QColor("#15803d");
                break;
            case 3:
                convBg = QColor("#f3e8ff");
                convFg = QColor("#7c3aed");
                break;
            default:
                convBg = QColor("#f8fafc");
                convFg = QColor("#64748b");
                break;
            }
            auto *convStsItem = UiKit::createTagTableItem(ServiceOrderStatus::label(convSts), convBg, convFg);
            table->setItem(row, 3, convStsItem);
            table->setItem(row, 4, new QTableWidgetItem(rec.value(4).toDateTime().toString("yyyy-MM-dd hh:mm")));
            row++;
        }
        UiKit::syncEmptyHint(table, emptyHint);
        pb->refreshData();
        table->setSortingEnabled(true);
    };
    loadConvenience();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadConvenience(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadConvenience(); });

    layout->addWidget(table);
    layout->addWidget(emptyHint);
    return page;
}
