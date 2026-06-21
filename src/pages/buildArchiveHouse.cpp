#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "ui_kit/PagedQuery.h"

void PageFactory::buildArchiveHouse(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint)
{
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_home"), QStringLiteral("房屋管理"), QStringLiteral("管理房屋信息、楼层、户型和居住状态"), UiKit::moduleColor("house"), page));
  layout->addSpacing(12);
  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  tbLayout->setSpacing(10);
  auto *searchEdit = new QLineEdit(toolbar);
  searchEdit->setPlaceholderText(QStringLiteral("搜索房屋编号/房号..."));
  searchEdit->setMinimumWidth(200);
  searchEdit->setClearButtonEnabled(true);
  tbLayout->addWidget(searchEdit);
  auto *statusCombo = new QComboBox(toolbar);
  statusCombo->addItem(QStringLiteral("全部状态"), -1);
  statusCombo->addItem(QStringLiteral("空置"), 0);
  statusCombo->addItem(QStringLiteral("自住"), 1);
  statusCombo->addItem(QStringLiteral("出租"), 2);
  statusCombo->addItem(QStringLiteral("出售"), 3);
  statusCombo->setMinimumWidth(120);
  tbLayout->addWidget(statusCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  table->setColumnCount(6);
  table->setHorizontalHeaderLabels({QStringLiteral("房屋编号"), QStringLiteral("楼层"), QStringLiteral("房号"), QStringLiteral("面积"), QStringLiteral("户型"), QStringLiteral("状态")});
  auto *pb = new PaginationBar(page);
  layout->addWidget(pb);
  std::function<void()> loadHouses = [table, searchEdit, statusCombo, emptyHint, pb]()
  {
    table->setRowCount(0);
    QString searchText = searchEdit->text().trimmed();
    int statusFilter = statusCombo->currentData().toInt();
    UiKit::PagedQuery pq("SELECT house_code, floor, room_number, area, house_type, house_status FROM cm_house WHERE del_flag = 0");
    if (!searchText.isEmpty())
      pq.where("(house_code LIKE :search OR room_number LIKE :search)", {{"search", "%" + searchText + "%"}});
    if (statusFilter >= 0)
      pq.where("house_status = :status", {{"status", statusFilter}});
    pq.orderBy("house_code");

    auto result = pq.execute(pb->pageSize(), pb->offset());
    pb->setTotalCount(result.totalCount);
    int row = 0;
    for (const auto& rec : result.rows)
    {
      table->insertRow(row);
      table->setItem(row, 0, new QTableWidgetItem(rec.value(0).toString()));
      table->setItem(row, 1, new QTableWidgetItem(rec.value(1).toString()));
      table->setItem(row, 2, new QTableWidgetItem(rec.value(2).toString()));
      table->setItem(row, 3, new QTableWidgetItem(rec.value(3).toString()));
      table->setItem(row, 4, new QTableWidgetItem(rec.value(4).toString()));
      int houseSts = rec.value(5).toInt();
      QColor hsBg, hsFg;
      QString hsText = HouseStatus::label(houseSts);
      switch (houseSts)
      {
      case HouseStatus::Vacant:
        hsBg = QColor("#f8fafc");
        hsFg = QColor("#64748b");
        break;
      case HouseStatus::OwnerOccupied:
        hsBg = QColor("#eff6ff");
        hsFg = QColor("#2563eb");
        break;
      case HouseStatus::Rented:
        hsBg = QColor("#f0fdf4");
        hsFg = QColor("#15803d");
        break;
      default:
        hsBg = QColor("#fffbeb");
        hsFg = QColor("#d97706");
        break;
      }
      auto *houseStsItem = UiKit::createTagTableItem(hsText, hsBg, hsFg);
      table->setItem(row, 5, houseStsItem);
      row++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
    pb->refreshData();
  };
  loadHouses();
  QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
          { loadHouses(); });
  QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadHouses(); });
}
