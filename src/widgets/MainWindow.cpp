#include "MainWindow.h"
#include "pages/BasePage.h"
#include "NavigationRail.h"
#include "BreadcrumbBar.h"
#include "GlobalSearchController.h"
#include "NotificationManager.h"
#include "PageCache.h"
#include "PagesCommon.h"
#include <QImage>
#include <QPainter>
#include <QVariantAnimation>
#include "dialogs/ChangePasswordDialog.h"
#include "dialogs/ProfileDialog.h"
#include "dialogs/WorkOrderDetailDialog.h"
#include "services/AuthService.h"

// ========== 统一页面注册表 ==========
// 单一数据源：面包屑、导航均从此处派生，消除多处维护
namespace {
struct PageInfo
{
    QString group; // 面包屑一级（空表示根页面）
    QString title; // 面包屑二级
};

static QMap<QString, PageInfo> &pageRegistry()
{
    static QMap<QString, PageInfo> reg = {
        // Dashboard & 个人中心
        {"1", {QString(), QStringLiteral("工作台")}},
        {"102", {QString(), QStringLiteral("待办中心")}},
        {"103", {QString(), QStringLiteral("消息中心")}},
        // 基础档案
        {"201", {QStringLiteral("基础档案"), QStringLiteral("组织管理")}},
        {"202", {QStringLiteral("基础档案"), QStringLiteral("小区管理")}},
        {"203", {QStringLiteral("基础档案"), QStringLiteral("房屋管理")}},
        {"204", {QStringLiteral("基础档案"), QStringLiteral("居民管理")}},
        {"205", {QStringLiteral("基础档案"), QStringLiteral("车辆管理")}},
        {"206", {QStringLiteral("基础档案"), QStringLiteral("设施管理")}},
        {"207", {QStringLiteral("基础档案"), QStringLiteral("网格管理")}},
        {"208", {QStringLiteral("基础档案"), QStringLiteral("特殊群体")}},
        // 小区管理
        {"301", {QStringLiteral("小区管理"), QStringLiteral("报事报修")}},
        {"302", {QStringLiteral("小区管理"), QStringLiteral("投诉建议")}},
        {"303", {QStringLiteral("小区管理"), QStringLiteral("物业巡检")}},
        {"304", {QStringLiteral("小区管理"), QStringLiteral("公告通知")}},
        {"305", {QStringLiteral("小区管理"), QStringLiteral("访客管理")}},
        {"306", {QStringLiteral("小区管理"), QStringLiteral("业委会议题")}},
        {"307", {QStringLiteral("小区管理"), QStringLiteral("停车管理")}},
        {"308", {QStringLiteral("小区管理"), QStringLiteral("物业缴费")}},
        {"309", {QStringLiteral("小区管理"), QStringLiteral("公共收益")}},
        // 社区治理
        {"401", {QStringLiteral("社区治理"), QStringLiteral("网格事件")}},
        {"402", {QStringLiteral("社区治理"), QStringLiteral("社区巡查")}},
        {"403", {QStringLiteral("社区治理"), QStringLiteral("重点人群关怀")}},
        {"404", {QStringLiteral("社区治理"), QStringLiteral("督办管理")}},
        {"405", {QStringLiteral("社区治理"), QStringLiteral("民意收集")}},
        {"406", {QStringLiteral("社区治理"), QStringLiteral("考核管理")}},
        // 社区服务
        {"501", {QStringLiteral("社区服务"), QStringLiteral("志愿服务")}},
        {"502", {QStringLiteral("社区服务"), QStringLiteral("便民服务")}},
        {"503", {QStringLiteral("社区服务"), QStringLiteral("就业服务")}},
        // 统计分析
        {"601", {QStringLiteral("统计分析"), QStringLiteral("工单统计")}},
        {"602", {QStringLiteral("统计分析"), QStringLiteral("事件统计")}},
        {"603", {QStringLiteral("统计分析"), QStringLiteral("服务统计")}},
        {"604", {QStringLiteral("统计分析"), QStringLiteral("综合看板")}},
        // 系统管理
        {"701", {QStringLiteral("系统管理"), QStringLiteral("用户管理")}},
        {"702", {QStringLiteral("系统管理"), QStringLiteral("角色管理")}},
        {"703", {QStringLiteral("系统管理"), QStringLiteral("菜单管理")}},
        {"704", {QStringLiteral("系统管理"), QStringLiteral("字典管理")}},
        {"705", {QStringLiteral("系统管理"), QStringLiteral("操作日志")}},
        {"706", {QStringLiteral("系统管理"), QStringLiteral("智能问答")}},
    };
    return reg;
}

// 从 pageRegistry 派生模块 -> 首页 key
static const QMap<QString, QString> &moduleFirstPageMap()
{
    static QMap<QString, QString> map;
    if (map.isEmpty())
    {
        const auto &reg = pageRegistry();
        for (auto it = reg.begin(); it != reg.end(); ++it)
        {
            const QString &group = it.value().group;
            if (!group.isEmpty() && !map.contains(group))
                map[group] = it.key();
        }
        map[QStringLiteral("工作台")] = QStringLiteral("1");
    }
    return map;
}

// 构建 NavigationRail 所需的模块页面列表
static QMap<QString, QList<QPair<QString, QString>>> buildModulePages()
{
    QMap<QString, QList<QPair<QString, QString>>> result;
    result[QStringLiteral("工作台")] = {
        {QStringLiteral("1"), QStringLiteral("工作台首页")},
        {QStringLiteral("102"), QStringLiteral("待办中心")},
        {QStringLiteral("103"), QStringLiteral("消息中心")},
    };
    const auto &reg = pageRegistry();
    for (auto it = reg.begin(); it != reg.end(); ++it)
    {
        const QString &group = it.value().group;
        if (!group.isEmpty())
            result[group].append({it.key(), it.value().title});
    }
    return result;
}
} // anonymous namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setMinimumSize(1200, 1000);
    setWindowIcon(QIcon(":/app.svg"));
    setWindowTitle(QStringLiteral("智慧社区管理平台"));
    setupUI();
    updateUserInfo();
}

MainWindow::~MainWindow() {}

void MainWindow::sendNotification(int userId, const QString &title, const QString &content,
                                  int type, const QString &bizType, int bizId)
{
    DatabaseManager::instance().insert("nt_notification", {{"user_id", userId},
                                                           {"title", title},
                                                           {"content", content},
                                                           {"notification_type", type},
                                                           {"biz_type", bizType},
                                                           {"biz_id", bizId},
                                                           {"is_read", 0},
                                                           {"create_time", QDateTime::currentDateTime()}});
    if (m_notificationManager)
        m_notificationManager->refreshBadge();
}

void MainWindow::setupUI()
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ========== 左侧双栏导航 ==========
    m_navigationRail = new NavigationRail(this);
    auto menus = AuthService::instance().currentUserMenus();
    m_navigationRail->build(menus, buildModulePages());
    mainLayout->addWidget(m_navigationRail);

    connect(m_navigationRail, &NavigationRail::moduleClicked, this,
            [this](const QString &moduleName, const QString &firstPageKey)
            {
                m_navigationRail->setCurrentModule(moduleName);
                switchPage(firstPageKey);
            });
    connect(m_navigationRail, &NavigationRail::pageClicked, this, &MainWindow::switchPage);

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

    // 面包屑
    m_breadcrumbBar = new BreadcrumbBar(m_topBar);
    const auto &moduleFirstPage = moduleFirstPageMap();
    for (auto it = moduleFirstPage.begin(); it != moduleFirstPage.end(); ++it)
        m_breadcrumbBar->registerModuleTarget(it.key(), it.value());
    topLayout->addWidget(m_breadcrumbBar);
    topLayout->addStretch();

    connect(m_breadcrumbBar, &BreadcrumbBar::itemClicked, this, &MainWindow::switchPage);

    // 全局搜索框
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
    auto *searchIconLabel = new QLabel(m_globalSearchEdit);
    searchIconLabel->setPixmap(UiKit::tintSvgIcon(QStringLiteral("ic_search"), QStringLiteral("#9A9A9A"), QSize(14, 14)));
    searchIconLabel->setGeometry(6, 11, 14, 14);
    searchIconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    topLayout->addWidget(m_globalSearchEdit);
    topLayout->addSpacing(16);

    m_searchController = new GlobalSearchController(this);
    m_searchController->attach(m_globalSearchEdit, this);
    connect(m_searchController, &GlobalSearchController::menuSelected, this, &MainWindow::switchPage);
    connect(m_searchController, &GlobalSearchController::workOrderSelected, this,
            [this](qint64 id)
            {
                switchPage(QStringLiteral("301"));
                if (id > 0)
                    WorkOrderDetailDialog::show(this, id);
            });
    connect(m_searchController, &GlobalSearchController::eventSelected, this,
            [this](qint64 id)
            {
                Q_UNUSED(id)
                switchPage(QStringLiteral("401"));
            });

    // 全局刷新按钮
    m_refreshBtn = new QToolButton(m_topBar);
    m_refreshBtn->setFixedSize(32, 32);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setToolTip(QStringLiteral("刷新当前页面"));
    m_refreshIconPix = UiKit::tintSvgIcon(QStringLiteral("ic_refresh"), QStringLiteral("#6B6B6B"), QSize(18, 18));
    if (!m_refreshIconPix.isNull())
    {
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
                if (m_refreshIconPix.isNull())
                    return;
                int angle = value.toInt();
                // 用 QPainter 旋转绘制（比 QPixmap::transformed + SmoothTransformation 高效得多）
                QImage img(18, 18, QImage::Format_ARGB32_Premultiplied);
                img.fill(Qt::transparent);
                QPainter painter(&img);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
                painter.translate(9, 9);
                painter.rotate(angle);
                painter.translate(-9, -9);
                painter.drawPixmap(0, 0, m_refreshIconPix);
                painter.end();
                m_refreshBtn->setIcon(QIcon(QPixmap::fromImage(img)));
            });
    connect(m_refreshAnimation, &QVariantAnimation::finished, this, [this]()
            { m_refreshBtn->setIcon(QIcon(m_refreshIconPix)); });
    connect(m_refreshBtn, &QToolButton::clicked, this, &MainWindow::onRefreshBtnClicked);
    topLayout->addWidget(m_refreshBtn);
    topLayout->addSpacing(8);

    // 通知铃铛
    m_notifyBtn = new QToolButton(m_topBar);
    m_notifyBtn->setFixedSize(36, 36);
    m_notifyBtn->setCursor(Qt::PointingHandCursor);
    m_notifyBtn->setToolTip(QStringLiteral("通知消息"));
    auto *notifyIcon = new QLabel(m_notifyBtn);
    notifyIcon->setPixmap(UiKit::tintSvgIcon(QStringLiteral("ic_bell"), QStringLiteral("#6B6B6B"), QSize(18, 18)));
    notifyIcon->setAlignment(Qt::AlignCenter);
    notifyIcon->setGeometry(9, 9, 18, 18);
    notifyIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    auto *notificationBadge = new QLabel(m_notifyBtn);
    notificationBadge->setAlignment(Qt::AlignCenter);
    notificationBadge->setStyleSheet(QStringLiteral(
        "QLabel { background: #DC2626; color: #FFFFFF; font-size: 11px; font-weight: bold;"
        " border-radius: 9px; min-width: 18px; min-height: 18px; padding: 0 4px; }"));
    notificationBadge->setFixedHeight(18);
    notificationBadge->setVisible(false);
    notificationBadge->setAttribute(Qt::WA_TransparentForMouseEvents);
    notificationBadge->raise();

    topLayout->addWidget(m_notifyBtn);
    topLayout->addSpacing(8);

    m_notificationManager = new NotificationManager(this);
    m_notificationManager->setBadgeLabel(notificationBadge, m_notifyBtn);
    connect(m_notifyBtn, &QToolButton::clicked, this, &MainWindow::onNotifyBtnClicked);

    // 用户下拉菜单
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

    connect(profileAction, &QAction::triggered, this, [this]() { ProfileDialog::show(this); });
    connect(pwdAction, &QAction::triggered, this, [this]() { ChangePasswordDialog::show(this); });
    connect(logoutAction, &QAction::triggered, this, [this]()
            {
                if (QMessageBox::question(this, QStringLiteral("确认"), QStringLiteral("确定要退出登录吗？")) == QMessageBox::Yes)
                    emit logoutRequested();
            });

    topLayout->addWidget(m_userMenuBtn);
    rightLayout->addWidget(m_topBar);

    // 内容堆叠区
    m_contentStack = new QStackedWidget(this);
    m_contentStack->setStyleSheet("QStackedWidget { background: #FAF9F6; }");
    rightLayout->addWidget(m_contentStack);

    mainLayout->addWidget(rightPanel, 1);

    // 页面缓存
    m_pageCache = new PageCache(m_contentStack, this);
    connect(m_pageCache, &PageCache::pageCreated, this, [this](BasePage *page)
            {
                connect(page, &BasePage::sendNotificationRequested, this, &MainWindow::sendNotification);
                connect(page, &BasePage::navigateToRequested, this, &MainWindow::switchPage);
                // 仅在首次创建时淡入，避免反复 setGraphicsEffect 导致内存增长
                UiKit::fadeInWidget(static_cast<QWidget*>(page), 200);
            });

    // 默认显示工作台
    m_navigationRail->setCurrentModule(QStringLiteral("工作台"));
    switchPage(QStringLiteral("1"));
    m_notificationManager->refreshBadge();
}

void MainWindow::switchPage(const QString &key)
{
    m_currentPage = key;

    BasePage *basePage = m_pageCache->getOrCreatePage(key);
    QWidget *page = basePage ? static_cast<QWidget *>(basePage) : m_pageCache->pageWidget(key);
    if (page)
    {
        m_contentStack->setCurrentWidget(page);
        // fadeInWidget 已移至 pageCreated 信号，仅首次创建时触发
    }

    const auto &reg = pageRegistry();
    auto it = reg.constFind(key);
    if (it != reg.constEnd())
    {
        QString module = it->group.isEmpty() ? QStringLiteral("工作台") : it->group;
        if (module != m_navigationRail->currentModule())
            m_navigationRail->setCurrentModule(module);
        m_navigationRail->setActivePage(key);
        m_breadcrumbBar->setPath(it->group, it->title);
    }
    else
    {
        m_breadcrumbBar->setPath(QString(), QStringLiteral("首页"));
    }

    if (m_notificationManager)
        m_notificationManager->refreshBadge();
}

void MainWindow::updateUserInfo()
{
    const auto &user = AuthService::instance().currentUser();
    QString displayName = user.nickname.isEmpty() ? user.username : user.nickname;
    QString roleText = user.roleNames.isEmpty() ? QStringLiteral("未分配角色") : user.roleNames.join(", ");
    if (m_userMenuBtn)
        m_userMenuBtn->setText(QStringLiteral("%1 | %2 ▼").arg(roleText).arg(displayName));
}

void MainWindow::refreshCurrentPage()
{
    if (m_currentPage.isEmpty())
        return;
    m_pageCache->refreshPage(m_currentPage);
    QWidget *page = m_pageCache->pageWidget(m_currentPage);
    if (page)
    {
        m_contentStack->setCurrentWidget(page);
        // fadeInWidget 已由 pageCreated 信号处理
    }
}

void MainWindow::onRefreshBtnClicked()
{
    if (m_refreshAnimation)
    {
        m_refreshAnimation->stop();
        m_refreshAnimation->start();
    }
    refreshCurrentPage();
}

void MainWindow::onNotifyBtnClicked()
{
    switchPage(QStringLiteral("103"));
}
