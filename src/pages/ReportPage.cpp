#include "pages/PageFactory.h"

#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QLineSeries>
#include <QScatterSeries>
#include <QCategoryAxis>
#include <QValueAxis>
#include <QtCharts>

#include "PagesCommon.h"

// ========== Report Pages ==========
BasePage *PageFactory::createReportPage(const QString &sub)
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

  auto &db = DatabaseManager::instance();

  // ===== 通用辅助：创建统计卡片（数值+标题+顶部色条） =====
  auto mkStatCard = [](const QString &title, const QString &val, const QString &color, QWidget *parent)
  {
    auto *card = new QFrame(parent);
    card->setFixedHeight(90);
    card->setStyleSheet(QString("QFrame{background:#fff;border-radius:6px;border:1px solid #e2e8f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(16, 12, 16, 12);
    cl->setSpacing(4);
    auto *indicator = new QFrame(card);
    indicator->setFixedHeight(3);
    indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
    auto *tl = new QLabel(title);
    tl->setStyleSheet("color:#64748b;font-size:12px;");
    auto *vl = new QLabel(val);
    vl->setStyleSheet(QString("color:%1;font-size:24px;font-weight:bold;").arg(color));
    cl->addWidget(indicator);
    cl->addWidget(tl);
    cl->addWidget(vl);
    UiKit::applyCardShadow(card);
    return card;
  };

  // ===== 通用辅助：创建折线图（月度趋势） =====
  auto mkTrendLineChart = [](const QString &title, const QString &sql, QWidget *parent)
  {
    auto *chart = new QChart();
    chart->setTitle(title);
    chart->setBackgroundVisible(false);
    QFont titleFont = chart->titleFont();
    titleFont.setPixelSize(14);
    titleFont.setBold(true);
    chart->setTitleFont(titleFont);
    chart->setTitleBrush(QBrush(QColor("#0f172a")));

    auto *lineSeries = new QLineSeries();
    lineSeries->setColor(QColor("#b45309"));
    QPen pen(QColor("#b45309"));
    pen.setWidth(2);
    lineSeries->setPen(pen);

    auto *scatter = new QScatterSeries();
    scatter->setColor(QColor("#b45309"));
    scatter->setMarkerSize(8.0);
    scatter->setBorderColor(QColor("#b45309"));

    QStringList cats;
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(sql);
    q.exec();
    int idx = 0;
    while (q.next())
    {
      QString m = q.value(0).toString();
      QString label = m.length() >= 7 ? m.mid(5) : m; // MM
      cats << label;
      lineSeries->append(idx, q.value(1).toDouble());
      scatter->append(idx, q.value(1).toDouble());
      ++idx;
    }
    chart->addSeries(lineSeries);
    chart->addSeries(scatter);

    if (!cats.isEmpty())
    {
      auto *axisX = new QCategoryAxis();
      for (int i = 0; i < cats.size(); ++i)
        axisX->append(cats.at(i), i);
      axisX->setRange(0, qMax(0, cats.size() - 1));
      axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
      chart->addAxis(axisX, Qt::AlignBottom);
      lineSeries->attachAxis(axisX);
      scatter->attachAxis(axisX);

      auto *axisY = new QValueAxis();
      axisY->setLabelFormat("%.0f");
      chart->addAxis(axisY, Qt::AlignLeft);
      lineSeries->attachAxis(axisY);
      scatter->attachAxis(axisY);

      // 样式：网格线、文字颜色
      QFont axFont = axisX->labelsFont();
      axFont.setPixelSize(12);
      axisX->setLabelsFont(axFont);
      axisY->setLabelsFont(axFont);
      axisX->setLabelsColor(QColor("#64748b"));
      axisY->setLabelsColor(QColor("#64748b"));
      axisY->setGridLineColor(QColor("#e2e8f0"));
      axisX->setGridLineVisible(false);
    }

    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(false);

    auto *view = new QChartView(chart, parent);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(280);
    view->setStyleSheet("QChartView{background:#fff;border-radius:6px;border:1px solid #e2e8f0;}");
    return view;
  };

  // ===== 通用辅助：创建柱状图（分类计数） =====
  auto mkBarChart = [](const QString &title, const QString &sql,
                       std::function<QString(int)> labelFn, const QString &barLabel,
                       QWidget *parent)
  {
    auto *chart = new QChart();
    chart->setTitle(title);
    chart->setBackgroundVisible(false);
    QFont titleFont = chart->titleFont();
    titleFont.setPixelSize(14);
    titleFont.setBold(true);
    chart->setTitleFont(titleFont);
    chart->setTitleBrush(QBrush(QColor("#0f172a")));

    auto *series = new QBarSeries();
    auto *set = new QBarSet(barLabel);
    set->setColor(QColor("#b45309"));
    QStringList cats;
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(sql);
    q.exec();
    while (q.next())
    {
      cats << labelFn(q.value(0).toInt());
      set->append(q.value(1).toDouble());
    }
    series->append(set);
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
      axisY->setGridLineColor(QColor("#e2e8f0"));
      axisX->setGridLineVisible(false);
    }

    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    QFont lgFont = chart->legend()->font();
    lgFont.setPixelSize(12);
    chart->legend()->setFont(lgFont);

    auto *view = new QChartView(chart, parent);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(280);
    view->setStyleSheet("QChartView{background:#fff;border-radius:6px;border:1px solid #e2e8f0;}");
    return view;
  };

  // ===== 通用辅助：创建饼图（保留原有交互） =====
  auto mkPieChart = [](const QString &title, const QString &sql,
                       std::function<QString(int)> labelFn, QWidget *parent)
  {
    auto *chart = new QChart();
    chart->setTitle(title);
    chart->setBackgroundVisible(false);
    QFont titleFont = chart->titleFont();
    titleFont.setPixelSize(14);
    titleFont.setBold(true);
    chart->setTitleFont(titleFont);
    chart->setTitleBrush(QBrush(QColor("#0f172a")));

    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(sql);
    q.exec();
    auto *series = new QPieSeries();
    QStringList piePalette = {"#b45309", "#2563eb", "#15803d", "#64748b", "#cbd5e1", "#d97706", "#0d9488", "#7c3aed"};
    int colorIdx = 0;
    while (q.next())
    {
      auto *slice = series->append(labelFn(q.value(0).toInt()), q.value(1).toInt());
      slice->setColor(QColor(piePalette[colorIdx % piePalette.size()]));
      ++colorIdx;
    }
    chart->addSeries(series);
    for (auto *slice : series->slices())
    {
      slice->setLabelVisible(true);
      slice->setLabel(QString("%1 %2%").arg(slice->label()).arg(slice->percentage() * 100, 0, 'f', 1));
      slice->setLabelPosition(QPieSlice::LabelOutside);
    }
    QObject::connect(series, &QPieSeries::hovered, [series](QPieSlice *slice, bool state)
            {
            if (state) {
                slice->setExploded(true);
                QToolTip::showText(QCursor::pos(),
                    QString("%1: %2 (%3%)").arg(slice->label()).arg(slice->value()).arg(slice->percentage() * 100, 0, 'f', 1));
            } else {
                slice->setExploded(false);
                QToolTip::hideText();
            } });
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    auto *view = new QChartView(chart, parent);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(280);
    view->setStyleSheet("QChartView{background:#fff;border-radius:6px;border:1px solid #e2e8f0;}");
    return view;
  };

  // ===== 通用辅助：导出按钮 =====
  auto mkExportBtn = [](QWidget *parent)
  {
    auto *btn = new QPushButton(QStringLiteral("导出 CSV"), parent);
    btn->setStyleSheet(
        "QPushButton { background: #ffffff; color: #b45309; border: 1px solid #b45309;"
        " border-radius: 4px; padding: 7px 16px; font-size: 14px; min-height: 36px; }"
        "QPushButton:hover { background: #e6f4ff; border-color: #d97706; color: #d97706; }"
        "QPushButton:pressed { background: #bae0ff; }");
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
  };

  if (sub == "workorder")
  {
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_chart"), QStringLiteral("工单统计分析"), QStringLiteral("工单类型、趋势和状态多维度数据分析"), UiKit::moduleColor(sub), page));
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
  }
  else if (sub == "event")
  {
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_chart"), QStringLiteral("事件统计分析"), QStringLiteral("网格事件类别、处理效率和趋势分析"), UiKit::moduleColor(sub), page));
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
  }
  else if (sub == "service")
  {
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_chart"), QStringLiteral("服务统计分析"), QStringLiteral("社区服务订单状态和趋势数据分析"), UiKit::moduleColor(sub), page));
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
  }
  else if (sub == "dashboard")
  {
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
    statsRow->addWidget(mkStatCard(QStringLiteral("待处理工单"), QString::number(woPend), "#d97706", scrollContent));
    statsRow->addWidget(mkStatCard(QStringLiteral("事件总数"), QString::number(evTotal), "#15803d", scrollContent));
    statsRow->addWidget(mkStatCard(QStringLiteral("待审核事件"), QString::number(evPend), "#b91c1c", scrollContent));
    statsRow->addWidget(mkStatCard(QStringLiteral("服务订单"), QString::number(svcTotal), "#7c3aed", scrollContent));
    statsRow->addWidget(mkStatCard(QStringLiteral("公告数量"), QString::number(annTotal), "#0d9488", scrollContent));
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
          "QPushButton { background:#fff; color:#64748b; border:1px solid #e2e8f0;"
          " border-radius:4px; padding:6px 16px; font-size:13px; min-height:32px; }"
          "QPushButton:checked { background:#b45309; color:#fff; border-color:#b45309; }"
          "QPushButton:hover:!checked { border-color:#d97706; color:#d97706; }");
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
      chart->setTitleBrush(QBrush(QColor("#0f172a")));

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
        axisY->setGridLineColor(QColor("#e2e8f0"));
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
      view->setStyleSheet("QChartView{background:#fff;border-radius:6px;border:1px solid #e2e8f0;}");
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
    perfTitle->setStyleSheet("font-size: 14px; font-weight: 600;color:#0f172a;padding:4px 0;");
    mainLay->addWidget(perfTitle);

    auto *perfTable = new QTableWidget(scrollContent);
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
          rankItem->setForeground(QColor("#d97706"));
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
  }

  layout->addStretch();
  scrollArea->setWidget(content);
  outerLayout->addWidget(scrollArea);
  return page;
}


