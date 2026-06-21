#include "pages/PageFactory.h"

#include "PagesCommon.h"
#include "ReportPageHelper.h"

#include <QChart>
#include <QChartView>
#include <QBarSeries>
#include <QBarSet>
#include <QCategoryAxis>
#include <QValueAxis>
#include <QtCharts>

using namespace ReportPageHelper;

BasePage *PageFactory::buildReportDashboard()
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

  // Enhanced dashboard with stats + multiple charts (复用外层滚动区域)
  QWidget *scrollContent = content;
  QVBoxLayout *mainLay = layout;

  auto *dashboardHeader = UiKit::createPageHeader(QStringLiteral("ic_chart"), QStringLiteral("综合数据看板"), QStringLiteral("多维度数据可视化，实时监控社区运营状态"), UiKit::moduleColor("dashboard"), scrollContent);
  mainLay->addWidget(dashboardHeader);
  mainLay->addSpacing(12);

  // Summary stat cards
  auto *statsRow = new QHBoxLayout();
  statsRow->setSpacing(12);

  // Count real data
  QSqlQuery woTotalQ("SELECT COUNT(*) FROM wo_work_order WHERE del_flag = 0");
  int woTotal = woTotalQ.next() ? woTotalQ.value(0).toInt() : 0;
  QSqlQuery woPendQ("SELECT COUNT(*) FROM wo_work_order WHERE del_flag = 0 AND status = 0");
  int woPend = woPendQ.next() ? woPendQ.value(0).toInt() : 0;
  QSqlQuery evTotalQ("SELECT COUNT(*) FROM ge_event WHERE del_flag = 0");
  int evTotal = evTotalQ.next() ? evTotalQ.value(0).toInt() : 0;
  QSqlQuery evPendQ("SELECT COUNT(*) FROM ge_event WHERE del_flag = 0 AND status = 0");
  int evPend = evPendQ.next() ? evPendQ.value(0).toInt() : 0;
  QSqlQuery svcTotalQ("SELECT COUNT(*) FROM sv_service_order WHERE del_flag = 0");
  int svcTotal = svcTotalQ.next() ? svcTotalQ.value(0).toInt() : 0;
  QSqlQuery annTotalQ("SELECT COUNT(*) FROM nt_announcement WHERE del_flag = 0");
  int annTotal = annTotalQ.next() ? annTotalQ.value(0).toInt() : 0;

  statsRow->addWidget(mkStatCard(QStringLiteral("工单总数"), QString::number(woTotal), "#b45309", scrollContent));
  statsRow->addWidget(mkStatCard(QStringLiteral("待处理工单"), QString::number(woPend), "#b45309", scrollContent));
  statsRow->addWidget(mkStatCard(QStringLiteral("事件总数"), QString::number(evTotal), "#15803d", scrollContent));
  statsRow->addWidget(mkStatCard(QStringLiteral("待审核事件"), QString::number(evPend), "#b91c1c", scrollContent));
  statsRow->addWidget(mkStatCard(QStringLiteral("服务订单"), QString::number(svcTotal), "#64748b", scrollContent));
  statsRow->addWidget(mkStatCard(QStringLiteral("公告数量"), QString::number(annTotal), "#64748b", scrollContent));
  mainLay->addLayout(statsRow);
  mainLay->addSpacing(16);

  // ===== Task 18: 柱状图时间跨度切换 =====
  auto *spanRow = new QHBoxLayout();
  spanRow->setSpacing(8);
  auto *spanLabel = new QLabel(QStringLiteral("时间跨度："), scrollContent);
  spanLabel->setStyleSheet("color:#64748b;font-size:13px;");
  spanRow->addWidget(spanLabel);
  auto *btn7 = new QPushButton(QStringLiteral("7天"), scrollContent);
  auto *btn30 = new QPushButton(QStringLiteral("30天"), scrollContent);
  auto *btn90 = new QPushButton(QStringLiteral("90天"), scrollContent);
  for (auto *b : {btn7, btn30, btn90})
  {
    b->setCheckable(true);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(
        "QPushButton { background:#fff; color:#64748b; border:1px solid #D4D0C8;"
        " border-radius:4px; padding:6px 16px; font-size:13px; min-height:32px; }"
        "QPushButton:checked { background:#b45309; color:#fff; border-color:#b45309; }"
        "QPushButton:hover:!checked { border-color:#b45309; color:#b45309; }");
    spanRow->addWidget(b);
  }
  btn7->setChecked(true);
  spanRow->addStretch();
  mainLay->addLayout(spanRow);
  mainLay->addSpacing(8);

  // 柱状图容器（动态切换）
  auto *trendContainer = new QWidget(scrollContent);
  auto *trendLay = new QVBoxLayout(trendContainer);
  trendLay->setContentsMargins(0, 0, 0, 0);
  mainLay->addWidget(trendContainer);

  // 工单/事件双柱状图刷新函数
  std::function<void(int)> refreshTrendChart = [trendLay, trendContainer](int days)
  {
    // 清空旧图表
    QLayoutItem *item;
    while ((item = trendLay->takeAt(0)) != nullptr)
    {
      if (item->widget())
        item->widget()->deleteLater();
      delete item;
    }

    auto *chart = new QChart();
    QString title = (days == 7) ? QStringLiteral("近7天工单/事件趋势（按天）") : (days == 30) ? QStringLiteral("近30天工单/事件趋势（按天）")
                                                                                              : QStringLiteral("近90天工单/事件趋势（按周）");
    chart->setTitle(title);
    chart->setBackgroundVisible(false);
    QFont titleFont = chart->titleFont();
    titleFont.setPixelSize(14);
    titleFont.setBold(true);
    chart->setTitleFont(titleFont);
    chart->setTitleBrush(QBrush(QColor("#141413")));

    auto *woSet = new QBarSet(QStringLiteral("工单数"));
    woSet->setColor(QColor("#b45309"));
    auto *evSet = new QBarSet(QStringLiteral("事件数"));
    evSet->setColor(QColor("#15803d"));

    QStringList cats;
    if (days == 90)
    {
      // 按周分组，近13周
      QSqlQuery woQ(DatabaseManager::instance().database());
      woQ.prepare("SELECT strftime('%Y-W%W', create_time) as wk, COUNT(*) FROM wo_work_order "
                  "WHERE create_time >= date('now', '-90 days') AND del_flag = 0 GROUP BY wk ORDER BY wk");
      woQ.exec();
      QMap<QString, int> woMap;
      while (woQ.next())
        woMap[woQ.value(0).toString()] = woQ.value(1).toInt();
      QSqlQuery evQ(DatabaseManager::instance().database());
      evQ.prepare("SELECT strftime('%Y-W%W', create_time) as wk, COUNT(*) FROM ge_event "
                  "WHERE create_time >= date('now', '-90 days') AND del_flag = 0 GROUP BY wk ORDER BY wk");
      evQ.exec();
      QMap<QString, int> evMap;
      while (evQ.next())
        evMap[evQ.value(0).toString()] = evQ.value(1).toInt();
      // 合并所有周
      QSet<QString> allKeys;
      for (auto it = woMap.begin(); it != woMap.end(); ++it)
        allKeys.insert(it.key());
      for (auto it = evMap.begin(); it != evMap.end(); ++it)
        allKeys.insert(it.key());
      QStringList sortedKeys = allKeys.values();
      sortedKeys.sort();
      for (const auto &k : sortedKeys)
      {
        cats << k.mid(5); // W##
        woSet->append(woMap.value(k, 0));
        evSet->append(evMap.value(k, 0));
      }
    }
    else
    {
      // 按天分组
      QSqlQuery woQ(DatabaseManager::instance().database());
      woQ.prepare(QString("SELECT DATE(create_time) as dt, COUNT(*) FROM wo_work_order "
                          "WHERE create_time >= DATE('now', '-%1 days') AND del_flag = 0 GROUP BY dt ORDER BY dt")
                      .arg(days));
      woQ.exec();
      QMap<QString, int> woMap;
      while (woQ.next())
        woMap[woQ.value(0).toString()] = woQ.value(1).toInt();
      QSqlQuery evQ(DatabaseManager::instance().database());
      evQ.prepare(QString("SELECT DATE(create_time) as dt, COUNT(*) FROM ge_event "
                          "WHERE create_time >= DATE('now', '-%1 days') AND del_flag = 0 GROUP BY dt ORDER BY dt")
                      .arg(days));
      evQ.exec();
      QMap<QString, int> evMap;
      while (evQ.next())
        evMap[evQ.value(0).toString()] = evQ.value(1).toInt();
      QSet<QString> allKeys;
      for (auto it = woMap.begin(); it != woMap.end(); ++it)
        allKeys.insert(it.key());
      for (auto it = evMap.begin(); it != evMap.end(); ++it)
        allKeys.insert(it.key());
      QStringList sortedKeys = allKeys.values();
      sortedKeys.sort();
      for (const auto &k : sortedKeys)
      {
        cats << k.mid(5); // MM-DD
        woSet->append(woMap.value(k, 0));
        evSet->append(evMap.value(k, 0));
      }
    }

    auto *series = new QBarSeries();
    series->append(woSet);
    series->append(evSet);
    chart->addSeries(series);

    if (!cats.isEmpty())
    {
      auto *axisX = new QCategoryAxis();
      for (int i = 0; i < cats.size(); ++i)
        axisX->append(cats.at(i), i + 1);
      axisX->setRange(0, cats.size());
      axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
      chart->addAxis(axisX, Qt::AlignBottom);
      series->attachAxis(axisX);

      auto *axisY = new QValueAxis();
      axisY->setLabelFormat("%.0f");
      chart->addAxis(axisY, Qt::AlignLeft);
      series->attachAxis(axisY);

      QFont axFont = axisX->labelsFont();
      axFont.setPixelSize(12);
      axisX->setLabelsFont(axFont);
      axisY->setLabelsFont(axFont);
      axisX->setLabelsColor(QColor("#64748b"));
      axisY->setLabelsColor(QColor("#64748b"));
      axisY->setGridLineColor(QColor("#D4D0C8"));
      axisX->setGridLineVisible(false);
    }

    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    QFont lgFont = chart->legend()->font();
    lgFont.setPixelSize(12);
    chart->legend()->setFont(lgFont);

    auto *view = new QChartView(chart, trendContainer);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(320);
    view->setStyleSheet("QChartView{background:#fff;border-radius:2px;border:1px solid #D4D0C8;}");
    trendLay->addWidget(view);
  };

  // 初始加载 7 天
  refreshTrendChart(7);

  QObject::connect(btn7, &QPushButton::clicked, page, [refreshTrendChart]()
          { refreshTrendChart(7); });
  QObject::connect(btn30, &QPushButton::clicked, page, [refreshTrendChart]()
          { refreshTrendChart(30); });
  QObject::connect(btn90, &QPushButton::clicked, page, [refreshTrendChart]()
          { refreshTrendChart(90); });

  mainLay->addSpacing(16);

  // Row 1: Work order status pie + Service status pie
  auto *chartRow1 = new QHBoxLayout();
  chartRow1->setSpacing(16);

  // Pie chart: work order status
  auto *woStsView = mkPieChart(QStringLiteral("工单状态分布"), "SELECT status, COUNT(*) FROM wo_work_order WHERE del_flag = 0 GROUP BY status", [](int s)
                               { return WorkOrderStatus::label(s); }, scrollContent);
  chartRow1->addWidget(woStsView);

  auto *svcStsView = mkPieChart(QStringLiteral("服务订单状态"), "SELECT status, COUNT(*) FROM sv_service_order WHERE del_flag = 0 GROUP BY status", [](int s)
                                { return ServiceOrderStatus::label(s); }, scrollContent);
  chartRow1->addWidget(svcStsView);
  mainLay->addLayout(chartRow1);
  mainLay->addSpacing(16);

  // Row 2: Event category pie
  auto *chartRow2 = new QHBoxLayout();
  chartRow2->setSpacing(16);
  auto *evCatView = mkPieChart(QStringLiteral("事件类别分布"), "SELECT event_category, COUNT(*) FROM ge_event WHERE del_flag = 0 GROUP BY event_category", [](int c)
                               { return EventCategory::label(c); }, scrollContent);
  chartRow2->addWidget(evCatView);
  chartRow2->addStretch();
  mainLay->addLayout(chartRow2);
  mainLay->addSpacing(16);

  // ===== Task 18: 网格员绩效排名表 =====
  auto *perfTitle = new QLabel(QStringLiteral("网格员绩效排名 Top 10"), scrollContent);
  perfTitle->setStyleSheet("font-size: 14px; font-weight: 600;color:#141413;padding:4px 0;");
  mainLay->addWidget(perfTitle);

  auto *perfTable = new QTableWidget(scrollContent);
  UiKit::configureTable(perfTable);
  perfTable->setColumnCount(6);
  perfTable->setHorizontalHeaderLabels({QStringLiteral("排名"), QStringLiteral("网格员"),
                                        QStringLiteral("处理数"), QStringLiteral("完结数"), QStringLiteral("完结率"), QStringLiteral("平均时长(h)")});
  perfTable->setAlternatingRowColors(true);
  perfTable->horizontalHeader()->setStretchLastSection(true);
  perfTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  perfTable->setStyleSheet(UiKit::TABLE_STYLE);
  perfTable->setShowGrid(false);
  perfTable->verticalHeader()->setVisible(false);
  perfTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  perfTable->setMinimumHeight(360);

  {
    // ge_event 表无 handler_name 字段，改用 ge_event_flow.operator_name 统计实际处理人绩效
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT t.operator_name, t.total, t.done, t.avg_hours FROM ("
              " SELECT ef.operator_name AS operator_name, "
              "  COUNT(DISTINCT ef.event_id) AS total, "
              "  COUNT(DISTINCT CASE WHEN e.status = 4 THEN ef.event_id END) AS done, "
              "  AVG(CASE WHEN e.finish_time IS NOT NULL "
              "   THEN (julianday(e.finish_time) - julianday(e.create_time)) * 24 END) AS avg_hours "
              " FROM ge_event_flow ef "
              " JOIN ge_event e ON e.id = ef.event_id "
              " WHERE e.del_flag = 0 AND ef.operator_name IS NOT NULL AND ef.operator_name != '' "
              "   AND ef.to_status IN (3, 4, 6) " // 处理中/已完成/已归档 的操作算作处理动作
              " GROUP BY ef.operator_name "
              ") t ORDER BY t.done DESC LIMIT 10");
    q.exec();
    int r = 0;
    while (q.next())
    {
      perfTable->insertRow(r);
      auto *rankItem = new QTableWidgetItem(QString::number(r + 1));
      if (r < 3)
      {
        rankItem->setBackground(QColor("#fff7e6"));
        rankItem->setForeground(QColor("#b45309"));
      }
      perfTable->setItem(r, 0, rankItem);
      perfTable->setItem(r, 1, new QTableWidgetItem(q.value(0).toString()));
      perfTable->setItem(r, 2, new QTableWidgetItem(q.value(1).toString()));
      perfTable->setItem(r, 3, new QTableWidgetItem(q.value(2).toString()));
      int total = q.value(1).toInt();
      int done = q.value(2).toInt();
      double rate = total > 0 ? (done * 100.0 / total) : 0.0;
      perfTable->setItem(r, 4, new QTableWidgetItem(QString::number(rate, 'f', 1) + "%"));
      perfTable->setItem(r, 5, new QTableWidgetItem(QString::number(q.value(3).toDouble(), 'f', 1)));
      r++;
    }
  }
  perfTable->resizeColumnsToContents();
  mainLay->addWidget(perfTable);

  layout->addStretch();
  scrollArea->setWidget(content);
  outerLayout->addWidget(scrollArea);
  return page;
}
