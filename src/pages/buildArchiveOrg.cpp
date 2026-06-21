#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "ui_kit/PagedQuery.h"

void PageFactory::buildArchiveOrg(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint)
{
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_building"), QStringLiteral("组织管理"), QStringLiteral("管理街道、社区、物业、业委会等组织架构"), UiKit::moduleColor("org"), page));
  layout->addSpacing(12);
  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  tbLayout->setSpacing(10);
  auto *searchEdit = new QLineEdit(toolbar);
  searchEdit->setPlaceholderText(QStringLiteral("搜索组织名称..."));
  searchEdit->setMinimumWidth(200);
  searchEdit->setClearButtonEnabled(true);
  tbLayout->addWidget(searchEdit);
  auto *orgTypeCombo = new QComboBox(toolbar);
  orgTypeCombo->addItem(QStringLiteral("全部类型"), -1);
  orgTypeCombo->addItem(QStringLiteral("街道"), 2);
  orgTypeCombo->addItem(QStringLiteral("社区"), 3);
  orgTypeCombo->addItem(QStringLiteral("物业"), 4);
  orgTypeCombo->addItem(QStringLiteral("业委会"), 5);
  orgTypeCombo->addItem(QStringLiteral("志愿"), 7);
  orgTypeCombo->addItem(QStringLiteral("社会"), 6);
  orgTypeCombo->setMinimumWidth(120);
  tbLayout->addWidget(orgTypeCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  table->setColumnCount(4);
  table->setHorizontalHeaderLabels({QStringLiteral("组织名称"), QStringLiteral("类型"), QStringLiteral("负责人"), QStringLiteral("电话")});
  auto *pb = new PaginationBar(page);
  layout->addWidget(pb);
  std::function<void()> loadOrgs = [table, searchEdit, orgTypeCombo, emptyHint, pb]()
  {
    table->setRowCount(0);
    QString searchText = searchEdit->text().trimmed();
    int typeFilter = orgTypeCombo->currentData().toInt();
    UiKit::PagedQuery pq("SELECT org_name, org_type, leader, phone FROM sys_org WHERE del_flag = 0");
    if (!searchText.isEmpty())
      pq.where("org_name LIKE :search", {{"search", "%" + searchText + "%"}});
    if (typeFilter >= 0)
      pq.where("org_type = :type", {{"type", typeFilter}});
    pq.orderBy("sort_order");

    auto result = pq.execute(pb->pageSize(), pb->offset());
    pb->setTotalCount(result.totalCount);
    int row = 0;
    for (const auto& rec : result.rows)
    {
      table->insertRow(row);
      table->setItem(row, 0, new QTableWidgetItem(rec.value(0).toString()));
      table->setItem(row, 1, new QTableWidgetItem(OrgType::label(rec.value(1).toInt())));
      table->setItem(row, 2, new QTableWidgetItem(rec.value(2).toString()));
      table->setItem(row, 3, new QTableWidgetItem(rec.value(3).toString()));
      row++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
    pb->refreshData();
  };
  loadOrgs();
  QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
          { loadOrgs(); });
  QObject::connect(orgTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadOrgs(); });
}
