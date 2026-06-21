#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "ui_kit/PagedQuery.h"

void PageFactory::buildPropertyInspection(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Property inspection - reuse from governance
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_search"), QStringLiteral("物业巡检管理"), QStringLiteral("物业巡检计划和记录查看"), UiKit::moduleColor("inspection"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索巡检员..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("进行中"), 0);
    statusCombo->addItem(QStringLiteral("已完成"), 1);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("巡检员"), QStringLiteral("开始时间"), QStringLiteral("时长(分)"), QStringLiteral("发现问题"), QStringLiteral("状态")});
    layout->addWidget(table);
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadInspections = [table, searchEdit, statusCombo, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      UiKit::PagedQuery pq("SELECT u.real_name, i.start_time, i.duration, i.issue_count, i.status FROM ge_inspection i LEFT JOIN sys_user u ON i.inspector_id = u.id WHERE i.del_flag = 0");
      if (!searchText.isEmpty())
        pq.where("u.real_name LIKE :search", {{"search", "%" + searchText + "%"}});
      if (statusFilter >= 0)
        pq.where("i.status = :status", {{"status", statusFilter}});
      pq.orderBy("i.start_time DESC");

      auto result = pq.execute(pb->pageSize(), pb->offset());
      pb->setTotalCount(result.totalCount);
      int iRow = 0;
      for (const auto& rec : result.rows)
      {
        table->insertRow(iRow);
        table->setItem(iRow, 0, new QTableWidgetItem(rec.value(0).toString()));
        table->setItem(iRow, 1, new QTableWidgetItem(rec.value(1).toDateTime().toString("MM-dd hh:mm")));
        table->setItem(iRow, 2, new QTableWidgetItem(rec.value(2).toString()));
        table->setItem(iRow, 3, new QTableWidgetItem(rec.value(3).toString()));
        int iSts = rec.value(4).toInt();
        auto *stsItem = UiKit::createTagTableItem(iSts == 0 ? QStringLiteral("进行中") : QStringLiteral("已完成"), QColor(iSts == 0 ? "#e6f4ff" : "#f6ffed"), QColor(iSts == 0 ? "#b45309" : "#15803d"));
        table->setItem(iRow, 4, stsItem);
        iRow++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadInspections();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadInspections(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadInspections(); });
}
