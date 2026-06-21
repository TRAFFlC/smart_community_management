#include "pages/PageFactory.h"
#include "PagesCommon.h"

void PageFactory::buildArchiveVehicle(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint)
{
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_car"), QStringLiteral("车辆管理"), QStringLiteral("管理小区车辆登记、类型和停车信息"), UiKit::moduleColor("vehicle"), page));
  layout->addSpacing(12);
  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  tbLayout->setSpacing(10);
  auto *searchEdit = new QLineEdit(toolbar);
  searchEdit->setPlaceholderText(QStringLiteral("搜索车牌号..."));
  searchEdit->setMinimumWidth(200);
  searchEdit->setClearButtonEnabled(true);
  tbLayout->addWidget(searchEdit);
  auto *typeCombo = new QComboBox(toolbar);
  typeCombo->addItem(QStringLiteral("全部类型"), -1);
  typeCombo->addItem(QStringLiteral("轿车"), 1);
  typeCombo->addItem(QStringLiteral("SUV"), 2);
  typeCombo->addItem(QStringLiteral("电动车"), 3);
  typeCombo->addItem(QStringLiteral("其他"), 4);
  typeCombo->setMinimumWidth(120);
  tbLayout->addWidget(typeCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  table->setColumnCount(4);
  table->setHorizontalHeaderLabels({QStringLiteral("车牌号"), QStringLiteral("品牌"), QStringLiteral("颜色"), QStringLiteral("类型")});
  std::function<void()> loadVehicles = [table, searchEdit, typeCombo, emptyHint]()
  {
    table->setRowCount(0);
    QString sql = "SELECT plate_number, vehicle_brand, vehicle_color, vehicle_type FROM cm_vehicle WHERE del_flag = 0";
    QString searchText = searchEdit->text().trimmed();
    int typeFilter = typeCombo->currentData().toInt();
    if (!searchText.isEmpty())
      sql += " AND plate_number LIKE :search";
    if (typeFilter >= 0)
      sql += " AND vehicle_type = :type";
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
      table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
      table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
      table->setItem(row, 3, new QTableWidgetItem(VehicleType::label(q.value(3).toInt())));
      row++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
  };
  loadVehicles();
  QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
          { loadVehicles(); });
  QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadVehicles(); });
}
