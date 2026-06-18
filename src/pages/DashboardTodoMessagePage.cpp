#include "pages/PageFactory.h"
#include "PagesCommon.h"

using namespace UiKit;
// ========== Dashboard Page ==========
BasePage *PageFactory::createDashboardPage()
{
  auto *page = new BasePage();
  auto *outerLayout = new QVBoxLayout(page);
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(0);

  auto *scrollArea = new QScrollArea(page);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setStyleSheet("QScrollArea { border: none; background: #f8fafc; }");

  auto *content = new QWidget();
  auto *layout = new QVBoxLayout(content);
  layout->setContentsMargins(24, 24, 24, 24);
  layout->setSpacing(24);

  const auto &user = AuthService::instance().currentUser();
  QString greeting = user.nickname.isEmpty() ? user.username : user.nickname;

  // Data-scope filter for dashboard: residents (data_scope >= 6) only see their own reported data.
  const int dataScope = AuthService::instance().currentUserDataScope();
  const bool onlySelf = dataScope >= 6;
  const QString woScopeFilter = onlySelf ? QStringLiteral(" AND reporter_id = :scope_uid") : QString();
  const QString evScopeFilter = onlySelf ? QStringLiteral(" AND reporter_id = :scope_uid") : QString();

  // Welcome banner
  auto *welcomeLabel = new QLabel(QStringLiteral("欢迎回来，%1！").arg(greeting), page);
  welcomeLabel->setStyleSheet("font-size: 24px; font-weight: 700; color: #0f172a; background: transparent; border: none;");
  layout->addWidget(welcomeLabel);

  // Today key metrics
  auto *metricsArea = new QWidget(page);
  auto *metricsLayout = new QHBoxLayout(metricsArea);
  metricsLayout->setContentsMargins(0, 0, 0, 0);
  metricsLayout->setSpacing(48);

  auto createMetric = [](const QString &label, int value, QWidget *parent) -> QWidget *
  {
    auto *container = new QWidget(parent);
    auto *vl = new QVBoxLayout(container);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(4);
    vl->setAlignment(Qt::AlignCenter);

    auto *numLabel = new QLabel(QString::number(value), container);
    numLabel->setStyleSheet("font-size: 32px; font-weight: 700; color: #0f172a; background: transparent; border: none;");
    numLabel->setAlignment(Qt::AlignCenter);

    auto *textLabel = new QLabel(label, container);
    textLabel->setStyleSheet("font-size: 13px; font-weight: 400; color: #64748b; background: transparent; border: none;");
    textLabel->setAlignment(Qt::AlignCenter);

    vl->addWidget(numLabel);
    vl->addWidget(textLabel);
    return container;
  };

  QSqlQuery woPendQ;
  woPendQ.prepare(QStringLiteral("SELECT COUNT(*) FROM wo_work_order WHERE status IN (0,1,2,3) AND del_flag = 0%1").arg(woScopeFilter));
  if (onlySelf)
    woPendQ.bindValue(":scope_uid", user.id);
  int woPendCount = (woPendQ.exec() && woPendQ.next()) ? woPendQ.value(0).toInt() : 0;
  metricsLayout->addStretch();
  metricsLayout->addWidget(createMetric(QStringLiteral("待处理工单"), woPendCount, page));

  QSqlQuery evPendQ;
  evPendQ.prepare(QStringLiteral("SELECT COUNT(*) FROM ge_event WHERE status = 0 AND del_flag = 0%1").arg(evScopeFilter));
  if (onlySelf)
    evPendQ.bindValue(":scope_uid", user.id);
  int evPendCount = (evPendQ.exec() && evPendQ.next()) ? evPendQ.value(0).toInt() : 0;
  metricsLayout->addWidget(createMetric(QStringLiteral("待审核事件"), evPendCount, page));

  QSqlQuery unreadQ;
  unreadQ.prepare("SELECT COUNT(*) FROM nt_notification WHERE user_id = :uid AND is_read = 0");
  unreadQ.bindValue(":uid", user.id);
  int unreadCount = (unreadQ.exec() && unreadQ.next()) ? unreadQ.value(0).toInt() : 0;
  metricsLayout->addWidget(createMetric(QStringLiteral("未读通知"), unreadCount, page));
  metricsLayout->addStretch();

  auto *metricsWrapper = new QVBoxLayout();
  metricsWrapper->setContentsMargins(0, 0, 0, 0);
  metricsWrapper->setSpacing(12);
  metricsWrapper->addWidget(metricsArea);

  auto *metricsDivider = new QWidget(page);
  metricsDivider->setFixedHeight(1);
  metricsDivider->setStyleSheet("background: #e2e8f0; border: none;");
  metricsWrapper->addWidget(metricsDivider);

  layout->addLayout(metricsWrapper);

  // Quick actions: 3-column icon grid
  auto *quickArea = new QWidget(page);
  auto *quickLayout = new QVBoxLayout(quickArea);
  quickLayout->setContentsMargins(0, 0, 0, 0);
  quickLayout->setSpacing(16);

  auto *quickTitle = new QLabel(QStringLiteral("快捷操作"), page);
  quickTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a; background: transparent; border: none;");
  quickLayout->addWidget(quickTitle);

  auto *quickGrid = new QGridLayout();
  quickGrid->setSpacing(0);
  quickGrid->setHorizontalSpacing(24);
  quickGrid->setVerticalSpacing(24);
  quickGrid->setColumnStretch(0, 1);
  quickGrid->setColumnStretch(1, 1);
  quickGrid->setColumnStretch(2, 1);

  struct QuickAction
  {
    QString label;
    QString iconKey;
    QString target;
  };
  QuickAction actions[] = {
      {QStringLiteral("报事报修"), QStringLiteral("ic_tool"), "301"},
      {QStringLiteral("投诉建议"), QStringLiteral("ic_chat"), "302"},
      {QStringLiteral("公告通知"), QStringLiteral("ic_announce"), "304"},
      {QStringLiteral("志愿服务"), QStringLiteral("ic_people"), "501"},
      {QStringLiteral("网格事件"), QStringLiteral("ic_route"), "401"},
      {QStringLiteral("物业缴费"), QStringLiteral("ic_money"), "308"},
  };

  for (int i = 0; i < 6; ++i)
  {
    auto *item = new QFrame(quickArea);
    item->setFrameStyle(QFrame::NoFrame);
    item->setCursor(Qt::PointingHandCursor);
    item->setStyleSheet(
        "QFrame { background: transparent; border: none; }"
        "QLabel { background: transparent; border: none; }");
    item->setProperty("targetPage", actions[i].target);

    auto *itemLayout = new QVBoxLayout(item);
    itemLayout->setContentsMargins(0, 0, 0, 0);
    itemLayout->setSpacing(8);
    itemLayout->setAlignment(Qt::AlignCenter);

    auto *iconLabel = new QLabel(item);
    iconLabel->setFixedSize(32, 32);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setPixmap(tintSvgIcon(actions[i].iconKey, "#64748b", QSize(32, 32)));

    auto *textLabel = new QLabel(actions[i].label, item);
    textLabel->setStyleSheet("font-size: 13px; color: #334155; background: transparent; border: none;");
    textLabel->setAlignment(Qt::AlignCenter);

    itemLayout->addWidget(iconLabel);
    itemLayout->addWidget(textLabel);

    item->installEventFilter(new QuickHoverFilter(iconLabel, actions[i].iconKey, item));
    item->installEventFilter(page);

    quickGrid->addWidget(item, i / 3, i % 3);
  }

  quickLayout->addLayout(quickGrid);
  layout->addWidget(quickArea);

  // Two-column layout: todo (left) + dynamics & chart (right)
  auto *twoColLayout = new QHBoxLayout();
  twoColLayout->setSpacing(24);
  twoColLayout->setContentsMargins(0, 0, 0, 0);

  // === Left column: Todo list ===
  auto *leftCol = new QVBoxLayout();
  leftCol->setSpacing(16);

  auto *todoCard = new QFrame(page);
  todoCard->setStyleSheet(
      "QFrame { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 4px; }"
      "QLabel { background: transparent; border: none; }");
  auto *todoLayout = new QVBoxLayout(todoCard);
  todoLayout->setContentsMargins(20, 16, 20, 16);
  todoLayout->setSpacing(0);

  auto *todoTitle = new QLabel(QStringLiteral("待办事项"), todoCard);
  todoTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a; background: transparent; border: none;");
  todoLayout->addWidget(todoTitle);
  todoLayout->addSpacing(8);

  auto createTodoItem = [&](const QString &label, int count, const QString &color, const QString &target, bool last) -> QFrame *
  {
    auto *item = new QFrame(todoCard);
    item->setFrameStyle(QFrame::NoFrame);
    item->setCursor(Qt::PointingHandCursor);
    item->setFixedHeight(44);
    item->setProperty("targetPage", target);

    QString borderStyle = last ? QString() : QStringLiteral("border-bottom: 1px solid #e2e8f0;");
    item->setStyleSheet(QString(
                            "QFrame { background: transparent; border: none; %1 }"
                            "QFrame:hover { background: #f8fafc; }"
                            "QLabel { background: transparent; border: none; }")
                            .arg(borderStyle));

    auto *hl = new QHBoxLayout(item);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *dot = new QLabel(item);
    dot->setFixedSize(8, 8);
    dot->setStyleSheet(QString("background: %1; border-radius: 4px;").arg(color));

    auto *text = new QLabel(QStringLiteral("%1   %2 件").arg(label).arg(count), item);
    text->setStyleSheet("font-size: 14px; color: #334155; background: transparent; border: none;");
    text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *arrow = new QLabel(QStringLiteral("→"), item);
    arrow->setStyleSheet("font-size: 14px; color: #94a3b8; background: transparent; border: none;");

    hl->addWidget(dot);
    hl->addWidget(text);
    hl->addStretch();
    hl->addWidget(arrow);

    item->installEventFilter(page);
    return item;
  };

  todoLayout->addWidget(createTodoItem(QStringLiteral("待处理工单"), woPendCount, "#b45309", "301", false));
  todoLayout->addWidget(createTodoItem(QStringLiteral("待审核事件"), evPendCount, "#2563eb", "401", false));

  QSqlQuery annQ("SELECT COUNT(*) FROM nt_announcement WHERE del_flag = 0 AND date(publish_time) >= date('now', '-7 days')");
  int annCount = annQ.next() ? annQ.value(0).toInt() : 0;
  todoLayout->addWidget(createTodoItem(QStringLiteral("近期公告"), annCount, "#15803d", "304", true));
  todoLayout->addStretch();

  leftCol->addWidget(todoCard);

  // === Right column: Community dynamics + chart ===
  auto *rightCol = new QVBoxLayout();
  rightCol->setSpacing(16);

  // Community dynamics - timeline
  auto *dynamicCard = new QFrame(page);
  dynamicCard->setMinimumHeight(340);
  dynamicCard->setStyleSheet(
      "QFrame { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 4px; }"
      "QLabel { background: transparent; border: none; }");
  auto *dynLayout = new QVBoxLayout(dynamicCard);
  dynLayout->setContentsMargins(20, 16, 20, 16);
  dynLayout->setSpacing(0);

  auto *dynHeader = new QWidget(dynamicCard);
  dynHeader->setStyleSheet("background: transparent; border: none;");
  auto *dynHeaderLayout = new QHBoxLayout(dynHeader);
  dynHeaderLayout->setContentsMargins(0, 0, 0, 0);
  dynHeaderLayout->setSpacing(0);

  auto *dynTitle = new QLabel(QStringLiteral("社区动态"), dynHeader);
  dynTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a; background: transparent; border: none;");
  dynHeaderLayout->addWidget(dynTitle);
  dynHeaderLayout->addStretch();

  auto *viewAllBtn = new QPushButton(QStringLiteral("查看全部"), dynHeader);
  viewAllBtn->setFlat(true);
  viewAllBtn->setCursor(Qt::PointingHandCursor);
  viewAllBtn->setStyleSheet(
      "QPushButton { color: #b45309; border: none; background: transparent; font-size: 13px; padding: 0; }"
      "QPushButton:hover { text-decoration: underline; }");
  dynHeaderLayout->addWidget(viewAllBtn);

  dynLayout->addWidget(dynHeader);
  dynLayout->addSpacing(12);

  auto *timelineContainer = new QWidget(dynamicCard);
  timelineContainer->setMinimumHeight(364);
  timelineContainer->setStyleSheet("background: transparent; border: none; border-left: 2px solid #e2e8f0;");
  auto *timelineLayout = new QVBoxLayout(timelineContainer);
  timelineLayout->setContentsMargins(12, 0, 0, 0);
  timelineLayout->setSpacing(16);

  QSqlQuery dynQ;
  dynQ.prepare(QStringLiteral(
                   "SELECT '工单' as type, title, create_time FROM wo_work_order WHERE del_flag = 0%1 "
                   "UNION ALL SELECT '事件' as type, title, create_time FROM ge_event WHERE del_flag = 0%2 "
                   "ORDER BY create_time DESC LIMIT 5")
                   .arg(woScopeFilter)
                   .arg(evScopeFilter));
  if (onlySelf)
    dynQ.bindValue(":scope_uid", user.id);
  dynQ.exec();
  while (dynQ.next())
  {
    QString type = dynQ.value(0).toString();
    QString title = dynQ.value(1).toString();
    QString time = dynQ.value(2).toDateTime().toString("MM-dd HH:mm");
    QString color = type == QStringLiteral("工单") ? "#b45309" : "#2563eb";

    auto *item = new QWidget(timelineContainer);
    item->setObjectName(QStringLiteral("dynItem_%1").arg(timelineLayout->count()));
    item->setMinimumHeight(60);
    item->setStyleSheet("background: transparent; border: none;");
    auto *itemLayout = new QHBoxLayout(item);
    itemLayout->setContentsMargins(0, 0, 0, 0);
    itemLayout->setSpacing(0);
    itemLayout->setAlignment(Qt::AlignTop);

    // Fixed-width dot column aligned with the left timeline border
    auto *dotContainer = new QWidget(item);
    dotContainer->setFixedWidth(8);
    dotContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    dotContainer->setStyleSheet("background: transparent; border: none;");
    auto *dotLayout = new QVBoxLayout(dotContainer);
    dotLayout->setContentsMargins(0, 4, 0, 0);
    dotLayout->setSpacing(0);
    dotLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto *dot = new QLabel(item);
    dot->setFixedSize(8, 8);
    dot->setStyleSheet(QString("background: %1; border-radius: 4px;").arg(color));
    dotLayout->addWidget(dot);

    // Text column: occupies remaining width and wraps long titles
    auto *textWidget = new QWidget(item);
    textWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    textWidget->setStyleSheet("background: transparent; border: none;");
    auto *textLayout = new QVBoxLayout(textWidget);
    textLayout->setContentsMargins(12, 0, 0, 0);
    textLayout->setSpacing(2);
    textLayout->setAlignment(Qt::AlignTop);

    auto *titleLabel = new QLabel(title, textWidget);
    titleLabel->setObjectName(QStringLiteral("dynTitleLabel_%1").arg(timelineLayout->count()));
    titleLabel->setStyleSheet("font-size: 14px; color: #334155; background: transparent; border: none;");
    titleLabel->setWordWrap(true);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    titleLabel->setMinimumWidth(0);

    auto *timeLabel = new QLabel(time, textWidget);
    timeLabel->setStyleSheet("font-size: 12px; color: #94a3b8; background: transparent; border: none;");

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(timeLabel);

    itemLayout->addWidget(dotContainer);
    itemLayout->addWidget(textWidget, 1);

    timelineLayout->addWidget(item);
  }
  timelineLayout->addStretch();

  auto *dynScroll = new QScrollArea(dynamicCard);
  dynScroll->setWidgetResizable(true);
  dynScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  dynScroll->setFrameShape(QFrame::NoFrame);
  dynScroll->setStyleSheet(
      "QScrollArea { border: none; background: transparent; }"
      "QScrollBar:vertical { width: 6px; background: transparent; }"
      "QScrollBar::handle:vertical { background: #cbd5e1; border-radius: 3px; }"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");
  dynScroll->setMaximumHeight(260);
  dynScroll->setWidget(timelineContainer);
  dynLayout->addWidget(dynScroll);
  dynLayout->addStretch();

  rightCol->addWidget(dynamicCard, 1);

  // Data overview - pie chart
  auto *chartCard = new QFrame(page);
  chartCard->setMaximumHeight(320);
  chartCard->setStyleSheet(
      "QFrame { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 4px; }"
      "QLabel { background: transparent; border: none; }");
  auto *chartLayout = new QVBoxLayout(chartCard);
  chartLayout->setContentsMargins(20, 16, 20, 16);
  chartLayout->setSpacing(8);

  auto *chartTitle = new QLabel(QStringLiteral("工单状态分布"), chartCard);
  chartTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a; background: transparent; border: none;");
  chartLayout->addWidget(chartTitle);

  auto *pieSeries = new QPieSeries();
  QSqlQuery woStsQ;
  woStsQ.prepare(QStringLiteral("SELECT status, COUNT(*) FROM wo_work_order WHERE del_flag = 0%1 GROUP BY status").arg(woScopeFilter));
  if (onlySelf)
    woStsQ.bindValue(":scope_uid", user.id);
  woStsQ.exec();
  QMap<int, QString> statusNames = {
      {0, QStringLiteral("待处理")}, {1, QStringLiteral("已受理")}, {2, QStringLiteral("处理中")}, {3, QStringLiteral("待评价")}, {4, QStringLiteral("已完成")}, {5, QStringLiteral("已关闭")}};
  QStringList palette = {"#b45309", "#2563eb", "#15803d", "#64748b", "#cbd5e1", "#d97706", "#0d9488", "#7c3aed"};
  int colorIdx = 0;
  while (woStsQ.next())
  {
    int sts = woStsQ.value(0).toInt();
    int cnt = woStsQ.value(1).toInt();
    QString name = statusNames.value(sts, QStringLiteral("未知"));
    auto *slice = pieSeries->append(name, cnt);
    slice->setColor(QColor(palette[colorIdx % palette.size()]));
    slice->setLabelVisible(true);
    slice->setLabelColor(QColor("#334155"));
    slice->setLabelPosition(QPieSlice::LabelOutside);
    slice->setLabel(QStringLiteral("%1\n%2").arg(name).arg(cnt));
    ++colorIdx;
  }
  if (pieSeries->count() == 0)
  {
    auto *emptySlice = pieSeries->append(QStringLiteral("暂无数据"), 1);
    emptySlice->setColor(QColor("#e2e8f0"));
    emptySlice->setLabelVisible(true);
    emptySlice->setLabelColor(QColor("#94a3b8"));
  }

  auto *pieChart = new QChart();
  pieChart->addSeries(pieSeries);
  pieChart->setAnimationOptions(QChart::SeriesAnimations);
  pieChart->legend()->setAlignment(Qt::AlignRight);
  pieChart->legend()->setLabelColor(QColor("#64748b"));
  pieChart->legend()->setFont(QFont("Microsoft YaHei", 10));
  pieSeries->setHoleSize(0.55);
  pieChart->setMargins(QMargins(8, 0, 8, 0));
  pieChart->setBackgroundVisible(false);

  auto *chartView = new QChartView(pieChart, chartCard);
  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setMinimumHeight(200);
  chartLayout->addWidget(chartView);
  rightCol->addWidget(chartCard, 0);

  auto *leftWidget = new QWidget(page);
  leftWidget->setLayout(leftCol);
  auto *rightWidget = new QWidget(page);
  rightWidget->setLayout(rightCol);

  twoColLayout->addWidget(leftWidget, 5);
  twoColLayout->addWidget(rightWidget, 7);

  layout->addLayout(twoColLayout, 1);

  scrollArea->setWidget(content);
  outerLayout->addWidget(scrollArea);

  return page;
}


// ========== Todo Center Page ==========
BasePage *PageFactory::createTodoPage()
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(16);

  // Page header
  layout->addWidget(createPageHeader(QStringLiteral("ic_todo"), QStringLiteral("待办中心"), QStringLiteral("聚合展示各模块待处理事项，快速定位处理"), moduleColor("todo"), page));

  // Stats cards
  auto *statsRow = new QHBoxLayout();
  statsRow->setSpacing(16);
  auto createMiniCard = [](const QString &label, const QString &val, const QString &color, QWidget *parent)
  {
    auto *card = new QFrame(parent);
    card->setFixedHeight(90);
    card->setStyleSheet(QString("QFrame{background:#fff;border-radius:6px;border:1px solid #e2e8f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
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
    applyCardShadow(card);
    return card;
  };

  // 统计待受理工单数
  QSqlQuery woQ("SELECT COUNT(*) FROM wo_work_order WHERE status = 0 AND del_flag = 0");
  int woCount = woQ.next() ? woQ.value(0).toInt() : 0;
  // 统计待审核事件数
  QSqlQuery evQ("SELECT COUNT(*) FROM ge_event WHERE status = 0 AND del_flag = 0");
  int evCount = evQ.next() ? evQ.value(0).toInt() : 0;
  // 统计待反馈督办数
  QSqlQuery supQ("SELECT COUNT(*) FROM ge_supervision WHERE status = 0 AND del_flag = 0");
  int supCount = supQ.next() ? supQ.value(0).toInt() : 0;
  // 统计待处理民意数
  QSqlQuery opQ("SELECT COUNT(*) FROM ge_opinion WHERE status = 0 AND del_flag = 0");
  int opCount = opQ.next() ? opQ.value(0).toInt() : 0;

  statsRow->addWidget(createMiniCard(QStringLiteral("待受理工单"), QString::number(woCount), "#b45309", page));
  statsRow->addWidget(createMiniCard(QStringLiteral("待审核事件"), QString::number(evCount), "#d97706", page));
  statsRow->addWidget(createMiniCard(QStringLiteral("待反馈督办"), QString::number(supCount), "#b91c1c", page));
  statsRow->addWidget(createMiniCard(QStringLiteral("待处理民意"), QString::number(opCount), "#15803d", page));
  layout->addLayout(statsRow);

  // Toolbar with filter
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  auto *typeCombo = new QComboBox(toolbar);
  typeCombo->addItem(QStringLiteral("全部类型"), 0);
  typeCombo->addItem(QStringLiteral("工单"), 1);
  typeCombo->addItem(QStringLiteral("事件"), 2);
  typeCombo->addItem(QStringLiteral("督办"), 3);
  typeCombo->addItem(QStringLiteral("民意"), 4);
  typeCombo->setMinimumWidth(120);
  tbLayout->addWidget(typeCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  // Table
  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setColumnCount(6);
  table->setHorizontalHeaderLabels({QStringLiteral("类型"), QStringLiteral("标题"), QStringLiteral("提交人"), QStringLiteral("优先级"), QStringLiteral("提交时间"), QStringLiteral("操作")});
  table->setColumnWidth(0, 100);
  table->setColumnWidth(5, 100);
  table->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(table);

  // 空状态提示
  auto *emptyHint = createEmptyHintLabel(QStringLiteral("暂无待办事项"), page);
  layout->addWidget(emptyHint);

  // 数据加载函数
  std::function<void()> loadTodos = [table, typeCombo, emptyHint]()
  {
    table->setRowCount(0);
    int filterType = typeCombo->currentData().toInt();
    int row = 0;

    // 类型颜色映射
    auto typeTag = [](int t) -> QPair<QString, QColor>
    {
      switch (t)
      {
      case 1:
        return {QStringLiteral("工单"), QColor("#b45309")};
      case 2:
        return {QStringLiteral("事件"), QColor("#d97706")};
      case 3:
        return {QStringLiteral("督办"), QColor("#b91c1c")};
      case 4:
        return {QStringLiteral("民意"), QColor("#15803d")};
      default:
        return {QStringLiteral("未知"), QColor("#64748b")};
      }
    };
    auto priorityLabel = [](int p) -> QString
    {
      switch (p)
      {
      case 0:
        return QStringLiteral("低");
      case 1:
        return QStringLiteral("中");
      case 2:
        return QStringLiteral("高");
      case 3:
        return QStringLiteral("紧急");
      default:
        return QStringLiteral("中");
      }
    };

    // 工单待受理 (status=0)
    if (filterType == 0 || filterType == 1)
    {
      QSqlQuery q("SELECT id, order_no, title, reporter_name, priority, create_time FROM wo_work_order WHERE status = 0 AND del_flag = 0 ORDER BY create_time DESC");
      while (q.next())
      {
        table->insertRow(row);
        auto tag = typeTag(1);
        auto *typeItem = createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 3, new QTableWidgetItem(priorityLabel(q.value(4).toInt())));
        table->setItem(row, 4, new QTableWidgetItem(q.value(5).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *btn = new QPushButton(QStringLiteral("去处理"));
        applyTextButton(btn);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setProperty("targetPage", QStringLiteral("301"));
        table->setCellWidget(row, 5, btn);
        row++;
      }
    }

    // 事件待审核 (status=0)
    if (filterType == 0 || filterType == 2)
    {
      QSqlQuery q("SELECT id, event_no, title, reporter_name, priority, create_time FROM ge_event WHERE status = 0 AND del_flag = 0 ORDER BY create_time DESC");
      while (q.next())
      {
        table->insertRow(row);
        auto tag = typeTag(2);
        auto *typeItem = createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 3, new QTableWidgetItem(priorityLabel(q.value(4).toInt())));
        table->setItem(row, 4, new QTableWidgetItem(q.value(5).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *btn = new QPushButton(QStringLiteral("去处理"));
        applyTextButton(btn);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setProperty("targetPage", QStringLiteral("401"));
        table->setCellWidget(row, 5, btn);
        row++;
      }
    }

    // 督办待反馈 (status=0)
    if (filterType == 0 || filterType == 3)
    {
      QSqlQuery q("SELECT id, reason, supervise_to, deadline, status FROM ge_supervision WHERE status = 0 AND del_flag = 0 ORDER BY deadline ASC");
      while (q.next())
      {
        table->insertRow(row);
        auto tag = typeTag(3);
        auto *typeItem = createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(QStringLiteral("高")));
        table->setItem(row, 4, new QTableWidgetItem(q.value(3).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *btn = new QPushButton(QStringLiteral("去处理"));
        applyTextButton(btn);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setProperty("targetPage", QStringLiteral("404"));
        table->setCellWidget(row, 5, btn);
        row++;
      }
    }

    // 民意待处理 (status=0)
    if (filterType == 0 || filterType == 4)
    {
      QSqlQuery q("SELECT id, title, category, create_time FROM ge_opinion WHERE status = 0 AND del_flag = 0 ORDER BY create_time DESC");
      while (q.next())
      {
        table->insertRow(row);
        auto tag = typeTag(4);
        auto *typeItem = createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(QStringLiteral("中")));
        table->setItem(row, 4, new QTableWidgetItem(q.value(3).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *btn = new QPushButton(QStringLiteral("去处理"));
        applyTextButton(btn);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setProperty("targetPage", QStringLiteral("405"));
        table->setCellWidget(row, 5, btn);
        row++;
      }
    }
    syncEmptyHint(table, emptyHint);
  };

  loadTodos();
  QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadTodos(); });

  // 为表格中的"去处理"按钮安装事件过滤器，实现页面跳转
  // 由于按钮是动态创建的，使用 cellWidget 遍历连接
  QObject::connect(table, &QTableWidget::cellClicked, page, [page, table](int r, int)
          {
        auto* w = table->cellWidget(r, 5);
        if (w) {
            QString target = w->property("targetPage").toString();
            if (!target.isEmpty()) page->requestNavigate(target);
        } });

  return page;
}


// ========== Message Center Page ==========
BasePage *PageFactory::createMessagePage()
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(16);

  // Page header
  layout->addWidget(createPageHeader(QStringLiteral("ic_bell"), QStringLiteral("消息中心"), QStringLiteral("查看系统通知、业务提醒和公告推送"), moduleColor("message"), page));

  const auto &user = AuthService::instance().currentUser();

  // Stats cards
  auto *statsRow = new QHBoxLayout();
  statsRow->setSpacing(16);
  auto createMiniCard = [](const QString &label, const QString &val, const QString &color, QWidget *parent)
  {
    auto *card = new QFrame(parent);
    card->setFixedHeight(90);
    card->setStyleSheet(QString("QFrame{background:#fff;border-radius:6px;border:1px solid #e2e8f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
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
    applyCardShadow(card);
    return card;
  };

  // 统计未读消息数
  QSqlQuery unreadQ;
  unreadQ.prepare("SELECT COUNT(*) FROM nt_notification WHERE user_id = :uid AND is_read = 0");
  unreadQ.bindValue(":uid", user.id);
  int unreadCount = (unreadQ.exec() && unreadQ.next()) ? unreadQ.value(0).toInt() : 0;
  // 统计今日新增数
  QSqlQuery todayQ;
  todayQ.prepare("SELECT COUNT(*) FROM nt_notification WHERE user_id = :uid AND date(create_time) = date('now')");
  todayQ.bindValue(":uid", user.id);
  int todayCount = (todayQ.exec() && todayQ.next()) ? todayQ.value(0).toInt() : 0;

  statsRow->addWidget(createMiniCard(QStringLiteral("未读消息"), QString::number(unreadCount), "#b91c1c", page));
  statsRow->addWidget(createMiniCard(QStringLiteral("今日新增"), QString::number(todayCount), "#b45309", page));
  statsRow->addStretch();
  layout->addLayout(statsRow);

  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  auto *typeCombo = new QComboBox(toolbar);
  typeCombo->addItem(QStringLiteral("全部类型"), 0);
  typeCombo->addItem(QStringLiteral("公告"), 1);
  typeCombo->addItem(QStringLiteral("工单"), 2);
  typeCombo->addItem(QStringLiteral("事件"), 3);
  typeCombo->addItem(QStringLiteral("督办"), 4);
  typeCombo->addItem(QStringLiteral("志愿"), 5);
  typeCombo->addItem(QStringLiteral("系统"), 6);
  typeCombo->setMinimumWidth(120);
  tbLayout->addWidget(typeCombo);
  tbLayout->addStretch();
  auto *readAllBtn = new QPushButton(QStringLiteral("全部已读"), toolbar);
  applyPrimaryButton(readAllBtn);
  readAllBtn->setCursor(Qt::PointingHandCursor);
  tbLayout->addWidget(readAllBtn);
  layout->addWidget(toolbar);

  // Table
  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setColumnCount(5);
  table->setHorizontalHeaderLabels({QStringLiteral("类型"), QStringLiteral("标题"), QStringLiteral("内容"), QStringLiteral("时间"), QStringLiteral("状态")});
  table->setColumnWidth(0, 100);
  table->setColumnWidth(1, 200);
  table->setColumnWidth(4, 100);
  table->horizontalHeader()->setStretchLastSection(true);
  auto *pb = new PaginationBar(page);
  layout->addWidget(pb);
  layout->addWidget(table);

  // 空状态提示
  auto *emptyHint = createEmptyHintLabel(QStringLiteral("暂无消息"), page);
  layout->addWidget(emptyHint);

  // 数据加载函数
  std::function<void()> loadMessages = [table, typeCombo, user, emptyHint, pb]()
  {
    table->setRowCount(0);
    int filterType = typeCombo->currentData().toInt();
    QString sql = "SELECT id, title, content, notification_type, is_read, create_time FROM nt_notification WHERE user_id = :uid";
    if (filterType > 0)
      sql += " AND notification_type = :type";
    sql += " ORDER BY is_read ASC, create_time DESC";
    sql += " LIMIT :pageSize OFFSET :offset";
    QSqlQuery q(DatabaseManager::instance().database());
    // 分页：查询满足条件的总记录数
    QVariantList cntBinds;
    cntBinds << ":uid" << (user.id);
    if (filterType > 0)
      cntBinds << ":type" << (filterType);
    cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
    pb->setTotalCount(executeCountQuery(sql, cntBinds));

    q.prepare(sql);
    q.bindValue(":uid", user.id);
    if (filterType > 0)
      q.bindValue(":type", filterType);
    q.bindValue(":pageSize", pb->pageSize());
    q.bindValue(":offset", pb->offset());
    q.exec();
    int row = 0;
    // 类型标签
    auto typeLabel = [](int t) -> QPair<QString, QColor>
    {
      switch (t)
      {
      case 1:
        return {QStringLiteral("公告"), QColor("#b45309")};
      case 2:
        return {QStringLiteral("工单"), QColor("#d97706")};
      case 3:
        return {QStringLiteral("事件"), QColor("#7c3aed")};
      case 4:
        return {QStringLiteral("督办"), QColor("#b91c1c")};
      case 5:
        return {QStringLiteral("志愿"), QColor("#15803d")};
      case 6:
        return {QStringLiteral("系统"), QColor("#64748b")};
      default:
        return {QStringLiteral("未知"), QColor("#64748b")};
      }
    };
    while (q.next())
    {
      table->insertRow(row);
      qint64 msgId = q.value(0).toLongLong();
      int ntype = q.value(3).toInt();
      int isRead = q.value(4).toInt();
      auto tag = typeLabel(ntype);
      auto *typeItem = createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
      typeItem->setData(Qt::UserRole, msgId);
      table->setItem(row, 0, typeItem);

      auto *titleItem = new QTableWidgetItem(q.value(1).toString());
      auto *contentItem = new QTableWidgetItem(q.value(2).toString());
      auto *timeItem = new QTableWidgetItem(q.value(5).toDateTime().toString("yyyy-MM-dd hh:mm"));
      // 未读消息加粗 + 浅蓝背景
      if (isRead == 0)
      {
        QFont boldFont = titleItem->font();
        boldFont.setBold(true);
        titleItem->setFont(boldFont);
        contentItem->setFont(boldFont);
        QColor lightBlue(230, 244, 255);
        titleItem->setBackground(lightBlue);
        contentItem->setBackground(lightBlue);
        timeItem->setBackground(lightBlue);
      }
      table->setItem(row, 1, titleItem);
      table->setItem(row, 2, contentItem);
      table->setItem(row, 3, timeItem);

      auto *statusItem = isRead == 0
                             ? createTagTableItem(QStringLiteral("未读"), QColor("#fef2f2"), QColor("#b91c1c"))
                             : createTagTableItem(QStringLiteral("已读"), QColor("#f1f5f9"), QColor("#64748b"));
      table->setItem(row, 4, statusItem);
      row++;
    }
    syncEmptyHint(table, emptyHint);
    pb->refreshData();
  };

  loadMessages();
  QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadMessages(); });

  // 双击消息行查看详情并标记为已读
  QObject::connect(table, &QTableWidget::cellDoubleClicked, page, [page, table, loadMessages](int r, int)
          {
        auto* item = table->item(r, 0);
        if (!item) return;
        qint64 msgId = item->data(Qt::UserRole).toLongLong();
        if (msgId <= 0) return;

        // 查询完整消息内容
        QSqlQuery detailQ(DatabaseManager::instance().database());
        detailQ.prepare("SELECT title, content, notification_type, is_read, create_time FROM nt_notification WHERE id = :id");
        detailQ.bindValue(":id", msgId);
        if (!detailQ.exec() || !detailQ.next()) {
            QMessageBox::warning(page, QStringLiteral("提示"), QStringLiteral("消息内容加载失败"));
            return;
        }

        QString title = detailQ.value(0).toString();
        QString content = detailQ.value(1).toString();
        int ntype = detailQ.value(2).toInt();
        int isRead = detailQ.value(3).toInt();
        QDateTime createTime = detailQ.value(4).toDateTime();

        // 弹出详情对话框
        QDialog detailDlg(page);
        detailDlg.setWindowTitle(QStringLiteral("消息详情"));
        detailDlg.setMinimumWidth(480);
        auto* detailLayout = new QVBoxLayout(&detailDlg);
        detailLayout->setContentsMargins(24, 20, 24, 20);
        detailLayout->setSpacing(12);

        // 类型标签
        auto typeLabelFn = [](int t) -> QString {
            switch (t) {
                case 1: return QStringLiteral("公告");
                case 2: return QStringLiteral("工单");
                case 3: return QStringLiteral("事件");
                case 4: return QStringLiteral("督办");
                case 5: return QStringLiteral("志愿");
                case 6: return QStringLiteral("系统");
                default: return QStringLiteral("未知");
            }
        };
        auto* typeTag = new QLabel(typeLabelFn(ntype), &detailDlg);
        typeTag->setStyleSheet("background:#fef3c7;color:#b45309;padding:2px 10px;border-radius:3px;font-size:12px;");
        detailLayout->addWidget(typeTag);

        // 标题
        auto* titleLabel = new QLabel(title, &detailDlg);
        titleLabel->setStyleSheet("font-size:18px;font-weight:600;color:#0f172a;");
        titleLabel->setWordWrap(true);
        detailLayout->addWidget(titleLabel);

        // 时间
        auto* timeLabel = new QLabel(QStringLiteral("时间：%1").arg(createTime.toString("yyyy-MM-dd hh:mm")), &detailDlg);
        timeLabel->setStyleSheet("color:#64748b;font-size:12px;");
        detailLayout->addWidget(timeLabel);

        // 分割线
        auto* sep = new QFrame(&detailDlg);
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("color:#e2e8f0;background:#e2e8f0;max-height:1px;border:none;");
        detailLayout->addWidget(sep);

        // 内容（支持滚动）
        auto* contentScroll = new QScrollArea(&detailDlg);
        contentScroll->setWidgetResizable(true);
        contentScroll->setStyleSheet("QScrollArea{border:none;background:transparent;}");
        auto* contentLabel = new QLabel(content);
        contentLabel->setWordWrap(true);
        contentLabel->setStyleSheet("font-size:14px;color:#334155;line-height:1.6;background:transparent;");
        contentLabel->setTextFormat(Qt::PlainText);
        contentScroll->setWidget(contentLabel);
        contentScroll->setMinimumHeight(180);
        detailLayout->addWidget(contentScroll, 1);

        // 关闭按钮
        auto* closeBtn = new QPushButton(QStringLiteral("关闭"), &detailDlg);
        closeBtn->setProperty("cssClass", "primary");
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setFixedWidth(100);
        auto* btnRow = new QHBoxLayout();
        btnRow->addStretch();
        btnRow->addWidget(closeBtn);
        detailLayout->addLayout(btnRow);

        QObject::connect(closeBtn, &QPushButton::clicked, &detailDlg, &QDialog::accept);

        // 如果未读，标记为已读
        if (isRead == 0) {
            DatabaseManager::instance().update("nt_notification", msgId, {
                {"is_read", 1},
                {"read_time", QDateTime::currentDateTime()}
            });
        }

        detailDlg.exec();
        loadMessages(); });

  // 全部已读按钮
  QObject::connect(readAllBtn, &QPushButton::clicked, page, [page, user, loadMessages]()
          {
        QSqlQuery q(DatabaseManager::instance().database());
        q.prepare("UPDATE nt_notification SET is_read = 1, read_time = :now WHERE user_id = :uid AND is_read = 0");
        q.bindValue(":now", QDateTime::currentDateTime());
        q.bindValue(":uid", user.id);
        q.exec();
        showToast(QStringLiteral("已全部标为已读"), page);
        loadMessages(); });

  return page;
}


