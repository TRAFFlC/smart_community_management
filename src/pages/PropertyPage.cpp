#include "pages/PageFactory.h"
#include "PagesCommon.h"

using namespace UiKit;
// ========== Property Pages ==========
BasePage *PageFactory::createPropertyPage(const QString &sub)
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setSortingEnabled(true);

  auto &db = DatabaseManager::instance();

  // 空状态提示（根据 sub 定制文案）
  QString propEmptyText;
  if (sub == "workorder")
    propEmptyText = QStringLiteral("暂无工单记录");
  else if (sub == "complaint")
    propEmptyText = QStringLiteral("暂无投诉记录");
  else if (sub == "inspection")
    propEmptyText = QStringLiteral("暂无巡查记录");
  else if (sub == "announcement")
    propEmptyText = QStringLiteral("暂无公告");
  else if (sub == "visitor")
    propEmptyText = QStringLiteral("暂无访客记录");
  else if (sub == "topic")
    propEmptyText = QStringLiteral("暂无议题记录");
  else if (sub == "billing")
    propEmptyText = QStringLiteral("暂无账单");
  else if (sub == "income")
    propEmptyText = QStringLiteral("暂无收益记录");
  else
    propEmptyText = QStringLiteral("暂无数据");
  auto *emptyHint = createEmptyHintLabel(propEmptyText, page);


  if (sub == "workorder") { buildPropertyWorkorder(page, layout, table, db, emptyHint); return page; }
  if (sub == "complaint") { buildPropertyComplaint(page, layout, table, db, emptyHint); return page; }
  if (sub == "inspection") { buildPropertyInspection(page, layout, table, db, emptyHint); return page; }
  if (sub == "announcement") { buildPropertyAnnouncement(page, layout, table, db, emptyHint); return page; }
  if (sub == "visitor") { buildPropertyVisitor(page, layout, table, db, emptyHint); return page; }
  if (sub == "topic") { buildPropertyTopic(page, layout, table, db, emptyHint); return page; }
  if (sub == "parking") { buildPropertyParking(page, layout, table, db, emptyHint); return page; }
  if (sub == "billing") { buildPropertyBilling(page, layout, table, db, emptyHint); return page; }
  if (sub == "income") { buildPropertyIncome(page, layout, table, db, emptyHint); return page; }
  return page;
}
