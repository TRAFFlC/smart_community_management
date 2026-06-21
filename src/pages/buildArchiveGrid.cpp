#include "pages/PageFactory.h"
#include "PagesCommon.h"

void PageFactory::buildArchiveGrid(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint)
{
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_route"), QStringLiteral("网格管理"), QStringLiteral("管理社区网格划分和网格员配置"), UiKit::moduleColor("grid"), page));
  layout->addSpacing(12);
  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  tbLayout->setSpacing(10);
  auto *searchEdit = new QLineEdit(toolbar);
  searchEdit->setPlaceholderText(QStringLiteral("搜索网格名称/编码..."));
  searchEdit->setMinimumWidth(200);
  searchEdit->setClearButtonEnabled(true);
  tbLayout->addWidget(searchEdit);
  auto *gridStatusCombo = new QComboBox(toolbar);
  gridStatusCombo->addItem(QStringLiteral("全部状态"), -1);
  gridStatusCombo->addItem(QStringLiteral("正常"), 0);
  gridStatusCombo->addItem(QStringLiteral("停用"), 1);
  gridStatusCombo->setMinimumWidth(120);
  tbLayout->addWidget(gridStatusCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  table->setColumnCount(4);
  table->setHorizontalHeaderLabels({QStringLiteral("网格名称"), QStringLiteral("编码"), QStringLiteral("描述"), QStringLiteral("网格员")});
  std::function<void()> loadGrids = [table, searchEdit, gridStatusCombo, emptyHint]()
  {
    table->setRowCount(0);
    QString sql = "SELECT g.grid_name, g.grid_code, g.description, u.real_name FROM cm_grid g LEFT JOIN sys_user u ON g.grid_worker_id = u.id WHERE g.del_flag = 0";
    QString searchText = searchEdit->text().trimmed();
    int statusFilter = gridStatusCombo->currentData().toInt();
    if (!searchText.isEmpty())
      sql += " AND (g.grid_name LIKE :search OR g.grid_code LIKE :search)";
    if (statusFilter >= 0)
      sql += " AND g.status = :status";
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(sql);
    if (!searchText.isEmpty())
      q.bindValue(":search", "%" + searchText + "%");
    if (statusFilter >= 0)
      q.bindValue(":status", statusFilter);
    q.exec();
    int row = 0;
    while (q.next())
    {
      table->insertRow(row);
      table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
      table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
      table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
      table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
      row++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
  };
  loadGrids();
  QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
          { loadGrids(); });
  QObject::connect(gridStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadGrids(); });
}
