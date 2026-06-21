#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "ui_kit/PagedQuery.h"

void PageFactory::buildPropertyBilling(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // T31 物业缴费
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_money"), QStringLiteral("物业缴费"), QStringLiteral("物业费账单生成、缴费记录查看（演示级，不涉及真实支付）"), UiKit::moduleColor("billing"), page));
    layout->addSpacing(12);

    // Stats
    auto *statsRow = new QHBoxLayout();
    QSqlQuery billTotalQ("SELECT COALESCE(SUM(amount),0) FROM pm_bill WHERE period = strftime('%Y-%m','now') AND del_flag = 0");
    double billTotal = billTotalQ.next() ? billTotalQ.value(0).toDouble() : 0;
    QSqlQuery billPaidQ("SELECT COALESCE(SUM(amount),0) FROM pm_bill WHERE period = strftime('%Y-%m','now') AND status = 1 AND del_flag = 0");
    double billPaid = billPaidQ.next() ? billPaidQ.value(0).toDouble() : 0;
    double billUnpaid = billTotal - billPaid;
    int billRate = billTotal > 0 ? qRound(billPaid / billTotal * 100) : 0;
    QString curMonth = QDate::currentDate().toString("M月");
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("本月应收(%1)").arg(curMonth), QString("¥%1").arg(billTotal, 0, 'f', 0), "#b45309", page));
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("本月已缴(%1)").arg(curMonth), QString("¥%1").arg(billPaid, 0, 'f', 0), "#15803d", page));
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("本月未缴(%1)").arg(curMonth), QString("¥%1").arg(billUnpaid, 0, 'f', 0), "#b91c1c", page));
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("本月缴费率(%1)").arg(curMonth), QString::number(billRate) + "%", "#d97706", page));
    statsRow->addStretch();
    layout->addLayout(statsRow);
    layout->addSpacing(12);

    // Toolbar
    auto *billToolbar = new QHBoxLayout();
    billToolbar->setSpacing(12);
    auto *billSearchEdit = new QLineEdit();
    billSearchEdit->setPlaceholderText(QStringLiteral("搜索账单编号/房屋..."));
    billSearchEdit->setMaximumWidth(260);

    billToolbar->addWidget(billSearchEdit);
    auto *billTypeCombo = new QComboBox();
    billTypeCombo->addItem(QStringLiteral("全部费用类型"), -1);
    billTypeCombo->addItem(QStringLiteral("物业费"), 1);
    billTypeCombo->addItem(QStringLiteral("水费"), 2);
    billTypeCombo->addItem(QStringLiteral("电费"), 3);
    billTypeCombo->addItem(QStringLiteral("停车费"), 4);
    billTypeCombo->addItem(QStringLiteral("综合"), 5);
    billTypeCombo->setMinimumWidth(120);
    billToolbar->addWidget(billTypeCombo);
    auto *billStatusCombo = new QComboBox();
    billStatusCombo->addItem(QStringLiteral("全部状态"), -1);
    billStatusCombo->addItem(QStringLiteral("待缴费"), 0);
    billStatusCombo->addItem(QStringLiteral("已缴费"), 1);
    billStatusCombo->addItem(QStringLiteral("逾期"), 2);
    billStatusCombo->setMinimumWidth(120);
    billToolbar->addWidget(billStatusCombo);
    // Month filter
    auto *billPeriodCombo = new QComboBox();
    billPeriodCombo->addItem(QStringLiteral("全部月份"), "");
    billPeriodCombo->addItem(QStringLiteral("本月"), QDate::currentDate().toString("yyyy-MM"));
    QSqlQuery periodQ("SELECT DISTINCT period FROM pm_bill WHERE del_flag = 0 ORDER BY period DESC");
    while (periodQ.next())
    {
      QString p = periodQ.value(0).toString();
      if (p != QDate::currentDate().toString("yyyy-MM"))
        billPeriodCombo->addItem(p, p);
    }
    billPeriodCombo->setMinimumWidth(120);
    billToolbar->addWidget(billPeriodCombo);
    billToolbar->addStretch();
    layout->insertLayout(layout->count() - 1, billToolbar);

    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("账单编号"), QStringLiteral("房屋"), QStringLiteral("费用类型"), QStringLiteral("金额"), QStringLiteral("账期"), QStringLiteral("状态")});
    table->setColumnWidth(0, 120);
    layout->addWidget(table);

    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadBills = [table, billSearchEdit, billTypeCombo, billStatusCombo, billPeriodCombo, emptyHint, pb]()
    {
      table->setSortingEnabled(false);
      table->setRowCount(0);
      QString baseSql = "SELECT b.bill_no, h.house_code || '-' || h.room_number AS house_info, "
                    "CASE b.bill_type WHEN 1 THEN '物业费' WHEN 2 THEN '水费' WHEN 3 THEN '电费' WHEN 4 THEN '停车费' WHEN 5 THEN '综合' END AS type_name, "
                    "b.amount, b.period, b.status "
                    "FROM pm_bill b LEFT JOIN cm_house h ON b.house_id = h.id "
                    "WHERE b.del_flag = 0";
      QString billSearch = billSearchEdit->text().trimmed();
      int billTypeFilter = billTypeCombo->currentData().toInt();
      int billStatusFilter = billStatusCombo->currentData().toInt();
      QString periodFilter = billPeriodCombo->currentData().toString();
      UiKit::PagedQuery pq(baseSql);
      if (!billSearch.isEmpty())
        pq.where("(b.bill_no LIKE :search OR h.house_code LIKE :search OR h.room_number LIKE :search)", {{"search", "%" + billSearch + "%"}});
      if (billTypeFilter >= 0)
        pq.where("b.bill_type = :btype", {{"btype", billTypeFilter}});
      if (billStatusFilter >= 0)
        pq.where("b.status = :status", {{"status", billStatusFilter}});
      if (!periodFilter.isEmpty())
        pq.where("b.period = :period", {{"period", periodFilter}});
      pq.orderBy("b.period DESC, b.create_time DESC");

      auto result = pq.execute(pb->pageSize(), pb->offset());
      pb->setTotalCount(result.totalCount);
      int bRow = 0;
      for (const auto& rec : result.rows)
      {
        table->insertRow(bRow);
        table->setItem(bRow, 0, new QTableWidgetItem(rec.value(0).toString()));
        table->setItem(bRow, 1, new QTableWidgetItem(rec.value(1).toString()));
        table->setItem(bRow, 2, new QTableWidgetItem(rec.value(2).toString()));
        table->setItem(bRow, 3, new NumericSortTableWidgetItem(QString("¥%1").arg(rec.value(3).toDouble(), 0, 'f', 2)));
        table->setItem(bRow, 4, new QTableWidgetItem(rec.value(4).toString()));
        int bSts = rec.value(5).toInt();
        auto *stsItem = UiKit::createTagTableItem(bSts == 1 ? QStringLiteral("已缴费") : bSts == 2 ? QStringLiteral("逾期")
                                                                                            : QStringLiteral("待缴费"),
                                           QColor(bSts == 1 ? "#f6ffed" : bSts == 2 ? "#fff1f0"
                                                                                    : "#fff7e6"),
                                           QColor(bSts == 1 ? "#15803d" : bSts == 2 ? "#b91c1c"
                                                                                    : "#d97706"));
        table->setItem(bRow, 5, stsItem);
        bRow++;
      }
      table->setSortingEnabled(true);
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadBills();
    QObject::connect(billSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadBills(); });
    QObject::connect(billTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadBills(); });
    QObject::connect(billStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadBills(); });
    QObject::connect(billPeriodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadBills(); });
}
