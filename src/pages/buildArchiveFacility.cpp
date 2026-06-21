#include "pages/PageFactory.h"
#include "PagesCommon.h"

void PageFactory::buildArchiveFacility(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint)
{
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_tool"), QStringLiteral("设施管理"), QStringLiteral("管理电梯、消防、监控等公共设施"), UiKit::moduleColor("facility"), page));
  layout->addSpacing(12);
  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  tbLayout->setSpacing(10);
  auto *searchEdit = new QLineEdit(toolbar);
  searchEdit->setPlaceholderText(QStringLiteral("搜索设施名称/编号..."));
  searchEdit->setMinimumWidth(200);
  searchEdit->setClearButtonEnabled(true);
  tbLayout->addWidget(searchEdit);
  auto *facTypeCombo = new QComboBox(toolbar);
  facTypeCombo->addItem(QStringLiteral("全部类型"), -1);
  facTypeCombo->addItem(QStringLiteral("电梯"), 1);
  facTypeCombo->addItem(QStringLiteral("消防"), 2);
  facTypeCombo->addItem(QStringLiteral("监控"), 3);
  facTypeCombo->addItem(QStringLiteral("门禁"), 4);
  facTypeCombo->addItem(QStringLiteral("照明"), 5);
  facTypeCombo->setMinimumWidth(120);
  tbLayout->addWidget(facTypeCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  table->setColumnCount(5);
  table->setHorizontalHeaderLabels({QStringLiteral("设施名称"), QStringLiteral("类型"), QStringLiteral("编号"), QStringLiteral("位置"), QStringLiteral("状态")});
  std::function<void()> loadFacilities = [table, searchEdit, facTypeCombo, emptyHint]()
  {
    table->setRowCount(0);
    QString sql = "SELECT facility_name, facility_type, facility_code, location, status FROM cm_facility WHERE del_flag = 0";
    QString searchText = searchEdit->text().trimmed();
    int typeFilter = facTypeCombo->currentData().toInt();
    if (!searchText.isEmpty())
      sql += " AND (facility_name LIKE :search OR facility_code LIKE :search)";
    if (typeFilter >= 0)
      sql += " AND facility_type = :type";
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(sql);
    if (!searchText.isEmpty())
      q.bindValue(":search", "%" + searchText + "%");
    if (typeFilter >= 0)
      q.bindValue(":type", typeFilter);
    q.exec();
    int row = 0;
    while (q.next())
    {
      table->insertRow(row);
      table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
      table->setItem(row, 1, new QTableWidgetItem(FacilityType::label(q.value(1).toInt())));
      table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
      table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
      int facSts = q.value(4).toInt();
      auto *facStsItem = facSts == 0
                             ? UiKit::createTagTableItem(QStringLiteral("正常"), QColor("#f0fdf4"), QColor("#15803d"))
                             : UiKit::createTagTableItem(QStringLiteral("故障"), QColor("#fef2f2"), QColor("#b91c1c"));
      table->setItem(row, 4, facStsItem);
      row++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
  };
  loadFacilities();
  QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
          { loadFacilities(); });
  QObject::connect(facTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadFacilities(); });
}
