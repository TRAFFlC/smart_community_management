#include "pages/PageFactory.h"

#include "PagesCommon.h"
#include "ReportPageHelper.h"

using namespace ReportPageHelper;

BasePage *PageFactory::buildReportService()
{
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

  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_chart"), QStringLiteral("服务统计分析"), QStringLiteral("社区服务订单状态和趋势数据分析"), UiKit::moduleColor("service"), page));
  layout->addSpacing(12);

  // 统计卡片
  auto *statsRow = new QHBoxLayout();
  statsRow->setSpacing(12);

  int svcTotal = 0, svcDone = 0, svcPending = 0;
  double avgRating = 0.0;
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(*) FROM sv_service_order WHERE del_flag = 0");
    q.exec();
    if (q.next())
      svcTotal = q.value(0).toInt();
  }
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(*) FROM sv_service_order WHERE del_flag = 0 AND status = 4");
    q.exec();
    if (q.next())
      svcDone = q.value(0).toInt();
  }
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(*) FROM sv_service_order WHERE del_flag = 0 AND status IN (0, 1, 2, 3)");
    q.exec();
    if (q.next())
      svcPending = q.value(0).toInt();
  }
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT AVG(rating) FROM sv_service_order WHERE del_flag = 0 AND rating IS NOT NULL");
    q.exec();
    if (q.next())
      avgRating = q.value(0).toDouble();
  }

  statsRow->addWidget(mkStatCard(QStringLiteral("服务订单"), QString::number(svcTotal), "#b45309", page));
  statsRow->addWidget(mkStatCard(QStringLiteral("已完成"), QString::number(svcDone), "#15803d", page));
  statsRow->addWidget(mkStatCard(QStringLiteral("进行中"), QString::number(svcPending), "#d97706", page));
  statsRow->addWidget(mkStatCard(QStringLiteral("平均评分"), QString::number(avgRating, 'f', 1), "#7c3aed", page));
  layout->addLayout(statsRow);
  layout->addSpacing(12);

  // 工具栏：导出按钮
  auto *toolbar = new QHBoxLayout();
  toolbar->addStretch();
  auto *exportBtn = mkExportBtn(page);
  toolbar->addWidget(exportBtn);
  layout->addLayout(toolbar);
  layout->addSpacing(8);

  // 月度趋势折线图
  layout->addWidget(mkTrendLineChart(QStringLiteral("服务订单月度趋势（近6个月）"),
                                     "SELECT strftime('%Y-%m', create_time) as month, COUNT(*) FROM sv_service_order "
                                     "WHERE create_time >= date('now', '-6 months') AND del_flag = 0 "
                                     "GROUP BY month ORDER BY month",
                                     page));

  layout->addSpacing(12);

  // 保留原有饼图：服务订单状态分布
  layout->addWidget(mkPieChart(QStringLiteral("服务订单状态分布"), "SELECT status, COUNT(*) FROM sv_service_order WHERE del_flag = 0 GROUP BY status", [](int s)
                               { return ServiceOrderStatus::label(s); }, page));

  // 导出按钮
  QObject::connect(exportBtn, &QPushButton::clicked, page, [page]()
          {
          auto* dlg = new QDialog(page);
          dlg->setWindowTitle(QStringLiteral("导出服务订单数据"));
          dlg->setMinimumWidth(700);
          auto* dl = new QVBoxLayout(dlg);
          auto* table = new QTableWidget(dlg);
          table->setColumnCount(6);
          table->setHorizontalHeaderLabels({QStringLiteral("订单号"), QStringLiteral("标题"),
              QStringLiteral("状态"), QStringLiteral("评分"), QStringLiteral("创建时间"), QStringLiteral("完成时间")});
          QSqlQuery q(DatabaseManager::instance().database());
          q.prepare("SELECT order_no, title, status, rating, create_time, finish_time FROM sv_service_order WHERE del_flag = 0 ORDER BY create_time DESC");
          q.exec();
          int r = 0;
          while (q.next()) {
              table->insertRow(r);
              table->setItem(r, 0, new QTableWidgetItem(q.value(0).toString()));
              table->setItem(r, 1, new QTableWidgetItem(q.value(1).toString()));
              table->setItem(r, 2, new QTableWidgetItem(ServiceOrderStatus::label(q.value(2).toInt())));
              table->setItem(r, 3, new QTableWidgetItem(q.value(3).isNull() ? "" : QString::number(q.value(3).toInt())));
              table->setItem(r, 4, new QTableWidgetItem(q.value(4).toDateTime().toString("yyyy-MM-dd HH:mm")));
              table->setItem(r, 5, new QTableWidgetItem(q.value(5).isNull() ? "" : q.value(5).toDateTime().toString("yyyy-MM-dd HH:mm")));
              r++;
          }
          table->resizeColumnsToContents();
          dl->addWidget(new QLabel(QStringLiteral("共 %1 条服务订单记录，点击下方按钮选择保存路径：").arg(r)));
          dl->addWidget(table);
          auto* btnRow = new QHBoxLayout();
          btnRow->addStretch();
          auto* okBtn = new QPushButton(QStringLiteral("选择路径并导出"), dlg);
          okBtn->setProperty("cssClass", "primary");
          okBtn->setCursor(Qt::PointingHandCursor);
          auto* cancelBtn = new QPushButton(QStringLiteral("取消"), dlg);
          cancelBtn->setCursor(Qt::PointingHandCursor);
          btnRow->addWidget(cancelBtn);
          btnRow->addWidget(okBtn);
          dl->addLayout(btnRow);
          QObject::connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
          QObject::connect(okBtn, &QPushButton::clicked, dlg, [dlg, table]() {
              UiKit::exportTableToCsv(table, QStringLiteral("服务统计.csv"), dlg);
          });
          dlg->exec();
          dlg->deleteLater(); });

  layout->addStretch();
  scrollArea->setWidget(content);
  outerLayout->addWidget(scrollArea);
  return page;
}
