#include "MainWindow.h"
#include "PagesCommon.h"

// ========== 统一页面注册表 ==========
// 单一数据源：页面创建、面包屑、导航均从此处派生，消除多处维护
namespace {
using PageCreator = std::function<BasePage *()>;
struct PageInfo
{
  QString group;     // 面包屑一级（空表示根页面）
  QString title;     // 面包屑二级
  PageCreator creator; // 页面工厂函数
};
static QMap<QString, PageInfo> &pageRegistry()
{
  static QMap<QString, PageInfo> reg = {
      // Dashboard & 个人中心
      {"1", {QString(), QStringLiteral("工作台"), []() { return PageFactory::createDashboardPage(); }}},
      {"102", {QString(), QStringLiteral("待办中心"), []() { return PageFactory::createTodoPage(); }}},
      {"103", {QString(), QStringLiteral("消息中心"), []() { return PageFactory::createMessagePage(); }}},
      // 基础档案
      {"201", {QStringLiteral("基础档案"), QStringLiteral("组织管理"), []() { return PageFactory::createArchivePage("org"); }}},
      {"202", {QStringLiteral("基础档案"), QStringLiteral("小区管理"), []() { return PageFactory::createArchivePage("estate"); }}},
      {"203", {QStringLiteral("基础档案"), QStringLiteral("房屋管理"), []() { return PageFactory::createArchivePage("house"); }}},
      {"204", {QStringLiteral("基础档案"), QStringLiteral("居民管理"), []() { return PageFactory::createArchivePage("resident"); }}},
      {"205", {QStringLiteral("基础档案"), QStringLiteral("车辆管理"), []() { return PageFactory::createArchivePage("vehicle"); }}},
      {"206", {QStringLiteral("基础档案"), QStringLiteral("设施管理"), []() { return PageFactory::createArchivePage("facility"); }}},
      {"207", {QStringLiteral("基础档案"), QStringLiteral("网格管理"), []() { return PageFactory::createArchivePage("grid"); }}},
      {"208", {QStringLiteral("基础档案"), QStringLiteral("特殊群体"), []() { return PageFactory::createArchivePage("special"); }}},
      // 小区管理
      {"301", {QStringLiteral("小区管理"), QStringLiteral("报事报修"), []() { return PageFactory::createPropertyPage("workorder"); }}},
      {"302", {QStringLiteral("小区管理"), QStringLiteral("投诉建议"), []() { return PageFactory::createPropertyPage("complaint"); }}},
      {"303", {QStringLiteral("小区管理"), QStringLiteral("物业巡检"), []() { return PageFactory::createPropertyPage("inspection"); }}},
      {"304", {QStringLiteral("小区管理"), QStringLiteral("公告通知"), []() { return PageFactory::createPropertyPage("announcement"); }}},
      {"305", {QStringLiteral("小区管理"), QStringLiteral("访客管理"), []() { return PageFactory::createPropertyPage("visitor"); }}},
      {"306", {QStringLiteral("小区管理"), QStringLiteral("业委会议题"), []() { return PageFactory::createPropertyPage("topic"); }}},
      {"307", {QStringLiteral("小区管理"), QStringLiteral("停车管理"), []() { return PageFactory::createPropertyPage("parking"); }}},
      {"308", {QStringLiteral("小区管理"), QStringLiteral("物业缴费"), []() { return PageFactory::createPropertyPage("billing"); }}},
      {"309", {QStringLiteral("小区管理"), QStringLiteral("公共收益"), []() { return PageFactory::createPropertyPage("income"); }}},
      // 社区治理
      {"401", {QStringLiteral("社区治理"), QStringLiteral("网格事件"), []() { return PageFactory::createGovernancePage("event"); }}},
      {"402", {QStringLiteral("社区治理"), QStringLiteral("社区巡查"), []() { return PageFactory::createGovernancePage("inspection"); }}},
      {"403", {QStringLiteral("社区治理"), QStringLiteral("重点人群关怀"), []() { return PageFactory::createGovernancePage("care"); }}},
      {"404", {QStringLiteral("社区治理"), QStringLiteral("督办管理"), []() { return PageFactory::createGovernancePage("supervision"); }}},
      {"405", {QStringLiteral("社区治理"), QStringLiteral("民意收集"), []() { return PageFactory::createGovernancePage("opinion"); }}},
      {"406", {QStringLiteral("社区治理"), QStringLiteral("考核管理"), []() { return PageFactory::createGovernancePage("assessment"); }}},
      // 社区服务
      {"501", {QStringLiteral("社区服务"), QStringLiteral("志愿服务"), []() { return PageFactory::createServicePage("volunteer"); }}},
      {"502", {QStringLiteral("社区服务"), QStringLiteral("便民服务"), []() { return PageFactory::createServicePage("convenience"); }}},
      {"503", {QStringLiteral("社区服务"), QStringLiteral("就业服务"), []() { return PageFactory::createServicePage("job"); }}},
      // 统计分析
      {"601", {QStringLiteral("统计分析"), QStringLiteral("工单统计"), []() { return PageFactory::createReportPage("workorder"); }}},
      {"602", {QStringLiteral("统计分析"), QStringLiteral("事件统计"), []() { return PageFactory::createReportPage("event"); }}},
      {"603", {QStringLiteral("统计分析"), QStringLiteral("服务统计"), []() { return PageFactory::createReportPage("service"); }}},
      {"604", {QStringLiteral("统计分析"), QStringLiteral("综合看板"), []() { return PageFactory::createReportPage("dashboard"); }}},
      // 系统管理
      {"701", {QStringLiteral("系统管理"), QStringLiteral("用户管理"), []() { return PageFactory::createSystemPage("user"); }}},
      {"702", {QStringLiteral("系统管理"), QStringLiteral("角色管理"), []() { return PageFactory::createSystemPage("role"); }}},
      {"703", {QStringLiteral("系统管理"), QStringLiteral("菜单管理"), []() { return PageFactory::createSystemPage("menu"); }}},
      {"704", {QStringLiteral("系统管理"), QStringLiteral("字典管理"), []() { return PageFactory::createSystemPage("dict"); }}},
      {"705", {QStringLiteral("系统管理"), QStringLiteral("操作日志"), []() { return PageFactory::createSystemPage("log"); }}},
      {"706", {QStringLiteral("系统管理"), QStringLiteral("智能问答"), []() { return PageFactory::createSystemPage("ai"); }}},
  };
  return reg;
}
} // anonymous namespace


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
  // Icon Rail 按钮 hover 颜色过渡动画
  if (event->type() == QEvent::Enter || event->type() == QEvent::Leave)
  {
    auto *btn = qobject_cast<QToolButton *>(watched);
    if (btn && btn->property("isIconRailItem").toBool())
    {
      QString iconKey = btn->property("iconKey").toString();
      bool isChecked = btn->isChecked();
      // 选中态不动画（保持琥珀色）
      if (!isChecked && !iconKey.isEmpty())
      {
        if (event->type() == QEvent::Enter)
        {
          // hover 进入：灰 → 浅琥珀
          UiKit::animateIconColor(btn, iconKey, QColor("#9A9A9A"), QColor("#D97706"), 150);
        }
        else
        {
          // hover 离开：浅琥珀 → 灰
          UiKit::animateIconColor(btn, iconKey, QColor("#D97706"), QColor("#9A9A9A"), 150);
        }
      }
      return QMainWindow::eventFilter(watched, event);
    }
  }

  if (event->type() == QEvent::MouseButtonPress)
  {
    auto *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton)
    {
      // 类型安全检查：确保 watched 是 QWidget
      if (!qobject_cast<QWidget *>(watched))
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

  // ========== 左侧第一栏：Icon Rail (64px) ==========
  m_iconRail = new QWidget(this);
  m_iconRail->setFixedWidth(64);
  m_iconRail->setStyleSheet(QStringLiteral(
      "QWidget { background: #0A0A0A; }"
      "QToolButton {"
      "  background: transparent; border: none; border-radius: 0;"
      "  padding: 0; margin: 0;"
      "}"
      "QToolButton:hover { background: #1A1A1A; }"
      "QToolButton:checked { background: #141413; border-left: 2px solid #B45309; }"
      "QLabel { background: transparent; color: #FAF9F6; }"
  ));
  auto *railLayout = new QVBoxLayout(m_iconRail);
  railLayout->setContentsMargins(0, 0, 0, 0);
  railLayout->setSpacing(0);

  // Logo 区（顶部 56px，与顶栏对齐）
  auto *logoWidget = new QWidget(m_iconRail);
  logoWidget->setFixedHeight(56);
  logoWidget->setStyleSheet("background: #0A0A0A; border-bottom: 1px solid #1A1A1A;");
  auto *logoLayout = new QVBoxLayout(logoWidget);
  logoLayout->setContentsMargins(0, 0, 0, 0);
  logoLayout->setAlignment(Qt::AlignCenter);
  auto *logoIcon = new QLabel(logoWidget);
  QPixmap logoPix(":/app.svg");
  // 着色为暖白
  QImage logoImg = logoPix.toImage();
  QPainter logoPainter(&logoImg);
  logoPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  logoPainter.fillRect(logoImg.rect(), QColor("#FAF9F6"));
  logoPainter.end();
  logoIcon->setPixmap(QPixmap::fromImage(logoImg).scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  logoIcon->setAlignment(Qt::AlignCenter);
  logoLayout->addWidget(logoIcon);
  railLayout->addWidget(logoWidget);

  // Icon Rail 内容区（可滚动）
  auto *railScroll = new QScrollArea(m_iconRail);
  railScroll->setWidgetResizable(true);
  railScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  railScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  railScroll->setStyleSheet(
      "QScrollArea { border: none; background: transparent; }"
      "QScrollBar:vertical { width: 0; }");
  auto *railContent = new QWidget(railScroll);
  auto *railContentLayout = new QVBoxLayout(railContent);
  railContentLayout->setContentsMargins(0, 8, 0, 8);
  railContentLayout->setSpacing(4);

  // 一级模块图标 — 从菜单数据构建
  auto menus = AuthService::instance().currentUserMenus();
  // 模块图标映射（menuName -> iconKey）
  static const QMap<QString, QString> moduleIcons = {
      {QStringLiteral("工作台"), QStringLiteral("ic_dashboard")},
      {QStringLiteral("基础档案"), QStringLiteral("ic_layers")},
      {QStringLiteral("小区管理"), QStringLiteral("ic_home")},
      {QStringLiteral("社区治理"), QStringLiteral("ic_route")},
      {QStringLiteral("社区服务"), QStringLiteral("ic_heart")},
      {QStringLiteral("统计分析"), QStringLiteral("ic_chart")},
      {QStringLiteral("系统管理"), QStringLiteral("ic_settings")},
  };
  // 模块对应的第一个子页面 key（用于点击图标时跳转）
  static const QMap<QString, QString> moduleFirstPage = {
      {QStringLiteral("工作台"), QStringLiteral("1")},
      {QStringLiteral("基础档案"), QStringLiteral("201")},
      {QStringLiteral("小区管理"), QStringLiteral("301")},
      {QStringLiteral("社区治理"), QStringLiteral("401")},
      {QStringLiteral("社区服务"), QStringLiteral("501")},
      {QStringLiteral("统计分析"), QStringLiteral("601")},
      {QStringLiteral("系统管理"), QStringLiteral("701")},
  };

  for (const auto &menu : menus)
  {
    QString moduleName = menu.menuName;
    QString iconKey = moduleIcons.value(moduleName, QStringLiteral("ic_grid"));
    QString firstPage = moduleFirstPage.value(moduleName, QString::number(menu.id));

    // 工作台特殊：id=1，无子菜单
    if (menu.id == 1)
    {
      firstPage = "1";
    }

    auto *btn = new QToolButton(railContent);
    btn->setFixedSize(64, 56);
    btn->setAutoRaise(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setToolTip(moduleName);
    // 图标着色为暖白
    QPixmap pix(QString(":/icons/%1.svg").arg(iconKey));
    QImage img = pix.toImage();
    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(img.rect(), QColor("#9A9A9A"));
    painter.end();
    btn->setIcon(QIcon(QPixmap::fromImage(img).scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    btn->setIconSize(QSize(22, 22));
    btn->setCheckable(true);

    // 点击切换模块
    connect(btn, &QToolButton::clicked, this, [this, moduleName, firstPage, btn]()
            {
              if (m_activeIconRailItem && m_activeIconRailItem != btn) {
                m_activeIconRailItem->setChecked(false);
                // 恢复旧图标颜色为灰
                QPixmap oldPix = m_activeIconRailItem->icon().pixmap(QSize(22, 22));
                QImage oldImg = oldPix.toImage();
                QPainter p(&oldImg);
                p.setCompositionMode(QPainter::CompositionMode_SourceIn);
                p.fillRect(oldImg.rect(), QColor("#9A9A9A"));
                p.end();
                m_activeIconRailItem->setIcon(QIcon(QPixmap::fromImage(oldImg)));
              }
              btn->setChecked(true);
              // 高亮当前图标为琥珀色
              QPixmap newPix = QPixmap(QString(":/icons/%1.svg").arg(
                  btn->property("iconKey").toString()));
              QImage newImg = newPix.toImage();
              QPainter p2(&newImg);
              p2.setCompositionMode(QPainter::CompositionMode_SourceIn);
              p2.fillRect(newImg.rect(), QColor("#B45309"));
              p2.end();
              btn->setIcon(QIcon(QPixmap::fromImage(newImg).scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
              m_activeIconRailItem = btn;
              switchModule(moduleName);
              switchPage(firstPage); });

    btn->setProperty("iconKey", iconKey);
    btn->setProperty("moduleName", moduleName);
    btn->setProperty("isIconRailItem", true);  // 标记为 Icon Rail 项，用于 hover 动画
    btn->installEventFilter(this);  // 安装事件过滤器，处理 hover 颜色过渡
    m_iconRailItems[moduleName] = btn;
    railContentLayout->addWidget(btn);

    // 默认选中工作台
    if (menu.id == 1 && m_activeIconRailItem == nullptr)
    {
      btn->setChecked(true);
      // 高亮为琥珀色
      QPixmap hpix(QString(":/icons/%1.svg").arg(iconKey));
      QImage himg = hpix.toImage();
      QPainter hp(&himg);
      hp.setCompositionMode(QPainter::CompositionMode_SourceIn);
      hp.fillRect(himg.rect(), QColor("#B45309"));
      hp.end();
      btn->setIcon(QIcon(QPixmap::fromImage(himg).scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
      m_activeIconRailItem = btn;
      m_currentModule = moduleName;
    }
  }

  railContentLayout->addStretch();
  railScroll->setWidget(railContent);
  railLayout->addWidget(railScroll, 1);
  mainLayout->addWidget(m_iconRail);

  // ========== 左侧第二栏：Section Panel (220px) ==========
  m_sectionPanel = new QWidget(this);
  m_sectionPanel->setFixedWidth(220);
  m_sectionPanel->setStyleSheet(QStringLiteral(
      "QWidget { background: #141413; }"
      "QLabel { background: transparent; color: #FAF9F6; }"
      "QPushButton {"
      "  background: transparent; color: rgba(250, 249, 246, 0.55);"
      "  border: none; border-left: 2px solid transparent;"
      "  padding: 12px 20px; text-align: left;"
      "  font-size: 13px; min-height: 38px; font-weight: 400;"
      "}"
      "QPushButton:hover { color: #FAF9F6; background: rgba(250, 249, 246, 0.04); }"
      "QPushButton[active=\"true\"] {"
      "  color: #FAF9F6; background: rgba(180, 83, 9, 0.12);"
      "  border-left: 2px solid #B45309; font-weight: 600;"
      "}"
      "QScrollArea { border: none; background: transparent; }"
      "QScrollBar:vertical { width: 4px; background: transparent; }"
      "QScrollBar::handle:vertical { background: rgba(250, 249, 246, 0.15); border-radius: 0; min-height: 20px; }"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
  ));

  auto *sectionLayout = new QVBoxLayout(m_sectionPanel);
  sectionLayout->setContentsMargins(0, 0, 0, 0);
  sectionLayout->setSpacing(0);

  // Section 标题区（顶部 56px，与顶栏对齐）
  auto *sectionHeader = new QWidget(m_sectionPanel);
  sectionHeader->setFixedHeight(56);
  sectionHeader->setStyleSheet("background: #141413; border-bottom: 1px solid #2A2A2A;");
  auto *headerLayout = new QVBoxLayout(sectionHeader);
  headerLayout->setContentsMargins(20, 0, 20, 0);
  headerLayout->setAlignment(Qt::AlignVCenter);
  m_sectionTitle = new QLabel(QStringLiteral("工作台"), sectionHeader);
  m_sectionTitle->setStyleSheet(
      "color: #FAF9F6; font-size: 15px; font-weight: 600;"
      " font-family: 'Noto Serif SC', 'Source Han Serif SC', 'SimSun', serif;"
      " background: transparent; letter-spacing: 1px;");
  headerLayout->addWidget(m_sectionTitle);
  sectionLayout->addWidget(sectionHeader);

  // Section 内容区（可滚动）
  auto *sectionScroll = new QScrollArea(m_sectionPanel);
  sectionScroll->setWidgetResizable(true);
  sectionScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto *sectionScrollContent = new QWidget(sectionScroll);
  m_sectionLayout = new QVBoxLayout(sectionScrollContent);
  m_sectionLayout->setContentsMargins(0, 8, 0, 8);
  m_sectionLayout->setSpacing(0);
  sectionScroll->setWidget(sectionScrollContent);
  sectionLayout->addWidget(sectionScroll, 1);

  mainLayout->addWidget(m_sectionPanel);

  // 初始化工作台的 Section Panel（工作台无子菜单，显示快捷入口）
  buildSectionPanel(m_currentModule);

  // ========== 右侧内容区 ==========
  auto *rightPanel = new QWidget(this);
  auto *rightLayout = new QVBoxLayout(rightPanel);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(0);

  // 顶部栏
  m_topBar = new QWidget(this);
  m_topBar->setFixedHeight(56);
  m_topBar->setStyleSheet(QStringLiteral(
      "QWidget { background: #FAF9F6; border-bottom: 1px solid #D4D0C8; }"
      "QLabel { color: #6B6B6B; font-size: 13px; background: transparent; }"
      "QPushButton {"
      "  background: transparent; color: #6B6B6B; border: 1px solid #D4D0C8;"
      "  padding: 6px 16px; font-size: 13px; border-radius: 2px;"
      "}"
      "QPushButton:hover { color: #141413; border-color: #141413; }"
      "QToolButton { background: transparent; border: none; border-radius: 2px; padding: 6px; }"
      "QToolButton:hover { background: #F5F2EB; }"
  ));
  auto *topLayout = new QHBoxLayout(m_topBar);
  topLayout->setContentsMargins(24, 0, 24, 0);
  topLayout->setSpacing(0);

  // 面包屑容器（可点击导航）
  m_breadcrumbContainer = new QWidget(m_topBar);
  m_breadcrumbContainer->setStyleSheet("background: transparent;");
  m_breadcrumbLayout = new QHBoxLayout(m_breadcrumbContainer);
  m_breadcrumbLayout->setContentsMargins(0, 0, 0, 0);
  m_breadcrumbLayout->setSpacing(8);
  m_breadcrumbLayout->addStretch();
  topLayout->addWidget(m_breadcrumbContainer);
  topLayout->addStretch();

  // 全局搜索框 — 极简下划线风格
  m_globalSearchEdit = new QLineEdit(m_topBar);
  m_globalSearchEdit->setPlaceholderText(QStringLiteral("搜索功能、数据..."));
  m_globalSearchEdit->setFixedWidth(240);
  m_globalSearchEdit->setStyleSheet(QStringLiteral(
      "QLineEdit {"
      "  background: transparent; border: none; border-bottom: 1px solid #D4D0C8;"
      "  padding: 6px 4px 6px 28px; font-size: 13px; color: #141413;"
      "}"
      "QLineEdit:focus { border-bottom: 2px solid #92400E; padding-bottom: 5px; }"
      "QLineEdit:hover { border-bottom: 1px solid #6B6B6B; }"
      "QLineEdit::placeholder { color: #9A9A9A; }"));
  // 添加搜索图标到左侧（通过 QLabel 覆盖）
  auto *searchIconLabel = new QLabel(m_globalSearchEdit);
  QPixmap searchPix(":/icons/ic_search.svg");
  QImage searchImg = searchPix.toImage();
  QPainter searchPainter(&searchImg);
  searchPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  searchPainter.fillRect(searchImg.rect(), QColor("#9A9A9A"));
  searchPainter.end();
  searchIconLabel->setPixmap(QPixmap::fromImage(searchImg).scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  searchIconLabel->setGeometry(6, 11, 14, 14);
  searchIconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
  topLayout->addWidget(m_globalSearchEdit);
  topLayout->addSpacing(16);

  // 全局搜索下拉结果列表
  m_searchDropdown = new QFrame(this);
  m_searchDropdown->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
  m_searchDropdown->setStyleSheet(QStringLiteral(
      "QFrame { background: #FFFFFF; border: 1px solid #141413; border-radius: 0; }"
      "QListWidget { background: transparent; border: none; outline: none; }"
      "QListWidget::item { padding: 10px 14px; border-bottom: 1px solid #E8E5DE; font-size: 13px; color: #141413; }"
      "QListWidget::item:hover { background: #F5F2EB; color: #92400E; }"
      "QListWidget::item:selected { background: #141413; color: #FAF9F6; }"));
  auto *dropdownLayout = new QVBoxLayout(m_searchDropdown);
  dropdownLayout->setContentsMargins(0, 0, 0, 0);
  dropdownLayout->setSpacing(0);
  m_searchResultList = new QListWidget(m_searchDropdown);
  m_searchResultList->setMinimumWidth(260);
  m_searchResultList->setMaximumHeight(360);
  dropdownLayout->addWidget(m_searchResultList);
  m_searchDropdown->hide();

  connect(m_globalSearchEdit, &QLineEdit::textChanged, this, [this](const QString &text)
          {
        QString keyword = text.trimmed();
        if (keyword.isEmpty()) {
            hideSearchDropdown();
            return;
        }
        performGlobalSearch(keyword); });

  connect(m_globalSearchEdit, &QLineEdit::editingFinished, this, [this]()
          { QTimer::singleShot(200, this, [this]()
                               { hideSearchDropdown(); }); });

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
  m_refreshIconPix = QPixmap(":/icons/ic_refresh.svg");
  if (!m_refreshIconPix.isNull())
  {
    QImage img = m_refreshIconPix.toImage();
    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(img.rect(), QColor("#6B6B6B"));
    painter.end();
    m_refreshIconPix = QPixmap::fromImage(img).scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_refreshBtn->setIcon(QIcon(m_refreshIconPix));
    m_refreshBtn->setIconSize(QSize(18, 18));
  }
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
        int xs = (rotated.width() - 18) / 2;
        int ys = (rotated.height() - 18) / 2;
        if (xs >= 0 && ys >= 0) {
            m_refreshBtn->setIcon(QIcon(rotated.copy(xs, ys, 18, 18)));
        } else {
            m_refreshBtn->setIcon(QIcon(rotated));
        } });
  connect(m_refreshAnimation, &QVariantAnimation::finished, this, [this]()
          { m_refreshBtn->setIcon(QIcon(m_refreshIconPix)); });
  connect(m_refreshBtn, &QToolButton::clicked, this, &MainWindow::onRefreshBtnClicked);
  topLayout->addWidget(m_refreshBtn);
  topLayout->addSpacing(8);

  // 通知铃铛按钮
  m_notifyBtn = new QToolButton(m_topBar);
  m_notifyBtn->setFixedSize(36, 36);
  m_notifyBtn->setCursor(Qt::PointingHandCursor);
  m_notifyBtn->setToolTip(QStringLiteral("通知消息"));
  auto *notifyIcon = new QLabel(m_notifyBtn);
  QPixmap bellPix(":/icons/ic_bell.svg");
  QImage bellImg = bellPix.toImage();
  QPainter bellPainter(&bellImg);
  bellPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  bellPainter.fillRect(bellImg.rect(), QColor("#6B6B6B"));
  bellPainter.end();
  notifyIcon->setPixmap(QPixmap::fromImage(bellImg).scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  notifyIcon->setAlignment(Qt::AlignCenter);
  notifyIcon->setGeometry(9, 9, 18, 18);
  notifyIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

  m_notificationBadge = new QLabel(m_notifyBtn);
  m_notificationBadge->setAlignment(Qt::AlignCenter);
  m_notificationBadge->setStyleSheet(QStringLiteral(
      "QLabel { background: #DC2626; color: #FFFFFF; font-size: 11px; font-weight: bold;"
      " border-radius: 0; min-width: 18px; min-height: 18px; padding: 0 4px; }"));
  m_notificationBadge->setFixedHeight(18);
  m_notificationBadge->setVisible(false);
  m_notificationBadge->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_notificationBadge->raise();

  topLayout->addWidget(m_notifyBtn);
  topLayout->addSpacing(8);

  connect(m_notifyBtn, &QToolButton::clicked, this, &MainWindow::onNotifyBtnClicked);

  // 用户下拉菜单按钮
  m_userMenuBtn = new QToolButton(this);
  m_userMenuBtn->setAutoRaise(true);
  m_userMenuBtn->setCursor(Qt::PointingHandCursor);
  m_userMenuBtn->setStyleSheet(QStringLiteral(
      "QToolButton { background: transparent; border: none; padding: 4px 8px;"
      " color: #141413; font-size: 13px; font-weight: 500; }"
      "QToolButton:hover { color: #92400E; }"));
  m_userMenuBtn->setText(QStringLiteral("admin ▼"));
  m_userMenuBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);

  QMenu *userMenu = new QMenu(m_userMenuBtn);
  userMenu->setStyleSheet(QStringLiteral(
      "QMenu { background: #FFFFFF; border: 1px solid #141413; border-radius: 0; padding: 4px; }"
      "QMenu::item { padding: 10px 24px; font-size: 13px; color: #141413; }"
      "QMenu::item:selected { background: #141413; color: #FAF9F6; }"
      "QMenu::separator { height: 1px; background: #E8E5DE; margin: 4px 8px; }"));
  auto *profileAction = userMenu->addAction(QStringLiteral("个人中心"));
  auto *pwdAction = userMenu->addAction(QStringLiteral("修改密码"));
  userMenu->addSeparator();
  auto *logoutAction = userMenu->addAction(QStringLiteral("退出登录"));

  m_userMenuBtn->setPopupMode(QToolButton::InstantPopup);
  m_userMenuBtn->setMenu(userMenu);

  connect(profileAction, &QAction::triggered, this, [this]()
          { ProfileDialog::show(this); });
  connect(pwdAction, &QAction::triggered, this, [this]()
          { ChangePasswordDialog::show(this); });
  connect(logoutAction, &QAction::triggered, this, [this]()
          {
        if (QMessageBox::question(this, QStringLiteral("确认"), QStringLiteral("确定要退出登录吗？")) == QMessageBox::Yes) {
            emit logoutRequested();
        } });

  topLayout->addWidget(m_userMenuBtn);
  rightLayout->addWidget(m_topBar);

  // 内容区域
  m_contentStack = new QStackedWidget(this);
  m_contentStack->setStyleSheet("QStackedWidget { background: #FAF9F6; }");
  rightLayout->addWidget(m_contentStack);

  mainLayout->addWidget(rightPanel, 1);

  // 默认显示首页
  switchPage("1");
  updateNotificationBadge();
}

// ========== 第二栏 Section Panel 构建 ==========
// 根据一级模块名构建二级面板内容；工作台模块显示快捷入口
void MainWindow::buildSectionPanel(const QString &moduleKey)
{
  if (!m_sectionLayout || moduleKey.isEmpty())
    return;

  // 清空旧内容（保留 stretch）
  QLayoutItem *child;
  while ((child = m_sectionLayout->takeAt(0)) != nullptr)
  {
    if (child->widget())
    {
      child->widget()->deleteLater();
    }
    delete child;
  }
  m_sectionItems.clear();
  m_activeSectionItem = nullptr;

  // 更新标题
  if (m_sectionTitle)
  {
    m_sectionTitle->setText(moduleKey);
  }

  // 模块对应的子页面列表（从统一页面注册表派生）
  // key=pageKey, title=显示名
  struct SectionEntry
  {
    QString pageKey;
    QString title;
  };
  static const QMap<QString, QList<SectionEntry>> moduleSections = {
      {QStringLiteral("工作台"), {
                                    {QStringLiteral("1"), QStringLiteral("工作台首页")},
                                    {QStringLiteral("102"), QStringLiteral("待办中心")},
                                    {QStringLiteral("103"), QStringLiteral("消息中心")},
                                }},
      {QStringLiteral("基础档案"), {
                                      {QStringLiteral("201"), QStringLiteral("组织管理")},
                                      {QStringLiteral("202"), QStringLiteral("小区管理")},
                                      {QStringLiteral("203"), QStringLiteral("房屋管理")},
                                      {QStringLiteral("204"), QStringLiteral("居民管理")},
                                      {QStringLiteral("205"), QStringLiteral("车辆管理")},
                                      {QStringLiteral("206"), QStringLiteral("设施管理")},
                                      {QStringLiteral("207"), QStringLiteral("网格管理")},
                                      {QStringLiteral("208"), QStringLiteral("特殊群体")},
                                  }},
      {QStringLiteral("小区管理"), {
                                      {QStringLiteral("301"), QStringLiteral("报事报修")},
                                      {QStringLiteral("302"), QStringLiteral("投诉建议")},
                                      {QStringLiteral("303"), QStringLiteral("物业巡检")},
                                      {QStringLiteral("304"), QStringLiteral("公告通知")},
                                      {QStringLiteral("305"), QStringLiteral("访客管理")},
                                      {QStringLiteral("306"), QStringLiteral("业委会议题")},
                                      {QStringLiteral("307"), QStringLiteral("停车管理")},
                                      {QStringLiteral("308"), QStringLiteral("物业缴费")},
                                      {QStringLiteral("309"), QStringLiteral("公共收益")},
                                  }},
      {QStringLiteral("社区治理"), {
                                      {QStringLiteral("401"), QStringLiteral("网格事件")},
                                      {QStringLiteral("402"), QStringLiteral("社区巡查")},
                                      {QStringLiteral("403"), QStringLiteral("重点人群关怀")},
                                      {QStringLiteral("404"), QStringLiteral("督办管理")},
                                      {QStringLiteral("405"), QStringLiteral("民意收集")},
                                      {QStringLiteral("406"), QStringLiteral("考核管理")},
                                  }},
      {QStringLiteral("社区服务"), {
                                      {QStringLiteral("501"), QStringLiteral("志愿服务")},
                                      {QStringLiteral("502"), QStringLiteral("便民服务")},
                                      {QStringLiteral("503"), QStringLiteral("就业服务")},
                                  }},
      {QStringLiteral("统计分析"), {
                                      {QStringLiteral("601"), QStringLiteral("工单统计")},
                                      {QStringLiteral("602"), QStringLiteral("事件统计")},
                                      {QStringLiteral("603"), QStringLiteral("服务统计")},
                                      {QStringLiteral("604"), QStringLiteral("综合看板")},
                                  }},
      {QStringLiteral("系统管理"), {
                                      {QStringLiteral("701"), QStringLiteral("用户管理")},
                                      {QStringLiteral("702"), QStringLiteral("角色管理")},
                                      {QStringLiteral("703"), QStringLiteral("菜单管理")},
                                      {QStringLiteral("704"), QStringLiteral("字典管理")},
                                      {QStringLiteral("705"), QStringLiteral("操作日志")},
                                      {QStringLiteral("706"), QStringLiteral("智能问答")},
                                  }},
  };

  const auto entries = moduleSections.value(moduleKey);
  for (const auto &entry : entries)
  {
    QWidget *item = createSectionItem(entry.title, entry.pageKey);
    m_sectionLayout->addWidget(item);
    m_sectionItems[entry.pageKey] = item;
  }

  m_sectionLayout->addStretch();

  // 内容区淡入动画（150ms，克制快速）
  // 只对内容容器做动画，标题保持稳定
  if (m_sectionLayout && m_sectionLayout->parentWidget())
  {
    UiKit::fadeInWidget(m_sectionLayout->parentWidget(), 150);
  }
}

// 创建二级面板项（QPushButton，左侧琥珀色边框表示选中态）
QWidget *MainWindow::createSectionItem(const QString &text, const QString &key)
{
  auto *btn = new QPushButton(text, m_sectionPanel);
  btn->setCursor(Qt::PointingHandCursor);
  btn->setProperty("key", key);
  btn->setProperty("active", false);

  connect(btn, &QPushButton::clicked, this, [this, key, btn]()
          {
            if (m_activeSectionItem && m_activeSectionItem != btn) {
              m_activeSectionItem->setProperty("active", false);
              m_activeSectionItem->style()->unpolish(m_activeSectionItem);
              m_activeSectionItem->style()->polish(m_activeSectionItem);
            }
            btn->setProperty("active", true);
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
            m_activeSectionItem = btn;
            switchPage(key); });

  return btn;
}

// 切换一级模块：更新 Section Panel 内容，并同步 Icon Rail 高亮
void MainWindow::switchModule(const QString &moduleKey)
{
  if (moduleKey == m_currentModule && m_sectionItems.size() > 0)
    return; // 已是当前模块，无需重建

  m_currentModule = moduleKey;
  buildSectionPanel(moduleKey);

  // 同步 Icon Rail 高亮
  auto *railBtn = m_iconRailItems.value(moduleKey, nullptr);
  if (railBtn && m_activeIconRailItem != railBtn)
  {
    // 恢复旧图标为灰色
    if (m_activeIconRailItem)
    {
      m_activeIconRailItem->setChecked(false);
      QString oldIconKey = m_activeIconRailItem->property("iconKey").toString();
      if (!oldIconKey.isEmpty())
      {
        QPixmap oldPix(QString(":/icons/%1.svg").arg(oldIconKey));
        QImage oldImg = oldPix.toImage();
        QPainter p(&oldImg);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(oldImg.rect(), QColor("#9A9A9A"));
        p.end();
        m_activeIconRailItem->setIcon(QIcon(QPixmap::fromImage(oldImg).scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
      }
    }
    // 高亮新图标为琥珀色
    railBtn->setChecked(true);
    QString newIconKey = railBtn->property("iconKey").toString();
    if (!newIconKey.isEmpty())
    {
      QPixmap newPix(QString(":/icons/%1.svg").arg(newIconKey));
      QImage newImg = newPix.toImage();
      QPainter p2(&newImg);
      p2.setCompositionMode(QPainter::CompositionMode_SourceIn);
      p2.fillRect(newImg.rect(), QColor("#B45309"));
      p2.end();
      railBtn->setIcon(QIcon(QPixmap::fromImage(newImg).scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
    m_activeIconRailItem = railBtn;
  }
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
    // Page transition: 淡入动画（200ms OutCubic，克制快速）
    // 注意：不对 stack 子页面做位移，避免布局错乱
    UiKit::fadeInWidget(page, 200);
  }

  // 更新面包屑（从统一页面注册表派生，消除重复维护）
  auto &reg = pageRegistry();
  auto it = reg.constFind(key);
  if (it != reg.constEnd())
  {
    updateBreadcrumb(it->group, it->title);

    // 同步 Section Panel 高亮（如果该页面属于当前模块）
    auto sectionIt = m_sectionItems.find(key);
    if (sectionIt != m_sectionItems.end() && sectionIt.value())
    {
      auto *btn = qobject_cast<QPushButton *>(sectionIt.value());
      if (btn && m_activeSectionItem != btn)
      {
        if (m_activeSectionItem)
        {
          m_activeSectionItem->setProperty("active", false);
          m_activeSectionItem->style()->unpolish(m_activeSectionItem);
          m_activeSectionItem->style()->polish(m_activeSectionItem);
        }
        btn->setProperty("active", true);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
        m_activeSectionItem = btn;
      }
    }
    else
    {
      // 该页面不在当前模块下，需要切换模块
      // 通过页面 key 前缀判断所属模块
      QString newModule;
      if (key == "1" || key == "102" || key == "103")
        newModule = QStringLiteral("工作台");
      else if (key.startsWith("2"))
        newModule = QStringLiteral("基础档案");
      else if (key.startsWith("3"))
        newModule = QStringLiteral("小区管理");
      else if (key.startsWith("4"))
        newModule = QStringLiteral("社区治理");
      else if (key.startsWith("5"))
        newModule = QStringLiteral("社区服务");
      else if (key.startsWith("6"))
        newModule = QStringLiteral("统计分析");
      else if (key.startsWith("7"))
        newModule = QStringLiteral("系统管理");

      if (!newModule.isEmpty() && newModule != m_currentModule)
      {
        switchModule(newModule);
        // 切换模块后再次同步 Section Panel 高亮
        auto it2 = m_sectionItems.find(key);
        if (it2 != m_sectionItems.end() && it2.value())
        {
          auto *btn = qobject_cast<QPushButton *>(it2.value());
          if (btn)
          {
            btn->setProperty("active", true);
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
            m_activeSectionItem = btn;
          }
        }
      }
    }
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

  // 获取上次的未读数（用于检测增加时触发脉冲动画）
  int lastUnread = m_notificationBadge->property("lastUnread").toInt();

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

    // 未读数增加时触发脉冲动画（克制反馈）
    if (unreadCount > lastUnread)
    {
      UiKit::pulseWidget(m_notificationBadge, 400);
    }
  }

  // 保存当前未读数
  m_notificationBadge->setProperty("lastUnread", unreadCount);
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
        qint64 woId = q.value(0).toLongLong();
        QString title = q.value(1).toString();
        QString display = QStringLiteral("[工单] %1").arg(title);
        auto *item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, QStringLiteral("workorder"));
        item->setData(Qt::UserRole + 1, QStringLiteral("301"));
        item->setData(Qt::UserRole + 2, woId);
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
        qint64 evId = q.value(0).toLongLong();
        QString title = q.value(1).toString();
        QString display = QStringLiteral("[事件] %1").arg(title);
        auto *item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, QStringLiteral("event"));
        item->setData(Qt::UserRole + 1, QStringLiteral("401"));
        item->setData(Qt::UserRole + 2, evId);
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

  if (type == "menu")
  {
    if (!target.isEmpty())
      switchPage(target);
  }
  else if (type == "workorder")
  {
    qint64 entityId = item->data(Qt::UserRole + 2).toLongLong();
    // 先导航到工单页面，再弹出详情对话框
    switchPage(QStringLiteral("301"));
    if (entityId > 0)
      WorkOrderDetailDialog::show(this, entityId);
  }
  else if (type == "event")
  {
    qint64 entityId = item->data(Qt::UserRole + 2).toLongLong();
    // 导航到事件页面
    switchPage(QStringLiteral("401"));
    Q_UNUSED(entityId);
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
      "QLabel { color: #6B6B6B; font-size: 13px; background: transparent; padding: 2px 4px; }"
      "QLabel:hover { color: #92400E; text-decoration: underline; }");
  homeLabel->setCursor(Qt::PointingHandCursor);
  homeLabel->installEventFilter(this);
  homeLabel->setProperty("breadcrumbTarget", QStringLiteral("1"));
  m_breadcrumbLayout->addWidget(homeLabel);

  // 模块层级（如果有）
  if (!module.isEmpty())
  {
    // 分隔符
    auto *sep1 = new QLabel(QStringLiteral("/"), m_breadcrumbContainer);
    sep1->setStyleSheet("color: #D4D0C8; font-size: 13px; background: transparent;");
    m_breadcrumbLayout->addWidget(sep1);

    // 模块名（可点击跳转到该模块第一个子页面）
    auto *moduleLabel = new QLabel(module, m_breadcrumbContainer);
    moduleLabel->setStyleSheet(
        "QLabel { color: #6B6B6B; font-size: 13px; background: transparent; padding: 2px 4px; }"
        "QLabel:hover { color: #92400E; text-decoration: underline; }");
    moduleLabel->setCursor(Qt::PointingHandCursor);
    // 查找模块对应的第一个子页面 key
    static const QMap<QString, QString> moduleFirstPage = {
        {QStringLiteral("工作台"), QStringLiteral("1")},
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
  auto *sep2 = new QLabel(QStringLiteral("/"), m_breadcrumbContainer);
  sep2->setStyleSheet("color: #D4D0C8; font-size: 13px; background: transparent;");
  m_breadcrumbLayout->addWidget(sep2);

  auto *pageLabel = new QLabel(page, m_breadcrumbContainer);
  pageLabel->setStyleSheet(
      "QLabel { color: #141413; font-size: 13px; font-weight: 600; background: transparent; padding: 2px 4px; }");
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

  // 从统一页面注册表查找创建函数
  auto &reg = pageRegistry();
  auto it = reg.constFind(key);
  if (it != reg.constEnd() && it->creator)
  {
    page = it->creator();
  }

  // Fallback: 功能开发中占位页面（非 BasePage，直接 QWidget）
  if (!page)
  {
    QWidget *placeholder = new QWidget();
    auto *placeholderLayout = new QVBoxLayout(placeholder);
    placeholderLayout->setContentsMargins(20, 20, 20, 20);
    placeholderLayout->addStretch();
    auto *placeholderLabel = new QLabel(QStringLiteral("功能开发中，敬请期待..."), placeholder);
    placeholderLabel->setStyleSheet("font-size: 18px; color: #6B6B6B; background: transparent;");
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
