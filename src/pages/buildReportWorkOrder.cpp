#include "pages/PageFactory.h"

#include "PagesCommon.h"
#include "ReportPageHelper.h"

using namespace ReportPageHelper;

BasePage *PageFactory::buildReportWorkOrder()
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

  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_chart"), QStringLiteral("工单统计分析"), QStringLiteral("工单类型、趋势和状态多维度数据分析"), UiKit::moduleColor("workorder"), page));
  layout->addSpacing(12);

  // ===== Task 16: 工单统计增强 =====
  // 统计卡片：总工单数 / 平均处理时长 / 完结率 / 超时率
  auto *statsRow = new QHBoxLayout();
  statsRow->setSpacing(12);

  int woTotal = 0;
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(*) FROM wo_work_order WHERE del_flag = 0");
    q.exec();
    if (q.next())
      woTotal = q.value(0).toInt();
  }
  double avgHours = 0.0;
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT AVG(julianday(finish_time) - julianday(create_time)) * 24 FROM wo_work_order WHERE status = 4 AND finish_time IS NOT NULL AND del_flag = 0");
    q.exec();
    if (q.next())
      avgHours = q.value(0).toDouble();
  }
  double completeRate = 0.0;
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(CASE WHEN status=4 THEN 1 END) * 100.0 / COUNT(*) FROM wo_work_order WHERE del_flag = 0");
    q.exec();
    if (q.next())
      completeRate = q.value(0).toDouble();
  }
  double timeoutRate = 0.0;
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(CASE WHEN finish_time > sla_deadline THEN 1 END) * 100.0 / COUNT(*) FROM wo_work_order WHERE finish_time IS NOT NULL AND sla_deadline IS NOT NULL AND del_flag = 0");
    q.exec();
    if (q.next())
      timeoutRate = q.value(0).toDouble();
  }

  statsRow->addWidget(mkStatCard(QStringLiteral("工单总数"), QString::number(woTotal), "#b45309", page));
  statsRow->addWidget(mkStatCard(QStringLiteral("平均处理时长(h)"), QString::number(avgHours, 'f', 1), "#15803d", page));
  statsRow->addWidget(mkStatCard(QStringLiteral("完结率"), QString::number(completeRate, 'f', 1) + "%", "#d97706", page));
  statsRow->addWidget(mkStatCard(QStringLiteral("超时率"), QString::number(timeoutRate, 'f', 1) + "%", "#b91c1c", page));
  layout->addLayout(statsRow);
  layout->addSpacing(12);

  // 工具栏：导出按钮
  auto *toolbar = new QHBoxLayout();
  toolbar->addStretch();
  auto *exportBtn = mkExportBtn(page);
  toolbar->addWidget(exportBtn);
  layout->addLayout(toolbar);
  layout->addSpacing(8);

  // 月度趋势折线图（近6个月）
  layout->addWidget(mkTrendLineChart(QStringLiteral("工单月度趋势（近6个月）"),
                                     "SELECT strftime('%Y-%m', create_time) as month, COUNT(*) FROM wo_work_order "
                                     "WHERE create_time >= date('now', '-6 months') AND del_flag = 0 "
                                     "GROUP BY month ORDER BY month",
                                     page));

  layout->addSpacing(12);

  // 优先级分布柱状图
  layout->addWidget(mkBarChart(QStringLiteral("工单优先级分布"), "SELECT priority, COUNT(*) FROM wo_work_order WHERE del_flag = 0 GROUP BY priority", [](int p)
                               { return WorkOrderPriority::label(p); }, QStringLiteral("工单数"), page));

  layout->addSpacing(12);

  // 保留原有饼图：工单类型分布
  layout->addWidget(mkPieChart(QStringLiteral("工单类型分布"), "SELECT order_type, COUNT(*) FROM wo_work_order WHERE del_flag = 0 GROUP BY order_type", [](int t)
                               { return WorkOrderType::label(t); }, page));

  // 导出按钮：导出工单明细表
  // 创建一个隐藏的明细表用于导出
  QObject::connect(exportBtn, &QPushButton::clicked, page, [page]()
          {
          auto* dlg = new QDialog(page);
          dlg->setWindowTitle(QStringLiteral("导出工单数据"));
          dlg->setMinimumWidth(700);
          auto* dl = new QVBoxLayout(dlg);
          auto* table = new QTableWidget(dlg);
          table->setColumnCount(6);
          table->setHorizontalHeaderLabels({QStringLiteral("工单号"), QStringLiteral("标题"),
              QStringLiteral("优先级"), QStringLiteral("状态"), QStringLiteral("创建时间"), QStringLiteral("完成时间")});
          QSqlQuery q(DatabaseManager::instance().database());
          q.prepare("SELECT order_no, title, priority, status, create_time, finish_time FROM wo_work_order WHERE del_flag = 0 ORDER BY create_time DESC");
          q.exec();
          int r = 0;
          while (q.next()) {
              table->insertRow(r);
              table->setItem(r, 0, new QTableWidgetItem(q.value(0).toString()));
              table->setItem(r, 1, new QTableWidgetItem(q.value(1).toString()));
              table->setItem(r, 2, new QTableWidgetItem(WorkOrderPriority::label(q.value(2).toInt())));
              table->setItem(r, 3, new QTableWidgetItem(WorkOrderStatus::label(q.value(3).toInt())));
              table->setItem(r, 4, new QTableWidgetItem(q.value(4).toDateTime().toString("yyyy-MM-dd HH:mm")));
              table->setItem(r, 5, new QTableWidgetItem(q.value(5).isNull() ? "" : q.value(5).toDateTime().toString("yyyy-MM-dd HH:mm")));
              r++;
          }
          table->resizeColumnsToContents();
          dl->addWidget(new QLabel(QStringLiteral("共 %1 条工单记录，点击下方按钮选择保存路径：").arg(r)));
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
              UiKit::exportTableToCsv(table, QStringLiteral("工单统计.csv"), dlg);
          });
          dlg->exec();
          dlg->deleteLater(); });

  layout->addStretch();
  scrollArea->setWidget(content);
  outerLayout->addWidget(scrollArea);
  return page;
}
