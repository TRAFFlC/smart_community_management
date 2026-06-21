#include "pages/PageFactory.h"
#include "PagesCommon.h"

void PageFactory::buildArchiveEstate(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint)
{
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_home"), QStringLiteral("小区管理"), QStringLiteral("管理小区基本信息、楼栋和户数配置"), UiKit::moduleColor("estate"), page));
  layout->addSpacing(12);
  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  tbLayout->setSpacing(10);
  auto *searchEdit = new QLineEdit(toolbar);
  searchEdit->setPlaceholderText(QStringLiteral("搜索小区名称/编码..."));
  searchEdit->setMinimumWidth(200);
  searchEdit->setClearButtonEnabled(true);
  tbLayout->addWidget(searchEdit);
  auto *estateStatusCombo = new QComboBox(toolbar);
  estateStatusCombo->addItem(QStringLiteral("全部状态"), -1);
  estateStatusCombo->addItem(QStringLiteral("正常"), 0);
  estateStatusCombo->addItem(QStringLiteral("停用"), 1);
  estateStatusCombo->setMinimumWidth(120);
  tbLayout->addWidget(estateStatusCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  table->setColumnCount(5);
  table->setHorizontalHeaderLabels({QStringLiteral("小区名称"), QStringLiteral("编码"), QStringLiteral("地址"), QStringLiteral("楼栋数"), QStringLiteral("总户数")});
  std::function<void()> loadEstates = [table, searchEdit, estateStatusCombo, emptyHint]()
  {
    table->setRowCount(0);
    QString sql = "SELECT estate_name, estate_code, address, total_buildings, total_houses FROM cm_estate WHERE del_flag = 0";
    QString searchText = searchEdit->text().trimmed();
    int statusFilter = estateStatusCombo->currentData().toInt();
    if (!searchText.isEmpty())
      sql += " AND (estate_name LIKE :search OR estate_code LIKE :search)";
    if (statusFilter >= 0)
      sql += " AND status = :status";
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
      table->setItem(row, 4, new QTableWidgetItem(q.value(4).toString()));
      row++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
  };
  loadEstates();
  QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
          { loadEstates(); });
  QObject::connect(estateStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadEstates(); });
}
