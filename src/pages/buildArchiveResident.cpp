#include "pages/PageFactory.h"
#include "PagesCommon.h"

void PageFactory::buildArchiveResident(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint)
{
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_people"), QStringLiteral("居民管理"), QStringLiteral("管理居民档案、联系方式和基本信息"), UiKit::moduleColor("resident"), page));
  layout->addSpacing(12);
  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  tbLayout->setSpacing(10);
  auto *searchEdit = new QLineEdit(toolbar);
  searchEdit->setPlaceholderText(QStringLiteral("搜索姓名/电话..."));
  searchEdit->setMinimumWidth(200);
  searchEdit->setClearButtonEnabled(true);
  tbLayout->addWidget(searchEdit);
  auto *genderCombo = new QComboBox(toolbar);
  genderCombo->addItem(QStringLiteral("全部性别"), -1);
  genderCombo->addItem(QStringLiteral("男"), 1);
  genderCombo->addItem(QStringLiteral("女"), 0);
  genderCombo->setMinimumWidth(120);
  tbLayout->addWidget(genderCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  table->setColumnCount(5);
  table->setHorizontalHeaderLabels({QStringLiteral("姓名"), QStringLiteral("性别"), QStringLiteral("手机号"), QStringLiteral("民族"), QStringLiteral("职业")});
  std::function<void()> loadResidents = [table, searchEdit, genderCombo, emptyHint]()
  {
    table->setRowCount(0);
    QString sql = "SELECT name, gender, phone_display, nationality, occupation FROM cm_resident WHERE del_flag = 0";
    QString searchText = searchEdit->text().trimmed();
    int genderFilter = genderCombo->currentData().toInt();
    if (!searchText.isEmpty())
      sql += " AND (name LIKE :search OR phone_display LIKE :search)";
    if (genderFilter >= 0)
      sql += " AND gender = :gender";
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(sql);
    if (!searchText.isEmpty())
      q.bindValue(":search", "%" + searchText + "%");
    if (genderFilter >= 0)
      q.bindValue(":gender", genderFilter);
    q.exec();
    int row = 0;
    while (q.next())
    {
      table->insertRow(row);
      table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
      table->setItem(row, 1, new QTableWidgetItem(q.value(1).toInt() == 1 ? QStringLiteral("男") : QStringLiteral("女")));
      table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
      table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
      table->setItem(row, 4, new QTableWidgetItem(q.value(4).toString()));
      row++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
  };
  loadResidents();
  QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
          { loadResidents(); });
  QObject::connect(genderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadResidents(); });
}
