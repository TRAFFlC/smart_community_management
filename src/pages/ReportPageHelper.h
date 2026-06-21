#ifndef REPORT_PAGE_HELPER_H
#define REPORT_PAGE_HELPER_H

#include "PagesCommon.h"

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

namespace ReportPageHelper {

// 创建统计卡片（数值+标题+顶部色条）
inline QFrame *mkStatCard(const QString &title, const QString &val, const QString &color, QWidget *parent)
{
  return UiKit::createStatCard(title, val, color, parent);
}

// 创建折线图（月度趋势）
inline QChartView *mkTrendLineChart(const QString &title, const QString &sql, QWidget *parent)
{
  auto *chart = new QChart();
  chart->setTitle(title);
  chart->setBackgroundVisible(false);
  QFont titleFont = chart->titleFont();
  titleFont.setPixelSize(14);
  titleFont.setBold(true);
  chart->setTitleFont(titleFont);
  chart->setTitleBrush(QBrush(QColor("#141413")));

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
  chart->legend()->setVisible(false);

  auto *view = new QChartView(chart, parent);
  view->setRenderHint(QPainter::Antialiasing);
  view->setMinimumHeight(280);
  view->setStyleSheet("QChartView{background:#fff;border-radius:2px;border:1px solid #D4D0C8;}");
  return view;
}

// 创建柱状图（分类计数）
inline QChartView *mkBarChart(const QString &title, const QString &sql,
                              const std::function<QString(int)> &labelFn, const QString &barLabel,
                              QWidget *parent)
{
  auto *chart = new QChart();
  chart->setTitle(title);
  chart->setBackgroundVisible(false);
  QFont titleFont = chart->titleFont();
  titleFont.setPixelSize(14);
  titleFont.setBold(true);
  chart->setTitleFont(titleFont);
  chart->setTitleBrush(QBrush(QColor("#141413")));

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
    axisY->setGridLineColor(QColor("#D4D0C8"));
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
  view->setStyleSheet("QChartView{background:#fff;border-radius:2px;border:1px solid #D4D0C8;}");
  return view;
}

// 创建饼图（保留原有交互）
inline QChartView *mkPieChart(const QString &title, const QString &sql,
                              const std::function<QString(int)> &labelFn, QWidget *parent)
{
  auto *chart = new QChart();
  chart->setTitle(title);
  chart->setBackgroundVisible(false);
  QFont titleFont = chart->titleFont();
  titleFont.setPixelSize(14);
  titleFont.setBold(true);
  chart->setTitleFont(titleFont);
  chart->setTitleBrush(QBrush(QColor("#141413")));

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
  view->setStyleSheet("QChartView{background:#fff;border-radius:2px;border:1px solid #D4D0C8;}");
  return view;
}

// 创建导出按钮
inline QPushButton *mkExportBtn(QWidget *parent)
{
  auto *btn = new QPushButton(QStringLiteral("导出 CSV"), parent);
  btn->setStyleSheet(
      "QPushButton { background: #ffffff; color: #b45309; border: 1px solid #b45309;"
      " border-radius: 4px; padding: 7px 16px; font-size: 14px; min-height: 36px; }"
      "QPushButton:hover { background: #e6f4ff; border-color: #d97706; color: #d97706; }"
      "QPushButton:pressed { background: #bae0ff; }");
  btn->setCursor(Qt::PointingHandCursor);
  return btn;
}

} // namespace ReportPageHelper

#endif // REPORT_PAGE_HELPER_H
