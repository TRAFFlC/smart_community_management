#include "pages/PageFactory.h"

#include "PagesCommon.h"

// ========== Report Pages ==========
BasePage *PageFactory::createReportPage(const QString &sub)
{
  if (sub == "workorder")
    return buildReportWorkOrder();
  if (sub == "event")
    return buildReportEvent();
  if (sub == "service")
    return buildReportService();
  if (sub == "dashboard")
    return buildReportDashboard();

  // 未知子类型：返回空白滚动页面
  auto *page = new BasePage();
  auto *outerLayout = new QVBoxLayout(page);
  outerLayout->setContentsMargins(0, 0, 0, 0);
  auto *scrollArea = new QScrollArea(page);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setStyleSheet("QScrollArea { border: none; background: #f8fafc; }");
  auto *content = new QWidget();
  auto *layout = new QVBoxLayout(content);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->addStretch();
  scrollArea->setWidget(content);
  outerLayout->addWidget(scrollArea);
  return page;
}
