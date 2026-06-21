#include "pages/PageFactory.h"

#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QtCharts>

#include "PagesCommon.h"
#include "services/EventService.h"

// ========== Dashboard Page ==========
// 现代档案室设计：大数字指标 + 紧凑列表 + 时间线 + 克制色板
BasePage *PageFactory::createDashboardPage()
{
  auto *page = new BasePage();
  auto *outerLayout = new QVBoxLayout(page);
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(0);

  auto *scrollArea = new QScrollArea(page);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setStyleSheet(
      "QScrollArea { border: none; background: #FAF9F6; }"
      "QScrollBar:vertical { width: 6px; background: transparent; }"
      "QScrollBar::handle:vertical { background: #D4D0C8; border-radius: 0; min-height: 30px; }"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");

  auto *content = new QWidget();
  content->setStyleSheet("background: #FAF9F6;");
  auto *layout = new QVBoxLayout(content);
  layout->setContentsMargins(32, 24, 32, 24);
  layout->setSpacing(24);

  const auto &user = AuthService::instance().currentUser();
  QString greeting = user.nickname.isEmpty() ? user.username : user.nickname;

  // Data-scope filter
  const int dataScope = AuthService::instance().currentUserDataScope();
  const bool onlySelf = dataScope >= 6;
  const QString woScopeFilter = onlySelf ? QStringLiteral(" AND reporter_id = :scope_uid") : QString();
  const QString evScopeFilter = onlySelf ? QStringLiteral(" AND reporter_id = :scope_uid") : QString();

  // ========== Hero 区：问候 + 日期 ==========
  auto *heroFrame = new QFrame(content);
  heroFrame->setStyleSheet("QFrame { background: transparent; border: none; } QLabel { background: transparent; border: none; }");
  auto *heroLayout = new QVBoxLayout(heroFrame);
  heroLayout->setContentsMargins(0, 0, 0, 0);
  heroLayout->setSpacing(4);

  auto *greetingLabel = new QLabel(QStringLiteral("欢迎回来，%1").arg(greeting), heroFrame);
  greetingLabel->setStyleSheet(
      "font-size: 28px; font-weight: 600; color: #141413;"
      " font-family: 'Noto Serif SC', 'Source Han Serif SC', 'SimSun', serif;"
      " letter-spacing: 0.5px;");
  heroLayout->addWidget(greetingLabel);

  auto *dateLabel = new QLabel(QDateTime::currentDateTime().toString(QStringLiteral("yyyy 年 MM 月 dd 日  dddd")), heroFrame);
  dateLabel->setStyleSheet("font-size: 12px; color: #6B6B6B; letter-spacing: 1px; text-transform: uppercase;");
  heroLayout->addWidget(dateLabel);
  layout->addWidget(heroFrame);

  // ========== 大数字指标区（3 列） ==========
  // 数据查询
  QSqlQuery woPendQ;
  woPendQ.prepare(QStringLiteral("SELECT COUNT(*) FROM wo_work_order WHERE status IN (0,1,2,3) AND del_flag = 0%1").arg(woScopeFilter));
  if (onlySelf) woPendQ.bindValue(":scope_uid", user.id);
  int woPendCount = (woPendQ.exec() && woPendQ.next()) ? woPendQ.value(0).toInt() : 0;

  int evPendCount = EventService::instance().countPendingEvents(onlySelf ? user.id : -1);

  QSqlQuery unreadQ;
  unreadQ.prepare("SELECT COUNT(*) FROM nt_notification WHERE user_id = :uid AND is_read = 0");
  unreadQ.bindValue(":uid", user.id);
  int unreadCount = (unreadQ.exec() && unreadQ.next()) ? unreadQ.value(0).toInt() : 0;

  auto *metricsRow = new QHBoxLayout();
  metricsRow->setSpacing(1);  // 1px 细线分隔
  metricsRow->setContentsMargins(0, 0, 0, 0);

  auto createBigMetric = [](const QString &label, int value, const QString &hint, QWidget *parent) -> QFrame *
  {
    auto *card = new QFrame(parent);
    card->setFixedHeight(120);
    card->setStyleSheet(
        "QFrame { background: #FAF9F6; border: none; }"
        "QLabel { background: transparent; border: none; }");
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(24, 20, 24, 20);
    cl->setSpacing(4);

    auto *labelEl = new QLabel(label, card);
    labelEl->setStyleSheet("font-size: 11px; color: #6B6B6B; letter-spacing: 1.5px; text-transform: uppercase; font-weight: 500;");

    // 初始显示 0，后续 countUp 动画滚动到 value
    auto *numLabel = new QLabel(QStringLiteral("0"), card);
    numLabel->setStyleSheet(
        "font-size: 48px; font-weight: 600; color: #141413;"
        " font-family: 'Noto Serif SC', 'Source Han Serif SC', 'SimSun', serif;"
        " letter-spacing: -1px;");

    auto *hintLabel = new QLabel(hint, card);
    hintLabel->setStyleSheet("font-size: 11px; color: #9A9A9A;");

    cl->addWidget(labelEl);
    cl->addWidget(numLabel);
    cl->addWidget(hintLabel);
    cl->addStretch();

    // 延迟启动 countUp 动画，确保 widget 已显示
    QTimer::singleShot(100, numLabel, [numLabel, value]() {
      UiKit::countUpLabel(numLabel, value, 700);
    });

    return card;
  };

  metricsRow->addWidget(createBigMetric(QStringLiteral("待处理工单"), woPendCount, QStringLiteral("需要尽快响应"), content), 1);

  // 1px 垂直分隔线
  auto *sep1 = new QFrame(content);
  sep1->setFixedWidth(1);
  sep1->setStyleSheet("background: #D4D0C8; border: none;");
  metricsRow->addWidget(sep1);

  metricsRow->addWidget(createBigMetric(QStringLiteral("待审核事件"), evPendCount, QStringLiteral("网格员上报"), content), 1);

  auto *sep2 = new QFrame(content);
  sep2->setFixedWidth(1);
  sep2->setStyleSheet("background: #D4D0C8; border: none;");
  metricsRow->addWidget(sep2);

  metricsRow->addWidget(createBigMetric(QStringLiteral("未读通知"), unreadCount, QStringLiteral("消息中心"), content), 1);

  // 顶部底部细线
  auto *metricsWrapper = new QVBoxLayout();
  metricsWrapper->setContentsMargins(0, 0, 0, 0);
  metricsWrapper->setSpacing(0);
  auto *topLine = new QFrame(content);
  topLine->setFixedHeight(1);
  topLine->setStyleSheet("background: #141413; border: none;");
  metricsWrapper->addWidget(topLine);
  metricsWrapper->addLayout(metricsRow);
  auto *bottomLine = new QFrame(content);
  bottomLine->setFixedHeight(1);
  bottomLine->setStyleSheet("background: #D4D0C8; border: none;");
  metricsWrapper->addWidget(bottomLine);
  layout->addLayout(metricsWrapper);

  // ========== 快捷操作（图标网格） ==========
  auto *quickArea = new QWidget(content);
  auto *quickLayout = new QVBoxLayout(quickArea);
  quickLayout->setContentsMargins(0, 0, 0, 0);
  quickLayout->setSpacing(12);

  auto *quickHeader = new QHBoxLayout();
  auto *quickTitle = new QLabel(QStringLiteral("快捷操作"), quickArea);
  quickTitle->setStyleSheet(
      "font-size: 11px; color: #6B6B6B; letter-spacing: 1.5px; text-transform: uppercase; font-weight: 600;");
  quickHeader->addWidget(quickTitle);
  auto *quickSep = new QFrame(quickArea);
  quickSep->setFixedHeight(1);
  quickSep->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  quickSep->setStyleSheet("background: #D4D0C8; border: none;");
  quickHeader->addWidget(quickSep, 1);
  quickLayout->addLayout(quickHeader);

  auto *quickGrid = new QGridLayout();
  quickGrid->setSpacing(1);
  quickGrid->setHorizontalSpacing(1);
  quickGrid->setVerticalSpacing(1);
  for (int i = 0; i < 3; ++i) quickGrid->setColumnStretch(i, 1);

  struct QuickAction { QString label; QString iconKey; QString target; };
  const QString roleDomain = AuthService::instance().currentUser().roleDomain;
  QList<QuickAction> actions;
  if (roleDomain == QStringLiteral("resident")) {
      actions = {
          {QStringLiteral("报事报修"), QStringLiteral("ic_tool"), "301"},
          {QStringLiteral("物业缴费"), QStringLiteral("ic_money"), "308"},
          {QStringLiteral("投诉建议"), QStringLiteral("ic_chat"), "302"},
          {QStringLiteral("公告通知"), QStringLiteral("ic_announce"), "304"},
          {QStringLiteral("志愿服务"), QStringLiteral("ic_people"), "501"},
          {QStringLiteral("便民服务"), QStringLiteral("ic_shop"), "502"},
      };
  } else if (roleDomain == QStringLiteral("property")) {
      actions = {
          {QStringLiteral("报事报修"), QStringLiteral("ic_tool"), "301"},
          {QStringLiteral("投诉建议"), QStringLiteral("ic_chat"), "302"},
          {QStringLiteral("公告通知"), QStringLiteral("ic_announce"), "304"},
          {QStringLiteral("停车管理"), QStringLiteral("ic_car"), "307"},
          {QStringLiteral("物业缴费"), QStringLiteral("ic_money"), "308"},
          {QStringLiteral("巡检管理"), QStringLiteral("ic_inspect"), "303"},
      };
  } else if (roleDomain == QStringLiteral("governance")) {
      actions = {
          {QStringLiteral("网格事件"), QStringLiteral("ic_route"), "401"},
          {QStringLiteral("民意收集"), QStringLiteral("ic_chat"), "405"},
          {QStringLiteral("重点人群关怀"), QStringLiteral("ic_heart"), "403"},
          {QStringLiteral("督办管理"), QStringLiteral("ic_flag"), "404"},
          {QStringLiteral("绩效考核"), QStringLiteral("ic_chart"), "406"},
          {QStringLiteral("社区巡查"), QStringLiteral("ic_inspect"), "402"},
      };
  } else {
      actions = {
          {QStringLiteral("用户管理"), QStringLiteral("ic_people"), "701"},
          {QStringLiteral("角色管理"), QStringLiteral("ic_lock"), "702"},
          {QStringLiteral("菜单管理"), QStringLiteral("ic_list"), "703"},
          {QStringLiteral("字典管理"), QStringLiteral("ic_table"), "704"},
          {QStringLiteral("操作日志"), QStringLiteral("ic_log"), "705"},
          {QStringLiteral("公告通知"), QStringLiteral("ic_announce"), "304"},
      };
  }

  const int actionCount = qMin(actions.size(), 6);
  for (int i = 0; i < actionCount; ++i)
  {
    auto *item = new QFrame(quickArea);
    item->setFrameStyle(QFrame::NoFrame);
    item->setCursor(Qt::PointingHandCursor);
    item->setFixedHeight(96);
    item->setStyleSheet(
        "QFrame { background: #FAF9F6; border: none; }"
        "QFrame:hover { background: #F5F2EB; }"
        "QLabel { background: transparent; border: none; }");
    item->setProperty("targetPage", actions[i].target);

    auto *itemLayout = new QVBoxLayout(item);
    itemLayout->setContentsMargins(20, 16, 20, 16);
    itemLayout->setSpacing(8);
    itemLayout->setAlignment(Qt::AlignCenter);

    auto *iconLabel = new QLabel(item);
    iconLabel->setFixedSize(28, 28);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setPixmap(UiKit::tintSvgIcon(actions[i].iconKey, "#6B6B6B", QSize(28, 28)));

    auto *textLabel = new QLabel(actions[i].label, item);
    textLabel->setStyleSheet("font-size: 12px; color: #141413; font-weight: 500; letter-spacing: 0.3px;");
    textLabel->setAlignment(Qt::AlignCenter);

    itemLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    itemLayout->addWidget(textLabel, 0, Qt::AlignCenter);

    item->installEventFilter(new QuickHoverFilter(iconLabel, actions[i].iconKey, item));
    item->installEventFilter(page);

    quickGrid->addWidget(item, i / 3, i % 3);
  }

  quickLayout->addLayout(quickGrid);

  // 网格边框（用 1px frame 包裹）
  auto *quickWrapper = new QFrame(content);
  quickWrapper->setStyleSheet("QFrame { background: #D4D0C8; border: 1px solid #D4D0C8; }");
  auto *qwLayout = new QVBoxLayout(quickWrapper);
  qwLayout->setContentsMargins(1, 1, 1, 1);
  qwLayout->setSpacing(0);
  qwLayout->addWidget(quickArea);
  layout->addWidget(quickWrapper);

  // ========== 双栏：待办 + 社区动态 ==========
  auto *twoColLayout = new QHBoxLayout();
  twoColLayout->setSpacing(24);
  twoColLayout->setContentsMargins(0, 0, 0, 0);

  // === 左栏：待办列表 ===
  auto *todoCard = new QFrame(content);
  todoCard->setStyleSheet(
      "QFrame { background: #FAF9F6; border: 1px solid #141413; border-radius: 0; }"
      "QLabel { background: transparent; border: none; }");
  auto *todoLayout = new QVBoxLayout(todoCard);
  todoLayout->setContentsMargins(0, 0, 0, 0);
  todoLayout->setSpacing(0);

  // 卡片头部（黑底白字）
  auto *todoHeader = new QFrame(todoCard);
  todoHeader->setFixedHeight(48);
  todoHeader->setStyleSheet("QFrame { background: #141413; border: none; } QLabel { background: transparent; color: #FAF9F6; border: none; }");
  auto *todoHeaderLayout = new QHBoxLayout(todoHeader);
  todoHeaderLayout->setContentsMargins(20, 0, 20, 0);
  auto *todoTitle = new QLabel(QStringLiteral("待办事项"), todoHeader);
  todoTitle->setStyleSheet("font-size: 11px; font-weight: 600; letter-spacing: 1.5px; text-transform: uppercase;");
  todoHeaderLayout->addWidget(todoTitle);
  todoHeaderLayout->addStretch();
  auto *todoCountLabel = new QLabel(QString::number(woPendCount + evPendCount) + QStringLiteral(" 项"), todoHeader);
  todoCountLabel->setStyleSheet("font-size: 11px; color: #B45309; font-weight: 600; letter-spacing: 0.5px;");
  todoHeaderLayout->addWidget(todoCountLabel);
  todoLayout->addWidget(todoHeader);

  // 待办项
  auto createTodoItem = [&](const QString &label, int count, const QString &target, bool last) -> QFrame *
  {
    auto *item = new QFrame(todoCard);
    item->setFrameStyle(QFrame::NoFrame);
    item->setCursor(Qt::PointingHandCursor);
    item->setFixedHeight(56);
    item->setProperty("targetPage", target);

    QString borderStyle = last ? QString() : QStringLiteral("border-bottom: 1px solid #E8E5DE;");
    item->setStyleSheet(QString(
        "QFrame { background: #FAF9F6; border: none; %1 }"
        "QFrame:hover { background: #F5F2EB; }"
        "QLabel { background: transparent; border: none; }").arg(borderStyle));

    auto *hl = new QHBoxLayout(item);
    hl->setContentsMargins(20, 0, 20, 0);
    hl->setSpacing(12);

    auto *text = new QLabel(label, item);
    text->setStyleSheet("font-size: 13px; color: #141413; font-weight: 500;");
    text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *countLabel = new QLabel(QString::number(count), item);
    countLabel->setStyleSheet(
        "font-size: 20px; font-weight: 600; color: #92400E;"
        " font-family: 'Noto Serif SC', 'Source Han Serif SC', 'SimSun', serif;");

    auto *arrow = new QLabel(QStringLiteral("→"), item);
    arrow->setStyleSheet("font-size: 14px; color: #9A9A9A;");

    hl->addWidget(text);
    hl->addStretch();
    hl->addWidget(countLabel);
    hl->addSpacing(8);
    hl->addWidget(arrow);

    item->installEventFilter(page);
    return item;
  };

  QSqlQuery annQ("SELECT COUNT(*) FROM nt_announcement WHERE del_flag = 0 AND date(publish_time) >= date('now', '-7 days')");
  int annCount = annQ.next() ? annQ.value(0).toInt() : 0;

  todoLayout->addWidget(createTodoItem(QStringLiteral("待处理工单"), woPendCount, "301", false));
  todoLayout->addWidget(createTodoItem(QStringLiteral("待审核事件"), evPendCount, "401", false));
  todoLayout->addWidget(createTodoItem(QStringLiteral("近期公告"), annCount, "304", true));
  todoLayout->addStretch();

  twoColLayout->addWidget(todoCard, 1);

  // === 右栏：社区动态时间线 ===
  auto *dynamicCard = new QFrame(content);
  dynamicCard->setStyleSheet(
      "QFrame { background: #FAF9F6; border: 1px solid #141413; border-radius: 0; }"
      "QLabel { background: transparent; border: none; }");
  auto *dynLayout = new QVBoxLayout(dynamicCard);
  dynLayout->setContentsMargins(0, 0, 0, 0);
  dynLayout->setSpacing(0);

  // 卡片头部
  auto *dynHeader = new QFrame(dynamicCard);
  dynHeader->setFixedHeight(48);
  dynHeader->setStyleSheet("QFrame { background: #141413; border: none; } QLabel { background: transparent; color: #FAF9F6; border: none; }");
  auto *dynHeaderLayout = new QHBoxLayout(dynHeader);
  dynHeaderLayout->setContentsMargins(20, 0, 20, 0);
  auto *dynTitle = new QLabel(QStringLiteral("社区动态"), dynHeader);
  dynTitle->setStyleSheet("font-size: 11px; font-weight: 600; letter-spacing: 1.5px; text-transform: uppercase;");
  dynHeaderLayout->addWidget(dynTitle);
  dynHeaderLayout->addStretch();
  auto *dynHint = new QLabel(QStringLiteral("最近 5 条"), dynHeader);
  dynHint->setStyleSheet("font-size: 11px; color: #9A9A9A; letter-spacing: 0.5px;");
  dynHeaderLayout->addWidget(dynHint);
  dynLayout->addWidget(dynHeader);

  // 时间线
  auto *timelineContainer = new QWidget(dynamicCard);
  timelineContainer->setStyleSheet("background: #FAF9F6; border: none;");
  auto *timelineLayout = new QVBoxLayout(timelineContainer);
  timelineLayout->setContentsMargins(24, 20, 24, 20);
  timelineLayout->setSpacing(16);

  QSqlQuery dynQ;
  dynQ.prepare(QStringLiteral(
      "SELECT '工单' as type, title, create_time FROM wo_work_order WHERE del_flag = 0%1 "
      "UNION ALL SELECT '事件' as type, title, create_time FROM ge_event WHERE del_flag = 0%2 "
      "ORDER BY create_time DESC LIMIT 5").arg(woScopeFilter).arg(evScopeFilter));
  if (onlySelf) dynQ.bindValue(":scope_uid", user.id);
  dynQ.exec();
  while (dynQ.next())
  {
    QString type = dynQ.value(0).toString();
    QString title = dynQ.value(1).toString();
    QString time = dynQ.value(2).toDateTime().toString("MM-dd HH:mm");

    auto *item = new QWidget(timelineContainer);
    item->setStyleSheet("background: transparent; border: none;");
    auto *itemLayout = new QHBoxLayout(item);
    itemLayout->setContentsMargins(0, 0, 0, 0);
    itemLayout->setSpacing(12);

    // 类型标签（小方块，琥珀色）
    auto *typeTag = new QLabel(type, item);
    typeTag->setStyleSheet(
        "background: #141413; color: #FAF9F6; padding: 2px 8px;"
        " font-size: 10px; font-weight: 600; letter-spacing: 0.5px;");
    typeTag->setFixedHeight(18);

    auto *textWidget = new QWidget(item);
    textWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    textWidget->setStyleSheet("background: transparent; border: none;");
    auto *textLayout = new QVBoxLayout(textWidget);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);

    auto *titleLabel = new QLabel(title, textWidget);
    titleLabel->setStyleSheet("font-size: 13px; color: #141413; font-weight: 500;");
    titleLabel->setWordWrap(true);

    auto *timeLabel = new QLabel(time, textWidget);
    timeLabel->setStyleSheet("font-size: 11px; color: #9A9A9A; letter-spacing: 0.3px;");

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(timeLabel);

    itemLayout->addWidget(typeTag, 0, Qt::AlignTop);
    itemLayout->addWidget(textWidget, 1);

    timelineLayout->addWidget(item);
  }
  timelineLayout->addStretch();

  dynLayout->addWidget(timelineContainer, 1);

  twoColLayout->addWidget(dynamicCard, 1);

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
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_todo"), QStringLiteral("待办中心"), QStringLiteral("聚合展示各模块待处理事项，快速定位处理"), UiKit::moduleColor("todo"), page));

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
    UiKit::applyCardShadow(card);
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
  table->setStyleSheet(UiKit::TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setColumnCount(6);
  table->setHorizontalHeaderLabels({QStringLiteral("类型"), QStringLiteral("标题"), QStringLiteral("提交人"), QStringLiteral("优先级"), QStringLiteral("提交时间"), QStringLiteral("操作")});
  table->setColumnWidth(0, 100);
  table->setColumnWidth(5, 100);
  table->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(table);

  // 空状态提示
  auto *emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无待办事项"), page);
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
        auto *typeItem = UiKit::createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 3, new QTableWidgetItem(priorityLabel(q.value(4).toInt())));
        table->setItem(row, 4, new QTableWidgetItem(q.value(5).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *btn = new QPushButton(QStringLiteral("去处理"));
        UiKit::applyTextButton(btn);
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
        auto *typeItem = UiKit::createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 3, new QTableWidgetItem(priorityLabel(q.value(4).toInt())));
        table->setItem(row, 4, new QTableWidgetItem(q.value(5).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *btn = new QPushButton(QStringLiteral("去处理"));
        UiKit::applyTextButton(btn);
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
        auto *typeItem = UiKit::createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(QStringLiteral("高")));
        table->setItem(row, 4, new QTableWidgetItem(q.value(3).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *btn = new QPushButton(QStringLiteral("去处理"));
        UiKit::applyTextButton(btn);
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
        auto *typeItem = UiKit::createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(QStringLiteral("中")));
        table->setItem(row, 4, new QTableWidgetItem(q.value(3).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *btn = new QPushButton(QStringLiteral("去处理"));
        UiKit::applyTextButton(btn);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setProperty("targetPage", QStringLiteral("405"));
        table->setCellWidget(row, 5, btn);
        row++;
      }
    }
    UiKit::syncEmptyHint(table, emptyHint);
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
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_bell"), QStringLiteral("消息中心"), QStringLiteral("查看系统通知、业务提醒和公告推送"), UiKit::moduleColor("message"), page));

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
    UiKit::applyCardShadow(card);
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
  UiKit::applyPrimaryButton(readAllBtn);
  readAllBtn->setCursor(Qt::PointingHandCursor);
  tbLayout->addWidget(readAllBtn);
  layout->addWidget(toolbar);

  // Table
  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(UiKit::TABLE_STYLE);
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
  auto *emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无消息"), page);
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
    pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

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
      auto *typeItem = UiKit::createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
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
                             ? UiKit::createTagTableItem(QStringLiteral("未读"), QColor("#fef2f2"), QColor("#b91c1c"))
                             : UiKit::createTagTableItem(QStringLiteral("已读"), QColor("#f1f5f9"), QColor("#64748b"));
      table->setItem(row, 4, statusItem);
      row++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
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

        // 标题：加大字号、加粗字重，确保低 DPI 屏幕清晰可读
        auto* titleLabel = new QLabel(title, &detailDlg);
        titleLabel->setStyleSheet(
            "font-size:20px;font-weight:700;color:#141413;"
            "font-family:'Noto Sans SC','Source Han Sans SC','Microsoft YaHei UI',sans-serif;");
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
        UiKit::showToast(QStringLiteral("已全部标为已读"), page);
        loadMessages(); });

  return page;
}


