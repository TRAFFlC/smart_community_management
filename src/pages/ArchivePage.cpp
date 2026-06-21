#include "pages/PageFactory.h"
#include "PagesCommon.h"

// ========== Archive Pages ==========
BasePage *PageFactory::createArchivePage(const QString &sub)
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  UiKit::configureTable(table);
  table->horizontalHeader()->setStretchLastSection(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(UiKit::TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setSortingEnabled(true);

  auto &db = DatabaseManager::instance();

  // 空状态提示（根据 sub 定制文案）
  QString archiveEmptyText;
  if (sub == "org")
    archiveEmptyText = QStringLiteral("暂无组织记录");
  else if (sub == "estate")
    archiveEmptyText = QStringLiteral("暂无小区记录");
  else if (sub == "house")
    archiveEmptyText = QStringLiteral("暂无房屋记录");
  else if (sub == "resident")
    archiveEmptyText = QStringLiteral("暂无居民记录");
  else if (sub == "vehicle")
    archiveEmptyText = QStringLiteral("暂无车辆记录");
  else if (sub == "facility")
    archiveEmptyText = QStringLiteral("暂无设施记录");
  else if (sub == "grid")
    archiveEmptyText = QStringLiteral("暂无网格记录");
  else if (sub == "special")
    archiveEmptyText = QStringLiteral("暂无关怀对象");
  else
    archiveEmptyText = QStringLiteral("暂无数据");
  auto *emptyHint = UiKit::createEmptyHintLabel(archiveEmptyText, page);

  if (sub == "org")
    buildArchiveOrg(page, layout, table, db, emptyHint);
  else if (sub == "estate")
    buildArchiveEstate(page, layout, table, db, emptyHint);
  else if (sub == "house")
    buildArchiveHouse(page, layout, table, db, emptyHint);
  else if (sub == "resident")
    buildArchiveResident(page, layout, table, db, emptyHint);
  else if (sub == "vehicle")
    buildArchiveVehicle(page, layout, table, db, emptyHint);
  else if (sub == "facility")
    buildArchiveFacility(page, layout, table, db, emptyHint);
  else if (sub == "grid")
    buildArchiveGrid(page, layout, table, db, emptyHint);
  else if (sub == "special")
    buildArchiveSpecial(page, layout, table, db, emptyHint);

  layout->addWidget(table);
  layout->addWidget(emptyHint);
  return page;
}
