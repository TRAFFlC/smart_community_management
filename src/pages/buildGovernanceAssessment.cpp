#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

BasePage *PageFactory::buildGovernanceAssessment()
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  // Page header
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_star"), QStringLiteral("考核管理"), QStringLiteral("考核指标设置、完成率统计和排名展示"), UiKit::moduleColor("assessment"), page));
  layout->addSpacing(12);

  // Stats
  auto *statsRow = new QHBoxLayout();
  auto createStatCard = [](const QString &label, const QString &val, const QString &color, QWidget *parent)
  {
    auto *card = new QFrame(parent);
    card->setFixedHeight(90);
    card->setStyleSheet(QString("QFrame{background:#fff;border-radius:2px;border:1px solid #D4D0C8;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(16, 10, 16, 10);
    cl->setSpacing(4);
    auto *indicator = new QFrame(card);
    indicator->setFixedHeight(3);
    indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
    auto *tl = new QLabel(label);
    tl->setStyleSheet("color:#64748b;font-size:12px;");
    auto *vl = new QLabel(val);
    vl->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(color));
    cl->addWidget(indicator);
    cl->addWidget(tl);
    cl->addWidget(vl);
    return card;
  };
  QSqlQuery asmtCntQ("SELECT COUNT(DISTINCT config_id) FROM kf_assessment_config");
  int asmtCnt = asmtCntQ.next() ? asmtCntQ.value(0).toInt() : 0;
  QSqlQuery avgRateQ("SELECT AVG(actual_value) FROM kf_assessment_result WHERE del_flag = 0");
  double avgRate = avgRateQ.next() ? avgRateQ.value(0).toDouble() : 0;
  QSqlQuery excellentQ("SELECT COUNT(DISTINCT target_user_id) FROM kf_assessment_result WHERE del_flag = 0 AND score >= 90");
  int excellent = excellentQ.next() ? excellentQ.value(0).toInt() : 0;
  QSqlQuery timeoutQ("SELECT COUNT(*) FROM ge_event WHERE del_flag = 0 AND sla_deadline IS NOT NULL AND finish_time > sla_deadline");
  int timeout = timeoutQ.next() ? timeoutQ.value(0).toInt() : 0;
  statsRow->addWidget(createStatCard(QStringLiteral("考核指标"), QString::number(asmtCnt), "#b45309", page));
  statsRow->addWidget(createStatCard(QStringLiteral("平均完成率"), QString::number(qRound(avgRate)) + "%", "#15803d", page));
  statsRow->addWidget(createStatCard(QStringLiteral("优秀网格员"), QString::number(excellent), "#d97706", page));
  statsRow->addWidget(createStatCard(QStringLiteral("超时事件"), QString::number(timeout), "#b91c1c", page));
  layout->addLayout(statsRow);
  layout->addSpacing(12);

  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  tbLayout->setSpacing(10);
  auto *searchEdit = new QLineEdit(toolbar);
  searchEdit->setPlaceholderText(QStringLiteral("搜索网格员姓名..."));
  searchEdit->setMinimumWidth(200);
  searchEdit->setClearButtonEnabled(true);
  tbLayout->addWidget(searchEdit);
  auto *periodCombo = new QComboBox(toolbar);
  periodCombo->addItem(QStringLiteral("全部周期"), QString());
  // Task 19: 动态查询周期 - 从 kf_assessment_config.assessment_period 查询
  {
    QSqlQuery periodQ(DatabaseManager::instance().database());
    periodQ.prepare("SELECT DISTINCT assessment_period FROM kf_assessment_config "
                    "WHERE assessment_period IS NOT NULL AND assessment_period != '' AND del_flag = 0 "
                    "ORDER BY assessment_period DESC");
    periodQ.exec();
    bool hasData = false;
    while (periodQ.next())
    {
      QString p = periodQ.value(0).toString();
      // 转换 "2026-06" 为 "2026年6月"
      QString display;
      if (p.length() >= 7)
      {
        int year = p.mid(0, 4).toInt();
        int month = p.mid(5, 2).toInt();
        if (year > 0 && month > 0 && month <= 12)
        {
          display = QStringLiteral("%1年%2月").arg(year).arg(month);
        }
      }
      if (display.isEmpty())
        display = p;
      periodCombo->addItem(display, p);
      hasData = true;
    }
    // 如果没有数据，显示当前月份和前两个月作为默认
    if (!hasData)
    {
      QDate today = QDate::currentDate();
      for (int i = 0; i < 3; ++i)
      {
        QDate d = today.addMonths(-i);
        QString p = QString("%1-%2").arg(d.year()).arg(d.month(), 2, 10, QChar('0'));
        QString display = QStringLiteral("%1年%2月").arg(d.year()).arg(d.month());
        periodCombo->addItem(display, p);
      }
    }
  }
  periodCombo->setMinimumWidth(120);
  tbLayout->addWidget(periodCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  // Ranking table
  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  UiKit::configureTable(table);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(UiKit::TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setSortingEnabled(true);
  table->setColumnCount(6);
  table->setHorizontalHeaderLabels({QStringLiteral("排名"), QStringLiteral("网格员"), QStringLiteral("处理事件"), QStringLiteral("平均时效(h)"), QStringLiteral("完成率"), QStringLiteral("评分")});
  auto *emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无考核记录"), page);
  auto *pb = new PaginationBar(page);
  layout->addWidget(pb);
  std::function<void()> loadAssessments = [table, searchEdit, periodCombo, emptyHint, pb]()
  {
    table->setSortingEnabled(false);
    table->setRowCount(0);
    QString searchText = searchEdit->text().trimmed();
    QString periodFilter = periodCombo->currentData().toString();
    UiKit::PagedQuery pq("SELECT ar.target_user_id, u.real_name, COUNT(*) as event_count, "
                  "AVG(ar.actual_value) as avg_actual, AVG(ar.score) as avg_score, MIN(ar.rank) as best_rank "
                  "FROM kf_assessment_result ar "
                  "LEFT JOIN sys_user u ON ar.target_user_id = u.id "
                  "WHERE ar.del_flag = 0");
    if (!searchText.isEmpty())
      pq.where("u.real_name LIKE :search", {{"search", "%" + searchText + "%"}});
    if (!periodFilter.isEmpty())
      pq.where("ar.period = :period", {{"period", periodFilter}});
    pq.where("1=1 GROUP BY ar.target_user_id");
    pq.orderBy("avg_score DESC");

    auto result = pq.execute(pb->pageSize(), pb->offset());
    pb->setTotalCount(result.totalCount);
    int rRow = 0;
    for (const auto& row : result.rows)
    {
      table->insertRow(rRow);
      auto *rankItem = new QTableWidgetItem(QString::number(rRow + 1));
      if (rRow < 3)
      {
        rankItem->setBackground(QColor("#fff7e6"));
        rankItem->setForeground(QColor("#d97706"));
      }
      table->setItem(rRow, 0, rankItem);
      table->setItem(rRow, 1, new QTableWidgetItem(row.value(1).toString()));
      table->setItem(rRow, 2, new QTableWidgetItem(row.value(2).toString()));
      table->setItem(rRow, 3, new QTableWidgetItem(QString::number(row.value(3).toDouble(), 'f', 1)));
      double avgActual = row.value(3).toDouble();
      table->setItem(rRow, 4, new QTableWidgetItem(QString::number(avgActual, 'f', 1) + "%"));
      int score = qRound(row.value(4).toDouble());
      auto *scoreItem = new QTableWidgetItem(QString::number(score));
      if (score >= 90)
      {
        scoreItem->setBackground(QColor("#f6ffed"));
        scoreItem->setForeground(QColor("#15803d"));
      }
      table->setItem(rRow, 5, scoreItem);
      rRow++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
    pb->refreshData();
    table->setSortingEnabled(true);
  };
  loadAssessments();
  QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
          { loadAssessments(); });
  QObject::connect(periodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadAssessments(); });

  layout->addWidget(table);
  layout->addWidget(emptyHint);
  return page;
}
