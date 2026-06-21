#include "pages/PageFactory.h"

#include "PagesCommon.h"
#include "services/AuthService.h"
#include "services/EventService.h"
#include <QSqlError>

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
  QString woPendSql = QStringLiteral("SELECT COUNT(*) FROM wo_work_order WHERE status IN (0,1,2,3) AND del_flag = 0");
  if (onlySelf) woPendSql += QStringLiteral(" AND reporter_id = :scope_uid");
  woPendQ.prepare(woPendSql);
  if (onlySelf) woPendQ.bindValue(":scope_uid", user.id);
  int woPendCount = 0;
  if (woPendQ.exec() && woPendQ.next()) {
      woPendCount = woPendQ.value(0).toInt();
  } else {
      qWarning() << "Dashboard: work order count query failed:" << woPendQ.lastError().text();
  }

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

  QSqlQuery annQ(QStringLiteral("SELECT COUNT(*) FROM nt_announcement WHERE del_flag = 0 AND date(publish_time) >= date('now', '-7 days')"));
  int annCount = 0;
  if (annQ.isActive() && annQ.next()) {
      annCount = annQ.value(0).toInt();
  } else {
      qWarning() << "Dashboard: announcement count query failed:" << annQ.lastError().text();
  }

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
  QString dynSql = QStringLiteral(
      "SELECT '工单' as type, title, create_time FROM wo_work_order WHERE del_flag = 0");
  if (onlySelf) dynSql += QStringLiteral(" AND reporter_id = :scope_uid");
  dynSql += QStringLiteral(
      " UNION ALL SELECT '事件' as type, title, create_time FROM ge_event WHERE del_flag = 0");
  if (onlySelf) dynSql += QStringLiteral(" AND reporter_id = :scope_uid");
  dynSql += QStringLiteral(" ORDER BY create_time DESC LIMIT 5");
  dynQ.prepare(dynSql);
  if (onlySelf) dynQ.bindValue(":scope_uid", user.id);
  if (!dynQ.exec()) {
      qWarning() << "Dashboard: timeline query failed:" << dynQ.lastError().text();
  }
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
