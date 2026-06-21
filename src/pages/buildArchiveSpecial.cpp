#include "pages/PageFactory.h"
#include "PagesCommon.h"

void PageFactory::buildArchiveSpecial(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint)
{
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_heart"), QStringLiteral("特殊群体"), QStringLiteral("管理独居老人、残疾人等特殊群体关怀"), UiKit::moduleColor("special"), page));
  layout->addSpacing(12);
  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  tbLayout->setSpacing(10);
  auto *searchEdit = new QLineEdit(toolbar);
  searchEdit->setPlaceholderText(QStringLiteral("搜索居民姓名..."));
  searchEdit->setMinimumWidth(200);
  searchEdit->setClearButtonEnabled(true);
  tbLayout->addWidget(searchEdit);
  auto *groupTypeCombo = new QComboBox(toolbar);
  groupTypeCombo->addItem(QStringLiteral("全部类型"), -1);
  groupTypeCombo->addItem(QStringLiteral("独居老人"), 1);
  groupTypeCombo->addItem(QStringLiteral("残疾人"), 2);
  groupTypeCombo->addItem(QStringLiteral("低保户"), 3);
  groupTypeCombo->addItem(QStringLiteral("留守儿童"), 4);
  groupTypeCombo->setMinimumWidth(120);
  tbLayout->addWidget(groupTypeCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  table->setColumnCount(4);
  table->setHorizontalHeaderLabels({QStringLiteral("居民"), QStringLiteral("类型"), QStringLiteral("关怀等级"), QStringLiteral("走访频率")});
  std::function<void()> loadSpecials = [table, searchEdit, groupTypeCombo, emptyHint]()
  {
    table->setRowCount(0);
    QString sql = "SELECT r.name, sg.group_type, sg.care_level, sg.care_frequency FROM cm_special_group sg JOIN cm_resident r ON sg.resident_id = r.id WHERE sg.del_flag = 0";
    QString searchText = searchEdit->text().trimmed();
    int typeFilter = groupTypeCombo->currentData().toInt();
    if (!searchText.isEmpty())
      sql += " AND r.name LIKE :search";
    if (typeFilter >= 0)
      sql += " AND sg.group_type = :type";
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
      table->setItem(row, 1, new QTableWidgetItem(SpecialGroupType::label(q.value(1).toInt())));
      table->setItem(row, 2, new QTableWidgetItem(q.value(2).toInt() == 1 ? QStringLiteral("一般") : q.value(2).toInt() == 2 ? QStringLiteral("重点")
                                                                                                                             : QStringLiteral("特殊")));
      table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
      row++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
  };
  loadSpecials();
  QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
          { loadSpecials(); });
  QObject::connect(groupTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadSpecials(); });
}
