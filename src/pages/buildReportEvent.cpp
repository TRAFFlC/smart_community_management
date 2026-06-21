#include "pages/PageFactory.h"

#include "PagesCommon.h"
#include "ReportPageHelper.h"

using namespace ReportPageHelper;

BasePage *PageFactory::buildReportEvent()
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

  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_chart"), QStringLiteral("事件统计分析"), QStringLiteral("网格事件类别、处理效率和趋势分析"), UiKit::moduleColor("event"), page));
  layout->addSpacing(12);

  // ===== Task 17: 事件统计增强 =====
  // 超时事件统计卡片
  auto *statsRow = new QHBoxLayout();
  statsRow->setSpacing(12);

  int evTotal = 0;
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(*) FROM ge_event WHERE del_flag = 0");
    q.exec();
    if (q.next())
      evTotal = q.value(0).toInt();
  }
  int timeoutCnt = 0;
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(*) FROM ge_event WHERE del_flag = 0 AND sla_deadline IS NOT NULL AND finish_time IS NOT NULL AND finish_time > sla_deadline");
    q.exec();
    if (q.next())
      timeoutCnt = q.value(0).toInt();
  }
  double timeoutRate = 0.0;
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(CASE WHEN finish_time > sla_deadline THEN 1 END) * 100.0 / COUNT(*) FROM ge_event WHERE finish_time IS NOT NULL AND sla_deadline IS NOT NULL AND del_flag = 0");
    q.exec();
    if (q.next())
      timeoutRate = q.value(0).toDouble();
  }
  int pendingCnt = 0;
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(*) FROM ge_event WHERE del_flag = 0 AND status NOT IN (4, 6)");
    q.exec();
    if (q.next())
      pendingCnt = q.value(0).toInt();
  }

  statsRow->addWidget(mkStatCard(QStringLiteral("事件总数"), QString::number(evTotal), "#b45309", page));
  statsRow->addWidget(mkStatCard(QStringLiteral("超时事件"), QString::number(timeoutCnt), "#b91c1c", page));
  statsRow->addWidget(mkStatCard(QStringLiteral("超时率"), QString::number(timeoutRate, 'f', 1) + "%", "#d97706", page));
  statsRow->addWidget(mkStatCard(QStringLiteral("未完结"), QString::number(pendingCnt), "#7c3aed", page));
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
  layout->addWidget(mkTrendLineChart(QStringLiteral("事件月度趋势（近6个月）"),
                                     "SELECT strftime('%Y-%m', create_time) as month, COUNT(*) FROM ge_event "
                                     "WHERE create_time >= date('now', '-6 months') AND del_flag = 0 "
                                     "GROUP BY month ORDER BY month",
                                     page));

  layout->addSpacing(12);

  // 处理效率柱状图：各状态平均停留时长（按 ge_event_flow 表计算）
  // 计算方式：对每个事件，相邻 flow 之间的 action_time 差值之和；按 to_status 分组取平均（小时）
  layout->addWidget(mkBarChart(QStringLiteral("各状态平均停留时长(h)"), "SELECT f.to_status, AVG(stay) FROM ("
                                                                        " SELECT event_id, to_status, "
                                                                        "  (julianday(COALESCE(LEAD(action_time) OVER (PARTITION BY event_id ORDER BY action_time), "
                                                                        "   COALESCE((SELECT MAX(finish_time) FROM ge_event e WHERE e.id = ef.event_id), action_time)) "
                                                                        "   - julianday(action_time)) * 24 AS stay "
                                                                        " FROM ge_event_flow ef"
                                                                        ") f WHERE stay >= 0 GROUP BY f.to_status",
                               [](int s)
                               { return EventStatus::label(s); }, QStringLiteral("平均时长"), page));

  layout->addSpacing(12);

  // 保留原有饼图：事件类别分布
  layout->addWidget(mkPieChart(QStringLiteral("事件类别分布"), "SELECT event_category, COUNT(*) FROM ge_event WHERE del_flag = 0 GROUP BY event_category", [](int c)
                               { return EventCategory::label(c); }, page));

  // 导出按钮
  QObject::connect(exportBtn, &QPushButton::clicked, page, [page]()
          {
          auto* dlg = new QDialog(page);
          dlg->setWindowTitle(QStringLiteral("导出事件数据"));
          dlg->setMinimumWidth(700);
          auto* dl = new QVBoxLayout(dlg);
          auto* table = new QTableWidget(dlg);
          table->setColumnCount(6);
          table->setHorizontalHeaderLabels({QStringLiteral("事件编号"), QStringLiteral("标题"),
              QStringLiteral("类别"), QStringLiteral("状态"), QStringLiteral("创建时间"), QStringLiteral("完成时间")});
          QSqlQuery q(DatabaseManager::instance().database());
          q.prepare("SELECT event_no, title, event_category, status, create_time, finish_time FROM ge_event WHERE del_flag = 0 ORDER BY create_time DESC");
          q.exec();
          int r = 0;
          while (q.next()) {
              table->insertRow(r);
              table->setItem(r, 0, new QTableWidgetItem(q.value(0).toString()));
              table->setItem(r, 1, new QTableWidgetItem(q.value(1).toString()));
              table->setItem(r, 2, new QTableWidgetItem(EventCategory::label(q.value(2).toInt())));
              table->setItem(r, 3, new QTableWidgetItem(EventStatus::label(q.value(3).toInt())));
              table->setItem(r, 4, new QTableWidgetItem(q.value(4).toDateTime().toString("yyyy-MM-dd HH:mm")));
              table->setItem(r, 5, new QTableWidgetItem(q.value(5).isNull() ? "" : q.value(5).toDateTime().toString("yyyy-MM-dd HH:mm")));
              r++;
          }
          table->resizeColumnsToContents();
          dl->addWidget(new QLabel(QStringLiteral("共 %1 条事件记录，点击下方按钮选择保存路径：").arg(r)));
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
              UiKit::exportTableToCsv(table, QStringLiteral("事件统计.csv"), dlg);
          });
          dlg->exec();
          dlg->deleteLater(); });

  layout->addStretch();
  scrollArea->setWidget(content);
  outerLayout->addWidget(scrollArea);
  return page;
}
