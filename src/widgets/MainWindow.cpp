#include "MainWindow.h"
#include "PagesCommon.h"


// 发送通知到指定用户（成员函数：写入后会刷新未读角标）
void MainWindow::sendNotification(int userId, const QString &title, const QString &content, int type, const QString &bizType, int bizId)
{
  DatabaseManager::instance().insert("nt_notification", {{"user_id", userId},
                                                         {"title", title},
                                                         {"content", content},
                                                         {"notification_type", type},
                                                         {"biz_type", bizType},
                                                         {"biz_id", bizId},
                                                         {"is_read", 0},
                                                         {"create_time", QDateTime::currentDateTime()}});
  // 写入后刷新未读角标
  updateNotificationBadge();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  setMinimumSize(1200, 1000);
  setWindowIcon(QIcon(":/app.svg"));
  setWindowTitle(QStringLiteral("智慧社区管理平台"));
  setupUI();
  updateUserInfo();
}

MainWindow::~MainWindow() {}

// eventFilter: 处理侧边栏项和面包屑项的点击事件
// 使用 Qt property 机制实现运行时多态（Qt 惯用模式）
// watched 必须是 QWidget* 或其子类，否则忽略事件
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::MouseButtonPress)
  {
    auto *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton)
    {
      // 类型安全检查：确保 watched 是 QWidget
      if (!qobject_cast<QWidget*>(watched))
        return QMainWindow::eventFilter(watched, event);

      // 侧边栏导航项点击
      QString targetPage = watched->property("targetPage").toString();
      if (!targetPage.isEmpty())
      {
        switchPage(targetPage);
        return true;
      }
      // 面包屑项点击
      QString breadcrumbTarget = watched->property("breadcrumbTarget").toString();
      if (!breadcrumbTarget.isEmpty())
      {
        onBreadcrumbClicked(breadcrumbTarget);
        return true;
      }
    }
  }
  return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setupUI()
{
  auto *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
  auto *mainLayout = new QHBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // 左侧导航栏
  m_sidebar = new QWidget(this);
  m_sidebar->setFixedWidth(240);
  m_sidebar->setStyleSheet(R"(
        QWidget { background: #0f172a; }
        QPushButton {
            background: transparent; color: rgba(255,255,255,0.65);
            border: none; border-left: 3px solid transparent;
            padding: 11px 20px; text-align: left;
            font-size: 14px; min-height: 42px;
        }
        QPushButton:hover { background: rgba(255,255,255,0.06); color: #ffffff; }
        QPushButton[active="true"] { background: rgba(180,83,9,0.12); color: #ffffff; font-weight: 600; border-left: 3px solid #b45309; }
        QLabel { background: transparent; }
    )");
  buildSidebar();
  mainLayout->addWidget(m_sidebar);

  // 右侧内容区
  auto *rightPanel = new QWidget(this);
  auto *rightLayout = new QVBoxLayout(rightPanel);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(0);

  // 顶部栏
  m_topBar = new QWidget(this);
  m_topBar->setFixedHeight(56);
  m_topBar->setStyleSheet(R"(
        QWidget { background: #ffffff; border-bottom: 1px solid #e2e8f0; }
        QLabel { color: #64748b; font-size: 14px; background: transparent; }
        QPushButton {
            background: transparent; color: #64748b; border: 1px solid #e2e8f0;
            padding: 6px 16px; font-size: 13px; border-radius: 4px;
        }
        QPushButton:hover { color: #b45309; border-color: #b45309; }
    )");
  auto *topLayout = new QHBoxLayout(m_topBar);
  topLayout->setContentsMargins(20, 0, 20, 0);

  // 面包屑容器（可点击导航）
  m_breadcrumbContainer = new QWidget(m_topBar);
  m_breadcrumbContainer->setStyleSheet("background: transparent;");
  m_breadcrumbLayout = new QHBoxLayout(m_breadcrumbContainer);
  m_breadcrumbLayout->setContentsMargins(0, 0, 0, 0);
  m_breadcrumbLayout->setSpacing(6);
  m_breadcrumbLayout->addStretch();
  topLayout->addWidget(m_breadcrumbContainer);
  topLayout->addStretch();

  // 全局搜索框
  m_globalSearchEdit = new QLineEdit(m_topBar);
  m_globalSearchEdit->setPlaceholderText(QStringLiteral("搜索功能、数据..."));
  m_globalSearchEdit->setFixedWidth(220);
  m_globalSearchEdit->setStyleSheet(
      "QLineEdit { background: #f1f5f9; border: 1px solid #e2e8f0; border-radius: 4px;"
      " padding: 6px 14px; font-size: 13px; color: #334155; }"
      "QLineEdit:focus { background: #ffffff; border-color: #b45309; }"
      "QLineEdit:hover { border-color: #cbd5e1; }"
      "QLineEdit::placeholder { color: #94a3b8; }");
  topLayout->addWidget(m_globalSearchEdit);
  topLayout->addSpacing(12);

  // 全局搜索下拉结果列表（绝对定位在搜索框下方）
  m_searchDropdown = new QFrame(this);
  m_searchDropdown->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
  m_searchDropdown->setStyleSheet(
      "QFrame { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 4px; }"
      "QListWidget { background: transparent; border: none; outline: none; }"
      "QListWidget::item { padding: 8px 14px; border-bottom: 1px solid #f1f5f9; font-size: 13px; color: #334155; }"
      "QListWidget::item:hover { background: #fff7ed; color: #b45309; }"
      "QListWidget::item:selected { background: #fff7ed; color: #b45309; }");
  auto *dropdownLayout = new QVBoxLayout(m_searchDropdown);
  dropdownLayout->setContentsMargins(0, 0, 0, 0);
  dropdownLayout->setSpacing(0);
  m_searchResultList = new QListWidget(m_searchDropdown);
  m_searchResultList->setMinimumWidth(240);
  m_searchResultList->setMaximumHeight(320);
  dropdownLayout->addWidget(m_searchResultList);
  m_searchDropdown->hide();

  // 搜索框文本变化触发搜索
  connect(m_globalSearchEdit, &QLineEdit::textChanged, this, [this](const QString &text)
          {
        QString keyword = text.trimmed();
        if (keyword.isEmpty()) {
            hideSearchDropdown();
            return;
        }
        performGlobalSearch(keyword); });

  // 搜索框失去焦点时延迟隐藏下拉列表
  connect(m_globalSearchEdit, &QLineEdit::editingFinished, this, [this]()
          { QTimer::singleShot(200, this, [this]()
                               { hideSearchDropdown(); }); });

  // 点击搜索结果项跳转
  connect(m_searchResultList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item)
          {
        if (!item) return;
        int row = m_searchResultList->row(item);
        onSearchResultClicked(row); });
  connect(m_searchResultList, &QListWidget::itemActivated, this, [this](QListWidgetItem *item)
          {
        if (!item) return;
        int row = m_searchResultList->row(item);
        onSearchResultClicked(row); });

  // 全局刷新按钮
  m_refreshBtn = new QToolButton(m_topBar);
  m_refreshBtn->setFixedSize(32, 32);
  m_refreshBtn->setCursor(Qt::PointingHandCursor);
  m_refreshBtn->setToolTip(QStringLiteral("刷新当前页面"));
  m_refreshBtn->setStyleSheet(
      "QToolButton { background: transparent; border: none; border-radius: 4px; }"
      "QToolButton:hover { background: #f1f5f9; }"
      "QToolButton:pressed { background: #e2e8f0; }");
  // 加载刷新图标并着色为灰色
  m_refreshIconPix = QPixmap(":/icons/ic_refresh.svg");
  if (!m_refreshIconPix.isNull())
  {
    QImage img = m_refreshIconPix.toImage();
    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(img.rect(), QColor("#64748b"));
    painter.end();
    m_refreshIconPix = QPixmap::fromImage(img).scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_refreshBtn->setIcon(QIcon(m_refreshIconPix));
    m_refreshBtn->setIconSize(QSize(18, 18));
  }
  // 刷新旋转动画
  m_refreshAnimation = new QVariantAnimation(this);
  m_refreshAnimation->setDuration(500);
  m_refreshAnimation->setStartValue(0);
  m_refreshAnimation->setEndValue(360);
  m_refreshAnimation->setEasingCurve(QEasingCurve::Linear);
  connect(m_refreshAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value)
          {
        if (m_refreshIconPix.isNull()) return;
        int angle = value.toInt();
        QTransform transform;
        transform.rotate(angle);
        QPixmap rotated = m_refreshIconPix.transformed(transform, Qt::SmoothTransformation);
        // 居中裁剪到原始尺寸
        int xs = (rotated.width() - 18) / 2;
        int ys = (rotated.height() - 18) / 2;
        if (xs >= 0 && ys >= 0) {
            m_refreshBtn->setIcon(QIcon(rotated.copy(xs, ys, 18, 18)));
        } else {
            m_refreshBtn->setIcon(QIcon(rotated));
        } });
  connect(m_refreshAnimation, &QVariantAnimation::finished, this, [this]()
          {
        // 动画结束恢复原图标
        m_refreshBtn->setIcon(QIcon(m_refreshIconPix)); });
  connect(m_refreshBtn, &QToolButton::clicked, this, &MainWindow::onRefreshBtnClicked);
  topLayout->addWidget(m_refreshBtn);
  topLayout->addSpacing(8);

  // 通知铃铛按钮（QToolButton + 未读数角标）
  m_notifyBtn = new QToolButton(m_topBar);
  m_notifyBtn->setFixedSize(36, 36);
  m_notifyBtn->setCursor(Qt::PointingHandCursor);
  m_notifyBtn->setStyleSheet(
      "QToolButton { background: transparent; border: none; border-radius: 4px; }"
      "QToolButton:hover { background: #f1f5f9; }");
  m_notifyBtn->setToolTip(QStringLiteral("通知消息"));
  auto *notifyIcon = new QLabel(m_notifyBtn);
  QPixmap bellPix(":/icons/ic_bell.svg");
  // 着色为灰色
  QImage bellImg = bellPix.toImage();
  QPainter bellPainter(&bellImg);
  bellPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  bellPainter.fillRect(bellImg.rect(), QColor("#64748b"));
  bellPainter.end();
  notifyIcon->setPixmap(QPixmap::fromImage(bellImg).scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  notifyIcon->setAlignment(Qt::AlignCenter);
  notifyIcon->setGeometry(9, 9, 18, 18);
  notifyIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

  // 未读数角标（红色圆形数字）
  m_notificationBadge = new QLabel(m_notifyBtn);
  m_notificationBadge->setAlignment(Qt::AlignCenter);
  m_notificationBadge->setStyleSheet(
      "QLabel { background: #dc2626; color: #ffffff; font-size: 10px; font-weight: bold;"
      " border-radius: 4px; min-width: 16px; padding: 0 4px; }");
  m_notificationBadge->setFixedHeight(16);
  m_notificationBadge->setVisible(false);
  m_notificationBadge->setAttribute(Qt::WA_TransparentForMouseEvents);
  // 角标定位到按钮右上角
  m_notificationBadge->raise();

  topLayout->addWidget(m_notifyBtn);
  topLayout->addSpacing(8);

  // 通知铃铛点击跳转到消息中心
  connect(m_notifyBtn, &QToolButton::clicked, this, &MainWindow::onNotifyBtnClicked);

  // 用户下拉菜单按钮（显示角色 | 用户名 + ▼）
  m_userMenuBtn = new QToolButton(this);
  m_userMenuBtn->setAutoRaise(true);
  m_userMenuBtn->setCursor(Qt::PointingHandCursor);
  m_userMenuBtn->setStyleSheet(
      "QToolButton { background: transparent; border: none; padding: 4px 8px;"
      " color: #334155; font-size: 14px; font-weight: 500; }"
      "QToolButton:hover { color: #b45309; }");
  // 占位文本，updateUserInfo() 中会刷新
  m_userMenuBtn->setText(QStringLiteral("admin ▼"));
  m_userMenuBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);

  QMenu *userMenu = new QMenu(m_userMenuBtn);
  userMenu->setStyleSheet(
      "QMenu { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 4px; padding: 4px; }"
      "QMenu::item { padding: 8px 24px; font-size: 13px; color: #334155; border-radius: 4px; }"
      "QMenu::item:selected { background: #fff7ed; color: #b45309; }"
      "QMenu::separator { height: 1px; background: #f1f5f9; margin: 4px 8px; }");
  auto *profileAction = userMenu->addAction(QStringLiteral("个人中心"));
  auto *pwdAction = userMenu->addAction(QStringLiteral("修改密码"));
  userMenu->addSeparator();
  auto *logoutAction = userMenu->addAction(QStringLiteral("退出登录"));

  m_userMenuBtn->setPopupMode(QToolButton::InstantPopup);
  m_userMenuBtn->setMenu(userMenu);

  // 个人中心：弹出个人资料对话框
  connect(profileAction, &QAction::triggered, this, [this]()
          { ProfileDialog::show(this); });

  // 修改密码
  connect(pwdAction, &QAction::triggered, this, [this]()
          { ChangePasswordDialog::show(this); });

  // 退出登录
  connect(logoutAction, &QAction::triggered, this, [this]()
          {
        if (QMessageBox::question(this, QStringLiteral("确认"), QStringLiteral("确定要退出登录吗？")) == QMessageBox::Yes) {
            emit logoutRequested();
        } });

  topLayout->addWidget(m_userMenuBtn);
  rightLayout->addWidget(m_topBar);

  // 内容区域
  m_contentStack = new QStackedWidget(this);
  m_contentStack->setStyleSheet("QStackedWidget { background: #f8fafc; }");
  rightLayout->addWidget(m_contentStack);

  mainLayout->addWidget(rightPanel);

  // 默认显示首页（菜单ID=1 对应工作台）
  switchPage("1");
  // 初始化完成后刷新未读角标
  updateNotificationBadge();
}

void MainWindow::buildSidebar()
{
  auto *scrollArea = new QScrollArea(m_sidebar);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setStyleSheet(
      "QScrollArea { border: none; background: transparent; }"
      "QScrollBar:vertical { width: 4px; background: transparent; }"
      "QScrollBar::handle:vertical { background: rgba(255,255,255,0.15); border-radius: 2px; min-height: 20px; }"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");

  auto *scrollContent = new QWidget(scrollArea);
  auto *layout = new QVBoxLayout(scrollContent);
  layout->setContentsMargins(0, 4, 0, 0);
  layout->setSpacing(0);

  auto menus = AuthService::instance().currentUserMenus();
  for (const auto &menu : menus)
  {
    // 工作台(id=1)特殊处理：直接作为可点击项，跳转到首页
    if (menu.id == 1)
    {
      auto *item = createSidebarItem(menu.icon, menu.menuName, "1", false);
      layout->addWidget(item);
      m_sidebarItems["1"] = item;
      if (m_activeSidebarItem == nullptr)
      {
        m_activeSidebarItem = item;
        item->setProperty("active", true);
        item->style()->unpolish(item);
        item->style()->polish(item);
      }
    }
    else if (!menu.children.isEmpty())
    {
      // 父菜单 - 可折叠分组
      auto *groupWidget = new QWidget(scrollContent);
      groupWidget->setStyleSheet("background: transparent;");

      auto *groupLayout = new QVBoxLayout(groupWidget);
      groupLayout->setContentsMargins(0, 0, 0, 0);
      groupLayout->setSpacing(0);

      // 分组标题按钮（可点击展开/折叠）
      auto *groupHeader = new QPushButton(menu.menuName, groupWidget);
      groupHeader->setCheckable(true);
      groupHeader->setObjectName("groupHeader");
      groupHeader->setStyleSheet(
          "QPushButton#groupHeader {"
          "   background: transparent; color: rgba(255,255,255,0.40); border: none;"
          "   padding: 12px 20px 6px 20px; text-align: left; font-size: 11px; font-weight: 600;"
          "   min-height: 28px; letter-spacing: 0; text-transform: none;"
          "}"
          "QPushButton#groupHeader:hover { color: rgba(255,255,255,0.65); }"
          "QPushButton#groupHeader::indicator { width: 0; height: 0; }");

      // 子菜单容器
      auto *childContainer = new QWidget(groupWidget);
      childContainer->setObjectName("childContainer");
      auto *childLayout = new QVBoxLayout(childContainer);
      childLayout->setContentsMargins(0, 0, 0, 2);
      childLayout->setSpacing(0);

      for (const auto &child : menu.children)
      {
        auto *item = createSidebarItem(child.icon, child.menuName, QString::number(child.id), false);
        childLayout->addWidget(item);
        m_sidebarItems[QString::number(child.id)] = item;
        if (m_activeSidebarItem == nullptr)
        {
          m_activeSidebarItem = item;
          item->setProperty("active", true);
          item->style()->unpolish(item);
          item->style()->polish(item);
        }
      }

      // 添加到列表（用于手风琴模式）
      m_groupHeaders.append(groupHeader);
      m_groupContainers.append(childContainer);

      // 手风琴模式：默认只有第一个分组展开
      bool isFirstGroup = (m_groupHeaders.size() == 1);
      groupHeader->setChecked(isFirstGroup);
      if (!isFirstGroup)
        childContainer->setVisible(false);

      // 连接折叠/展开信号 - 手风琴模式
      connect(groupHeader, &QPushButton::toggled, this, [this, groupHeader, childContainer](bool checked)
              {
                childContainer->setVisible(checked);
                if (checked) {
                    // 手风琴模式：折叠其他分组
                    for (int i = 0; i < m_groupHeaders.size(); ++i) {
                        if (m_groupHeaders[i] != groupHeader && m_groupHeaders[i]->isChecked()) {
                            m_groupHeaders[i]->blockSignals(true);
                            m_groupHeaders[i]->setChecked(false);
                            m_groupHeaders[i]->blockSignals(false);
                            m_groupContainers[i]->setVisible(false);
                        }
                    }
                } });

      groupLayout->addWidget(groupHeader);
      groupLayout->addWidget(childContainer);
      layout->addWidget(groupWidget);
    }
  }

  layout->addStretch();
  scrollArea->setWidget(scrollContent);

  auto *sidebarLayout = new QVBoxLayout(m_sidebar);
  sidebarLayout->setContentsMargins(0, 0, 0, 0);
  sidebarLayout->setSpacing(0);

  // Logo
  auto *logoWidget = new QWidget(m_sidebar);
  logoWidget->setFixedHeight(56);
  logoWidget->setStyleSheet("background: #0f172a;");
  auto *logoLayout = new QHBoxLayout(logoWidget);
  logoLayout->setContentsMargins(20, 0, 20, 0);
  logoLayout->setSpacing(10);
  auto *logoIcon = new QLabel(logoWidget);
  QPixmap logoPix(":/app.svg");
  logoIcon->setPixmap(logoPix.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  logoIcon->setStyleSheet("background: transparent;");
  auto *logoText = new QLabel(QStringLiteral("智慧社区"), logoWidget);
  logoText->setStyleSheet("color: #ffffff; font-size: 16px; font-weight: 700; background: transparent; letter-spacing: 2px;");
  logoLayout->addWidget(logoIcon);
  logoLayout->addWidget(logoText);
  logoLayout->addStretch();
  sidebarLayout->addWidget(logoWidget);
  sidebarLayout->addWidget(scrollArea);
}

QWidget *MainWindow::createSidebarItem(const QString &icon, const QString &text, const QString &key, bool isHeader)
{
  auto *btn = new QPushButton(text, m_sidebar);
  btn->setProperty("key", key);
  if (isHeader)
  {
    btn->setEnabled(false);
  }
  else
  {
    connect(btn, &QPushButton::clicked, this, [this, key, btn]()
            {
            if (m_activeSidebarItem && m_activeSidebarItem != btn) {
                m_activeSidebarItem->setProperty("active", false);
                m_activeSidebarItem->style()->unpolish(m_activeSidebarItem);
                m_activeSidebarItem->style()->polish(m_activeSidebarItem);
            }
            btn->setProperty("active", true);
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
            m_activeSidebarItem = btn;
            switchPage(key); });
  }
  return btn;
}

void MainWindow::switchPage(const QString &key)
{
  m_currentPage = key;
  getOrCreatePage(key); // 确保页面已创建并缓存
  QWidget *page = m_pageCache.value(key);
  if (page)
  {
    int idx = m_contentStack->indexOf(page);
    if (idx < 0)
    {
      m_contentStack->addWidget(page);
      idx = m_contentStack->count() - 1;
    }
    m_contentStack->setCurrentIndex(idx);
    // Page transition: fade-in animation
    UiKit::fadeInWidget(page);
  }

  // 更新面包屑（可点击导航）
  static const QMap<QString, QPair<QString, QString>> breadcrumbMap = {
      {"1", {"", QStringLiteral("工作台")}},
      {"102", {"", QStringLiteral("待办中心")}},
      {"103", {"", QStringLiteral("消息中心")}},
      {"201", {QStringLiteral("基础档案"), QStringLiteral("组织管理")}},
      {"202", {QStringLiteral("基础档案"), QStringLiteral("小区管理")}},
      {"203", {QStringLiteral("基础档案"), QStringLiteral("房屋管理")}},
      {"204", {QStringLiteral("基础档案"), QStringLiteral("居民管理")}},
      {"205", {QStringLiteral("基础档案"), QStringLiteral("车辆管理")}},
      {"206", {QStringLiteral("基础档案"), QStringLiteral("设施管理")}},
      {"207", {QStringLiteral("基础档案"), QStringLiteral("网格管理")}},
      {"208", {QStringLiteral("基础档案"), QStringLiteral("特殊群体")}},
      {"301", {QStringLiteral("小区管理"), QStringLiteral("报事报修")}},
      {"302", {QStringLiteral("小区管理"), QStringLiteral("投诉建议")}},
      {"303", {QStringLiteral("小区管理"), QStringLiteral("物业巡检")}},
      {"304", {QStringLiteral("小区管理"), QStringLiteral("公告通知")}},
      {"305", {QStringLiteral("小区管理"), QStringLiteral("访客管理")}},
      {"306", {QStringLiteral("小区管理"), QStringLiteral("业委会议题")}},
      {"307", {QStringLiteral("小区管理"), QStringLiteral("停车管理")}},
      {"308", {QStringLiteral("小区管理"), QStringLiteral("物业缴费")}},
      {"309", {QStringLiteral("小区管理"), QStringLiteral("公共收益")}},
      {"401", {QStringLiteral("社区治理"), QStringLiteral("网格事件")}},
      {"402", {QStringLiteral("社区治理"), QStringLiteral("社区巡查")}},
      {"403", {QStringLiteral("社区治理"), QStringLiteral("重点人群关怀")}},
      {"404", {QStringLiteral("社区治理"), QStringLiteral("督办管理")}},
      {"405", {QStringLiteral("社区治理"), QStringLiteral("民意收集")}},
      {"406", {QStringLiteral("社区治理"), QStringLiteral("考核管理")}},
      {"501", {QStringLiteral("社区服务"), QStringLiteral("志愿服务")}},
      {"502", {QStringLiteral("社区服务"), QStringLiteral("便民服务")}},
      {"503", {QStringLiteral("社区服务"), QStringLiteral("就业服务")}},
      {"601", {QStringLiteral("统计分析"), QStringLiteral("工单统计")}},
      {"602", {QStringLiteral("统计分析"), QStringLiteral("事件统计")}},
      {"603", {QStringLiteral("统计分析"), QStringLiteral("服务统计")}},
      {"604", {QStringLiteral("统计分析"), QStringLiteral("综合看板")}},
      {"701", {QStringLiteral("系统管理"), QStringLiteral("用户管理")}},
      {"702", {QStringLiteral("系统管理"), QStringLiteral("角色管理")}},
      {"703", {QStringLiteral("系统管理"), QStringLiteral("菜单管理")}},
      {"704", {QStringLiteral("系统管理"), QStringLiteral("字典管理")}},
      {"705", {QStringLiteral("系统管理"), QStringLiteral("操作日志")}},
      {"706", {QStringLiteral("系统管理"), QStringLiteral("智能问答")}}};
  auto it = breadcrumbMap.constFind(key);
  if (it != breadcrumbMap.constEnd())
  {
    updateBreadcrumb(it.value().first, it.value().second);
  }
  else
  {
    updateBreadcrumb(QString(), QStringLiteral("首页"));
  }

  // 切换页面时刷新未读角标
  updateNotificationBadge();
}

void MainWindow::updateUserInfo()
{
  const auto &user = AuthService::instance().currentUser();
  QString displayName = user.nickname.isEmpty() ? user.username : user.nickname;
  QString roleText = user.roleNames.isEmpty() ? QStringLiteral("未分配角色") : user.roleNames.join(", ");
  if (m_userMenuBtn)
  {
    m_userMenuBtn->setText(QStringLiteral("%1 | %2 ▼").arg(roleText).arg(displayName));
  }
}

// ========== 通知铃铛未读数角标 ==========
void MainWindow::updateNotificationBadge()
{
  if (!m_notificationBadge || !m_notifyBtn)
    return;

  qint64 uid = AuthService::instance().currentUser().id;
  int unreadCount = 0;
  if (uid > 0)
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT COUNT(*) FROM nt_notification WHERE user_id = :uid AND is_read = 0");
    q.bindValue(":uid", uid);
    if (q.exec() && q.next())
    {
      unreadCount = q.value(0).toInt();
    }
  }

  if (unreadCount <= 0)
  {
    m_notificationBadge->setVisible(false);
  }
  else
  {
    QString text = unreadCount > 99 ? QStringLiteral("99+") : QString::number(unreadCount);
    m_notificationBadge->setText(text);
    m_notificationBadge->setVisible(true);
    m_notificationBadge->adjustSize();
    // 角标定位到按钮右上角
    int badgeW = m_notificationBadge->sizeHint().width();
    int badgeH = 16;
    int x = m_notifyBtn->width() - badgeW - 2;
    int y = 2;
    if (x < 0)
      x = 0;
    m_notificationBadge->setGeometry(x, y, badgeW, badgeH);
    m_notificationBadge->raise();
  }
}

// ========== 全局搜索功能 ==========
void MainWindow::performGlobalSearch(const QString &keyword)
{
  if (!m_searchResultList || !m_searchDropdown)
    return;

  m_searchResultList->clear();
  QString likePattern = "%" + keyword + "%";

  // 1. 搜索菜单名
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT id, menu_name, path FROM sys_menu WHERE menu_name LIKE :keyword AND status = 0 AND menu_type = 2 ORDER BY sort_order LIMIT 10");
    q.bindValue(":keyword", likePattern);
    if (q.exec())
    {
      while (q.next())
      {
        QString menuName = q.value(1).toString();
        QString path = q.value(2).toString();
        QString menuKey = QString::number(q.value(0).toLongLong());
        QString display = QStringLiteral("[菜单] %1").arg(menuName);
        auto *item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, QStringLiteral("menu"));
        item->setData(Qt::UserRole + 1, menuKey);
        m_searchResultList->addItem(item);
      }
    }
  }

  // 2. 搜索工单号/标题
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT id, title FROM wo_work_order WHERE (order_no LIKE :keyword OR title LIKE :keyword) AND del_flag = 0 LIMIT 10");
    q.bindValue(":keyword", likePattern);
    if (q.exec())
    {
      while (q.next())
      {
        QString title = q.value(1).toString();
        QString display = QStringLiteral("[工单] %1").arg(title);
        auto *item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, QStringLiteral("workorder"));
        item->setData(Qt::UserRole + 1, QStringLiteral("301"));
        m_searchResultList->addItem(item);
      }
    }
  }

  // 3. 搜索事件号/标题
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT id, title FROM ge_event WHERE (event_no LIKE :keyword OR title LIKE :keyword) AND del_flag = 0 LIMIT 10");
    q.bindValue(":keyword", likePattern);
    if (q.exec())
    {
      while (q.next())
      {
        QString title = q.value(1).toString();
        QString display = QStringLiteral("[事件] %1").arg(title);
        auto *item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, QStringLiteral("event"));
        item->setData(Qt::UserRole + 1, QStringLiteral("401"));
        m_searchResultList->addItem(item);
      }
    }
  }

  if (m_searchResultList->count() == 0)
  {
    auto *emptyItem = new QListWidgetItem(QStringLiteral("未找到相关结果"));
    emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
    m_searchResultList->addItem(emptyItem);
  }

  showSearchDropdown();
}

void MainWindow::showSearchDropdown()
{
  if (!m_searchDropdown || !m_globalSearchEdit)
    return;
  // 定位到搜索框下方
  QPoint bottomLeft = m_globalSearchEdit->mapToGlobal(QPoint(0, m_globalSearchEdit->height()));
  int dropdownWidth = m_globalSearchEdit->width();
  m_searchDropdown->setFixedWidth(dropdownWidth);
  m_searchDropdown->move(bottomLeft);
  m_searchDropdown->show();
  m_searchDropdown->raise();
}

void MainWindow::hideSearchDropdown()
{
  if (m_searchDropdown)
  {
    m_searchDropdown->hide();
  }
}

void MainWindow::onSearchResultClicked(int row)
{
  if (!m_searchResultList || row < 0)
    return;
  QListWidgetItem *item = m_searchResultList->item(row);
  if (!item)
    return;

  QString type = item->data(Qt::UserRole).toString();
  QString target = item->data(Qt::UserRole + 1).toString();

  hideSearchDropdown();
  if (m_globalSearchEdit)
    m_globalSearchEdit->clear();

  if (type == "menu" || type == "workorder" || type == "event")
  {
    if (!target.isEmpty())
    {
      switchPage(target);
    }
  }
}


// ========== 面包屑可点击导航 ==========
void MainWindow::updateBreadcrumb(const QString &module, const QString &page)
{
  if (!m_breadcrumbContainer || !m_breadcrumbLayout)
    return;

  // 清除旧的面包屑项
  QLayoutItem *item;
  while ((item = m_breadcrumbLayout->takeAt(0)) != nullptr)
  {
    if (item->widget())
    {
      item->widget()->deleteLater();
    }
    delete item;
  }

  // "首页" 项：可点击跳转到工作台
  auto *homeLabel = new QLabel(QStringLiteral("首页"), m_breadcrumbContainer);
  homeLabel->setStyleSheet(
      "QLabel { color: #64748b; font-size: 13px; background: transparent; padding: 2px 4px; }"
      "QLabel:hover { color: #64748b; text-decoration: underline; }");
  homeLabel->setCursor(Qt::PointingHandCursor);
  homeLabel->installEventFilter(this);
  homeLabel->setProperty("breadcrumbTarget", QStringLiteral("1"));
  m_breadcrumbLayout->addWidget(homeLabel);

  // 模块层级（如果有）
  if (!module.isEmpty())
  {
    // 分隔符
    auto *sep1 = new QLabel(QStringLiteral(" > "), m_breadcrumbContainer);
    sep1->setStyleSheet("color: #cbd5e1; font-size: 13px; background: transparent;");
    m_breadcrumbLayout->addWidget(sep1);

    // 模块名（可点击跳转到该模块第一个子页面）
    auto *moduleLabel = new QLabel(module, m_breadcrumbContainer);
    moduleLabel->setStyleSheet(
        "QLabel { color: #64748b; font-size: 13px; background: transparent; padding: 2px 4px; }"
        "QLabel:hover { color: #64748b; text-decoration: underline; }");
    moduleLabel->setCursor(Qt::PointingHandCursor);
    // 查找模块对应的第一个子页面 key
    static const QMap<QString, QString> moduleFirstPage = {
        {QStringLiteral("基础档案"), QStringLiteral("201")},
        {QStringLiteral("小区管理"), QStringLiteral("301")},
        {QStringLiteral("社区治理"), QStringLiteral("401")},
        {QStringLiteral("社区服务"), QStringLiteral("501")},
        {QStringLiteral("统计分析"), QStringLiteral("601")},
        {QStringLiteral("系统管理"), QStringLiteral("701")}};
    QString target = moduleFirstPage.value(module, QString());
    if (!target.isEmpty())
    {
      moduleLabel->installEventFilter(this);
      moduleLabel->setProperty("breadcrumbTarget", target);
    }
    m_breadcrumbLayout->addWidget(moduleLabel);
  }

  // 分隔符 + 当前页面（不可点击，加粗）
  auto *sep2 = new QLabel(QStringLiteral(" > "), m_breadcrumbContainer);
  sep2->setStyleSheet("color: #cbd5e1; font-size: 13px; background: transparent;");
  m_breadcrumbLayout->addWidget(sep2);

  auto *pageLabel = new QLabel(page, m_breadcrumbContainer);
  pageLabel->setStyleSheet(
      "QLabel { color: #0f172a; font-size: 13px; font-weight: 600; background: transparent; padding: 2px 4px; }");
  m_breadcrumbLayout->addWidget(pageLabel);

  m_breadcrumbLayout->addStretch();
}

void MainWindow::onBreadcrumbClicked(const QString &target)
{
  if (target.isEmpty())
    return;
  switchPage(target);
}

// ========== 全局刷新按钮 ==========
void MainWindow::refreshCurrentPage()
{
  if (m_currentPage.isEmpty())
    return;

  // 清除当前页面在缓存中的记录
  if (m_pageCache.contains(m_currentPage))
  {
    QWidget *oldPage = m_pageCache.take(m_currentPage);
    if (oldPage)
    {
      int idx = m_contentStack->indexOf(oldPage);
      if (idx >= 0)
      {
        m_contentStack->removeWidget(oldPage);
      }
      oldPage->deleteLater();
    }
  }

  // 重新创建页面
  getOrCreatePage(m_currentPage);
  QWidget *page = m_pageCache.value(m_currentPage);
  if (page)
  {
    int idx = m_contentStack->indexOf(page);
    if (idx < 0)
    {
      m_contentStack->addWidget(page);
      idx = m_contentStack->count() - 1;
    }
    m_contentStack->setCurrentIndex(idx);
    UiKit::fadeInWidget(page);
  }
}

void MainWindow::onRefreshBtnClicked()
{
  // 立即开始旋转动画
  if (m_refreshAnimation)
  {
    m_refreshAnimation->stop();
    m_refreshAnimation->start();
  }
  // 执行刷新
  refreshCurrentPage();
}

void MainWindow::onNotifyBtnClicked()
{
  switchPage("103");
}

BasePage *MainWindow::getOrCreatePage(const QString &key)
{
  if (m_pageCache.contains(key))
    return qobject_cast<BasePage *>(m_pageCache[key]);

  BasePage *page = nullptr;

  // Dashboard
  if (key == "1")
  {
    page = PageFactory::createDashboardPage();
  }
  // Todo & Message center
  else if (key == "102")
    page = PageFactory::createTodoPage();
  else if (key == "103")
    page = PageFactory::createMessagePage();
  // Archive pages
  else if (key == "201")
    page = PageFactory::createArchivePage("org");
  else if (key == "202")
    page = PageFactory::createArchivePage("estate");
  else if (key == "203")
    page = PageFactory::createArchivePage("house");
  else if (key == "204")
    page = PageFactory::createArchivePage("resident");
  else if (key == "205")
    page = PageFactory::createArchivePage("vehicle");
  else if (key == "206")
    page = PageFactory::createArchivePage("facility");
  else if (key == "207")
    page = PageFactory::createArchivePage("grid");
  else if (key == "208")
    page = PageFactory::createArchivePage("special");
  // Property pages
  else if (key == "301")
    page = PageFactory::createPropertyPage("workorder");
  else if (key == "302")
    page = PageFactory::createPropertyPage("complaint");
  else if (key == "303")
    page = PageFactory::createPropertyPage("inspection");
  else if (key == "304")
    page = PageFactory::createPropertyPage("announcement");
  else if (key == "305")
    page = PageFactory::createPropertyPage("visitor");
  else if (key == "306")
    page = PageFactory::createPropertyPage("topic");
  else if (key == "307")
    page = PageFactory::createPropertyPage("parking");
  else if (key == "308")
    page = PageFactory::createPropertyPage("billing");
  else if (key == "309")
    page = PageFactory::createPropertyPage("income");
  // Governance pages
  else if (key == "401")
    page = PageFactory::createGovernancePage("event");
  else if (key == "402")
    page = PageFactory::createGovernancePage("inspection");
  else if (key == "403")
    page = PageFactory::createGovernancePage("care");
  else if (key == "404")
    page = PageFactory::createGovernancePage("supervision");
  else if (key == "405")
    page = PageFactory::createGovernancePage("opinion");
  else if (key == "406")
    page = PageFactory::createGovernancePage("assessment");
  // Service pages
  else if (key == "501")
    page = PageFactory::createServicePage("volunteer");
  else if (key == "502")
    page = PageFactory::createServicePage("convenience");
  else if (key == "503")
    page = PageFactory::createServicePage("job");
  // Report pages
  else if (key == "601")
    page = PageFactory::createReportPage("workorder");
  else if (key == "602")
    page = PageFactory::createReportPage("event");
  else if (key == "603")
    page = PageFactory::createReportPage("service");
  else if (key == "604")
    page = PageFactory::createReportPage("dashboard");
  // System pages
  else if (key == "701")
    page = PageFactory::createSystemPage("user");
  else if (key == "702")
    page = PageFactory::createSystemPage("role");
  else if (key == "703")
    page = PageFactory::createSystemPage("menu");
  else if (key == "704")
    page = PageFactory::createSystemPage("dict");
  else if (key == "705")
    page = PageFactory::createSystemPage("log");
  else if (key == "706")
    page = PageFactory::createSystemPage("ai");

  // Fallback: 功能开发中占位页面（非 BasePage，直接 QWidget）
  if (!page)
  {
    QWidget *placeholder = new QWidget();
    auto *placeholderLayout = new QVBoxLayout(placeholder);
    placeholderLayout->setContentsMargins(20, 20, 20, 20);
    placeholderLayout->addStretch();
    auto *placeholderLabel = new QLabel(QStringLiteral("功能开发中，敬请期待..."), placeholder);
    placeholderLabel->setStyleSheet("font-size: 18px; color: #64748b; background: transparent;");
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLayout->addWidget(placeholderLabel);
    placeholderLayout->addStretch();
    m_pageCache[key] = placeholder;
    return nullptr;
  }

  // 信号解耦：页面通过信号请求服务，不直接调用 MainWindow 方法
  connect(page, &BasePage::sendNotificationRequested, this, &MainWindow::sendNotification);
  connect(page, &BasePage::navigateToRequested, this, &MainWindow::switchPage);

  m_pageCache[key] = page;
  return page;
}
