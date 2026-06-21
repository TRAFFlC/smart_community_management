#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "ui_kit/PagedQuery.h"

void PageFactory::buildPropertyIncome(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // T36 公共收益公示
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_chart"), QStringLiteral("公共收益公示"), QStringLiteral("业委会公共收益和支出明细公示"), UiKit::moduleColor("income"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索账期..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("已公示"), 0);
    statusCombo->addItem(QStringLiteral("待审核"), 1);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("账期"), QStringLiteral("收入金额"), QStringLiteral("支出金额"), QStringLiteral("结余"), QStringLiteral("公示时间"), QStringLiteral("状态")});
    layout->addWidget(table);
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadIncomes = [table, searchEdit, statusCombo, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      UiKit::PagedQuery pq("SELECT period, income_amount, expense_amount, balance, publish_time, status FROM oc_public_income WHERE del_flag = 0");
      if (!searchText.isEmpty())
        pq.where("period LIKE :search", {{"search", "%" + searchText + "%"}});
      if (statusFilter >= 0)
        pq.where("status = :status", {{"status", statusFilter}});
      pq.orderBy("publish_time DESC");

      auto result = pq.execute(pb->pageSize(), pb->offset());
      pb->setTotalCount(result.totalCount);
      int iRow = 0;
      for (const auto& rec : result.rows)
      {
        table->insertRow(iRow);
        table->setItem(iRow, 0, new QTableWidgetItem(rec.value(0).toString()));
        table->setItem(iRow, 1, new QTableWidgetItem(QString("¥%1").arg(rec.value(1).toDouble(), 0, 'f', 2)));
        table->setItem(iRow, 2, new QTableWidgetItem(QString("¥%1").arg(rec.value(2).toDouble(), 0, 'f', 2)));
        auto *balItem = new QTableWidgetItem(QString("¥%1").arg(rec.value(3).toDouble(), 0, 'f', 2));
        if (rec.value(3).toDouble() > 0)
          balItem->setForeground(QColor("#15803d"));
        table->setItem(iRow, 3, balItem);
        table->setItem(iRow, 4, new QTableWidgetItem(rec.value(4).toDateTime().toString("yyyy-MM-dd")));
        int incSts = rec.value(5).toInt();
        auto *stsItem = incSts == 0
                            ? UiKit::createTagTableItem(QStringLiteral("已公示"), QColor("#f0fdf4"), QColor("#15803d"))
                            : UiKit::createTagTableItem(QStringLiteral("待审核"), QColor("#fffbeb"), QColor("#d97706"));
        table->setItem(iRow, 5, stsItem);
        iRow++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadIncomes();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadIncomes(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadIncomes(); });
}
