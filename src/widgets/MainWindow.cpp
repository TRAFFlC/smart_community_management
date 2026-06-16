#include "MainWindow.h"
#include "services/AuthService.h"
#include "utils/Utils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QGroupBox>
#include <QFormLayout>
#include <QTabWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QInputDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QCategoryAxis>
#include <QValueAxis>
#include <QToolTip>
#include <QtCharts>
#include <functional>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

namespace {
const char* TABLE_STYLE = R"(
    QTableWidget {
        background: #ffffff; border: none; border-radius: 8px;
        gridline-color: #f5f5f5; font-size: 13px;
    }
    QTableWidget::item { padding: 8px 12px; border-bottom: 1px solid #f5f5f5; }
    QTableWidget::item:hover { background: #e6f4ff; }
    QHeaderView::section {
        background: #fafafa; color: #8c8c8c; font-weight: 600;
        padding: 10px 12px; border: none; border-bottom: 2px solid #f0f0f0;
        font-size: 12px; text-transform: uppercase;
    }
    QTableWidget::item:selected { background: #e6f4ff; color: #1677ff; }
)";
}


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setMinimumSize(1200, 800);
    setWindowIcon(QIcon(":/app.svg"));
    setWindowTitle(QStringLiteral("智慧社区管理平台"));
    setupUI();
    updateUserInfo();
}

MainWindow::~MainWindow() {}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QString targetPage = watched->property("targetPage").toString();
            if (!targetPage.isEmpty()) {
                switchPage(targetPage);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setupUI() {
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 左侧导航栏
    m_sidebar = new QWidget(this);
    m_sidebar->setFixedWidth(220);
    m_sidebar->setStyleSheet(R"(
        QWidget { background: #001529; }
        QPushButton {
            background: transparent; color: rgba(255,255,255,0.70);
            border: none; border-left: 3px solid transparent;
            padding: 12px 20px; text-align: left;
            font-size: 14px; min-height: 40px; border-radius: 0;
        }
        QPushButton:hover { background: rgba(255,255,255,0.08); color: #ffffff; border-left: 3px solid #1677ff; }
        QPushButton[active="true"] { background: rgba(22,119,255,0.15); color: #ffffff; font-weight: 600; border-left: 3px solid #1677ff; }
        QLabel { background: transparent; }
    )");
    buildSidebar();
    mainLayout->addWidget(m_sidebar);

    // 右侧内容区
    auto* rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // 顶部栏
    m_topBar = new QWidget(this);
    m_topBar->setFixedHeight(50);
    m_topBar->setStyleSheet(R"(
        QWidget { background: #ffffff; border-bottom: 1px solid #f0f0f0; }
        QLabel { color: #595959; font-size: 14px; background: transparent; }
        QPushButton {
            background: transparent; color: #595959; border: 1px solid #d9d9d9;
            padding: 6px 16px; font-size: 13px; border-radius: 6px;
        }
        QPushButton:hover { color: #1677ff; border-color: #1677ff; }
    )");
    auto* topLayout = new QHBoxLayout(m_topBar);
    topLayout->setContentsMargins(16, 0, 16, 0);

    m_breadcrumbLabel = new QLabel(QStringLiteral("首页 / 工作台"), this);
    m_breadcrumbLabel->setStyleSheet("color: #8c8c8c; font-size: 13px;");
    topLayout->addWidget(m_breadcrumbLabel);
    topLayout->addStretch();

    m_userInfoLabel = new QLabel(this);
    m_roleInfoLabel = new QLabel(this);
    m_roleInfoLabel->setStyleSheet("color: #8c8c8c; font-size: 12px; padding: 4px 10px; background: #f5f5f5; border-radius: 4px;");

    auto* logoutBtn = new QPushButton(QStringLiteral("退出登录"), this);
    connect(logoutBtn, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, QStringLiteral("确认"), QStringLiteral("确定要退出登录吗？")) == QMessageBox::Yes) {
            emit logoutRequested();
        }
    });

    topLayout->addWidget(m_roleInfoLabel);
    topLayout->addWidget(m_userInfoLabel);
    topLayout->addWidget(logoutBtn);
    rightLayout->addWidget(m_topBar);

    // 内容区域
    m_contentStack = new QStackedWidget(this);
    m_contentStack->setStyleSheet("QStackedWidget { background: #F0F2F5; }");
    rightLayout->addWidget(m_contentStack);

    mainLayout->addWidget(rightPanel);

    // 默认显示首页（菜单ID=1 对应工作台）
    switchPage("1");
}

void MainWindow::buildSidebar() {
    auto* scrollArea = new QScrollArea(m_sidebar);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { background: rgba(255,255,255,0.15); border-radius: 2px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");

    auto* scrollContent = new QWidget(scrollArea);
    auto* layout = new QVBoxLayout(scrollContent);
    layout->setContentsMargins(0, 4, 0, 0);
    layout->setSpacing(0);

    auto menus = AuthService::instance().currentUserMenus();
    for (const auto& menu : menus) {
        // 工作台(id=1)特殊处理：直接作为可点击项，跳转到首页
        if (menu.id == 1) {
            auto* item = createSidebarItem(menu.icon, menu.menuName, "1", false);
            layout->addWidget(item);
            m_sidebarItems["1"] = item;
            if (m_activeSidebarItem == nullptr) {
                m_activeSidebarItem = item;
                item->setProperty("active", true);
                item->style()->unpolish(item);
                item->style()->polish(item);
            }
        } else if (!menu.children.isEmpty()) {
            // 父菜单 - 可折叠分组
            auto* groupWidget = new QWidget(scrollContent);
            groupWidget->setStyleSheet("background: transparent;");

            auto* groupLayout = new QVBoxLayout(groupWidget);
            groupLayout->setContentsMargins(0, 0, 0, 0);
            groupLayout->setSpacing(0);

            // 分组标题按钮（可点击展开/折叠）
            auto* groupHeader = new QPushButton(menu.menuName, groupWidget);
            groupHeader->setCheckable(true);
            groupHeader->setObjectName("groupHeader");
            groupHeader->setStyleSheet(
                "QPushButton#groupHeader {"
                "   background: transparent; color: rgba(255,255,255,0.65); border: none;"
                "   padding: 10px 16px 6px 16px; text-align: left; font-size: 12px; font-weight: bold;"
                "   min-height: 28px;"
                "}"
                "QPushButton#groupHeader:hover { color: rgba(255,255,255,0.85); }"
                "QPushButton#groupHeader::indicator { width: 0; height: 0; }");

            // 子菜单容器
            auto* childContainer = new QWidget(groupWidget);
            childContainer->setObjectName("childContainer");
            auto* childLayout = new QVBoxLayout(childContainer);
            childLayout->setContentsMargins(0, 0, 0, 2);
            childLayout->setSpacing(0);

            for (const auto& child : menu.children) {
                auto* item = createSidebarItem(child.icon, child.menuName, QString::number(child.id), false);
                childLayout->addWidget(item);
                m_sidebarItems[QString::number(child.id)] = item;
                if (m_activeSidebarItem == nullptr) {
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
            if (!isFirstGroup) childContainer->setVisible(false);

            // 连接折叠/展开信号 - 手风琴模式
            connect(groupHeader, &QPushButton::toggled, this, [this, groupHeader, childContainer](bool checked) {
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
                }
            });

            groupLayout->addWidget(groupHeader);
            groupLayout->addWidget(childContainer);
            layout->addWidget(groupWidget);
        }
    }

    layout->addStretch();
    scrollArea->setWidget(scrollContent);

    auto* sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    // Logo
    auto* logoLabel = new QLabel(QStringLiteral("  智慧社区"), m_sidebar);
    logoLabel->setFixedHeight(50);
    logoLabel->setStyleSheet("color: #ffffff; font-size: 17px; font-weight: bold; background: #002140; padding-left: 8px;");
    sidebarLayout->addWidget(logoLabel);
    sidebarLayout->addWidget(scrollArea);
}

QWidget* MainWindow::createSidebarItem(const QString& icon, const QString& text, const QString& key, bool isHeader) {
    auto* btn = new QPushButton(text, m_sidebar);
    btn->setProperty("key", key);
    if (isHeader) {
        btn->setEnabled(false);
    } else {
        connect(btn, &QPushButton::clicked, this, [this, key, btn]() {
            if (m_activeSidebarItem && m_activeSidebarItem != btn) {
                m_activeSidebarItem->setProperty("active", false);
                m_activeSidebarItem->style()->unpolish(m_activeSidebarItem);
                m_activeSidebarItem->style()->polish(m_activeSidebarItem);
            }
            btn->setProperty("active", true);
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
            m_activeSidebarItem = btn;
            switchPage(key);
        });
    }
    return btn;
}

void MainWindow::switchPage(const QString& key) {
    m_currentPage = key;
    QWidget* page = getOrCreatePage(key);
    if (page) {
        int idx = m_contentStack->indexOf(page);
        if (idx < 0) {
            m_contentStack->addWidget(page);
            idx = m_contentStack->count() - 1;
        }
        m_contentStack->setCurrentIndex(idx);
    }

    // Update breadcrumb
    if (m_breadcrumbLabel) {
        static const QMap<QString, QString> breadcrumbMap = {
            {"1", "首页 / 工作台"},
            {"201", "首页 / 基础档案 / 组织管理"}, {"202", "首页 / 基础档案 / 小区管理"}, {"203", "首页 / 基础档案 / 房屋管理"},
            {"204", "首页 / 基础档案 / 居民管理"}, {"205", "首页 / 基础档案 / 车辆管理"}, {"206", "首页 / 基础档案 / 设施管理"},
            {"207", "首页 / 基础档案 / 网格管理"}, {"208", "首页 / 基础档案 / 特殊群体"},
            {"301", "首页 / 小区管理 / 报事报修"}, {"302", "首页 / 小区管理 / 投诉建议"}, {"303", "首页 / 小区管理 / 物业巡检"},
            {"304", "首页 / 小区管理 / 公告通知"}, {"305", "首页 / 小区管理 / 访客管理"}, {"306", "首页 / 小区管理 / 业委会议题"},
            {"307", "首页 / 小区管理 / 停车管理"}, {"308", "首页 / 小区管理 / 物业缴费"}, {"309", "首页 / 小区管理 / 公共收益"},
            {"401", "首页 / 社区治理 / 网格事件"}, {"402", "首页 / 社区治理 / 社区巡查"}, {"403", "首页 / 社区治理 / 重点人群关怀"},
            {"404", "首页 / 社区治理 / 督办管理"}, {"405", "首页 / 社区治理 / 民意收集"}, {"406", "首页 / 社区治理 / 考核管理"},
            {"501", "首页 / 社区服务 / 志愿服务"}, {"502", "首页 / 社区服务 / 便民服务"}, {"503", "首页 / 社区服务 / 就业服务"},
            {"601", "首页 / 统计分析 / 工单统计"}, {"602", "首页 / 统计分析 / 事件统计"}, {"603", "首页 / 统计分析 / 服务统计"},
            {"604", "首页 / 统计分析 / 综合看板"},
            {"701", "首页 / 系统管理 / 用户管理"}, {"702", "首页 / 系统管理 / 角色管理"}, {"703", "首页 / 系统管理 / 菜单管理"},
            {"704", "首页 / 系统管理 / 字典管理"}, {"705", "首页 / 系统管理 / 操作日志"}, {"706", "首页 / 系统管理 / 智能问答"}
        };
        m_breadcrumbLabel->setText(breadcrumbMap.value(key, QStringLiteral("首页")));
    }
}

void MainWindow::updateUserInfo() {
    const auto& user = AuthService::instance().currentUser();
    QString displayName = user.nickname.isEmpty() ? user.username : user.nickname;
    m_userInfoLabel->setText(displayName);
    m_roleInfoLabel->setText(user.roleNames.join(", "));
}

QWidget* MainWindow::getOrCreatePage(const QString& key) {
    if (m_pageCache.contains(key)) return m_pageCache[key];

    QWidget* page = nullptr;

    // Dashboard
    if (key == "1") {
        page = createDashboardPage();
    }
    // Archive pages
    else if (key == "201") page = createArchivePage("org");
    else if (key == "202") page = createArchivePage("estate");
    else if (key == "203") page = createArchivePage("house");
    else if (key == "204") page = createArchivePage("resident");
    else if (key == "205") page = createArchivePage("vehicle");
    else if (key == "206") page = createArchivePage("facility");
    else if (key == "207") page = createArchivePage("grid");
    else if (key == "208") page = createArchivePage("special");
    // Property pages
    else if (key == "301") page = createPropertyPage("workorder");
    else if (key == "302") page = createPropertyPage("complaint");
    else if (key == "303") page = createPropertyPage("inspection");
    else if (key == "304") page = createPropertyPage("announcement");
    else if (key == "305") page = createPropertyPage("visitor");
    else if (key == "306") page = createPropertyPage("topic");
    else if (key == "307") page = createPropertyPage("parking");
    else if (key == "308") page = createPropertyPage("billing");
    else if (key == "309") page = createPropertyPage("income");
    // Governance pages
    else if (key == "401") page = createGovernancePage("event");
    else if (key == "402") page = createGovernancePage("inspection");
    else if (key == "403") page = createGovernancePage("care");
    else if (key == "404") page = createGovernancePage("supervision");
    else if (key == "405") page = createGovernancePage("opinion");
    else if (key == "406") page = createGovernancePage("assessment");
    // Service pages
    else if (key == "501") page = createServicePage("volunteer");
    else if (key == "502") page = createServicePage("convenience");
    else if (key == "503") page = createServicePage("job");
    // Report pages
    else if (key == "601") page = createReportPage("workorder");
    else if (key == "602") page = createReportPage("event");
    else if (key == "603") page = createReportPage("service");
    else if (key == "604") page = createReportPage("dashboard");
    // System pages
    else if (key == "701") page = createSystemPage("user");
    else if (key == "702") page = createSystemPage("role");
    else if (key == "703") page = createSystemPage("menu");
    else if (key == "704") page = createSystemPage("dict");
    else if (key == "705") page = createSystemPage("log");
    else if (key == "706") page = createSystemPage("ai");

    // Fallback: 功能开发中占位页面
    if (!page) {
        page = new QWidget();
        auto* placeholderLayout = new QVBoxLayout(page);
        placeholderLayout->setContentsMargins(20, 20, 20, 20);
        placeholderLayout->addStretch();
        auto* placeholderLabel = new QLabel(QStringLiteral("功能开发中，敬请期待..."), page);
        placeholderLabel->setStyleSheet("font-size: 18px; color: #8c8c8c; background: transparent;");
        placeholderLabel->setAlignment(Qt::AlignCenter);
        placeholderLayout->addWidget(placeholderLabel);
        placeholderLayout->addStretch();
    }

    if (page) m_pageCache[key] = page;
    return page;
}

// ========== Dashboard Page ==========
QWidget* MainWindow::createDashboardPage() {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    // Welcome banner
    const auto& user = AuthService::instance().currentUser();
    QString greeting = user.nickname.isEmpty() ? user.username : user.nickname;
    auto* welcomeLabel = new QLabel(QStringLiteral("欢迎回来，%1！").arg(greeting), page);
    welcomeLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #1f1f1f; margin-bottom: 8px; background: transparent;");
    layout->addWidget(welcomeLabel);

    // Stats cards
    auto* statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(16);

    auto createCard = [](const QString& title, const QString& value, const QString& color, QWidget* parent) -> QWidget* {
        auto* card = new QFrame(parent);
        card->setFixedHeight(120);
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(QString(R"(
            QFrame {
                background: #ffffff; border-radius: 10px;
                border: 1px solid #f0f0f0; padding: 0;
            }
            QFrame:hover { border-color: %1; background: #fafcff; }
            QLabel { background: transparent; border: none; }
        )").arg(color));

        auto* cl = new QVBoxLayout(card);
        cl->setContentsMargins(20, 16, 20, 16);
        cl->setSpacing(8);

        auto* tl = new QLabel(title);
        tl->setStyleSheet("color: #8c8c8c; font-size: 13px;");
        auto* vl = new QLabel(value);
        vl->setStyleSheet(QString("color: %1; font-size: 32px; font-weight: bold;").arg(color));

        // Color indicator bar at top
        auto* indicator = new QFrame(card);
        indicator->setFixedHeight(3);
        indicator->setStyleSheet(QString("background: %1; border-radius: 2px;").arg(color));

        cl->addWidget(indicator);
        cl->addWidget(tl);
        cl->addWidget(vl);
        cl->addStretch();
        return card;
    };

    auto& db = DatabaseManager::instance();

    // Count work orders
    QSqlQuery woQ("SELECT COUNT(*) FROM wo_work_order WHERE del_flag = 0");
    int woCount = 0;
    if (woQ.next()) woCount = woQ.value(0).toInt();

    // Count events
    QSqlQuery evQ("SELECT COUNT(*) FROM ge_event WHERE del_flag = 0");
    int evCount = 0;
    if (evQ.next()) evCount = evQ.value(0).toInt();

    // Count residents
    QSqlQuery resQ("SELECT COUNT(*) FROM cm_resident WHERE del_flag = 0");
    int resCount = 0;
    if (resQ.next()) resCount = resQ.value(0).toInt();

    // Count estates
    QSqlQuery estQ("SELECT COUNT(*) FROM cm_estate WHERE del_flag = 0");
    int estCount = 0;
    if (estQ.next()) estCount = estQ.value(0).toInt();

    statsLayout->addWidget(createCard(QStringLiteral("工单总数"), QString::number(woCount), "#1677ff", page));
    statsLayout->addWidget(createCard(QStringLiteral("治理事件"), QString::number(evCount), "#52c41a", page));
    statsLayout->addWidget(createCard(QStringLiteral("居民档案"), QString::number(resCount), "#fa8c16", page));
    statsLayout->addWidget(createCard(QStringLiteral("小区数量"), QString::number(estCount), "#ff4d4f", page));

    layout->addLayout(statsLayout);
    layout->addSpacing(20);

    // Two-column layout: left (60%) + right (40%)
    auto* twoColLayout = new QHBoxLayout();
    twoColLayout->setSpacing(20);

    // === Left column: Quick actions + Todo ===
    auto* leftCol = new QVBoxLayout();
    leftCol->setSpacing(16);

    // Quick actions - card grid
    auto* quickFrame = new QFrame(page);
    quickFrame->setStyleSheet("QFrame { background: #fff; border: 1px solid #f0f0f0; border-radius: 8px; } QLabel { background: transparent; border: none; }");
    auto* quickOuterLayout = new QVBoxLayout(quickFrame);
    quickOuterLayout->setContentsMargins(20, 16, 20, 16);
    auto* quickTitle = new QLabel(QStringLiteral("快捷操作"));
    quickTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #262626;");
    quickOuterLayout->addWidget(quickTitle);

    auto* quickGrid = new QGridLayout();
    quickGrid->setSpacing(16);
    struct QuickAction { QString label; QString color; QString page; };
    QuickAction actions[] = {
        {QStringLiteral("报事报修"), "#1677ff", "301"},
        {QStringLiteral("投诉建议"), "#fa8c16", "302"},
        {QStringLiteral("公告通知"), "#52c41a", "304"},
        {QStringLiteral("志愿服务"), "#722ed1", "501"},
        {QStringLiteral("网格事件"), "#eb2f96", "401"},
        {QStringLiteral("物业缴费"), "#13c2c2", "308"},
    };
    for (int i = 0; i < 6; ++i) {
        auto* card = new QFrame(quickFrame);
        card->setCursor(Qt::PointingHandCursor);
        card->setFixedHeight(80);
        card->setStyleSheet(QString(
            "QFrame { background: #fff; border: 1px solid #f0f0f0; border-radius: 8px; border-left: 3px solid %1; } "
            "QFrame:hover { border-color: %1; } "
            "QLabel { background: transparent; border: none; }"
        ).arg(actions[i].color));
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(16, 12, 16, 12);
        auto* lbl = new QLabel(actions[i].label);
        lbl->setStyleSheet("font-size: 15px; font-weight: bold; color: #262626;");
        cardLayout->addWidget(lbl);
        auto* desc = new QLabel(QStringLiteral("点击进入"));
        desc->setStyleSheet("font-size: 12px; color: #8c8c8c;");
        cardLayout->addWidget(desc);
        quickGrid->addWidget(card, i / 3, i % 3);
        card->installEventFilter(this);
        card->setProperty("targetPage", actions[i].page);
    }
    quickOuterLayout->addLayout(quickGrid);
    leftCol->addWidget(quickFrame);

    // Todo card
    auto* todoCard = new QFrame(page);
    todoCard->setStyleSheet("QFrame { background: #fff; border: 1px solid #f0f0f0; border-radius: 8px; } QLabel { background: transparent; border: none; }");
    auto* todoLayout = new QVBoxLayout(todoCard);
    todoLayout->setContentsMargins(20, 16, 20, 16);
    auto* todoTitle = new QLabel(QStringLiteral("待办事项"));
    todoTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #262626;");
    todoLayout->addWidget(todoTitle);

    // 查询待处理工单数
    QSqlQuery woPendQ("SELECT COUNT(*) FROM wo_work_order WHERE status IN (0,1,2,3) AND del_flag = 0");
    int woPendCount = woPendQ.next() ? woPendQ.value(0).toInt() : 0;
    auto* woItem = new QLabel(QString("🔧 待处理工单：%1 件").arg(woPendCount));
    woItem->setStyleSheet("font-size: 14px; color: #595959; padding: 4px 0;");
    todoLayout->addWidget(woItem);

    // 查询待审核事件数
    QSqlQuery evPendQ("SELECT COUNT(*) FROM ge_event WHERE status = 0 AND del_flag = 0");
    int evPendCount = evPendQ.next() ? evPendQ.value(0).toInt() : 0;
    auto* evItem = new QLabel(QString("📋 待审核事件：%1 件").arg(evPendCount));
    evItem->setStyleSheet("font-size: 14px; color: #595959; padding: 4px 0;");
    todoLayout->addWidget(evItem);

    // 查询未读公告数
    QSqlQuery annQ("SELECT COUNT(*) FROM nt_announcement WHERE del_flag = 0 AND date(publish_time) >= date('now', '-7 days')");
    int annCount = annQ.next() ? annQ.value(0).toInt() : 0;
    auto* annItem = new QLabel(QString("📢 近期公告：%1 条").arg(annCount));
    annItem->setStyleSheet("font-size: 14px; color: #595959; padding: 4px 0;");
    todoLayout->addWidget(annItem);
    todoLayout->addStretch();
    leftCol->addWidget(todoCard);

    // === Right column: Community dynamics ===
    auto* rightCol = new QVBoxLayout();
    rightCol->setSpacing(16);

    auto* dynamicCard = new QFrame(page);
    dynamicCard->setStyleSheet("QFrame { background: #fff; border: 1px solid #f0f0f0; border-radius: 8px; } QLabel { background: transparent; border: none; }");
    auto* dynLayout = new QVBoxLayout(dynamicCard);
    dynLayout->setContentsMargins(20, 16, 20, 16);
    auto* dynTitle = new QLabel(QStringLiteral("社区动态"));
    dynTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #262626;");
    dynLayout->addWidget(dynTitle);

    QSqlQuery dynQ("SELECT '工单' as type, title, create_time FROM wo_work_order WHERE del_flag = 0 UNION ALL SELECT '事件' as type, title, create_time FROM ge_event WHERE del_flag = 0 ORDER BY create_time DESC LIMIT 5");
    while (dynQ.next()) {
        QString type = dynQ.value(0).toString();
        QString title = dynQ.value(1).toString();
        QString time = dynQ.value(2).toDateTime().toString("MM-dd HH:mm");
        QString color = type == QStringLiteral("工单") ? "#1677ff" : "#fa8c16";
        auto* item = new QLabel(QString("<span style='color:%1;font-weight:bold;'>[%2]</span> %3 <span style='color:#8c8c8c;font-size:12px;'>%4</span>").arg(color, type, title, time));
        item->setStyleSheet("font-size: 13px; padding: 3px 0;");
        dynLayout->addWidget(item);
    }
    dynLayout->addStretch();
    rightCol->addWidget(dynamicCard);

    // 数据概览卡片 - 饼图
    auto* chartCard = new QFrame(page);
    chartCard->setStyleSheet("QFrame { background: #fff; border: 1px solid #f0f0f0; border-radius: 8px; } QLabel { background: transparent; border: none; }");
    auto* chartLayout = new QVBoxLayout(chartCard);
    chartLayout->setContentsMargins(20, 16, 20, 16);
    auto* chartTitle = new QLabel(QStringLiteral("工单状态分布"));
    chartTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #262626;");
    chartLayout->addWidget(chartTitle);

    auto* pieSeries = new QPieSeries();
    QSqlQuery woStsQ("SELECT status, COUNT(*) FROM wo_work_order WHERE del_flag = 0 GROUP BY status");
    QMap<int, QString> statusNames = {{0, QStringLiteral("待处理")}, {1, QStringLiteral("已受理")}, {2, QStringLiteral("处理中")}, {3, QStringLiteral("待评价")}, {4, QStringLiteral("已完成")}, {5, QStringLiteral("已关闭")}};
    QMap<int, QColor> statusColors = {{0, QColor("#faad14")}, {1, QColor("#1677ff")}, {2, QColor("#13c2c2")}, {3, QColor("#722ed1")}, {4, QColor("#52c41a")}, {5, QColor("#8c8c8c")}};
    while (woStsQ.next()) {
        int sts = woStsQ.value(0).toInt();
        int cnt = woStsQ.value(1).toInt();
        auto* slice = pieSeries->append(statusNames.value(sts, QStringLiteral("未知")), cnt);
        slice->setColor(statusColors.value(sts, QColor("#d9d9d9")));
        slice->setLabelVisible(true);
        slice->setLabelColor(QColor("#595959"));
    }
    if (pieSeries->count() == 0) {
        pieSeries->append(QStringLiteral("暂无数据"), 1)->setColor(QColor("#f0f0f0"));
    }

    auto* pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setAnimationOptions(QChart::SeriesAnimations);
    pieChart->legend()->setAlignment(Qt::AlignBottom);
    pieChart->legend()->setLabelColor(QColor("#8c8c8c"));
    pieChart->setMargins(QMargins(0, 0, 0, 0));
    pieChart->setBackgroundVisible(false);

    auto* chartView = new QChartView(pieChart, chartCard);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setFixedHeight(220);
    chartLayout->addWidget(chartView);
    rightCol->addWidget(chartCard);

    // Assemble two-column layout with stretch factors
    auto* leftWidget = new QWidget(page);
    leftWidget->setLayout(leftCol);
    auto* rightWidget = new QWidget(page);
    rightWidget->setLayout(rightCol);
    twoColLayout->addWidget(leftWidget, 3);  // 60%
    twoColLayout->addWidget(rightWidget, 2); // 40%

    layout->addLayout(twoColLayout, 1);
    return page;
}

// ========== Archive Pages ==========
QWidget* MainWindow::createArchivePage(const QString& sub) {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    auto& db = DatabaseManager::instance();

    if (sub == "org") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索组织名称..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({QStringLiteral("组织名称"), QStringLiteral("类型"), QStringLiteral("负责人"), QStringLiteral("电话")});
        std::function<void()> loadOrgs = [table, searchEdit]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT org_name, org_type, leader, phone FROM sys_org WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND org_name LIKE '%" + searchText + "%'";
            sql += " ORDER BY sort_order";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(OrgType::label(q.value(1).toInt())));
                table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
                row++;
            }
        };
        loadOrgs();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadOrgs(); });
    } else if (sub == "estate") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索小区名称/编码..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({QStringLiteral("小区名称"), QStringLiteral("编码"), QStringLiteral("地址"), QStringLiteral("楼栋数"), QStringLiteral("总户数")});
        std::function<void()> loadEstates = [table, searchEdit]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT estate_name, estate_code, address, total_buildings, total_houses FROM cm_estate WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND (estate_name LIKE '%" + searchText + "%' OR estate_code LIKE '%" + searchText + "%')";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
                table->setItem(row, 4, new QTableWidgetItem(q.value(4).toString()));
                row++;
            }
        };
        loadEstates();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadEstates(); });
    } else if (sub == "house") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索房屋编号/房号..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        auto* statusCombo = new QComboBox(toolbar);
        statusCombo->addItem(QStringLiteral("全部状态"), -1);
        statusCombo->addItem(QStringLiteral("空置"), 0);
        statusCombo->addItem(QStringLiteral("自住"), 1);
        statusCombo->addItem(QStringLiteral("出租"), 2);
        statusCombo->addItem(QStringLiteral("出售"), 3);
        statusCombo->setMinimumWidth(120);
        tbLayout->addWidget(statusCombo);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({QStringLiteral("房屋编号"), QStringLiteral("楼层"), QStringLiteral("房号"), QStringLiteral("面积"), QStringLiteral("户型"), QStringLiteral("状态")});
        std::function<void()> loadHouses = [table, searchEdit, statusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT house_code, floor, room_number, area, house_type, house_status FROM cm_house WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int statusFilter = statusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND (house_code LIKE '%" + searchText + "%' OR room_number LIKE '%" + searchText + "%')";
            if (statusFilter >= 0) sql += " AND house_status = " + QString::number(statusFilter);
            sql += " ORDER BY house_code";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
                table->setItem(row, 4, new QTableWidgetItem(q.value(4).toString()));
                int houseSts = q.value(5).toInt();
                auto* houseStsItem = new QTableWidgetItem(HouseStatus::label(houseSts));
                houseStsItem->setTextAlignment(Qt::AlignCenter);
                if (houseSts == 0) {
                    houseStsItem->setBackground(QColor(245, 245, 245));
                    houseStsItem->setForeground(QColor(140, 140, 140));
                } else if (houseSts == 1) {
                    houseStsItem->setBackground(QColor(230, 244, 255));
                    houseStsItem->setForeground(QColor(22, 119, 255));
                } else if (houseSts == 2) {
                    houseStsItem->setBackground(QColor(246, 255, 237));
                    houseStsItem->setForeground(QColor(82, 196, 26));
                } else {
                    houseStsItem->setBackground(QColor(255, 247, 230));
                    houseStsItem->setForeground(QColor(250, 140, 22));
                }
                table->setItem(row, 5, houseStsItem);
                row++;
            }
        };
        loadHouses();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadHouses(); });
        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadHouses(); });
    } else if (sub == "resident") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索姓名/电话..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({QStringLiteral("姓名"), QStringLiteral("性别"), QStringLiteral("手机号"), QStringLiteral("民族"), QStringLiteral("职业")});
        std::function<void()> loadResidents = [table, searchEdit]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT name, gender, phone_display, nationality, occupation FROM cm_resident WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND (name LIKE '%" + searchText + "%' OR phone_display LIKE '%" + searchText + "%')";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(1).toInt() == 1 ? QStringLiteral("男") : QStringLiteral("女")));
                table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
                table->setItem(row, 4, new QTableWidgetItem(q.value(4).toString()));
                row++;
            }
        };
        loadResidents();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadResidents(); });
    } else if (sub == "vehicle") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索车牌号..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        auto* typeCombo = new QComboBox(toolbar);
        typeCombo->addItem(QStringLiteral("全部类型"), -1);
        typeCombo->addItem(QStringLiteral("轿车"), 1);
        typeCombo->addItem(QStringLiteral("SUV"), 2);
        typeCombo->addItem(QStringLiteral("电动车"), 3);
        typeCombo->addItem(QStringLiteral("其他"), 4);
        typeCombo->setMinimumWidth(120);
        tbLayout->addWidget(typeCombo);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({QStringLiteral("车牌号"), QStringLiteral("品牌"), QStringLiteral("颜色"), QStringLiteral("类型")});
        std::function<void()> loadVehicles = [table, searchEdit, typeCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT plate_number, vehicle_brand, vehicle_color, vehicle_type FROM cm_vehicle WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int typeFilter = typeCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND plate_number LIKE '%" + searchText + "%'";
            if (typeFilter >= 0) sql += " AND vehicle_type = " + QString::number(typeFilter);
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 3, new QTableWidgetItem(VehicleType::label(q.value(3).toInt())));
                row++;
            }
        };
        loadVehicles();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadVehicles(); });
        connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadVehicles(); });
    } else if (sub == "facility") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索设施名称/编号..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({QStringLiteral("设施名称"), QStringLiteral("类型"), QStringLiteral("编号"), QStringLiteral("位置"), QStringLiteral("状态")});
        std::function<void()> loadFacilities = [table, searchEdit]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT facility_name, facility_type, facility_code, location, status FROM cm_facility WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND (facility_name LIKE '%" + searchText + "%' OR facility_code LIKE '%" + searchText + "%')";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(FacilityType::label(q.value(1).toInt())));
                table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
                int facSts = q.value(4).toInt();
                auto* facStsItem = new QTableWidgetItem(facSts == 0 ? QStringLiteral("正常") : QStringLiteral("故障"));
                facStsItem->setTextAlignment(Qt::AlignCenter);
                if (facSts == 0) {
                    facStsItem->setBackground(QColor(246, 255, 237));
                    facStsItem->setForeground(QColor(82, 196, 26));
                } else {
                    facStsItem->setBackground(QColor(255, 242, 240));
                    facStsItem->setForeground(QColor(255, 77, 79));
                }
                table->setItem(row, 4, facStsItem);
                row++;
            }
        };
        loadFacilities();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadFacilities(); });
    } else if (sub == "grid") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索网格名称/编码..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({QStringLiteral("网格名称"), QStringLiteral("编码"), QStringLiteral("描述"), QStringLiteral("网格员")});
        std::function<void()> loadGrids = [table, searchEdit]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT g.grid_name, g.grid_code, g.description, u.real_name FROM cm_grid g LEFT JOIN sys_user u ON g.grid_worker_id = u.id WHERE g.del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND (g.grid_name LIKE '%" + searchText + "%' OR g.grid_code LIKE '%" + searchText + "%')";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
                row++;
            }
        };
        loadGrids();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadGrids(); });
    } else if (sub == "special") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索居民姓名..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({QStringLiteral("居民"), QStringLiteral("类型"), QStringLiteral("关怀等级"), QStringLiteral("走访频率")});
        std::function<void()> loadSpecials = [table, searchEdit]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT r.name, sg.group_type, sg.care_level, sg.care_frequency FROM cm_special_group sg JOIN cm_resident r ON sg.resident_id = r.id WHERE sg.del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND r.name LIKE '%" + searchText + "%'";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(SpecialGroupType::label(q.value(1).toInt())));
                table->setItem(row, 2, new QTableWidgetItem(q.value(2).toInt() == 1 ? QStringLiteral("一般") : q.value(2).toInt() == 2 ? QStringLiteral("重点") : QStringLiteral("特殊")));
                table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
                row++;
            }
        };
        loadSpecials();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadSpecials(); });
    }

    layout->addWidget(table);
    return page;
}

// ========== Property Pages ==========
QWidget* MainWindow::createPropertyPage(const QString& sub) {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    auto& db = DatabaseManager::instance();

    if (sub == "workorder") {
        // Page header
        auto* header = new QLabel(QStringLiteral("报事报修管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("管理居民报修工单，支持受理、派单、处理、评价全流程"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setStyleSheet("background: transparent;");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 12);

        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索工单号/标题..."));
        searchEdit->setFixedWidth(220);
        tbLayout->addWidget(searchEdit);

        auto* filterCombo = new QComboBox(toolbar);
        filterCombo->addItems({QStringLiteral("全部状态"), QStringLiteral("待受理"), QStringLiteral("已受理"), QStringLiteral("处理中"), QStringLiteral("已完成"), QStringLiteral("已关闭")});
        filterCombo->setFixedWidth(130);
        tbLayout->addWidget(filterCombo);
        tbLayout->addStretch();

        auto* newBtn = new QPushButton(QStringLiteral("+ 新建工单"), toolbar);
        newBtn->setProperty("cssClass", "primary");
        newBtn->setCursor(Qt::PointingHandCursor);
        tbLayout->addWidget(newBtn);
        layout->addWidget(toolbar);

        // Table
        table->setColumnCount(8);
        table->setHorizontalHeaderLabels({QStringLiteral("工单号"), QStringLiteral("标题"), QStringLiteral("类型"), QStringLiteral("优先级"), QStringLiteral("状态"), QStringLiteral("报修人"), QStringLiteral("创建时间"), QStringLiteral("操作")});
        table->setColumnWidth(1, 180);
        table->setColumnWidth(7, 100);
        table->horizontalHeader()->setStretchLastSection(false);

        std::function<void()> loadWorkOrders = [table, searchEdit, filterCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT id, order_no, title, order_type, priority, status, reporter_name, create_time FROM wo_work_order WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int filterIdx = filterCombo->currentIndex();
            if (!searchText.isEmpty()) {
                sql += " AND (order_no LIKE :search OR title LIKE :search)";
            }
            if (filterIdx > 0) {
                sql += " AND status = :status";
            }
            sql += " ORDER BY create_time DESC";
            QSqlQuery q(DatabaseManager::instance().database());
            q.prepare(sql);
            if (!searchText.isEmpty()) {
                q.bindValue(":search", "%" + searchText + "%");
            }
            if (filterIdx > 0) {
                int statusMap[] = {0, 0, 1, 3, 4, 5};
                q.bindValue(":status", statusMap[filterIdx]);
            }
            q.exec();
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 2, new QTableWidgetItem(WorkOrderType::label(q.value(3).toInt())));
                table->setItem(row, 3, new QTableWidgetItem(WorkOrderPriority::label(q.value(4).toInt())));
                auto* statusItem = new QTableWidgetItem(WorkOrderStatus::label(q.value(5).toInt()));
                statusItem->setTextAlignment(Qt::AlignCenter);
                QColor tagBg(WorkOrderStatus::color(q.value(5).toInt()));
                tagBg.setAlpha(30);
                statusItem->setBackground(tagBg);
                statusItem->setForeground(QColor(WorkOrderStatus::color(q.value(5).toInt())));
                table->setItem(row, 4, statusItem);
                table->setItem(row, 5, new QTableWidgetItem(q.value(6).toString()));
                table->setItem(row, 6, new QTableWidgetItem(q.value(7).toDateTime().toString("yyyy-MM-dd hh:mm")));
                // Action text item
                int sts = q.value(5).toInt();
                qint64 woId = q.value(0).toLongLong();
                QString actionText;
                QString actionColor;
                if (sts == 0) {
                    actionText = QStringLiteral("受理");
                    actionColor = "#1677ff";
                } else if (sts == 1 || sts == 2 || sts == 3) {
                    actionText = QStringLiteral("完成");
                    actionColor = "#52c41a";
                } else {
                    actionText = QStringLiteral("--");
                    actionColor = "#8c8c8c";
                }
                auto* actionItem = new QTableWidgetItem(actionText);
                actionItem->setTextAlignment(Qt::AlignCenter);
                actionItem->setForeground(QColor(actionColor));
                actionItem->setData(Qt::UserRole, woId);
                actionItem->setData(Qt::UserRole + 1, sts);
                actionItem->setFont(QFont("Microsoft YaHei", 12));
                table->setItem(row, 7, actionItem);
                row++;
            }
        };
        loadWorkOrders();
        connect(table, &QTableWidget::cellClicked, this, [=](int r, int c) {
            if (c != 7) return;
            auto* item = table->item(r, c);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 1).toInt();
            if (sts == 0) {
                DatabaseManager::instance().update("wo_work_order", id, {{"status", 1}, {"accept_time", QDateTime::currentDateTime()}});
                loadWorkOrders();
            } else if (sts == 1 || sts == 2 || sts == 3) {
                DatabaseManager::instance().update("wo_work_order", id, {{"status", 4}, {"finish_time", QDateTime::currentDateTime()}});
                loadWorkOrders();
            }
        });
        connect(table, &QTableWidget::cellEntered, this, [=](int r, int c) {
            table->viewport()->setCursor(c == 7 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        });
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadWorkOrders(); });
        connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadWorkOrders(); });

        // New work order dialog
        connect(newBtn, &QPushButton::clicked, this, [=]() {
            QDialog dlg(this);
            dlg.setWindowTitle(QStringLiteral("新建报修工单"));
            dlg.setMinimumWidth(500);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* formTitle = new QLabel(QStringLiteral("填写报修信息"), &dlg);
            formTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1f1f1f;");
            dlgLayout->addWidget(formTitle);
            dlgLayout->addSpacing(8);

            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);

            auto* titleEdit = new QLineEdit(&dlg);
            titleEdit->setPlaceholderText(QStringLiteral("请简要描述问题"));
            form->addRow(QStringLiteral("标题:"), titleEdit);

            auto* typeCombo = new QComboBox(&dlg);
            typeCombo->addItems({QStringLiteral("水电维修"), QStringLiteral("公共设施"), QStringLiteral("环境卫生"), QStringLiteral("安全秩序"), QStringLiteral("其他")});
            form->addRow(QStringLiteral("类型:"), typeCombo);

            auto* prioCombo = new QComboBox(&dlg);
            prioCombo->addItems({QStringLiteral("普通"), QStringLiteral("紧急"), QStringLiteral("特急")});
            form->addRow(QStringLiteral("优先级:"), prioCombo);

            auto* descEdit = new QTextEdit(&dlg);
            descEdit->setPlaceholderText(QStringLiteral("详细描述报修问题..."));
            descEdit->setFixedHeight(80);
            form->addRow(QStringLiteral("描述:"), descEdit);

            auto* locEdit = new QLineEdit(&dlg);
            locEdit->setPlaceholderText(QStringLiteral("如: 3号楼2单元501"));
            form->addRow(QStringLiteral("位置:"), locEdit);

            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);

            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (titleEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题"));
                    return;
                }
                auto& d = DatabaseManager::instance();
                QString orderNo = Utils::generateOrderNo();
                const auto& user = AuthService::instance().currentUser();
                d.insert("wo_work_order", {
                    {"order_no", orderNo}, {"title", titleEdit->text().trimmed()},
                    {"order_type", typeCombo->currentIndex() + 1},
                    {"priority", prioCombo->currentIndex() + 1},
                    {"description", descEdit->toPlainText().trimmed()},
                    {"location_desc", locEdit->text().trimmed()},
                    {"reporter_id", user.id}, {"reporter_name", user.nickname.isEmpty() ? user.username : user.nickname},
                    {"reporter_phone", user.phone},
                    {"status", 0}, {"source", 0},
                    {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                });
                dlg.accept();
                loadWorkOrders();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec();
        });
    } else if (sub == "complaint") {
        // Page header
        auto* header = new QLabel(QStringLiteral("投诉建议管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("管理居民投诉和建议，跟踪处理进度和满意度评价"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setStyleSheet("background: transparent;");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 12);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索标题/编号..."));
        searchEdit->setMaximumWidth(220);

        tbLayout->addWidget(searchEdit);
        auto* typeCombo = new QComboBox(toolbar);
        typeCombo->addItem(QStringLiteral("全部类型"), -1);
        typeCombo->addItem(QStringLiteral("投诉"), 0);
        typeCombo->addItem(QStringLiteral("建议"), 1);
        typeCombo->setMinimumWidth(120);
        tbLayout->addWidget(typeCombo);
        auto* filterCombo = new QComboBox(toolbar);
        filterCombo->addItems({QStringLiteral("全部状态"), QStringLiteral("待处理"), QStringLiteral("已受理"), QStringLiteral("处理中"), QStringLiteral("已完成"), QStringLiteral("已评价")});
        filterCombo->setMinimumWidth(120);
        tbLayout->addWidget(filterCombo);
        tbLayout->addStretch();
        auto* newBtn = new QPushButton(QStringLiteral("+ 提交投诉"), toolbar);
        newBtn->setProperty("cssClass", "primary");
        newBtn->setCursor(Qt::PointingHandCursor);
        tbLayout->addWidget(newBtn);
        layout->addWidget(toolbar);

        table->setColumnCount(7);
        table->setHorizontalHeaderLabels({QStringLiteral("编号"), QStringLiteral("标题"), QStringLiteral("类型"), QStringLiteral("投诉人"), QStringLiteral("时间"), QStringLiteral("状态"), QStringLiteral("操作")});
        table->setColumnWidth(1, 200);
        table->setColumnWidth(6, 80);
        table->horizontalHeader()->setStretchLastSection(false);

        connect(table, &QTableWidget::cellClicked, this, [=](int r, int c) {
            if (c != 6) return;
            auto* item = table->item(r, c);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            QSqlQuery detail(DatabaseManager::instance().database());
            detail.prepare("SELECT title, description FROM wo_work_order WHERE id = :id");
            detail.bindValue(":id", id);
            if (detail.exec() && detail.next()) {
                QMessageBox::information(this, detail.value(0).toString(), detail.value(1).toString());
            }
        });
        connect(table, &QTableWidget::cellEntered, this, [=](int r, int c) {
            table->viewport()->setCursor(c == 6 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        });

        std::function<void()> loadComplaints = [table, searchEdit, typeCombo, filterCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT id, order_no, title, order_type, reporter_name, create_time, status FROM wo_work_order WHERE del_flag = 0 AND source = 2";
            QString searchText = searchEdit->text().trimmed();
            int filterType = typeCombo->currentData().toInt();
            int filterIdx = filterCombo->currentIndex();
            if (!searchText.isEmpty()) sql += " AND (order_no LIKE '%" + searchText + "%' OR title LIKE '%" + searchText + "%')";
            if (filterType >= 0) sql += " AND order_type = " + QString::number(filterType);
            if (filterIdx > 0) {
                int statusMap[] = {0, 0, 1, 3, 4, 5};
                sql += " AND status = " + QString::number(statusMap[filterIdx]);
            }
            sql += " ORDER BY create_time DESC";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 2, new QTableWidgetItem(q.value(3).toInt() == 0 ? QStringLiteral("投诉") : QStringLiteral("建议")));
                table->setItem(row, 3, new QTableWidgetItem(q.value(4).toString()));
                table->setItem(row, 4, new QTableWidgetItem(q.value(5).toDateTime().toString("yyyy-MM-dd")));
                auto* stsItem = new QTableWidgetItem(WorkOrderStatus::label(q.value(6).toInt()));
                stsItem->setTextAlignment(Qt::AlignCenter);
                { QColor tagBg(WorkOrderStatus::color(q.value(6).toInt())); tagBg.setAlpha(30); stsItem->setBackground(tagBg); }
                stsItem->setForeground(QColor(WorkOrderStatus::color(q.value(6).toInt())));
                table->setItem(row, 5, stsItem);
                auto* actionItem = new QTableWidgetItem(QStringLiteral("查看"));
                actionItem->setTextAlignment(Qt::AlignCenter);
                actionItem->setForeground(QColor("#1677ff"));
                actionItem->setData(Qt::UserRole, q.value(0).toLongLong());
                actionItem->setFont(QFont("Microsoft YaHei", 12));
                table->setItem(row, 6, actionItem);
                row++;
            }
        };
        loadComplaints();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadComplaints(); });
        connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadComplaints(); });
        connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadComplaints(); });

        connect(newBtn, &QPushButton::clicked, this, [=]() {
            QDialog dlg(this);
            dlg.setWindowTitle(QStringLiteral("提交投诉建议"));
            dlg.setMinimumWidth(480);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);
            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* titleEdit = new QLineEdit(&dlg); titleEdit->setPlaceholderText(QStringLiteral("请简要描述"));
            form->addRow(QStringLiteral("标题:"), titleEdit);
            auto* typeCombo = new QComboBox(&dlg);
            typeCombo->addItems({QStringLiteral("投诉"), QStringLiteral("建议"), QStringLiteral("表扬")});
            form->addRow(QStringLiteral("类型:"), typeCombo);
            auto* descEdit = new QTextEdit(&dlg); descEdit->setPlaceholderText(QStringLiteral("详细内容...")); descEdit->setFixedHeight(100);
            form->addRow(QStringLiteral("内容:"), descEdit);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);
            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (titleEdit->text().trimmed().isEmpty()) { QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题")); return; }
                auto& d = DatabaseManager::instance();
                const auto& user = AuthService::instance().currentUser();
                d.insert("wo_work_order", {
                    {"order_no", Utils::generateOrderNo()}, {"title", titleEdit->text().trimmed()},
                    {"order_type", typeCombo->currentIndex() + 1},
                    {"description", descEdit->toPlainText().trimmed()},
                    {"reporter_id", user.id}, {"reporter_name", user.nickname.isEmpty() ? user.username : user.nickname},
                    {"reporter_phone", user.phone}, {"status", 0}, {"source", 2},
                    {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                });
                dlg.accept(); loadComplaints();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec();
        });
    } else if (sub == "inspection") {
        // Property inspection - reuse from governance
        auto* header = new QLabel(QStringLiteral("物业巡检管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("物业巡检计划和记录查看"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索巡检员..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        auto* statusCombo = new QComboBox(toolbar);
        statusCombo->addItem(QStringLiteral("全部状态"), -1);
        statusCombo->addItem(QStringLiteral("进行中"), 0);
        statusCombo->addItem(QStringLiteral("已完成"), 1);
        statusCombo->setMinimumWidth(120);
        tbLayout->addWidget(statusCombo);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({QStringLiteral("巡检员"), QStringLiteral("开始时间"), QStringLiteral("时长(分)"), QStringLiteral("发现问题"), QStringLiteral("状态")});
        std::function<void()> loadInspections = [table, searchEdit, statusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT u.real_name, i.start_time, i.duration, i.issue_count, i.status FROM ge_inspection i LEFT JOIN sys_user u ON i.inspector_id = u.id WHERE i.del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int statusFilter = statusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND u.real_name LIKE '%" + searchText + "%'";
            if (statusFilter >= 0) sql += " AND i.status = " + QString::number(statusFilter);
            sql += " ORDER BY i.start_time DESC";
            QSqlQuery inspQ(sql);
            int iRow = 0;
            while (inspQ.next()) {
                table->insertRow(iRow);
                table->setItem(iRow, 0, new QTableWidgetItem(inspQ.value(0).toString()));
                table->setItem(iRow, 1, new QTableWidgetItem(inspQ.value(1).toDateTime().toString("MM-dd hh:mm")));
                table->setItem(iRow, 2, new QTableWidgetItem(inspQ.value(2).toString()));
                table->setItem(iRow, 3, new QTableWidgetItem(inspQ.value(3).toString()));
                int iSts = inspQ.value(4).toInt();
                auto* stsItem = new QTableWidgetItem(iSts == 0 ? QStringLiteral("进行中") : QStringLiteral("已完成"));
                stsItem->setTextAlignment(Qt::AlignCenter);
                stsItem->setBackground(QColor(iSts == 0 ? "#e6f4ff" : "#f6ffed"));
                stsItem->setForeground(QColor(iSts == 0 ? "#1677ff" : "#52c41a"));
                table->setItem(iRow, 4, stsItem);
                iRow++;
            }
        };
        loadInspections();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadInspections(); });
        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadInspections(); });
    } else if (sub == "announcement") {
        // Page header
        auto* header = new QLabel(QStringLiteral("公告通知管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("发布和管理小区公告、社区通知、物业公告和系统公告"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setStyleSheet("background: transparent;");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 12);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索公告标题..."));
        searchEdit->setMaximumWidth(260);

        tbLayout->addWidget(searchEdit);
        auto* typeCombo = new QComboBox(toolbar);
        typeCombo->addItem(QStringLiteral("全部类型"), -1);
        typeCombo->addItem(QStringLiteral("通知"), 1);
        typeCombo->addItem(QStringLiteral("公告"), 2);
        typeCombo->addItem(QStringLiteral("制度"), 3);
        typeCombo->addItem(QStringLiteral("其他"), 4);
        typeCombo->setMinimumWidth(120);
        tbLayout->addWidget(typeCombo);
        tbLayout->addStretch();
        auto* newBtn = new QPushButton(QStringLiteral("+ 发布公告"), toolbar);
        newBtn->setProperty("cssClass", "primary");
        newBtn->setCursor(Qt::PointingHandCursor);
        tbLayout->addWidget(newBtn);
        layout->addWidget(toolbar);

        // Table
        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({QStringLiteral("标题"), QStringLiteral("类型"), QStringLiteral("发布人"), QStringLiteral("发布时间"), QStringLiteral("阅读数"), QStringLiteral("操作")});
        table->setColumnWidth(0, 250);
        table->setColumnWidth(5, 80);
        table->horizontalHeader()->setStretchLastSection(false);

        connect(table, &QTableWidget::cellClicked, this, [=](int r, int c) {
            if (c != 5) return;
            auto* item = table->item(r, c);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            QString title = item->data(Qt::UserRole + 1).toString();
            QSqlQuery detail(DatabaseManager::instance().database());
            detail.prepare("SELECT content FROM nt_announcement WHERE id = :id");
            detail.bindValue(":id", id);
            if (detail.exec() && detail.next()) {
                QMessageBox::information(this, title, detail.value(0).toString());
            }
        });
        connect(table, &QTableWidget::cellEntered, this, [=](int r, int c) {
            table->viewport()->setCursor(c == 5 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        });

        std::function<void()> loadAnnouncements = [table, searchEdit, typeCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT id, title, announcement_type, publisher_id, publish_time, read_count FROM nt_announcement WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int filterType = typeCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND title LIKE '%" + searchText + "%'";
            if (filterType >= 0) sql += " AND announcement_type = " + QString::number(filterType);
            sql += " ORDER BY publish_time DESC";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 1, new QTableWidgetItem(AnnouncementType::label(q.value(2).toInt())));
                table->setItem(row, 2, new QTableWidgetItem(QStringLiteral("管理员")));
                table->setItem(row, 3, new QTableWidgetItem(q.value(4).toDateTime().toString("yyyy-MM-dd")));
                table->setItem(row, 4, new QTableWidgetItem(QString::number(q.value(5).toInt())));
                auto* actionItem = new QTableWidgetItem(QStringLiteral("查看"));
                actionItem->setTextAlignment(Qt::AlignCenter);
                actionItem->setForeground(QColor("#1677ff"));
                actionItem->setData(Qt::UserRole, q.value(0).toLongLong());
                actionItem->setData(Qt::UserRole + 1, q.value(1).toString());
                actionItem->setFont(QFont("Microsoft YaHei", 12));
                table->setItem(row, 5, actionItem);
                row++;
            }
        };
        loadAnnouncements();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadAnnouncements(); });
        connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadAnnouncements(); });

        // New announcement dialog
        connect(newBtn, &QPushButton::clicked, this, [=]() {
            QDialog dlg(this);
            dlg.setWindowTitle(QStringLiteral("发布公告"));
            dlg.setMinimumWidth(520);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);
            auto* formTitle = new QLabel(QStringLiteral("填写公告信息"), &dlg);
            formTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1f1f1f;");
            dlgLayout->addWidget(formTitle);
            dlgLayout->addSpacing(8);
            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* titleEdit = new QLineEdit(&dlg);
            titleEdit->setPlaceholderText(QStringLiteral("公告标题"));
            form->addRow(QStringLiteral("标题:"), titleEdit);
            auto* typeCombo = new QComboBox(&dlg);
            typeCombo->addItems({QStringLiteral("小区公告"), QStringLiteral("社区公告"), QStringLiteral("物业公告"), QStringLiteral("系统公告")});
            form->addRow(QStringLiteral("类型:"), typeCombo);
            auto* contentEdit = new QTextEdit(&dlg);
            contentEdit->setPlaceholderText(QStringLiteral("公告内容..."));
            contentEdit->setFixedHeight(160);
            form->addRow(QStringLiteral("内容:"), contentEdit);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);
            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("发布"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (titleEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题"));
                    return;
                }
                auto& d = DatabaseManager::instance();
                const auto& user = AuthService::instance().currentUser();
                d.insert("nt_announcement", {
                    {"title", titleEdit->text().trimmed()},
                    {"content", contentEdit->toPlainText().trimmed()},
                    {"announcement_type", typeCombo->currentIndex() + 1},
                    {"publisher_id", user.id},
                    {"publish_time", QDateTime::currentDateTime()},
                    {"status", 1}, {"read_count", 0},
                    {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                });
                dlg.accept();
                loadAnnouncements();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec();
        });
    } else if (sub == "visitor") {
        // Page header
        auto* header = new QLabel(QStringLiteral("访客管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("访客登记、临时通行码管理和访客记录查看"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Stats
        auto* statsRow = new QHBoxLayout();
        auto createMiniCard = [](const QString& label, const QString& val, const QString& color, QWidget* parent) {
            auto* card = new QFrame(parent);
            card->setFixedHeight(90);
            card->setStyleSheet(QString("QFrame{background:#fff;border-radius:8px;border:1px solid #f0f0f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
            auto* cl = new QVBoxLayout(card);
            cl->setContentsMargins(16, 10, 16, 10);
            cl->setSpacing(4);
            auto* indicator = new QFrame(card); indicator->setFixedHeight(3);
            indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
            auto* tl = new QLabel(label); tl->setStyleSheet("color:#8c8c8c;font-size:12px;");
            auto* vl = new QLabel(val); vl->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(color));
            cl->addWidget(indicator); cl->addWidget(tl); cl->addWidget(vl);
            return card;
        };
        QSqlQuery todayVisQ("SELECT COUNT(*) FROM cm_visitor WHERE date(arrive_time) = date('now') AND del_flag = 0"); int todayVis = todayVisQ.next() ? todayVisQ.value(0).toInt() : 0;
        QSqlQuery weekVisQ("SELECT COUNT(*) FROM cm_visitor WHERE arrive_time >= date('now', '-7 days') AND del_flag = 0"); int weekVis = weekVisQ.next() ? weekVisQ.value(0).toInt() : 0;
        QSqlQuery activeVisQ("SELECT COUNT(*) FROM cm_visitor WHERE status = 0 AND del_flag = 0"); int activeVis = activeVisQ.next() ? activeVisQ.value(0).toInt() : 0;
        statsRow->addWidget(createMiniCard(QStringLiteral("今日访客"), QString::number(todayVis), "#1677ff", page));
        statsRow->addWidget(createMiniCard(QStringLiteral("本周访客"), QString::number(weekVis), "#52c41a", page));
        statsRow->addWidget(createMiniCard(QStringLiteral("在访中"), QString::number(activeVis), "#fa8c16", page));
        statsRow->addStretch();
        layout->addLayout(statsRow);
        layout->addSpacing(12);

        // Toolbar
        auto* visToolbar = new QHBoxLayout();
        visToolbar->setSpacing(12);
        auto* visSearchEdit = new QLineEdit();
        visSearchEdit->setPlaceholderText(QStringLiteral("搜索姓名/手机号..."));
        visSearchEdit->setMaximumWidth(260);

        visToolbar->addWidget(visSearchEdit);
        auto* visStatusCombo = new QComboBox();
        visStatusCombo->addItem(QStringLiteral("全部状态"), -1);
        visStatusCombo->addItem(QStringLiteral("在访"), 0);
        visStatusCombo->addItem(QStringLiteral("已离开"), 1);
        visStatusCombo->setMinimumWidth(120);
        visToolbar->addWidget(visStatusCombo);
        visToolbar->addStretch();
        layout->insertLayout(layout->count() - 1, visToolbar);

        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({QStringLiteral("访客姓名"), QStringLiteral("手机号"), QStringLiteral("拜访业主"), QStringLiteral("来访时间"), QStringLiteral("离开时间"), QStringLiteral("状态")});

        std::function<void()> loadVisitors = [table, visSearchEdit, visStatusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT visitor_name, phone, host_name, arrive_time, leave_time, status FROM cm_visitor WHERE del_flag = 0";
            QString visSearch = visSearchEdit->text().trimmed();
            int visFilter = visStatusCombo->currentData().toInt();
            if (!visSearch.isEmpty()) sql += " AND (visitor_name LIKE '%" + visSearch + "%' OR phone LIKE '%" + visSearch + "%')";
            if (visFilter >= 0) sql += " AND status = " + QString::number(visFilter);
            sql += " ORDER BY arrive_time DESC";
            QSqlQuery visQ(sql);
            int vRow = 0;
        while (visQ.next()) {
            table->insertRow(vRow);
            table->setItem(vRow, 0, new QTableWidgetItem(visQ.value(0).toString()));
            table->setItem(vRow, 1, new QTableWidgetItem(visQ.value(1).toString()));
            table->setItem(vRow, 2, new QTableWidgetItem(visQ.value(2).toString()));
            table->setItem(vRow, 3, new QTableWidgetItem(visQ.value(3).toDateTime().toString("yyyy-MM-dd HH:mm")));
            table->setItem(vRow, 4, new QTableWidgetItem(visQ.value(4).isNull() ? "-" : visQ.value(4).toDateTime().toString("yyyy-MM-dd HH:mm")));
            int vSts = visQ.value(5).toInt();
            auto* stsItem = new QTableWidgetItem(vSts == 0 ? QStringLiteral("在访") : QStringLiteral("已离开"));
            stsItem->setTextAlignment(Qt::AlignCenter);
            stsItem->setBackground(QColor(vSts == 0 ? "#e6f4ff" : "#f0f0f0"));
            stsItem->setForeground(QColor(vSts == 0 ? "#1677ff" : "#595959"));
            table->setItem(vRow, 5, stsItem);
            vRow++;
        }
        };
        loadVisitors();
        connect(visSearchEdit, &QLineEdit::textChanged, this, [=]() { loadVisitors(); });
        connect(visStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadVisitors(); });
    } else if (sub == "topic") {
        // Page header
        auto* header = new QLabel(QStringLiteral("业委会议题管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("议题发布、投票管理和结果公示"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索议题标题..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        auto* statusCombo = new QComboBox(toolbar);
        statusCombo->addItem(QStringLiteral("全部状态"), -1);
        statusCombo->addItem(QStringLiteral("投票中"), 0);
        statusCombo->addItem(QStringLiteral("已结束"), 1);
        statusCombo->addItem(QStringLiteral("已通过"), 2);
        statusCombo->setMinimumWidth(120);
        tbLayout->addWidget(statusCombo);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({QStringLiteral("议题标题"), QStringLiteral("类型"), QStringLiteral("发起人"), QStringLiteral("截止日期"), QStringLiteral("投票结果"), QStringLiteral("状态")});
        table->setColumnWidth(0, 220);
        std::function<void()> loadTopics = [table, searchEdit, statusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT t.id, t.title, t.topic_type, t.vote_end, t.status, "
                "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 1) as approve_cnt, "
                "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 2) as oppose_cnt, "
                "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 3) as abstain_cnt, "
                "COALESCE((SELECT real_name FROM sys_user WHERE id = t.publisher_id), '业委会') as publisher "
                "FROM oc_topic t WHERE t.del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int statusFilter = statusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND t.title LIKE '%" + searchText + "%'";
            if (statusFilter >= 0) sql += " AND t.status = " + QString::number(statusFilter);
            sql += " ORDER BY t.publish_time DESC";
            QSqlQuery topicQ(sql);
            int tRow = 0;
            while (topicQ.next()) {
                table->insertRow(tRow);
                table->setItem(tRow, 0, new QTableWidgetItem(topicQ.value(1).toString()));
                table->setItem(tRow, 1, new QTableWidgetItem(TopicType::label(topicQ.value(2).toInt())));
                table->setItem(tRow, 2, new QTableWidgetItem(topicQ.value(8).toString()));
                table->setItem(tRow, 3, new QTableWidgetItem(topicQ.value(3).toDateTime().toString("yyyy-MM-dd")));
                int approve = topicQ.value(5).toInt();
                int oppose = topicQ.value(6).toInt();
                int abstain = topicQ.value(7).toInt();
                int total = approve + oppose + abstain;
                int tSts = topicQ.value(4).toInt();
                QString resultText;
                if (tSts == 0) {
                    resultText = QStringLiteral("投票中");
                } else if (total > 0) {
                    resultText = QStringLiteral("赞成:%1% 反对:%2% 弃权:%3%")
                        .arg(approve * 100 / total).arg(oppose * 100 / total).arg(abstain * 100 / total);
                } else {
                    resultText = QStringLiteral("已结束");
                }
                table->setItem(tRow, 4, new QTableWidgetItem(resultText));
                auto* stsItem = new QTableWidgetItem(tSts == 0 ? QStringLiteral("投票中") : tSts == 1 ? QStringLiteral("已结束") : QStringLiteral("已通过"));
                stsItem->setTextAlignment(Qt::AlignCenter);
                stsItem->setBackground(QColor(tSts == 0 ? "#e6f4ff" : tSts == 1 ? "#f0f0f0" : "#f6ffed"));
                stsItem->setForeground(QColor(tSts == 0 ? "#1677ff" : tSts == 1 ? "#595959" : "#52c41a"));
                table->setItem(tRow, 5, stsItem);
                tRow++;
            }
        };
        loadTopics();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadTopics(); });
        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadTopics(); });
    } else if (sub == "parking") {
        // T33 停车管理
        auto* header = new QLabel(QStringLiteral("停车管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("车位管理、月卡办理和临停记录查看"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Stats
        auto* statsRow = new QHBoxLayout();
        auto mkCard = [](const QString& label, const QString& val, const QString& color, QWidget* parent) {
            auto* card = new QFrame(parent);
            card->setFixedHeight(90);
            card->setStyleSheet(QString("QFrame{background:#fff;border-radius:8px;border:1px solid #f0f0f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
            auto* cl = new QVBoxLayout(card); cl->setContentsMargins(16, 10, 16, 10); cl->setSpacing(4);
            auto* indicator = new QFrame(card); indicator->setFixedHeight(3);
            indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
            auto* tl = new QLabel(label); tl->setStyleSheet("color:#8c8c8c;font-size:12px;");
            auto* vl = new QLabel(val); vl->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(color));
            cl->addWidget(indicator); cl->addWidget(tl); cl->addWidget(vl);
            return card;
        };
        QSqlQuery spaceQ("SELECT COUNT(*) FROM cm_parking_space WHERE del_flag = 0");
        int totalSpaces = spaceQ.next() ? spaceQ.value(0).toInt() : 0;
        QSqlQuery occQ("SELECT COUNT(*) FROM cm_parking_space WHERE del_flag = 0 AND status = 1");
        int occSpaces = occQ.next() ? occQ.value(0).toInt() : 0;
        statsRow->addWidget(mkCard(QStringLiteral("总车位"), QString::number(totalSpaces), "#1677ff", page));
        statsRow->addWidget(mkCard(QStringLiteral("已占用"), QString::number(occSpaces), "#52c41a", page));
        statsRow->addWidget(mkCard(QStringLiteral("空余车位"), QString::number(totalSpaces - occSpaces), "#fa8c16", page));
        QSqlQuery cardQ("SELECT COUNT(*) FROM pm_monthly_card WHERE del_flag = 0 AND status = 1");
        int cardCount = cardQ.next() ? cardQ.value(0).toInt() : 0;
        statsRow->addWidget(mkCard(QStringLiteral("月卡数"), QString::number(cardCount), "#722ed1", page));
        statsRow->addStretch();
        layout->addLayout(statsRow);
        layout->addSpacing(12);

        auto* tabWidget = new QTabWidget(page);
        tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; padding: 8px 16px; }");

        // Parking spaces tab
        auto* spacePage = new QWidget();
        auto* spaceLayout = new QVBoxLayout(spacePage);
        // Space search toolbar
        auto* spaceToolbar = new QWidget(spacePage);
        spaceToolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* spaceTbLayout = new QHBoxLayout(spaceToolbar);
        spaceTbLayout->setContentsMargins(0, 0, 0, 8);
        spaceTbLayout->setSpacing(10);
        auto* spaceSearchEdit = new QLineEdit(spaceToolbar);
        spaceSearchEdit->setPlaceholderText(QStringLiteral("搜索车位编号/区域..."));
        spaceSearchEdit->setMinimumWidth(200);
        spaceSearchEdit->setClearButtonEnabled(true);
        spaceTbLayout->addWidget(spaceSearchEdit);
        auto* spaceStatusCombo = new QComboBox(spaceToolbar);
        spaceStatusCombo->addItem(QStringLiteral("全部状态"), -1);
        spaceStatusCombo->addItem(QStringLiteral("空闲"), 0);
        spaceStatusCombo->addItem(QStringLiteral("已占用"), 1);
        spaceStatusCombo->setMinimumWidth(120);
        spaceTbLayout->addWidget(spaceStatusCombo);
        spaceTbLayout->addStretch();
        spaceLayout->addWidget(spaceToolbar);

        auto* spaceTable = new QTableWidget(spacePage);
        spaceTable->setAlternatingRowColors(true);
        spaceTable->horizontalHeader()->setStretchLastSection(false);
        spaceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        spaceTable->setStyleSheet(TABLE_STYLE);
        spaceTable->setShowGrid(false);
        spaceTable->verticalHeader()->setVisible(false);
        spaceTable->setColumnCount(5);
        spaceTable->setHorizontalHeaderLabels({QStringLiteral("车位编号"), QStringLiteral("区域"), QStringLiteral("类型"), QStringLiteral("关联车辆"), QStringLiteral("状态")});
        std::function<void()> loadSpaces = [spaceTable, spaceSearchEdit, spaceStatusCombo]() {
            while (spaceTable->rowCount() > 0) spaceTable->removeRow(0);
            QString sql = "SELECT ps.space_code, ps.area_name, ps.space_type, v.plate_number, ps.status FROM cm_parking_space ps LEFT JOIN cm_vehicle v ON v.parking_space_id = ps.id WHERE ps.del_flag = 0";
            QString searchText = spaceSearchEdit->text().trimmed();
            int statusFilter = spaceStatusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND (ps.space_code LIKE '%" + searchText + "%' OR ps.area_name LIKE '%" + searchText + "%')";
            if (statusFilter >= 0) sql += " AND ps.status = " + QString::number(statusFilter);
            QSqlQuery spQ(sql);
            int spRow = 0;
            while (spQ.next()) {
                spaceTable->insertRow(spRow);
                spaceTable->setItem(spRow, 0, new QTableWidgetItem(spQ.value(0).toString()));
                spaceTable->setItem(spRow, 1, new QTableWidgetItem(spQ.value(1).toString()));
                spaceTable->setItem(spRow, 2, new QTableWidgetItem(spQ.value(2).toInt() == 1 ? QStringLiteral("产权车位") : QStringLiteral("租赁车位")));
                spaceTable->setItem(spRow, 3, new QTableWidgetItem(spQ.value(3).toString()));
                int spSts = spQ.value(4).toInt();
                auto* stsItem = new QTableWidgetItem(spSts == 0 ? QStringLiteral("空闲") : QStringLiteral("已占用"));
                stsItem->setTextAlignment(Qt::AlignCenter);
                stsItem->setBackground(QColor(spSts == 0 ? "#f6ffed" : "#e6f4ff"));
                stsItem->setForeground(QColor(spSts == 0 ? "#52c41a" : "#1677ff"));
                spaceTable->setItem(spRow, 4, stsItem);
                spRow++;
            }
        };
        loadSpaces();
        connect(spaceSearchEdit, &QLineEdit::textChanged, this, [=]() { loadSpaces(); });
        connect(spaceStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadSpaces(); });
        spaceLayout->addWidget(spaceTable);
        tabWidget->addTab(spacePage, QStringLiteral("车位列表"));

        // Monthly cards tab
        auto* cardPage = new QWidget();
        auto* cardLayout = new QVBoxLayout(cardPage);
        // Card search toolbar
        auto* cardToolbar = new QWidget(cardPage);
        cardToolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* cardTbLayout = new QHBoxLayout(cardToolbar);
        cardTbLayout->setContentsMargins(0, 0, 0, 8);
        cardTbLayout->setSpacing(10);
        auto* cardSearchEdit = new QLineEdit(cardToolbar);
        cardSearchEdit->setPlaceholderText(QStringLiteral("搜索车牌号/车主..."));
        cardSearchEdit->setMinimumWidth(200);
        cardSearchEdit->setClearButtonEnabled(true);
        cardTbLayout->addWidget(cardSearchEdit);
        auto* cardStatusCombo = new QComboBox(cardToolbar);
        cardStatusCombo->addItem(QStringLiteral("全部状态"), -1);
        cardStatusCombo->addItem(QStringLiteral("有效"), 1);
        cardStatusCombo->addItem(QStringLiteral("待续费"), 2);
        cardStatusCombo->addItem(QStringLiteral("已过期"), 3);
        cardStatusCombo->setMinimumWidth(120);
        cardTbLayout->addWidget(cardStatusCombo);
        cardTbLayout->addStretch();
        cardLayout->addWidget(cardToolbar);

        auto* cardTable = new QTableWidget(cardPage);
        cardTable->setAlternatingRowColors(true);
        cardTable->horizontalHeader()->setStretchLastSection(false);
        cardTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        cardTable->setStyleSheet(TABLE_STYLE);
        cardTable->setShowGrid(false);
        cardTable->verticalHeader()->setVisible(false);
        cardTable->setColumnCount(5);
        cardTable->setHorizontalHeaderLabels({QStringLiteral("车牌号"), QStringLiteral("车主"), QStringLiteral("车位"), QStringLiteral("有效期"), QStringLiteral("状态")});
        std::function<void()> loadCards = [cardTable, cardSearchEdit, cardStatusCombo]() {
            while (cardTable->rowCount() > 0) cardTable->removeRow(0);
            QString sql = "SELECT mc.plate_no, mc.owner_name, ps.space_code, mc.start_date, mc.end_date, mc.status "
                "FROM pm_monthly_card mc LEFT JOIN cm_parking_space ps ON mc.space_id = ps.id "
                "WHERE mc.del_flag = 0";
            QString searchText = cardSearchEdit->text().trimmed();
            int statusFilter = cardStatusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND (mc.plate_no LIKE '%" + searchText + "%' OR mc.owner_name LIKE '%" + searchText + "%')";
            if (statusFilter >= 0) sql += " AND mc.status = " + QString::number(statusFilter);
            sql += " ORDER BY mc.end_date DESC";
            QSqlQuery cardQ(sql);
            int cRow = 0;
            while (cardQ.next()) {
                cardTable->insertRow(cRow);
                cardTable->setItem(cRow, 0, new QTableWidgetItem(cardQ.value(0).toString()));
                cardTable->setItem(cRow, 1, new QTableWidgetItem(cardQ.value(1).toString()));
                cardTable->setItem(cRow, 2, new QTableWidgetItem(cardQ.value(2).toString()));
                QString period = cardQ.value(3).toString() + " ~ " + cardQ.value(4).toString();
                cardTable->setItem(cRow, 3, new QTableWidgetItem(period));
                int cSts = cardQ.value(5).toInt();
                auto* stsItem = new QTableWidgetItem(cSts == 1 ? QStringLiteral("有效") : cSts == 2 ? QStringLiteral("待续费") : QStringLiteral("已过期"));
                stsItem->setTextAlignment(Qt::AlignCenter);
                stsItem->setBackground(QColor(cSts == 1 ? "#f6ffed" : cSts == 2 ? "#fff7e6" : "#fff1f0"));
                stsItem->setForeground(QColor(cSts == 1 ? "#52c41a" : cSts == 2 ? "#fa8c16" : "#ff4d4f"));
                cardTable->setItem(cRow, 4, stsItem);
                cRow++;
            }
        };
        loadCards();
        connect(cardSearchEdit, &QLineEdit::textChanged, this, [=]() { loadCards(); });
        connect(cardStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadCards(); });
        cardLayout->addWidget(cardTable);
        tabWidget->addTab(cardPage, QStringLiteral("月卡管理"));
        layout->addWidget(tabWidget);
        return page;
    } else if (sub == "billing") {
        // T31 物业缴费
        auto* header = new QLabel(QStringLiteral("物业缴费"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("物业费账单生成、缴费记录查看（演示级，不涉及真实支付）"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Stats
        auto* statsRow = new QHBoxLayout();
        auto mkCard2 = [](const QString& label, const QString& val, const QString& color, QWidget* parent) {
            auto* card = new QFrame(parent);
            card->setFixedHeight(90);
            card->setStyleSheet(QString("QFrame{background:#fff;border-radius:8px;border:1px solid #f0f0f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
            auto* cl = new QVBoxLayout(card); cl->setContentsMargins(16, 10, 16, 10); cl->setSpacing(4);
            auto* indicator = new QFrame(card); indicator->setFixedHeight(3);
            indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
            auto* tl = new QLabel(label); tl->setStyleSheet("color:#8c8c8c;font-size:12px;");
            auto* vl = new QLabel(val); vl->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(color));
            cl->addWidget(indicator); cl->addWidget(tl); cl->addWidget(vl);
            return card;
        };
        QSqlQuery billTotalQ("SELECT COALESCE(SUM(amount),0) FROM pm_bill WHERE period = strftime('%Y-%m','now') AND del_flag = 0"); double billTotal = billTotalQ.next() ? billTotalQ.value(0).toDouble() : 0;
        QSqlQuery billPaidQ("SELECT COALESCE(SUM(amount),0) FROM pm_bill WHERE period = strftime('%Y-%m','now') AND status = 1 AND del_flag = 0"); double billPaid = billPaidQ.next() ? billPaidQ.value(0).toDouble() : 0;
        double billUnpaid = billTotal - billPaid;
        int billRate = billTotal > 0 ? qRound(billPaid / billTotal * 100) : 0;
        statsRow->addWidget(mkCard2(QStringLiteral("本月应收"), QString("¥%1").arg(billTotal, 0, 'f', 0), "#1677ff", page));
        statsRow->addWidget(mkCard2(QStringLiteral("已缴金额"), QString("¥%1").arg(billPaid, 0, 'f', 0), "#52c41a", page));
        statsRow->addWidget(mkCard2(QStringLiteral("未缴金额"), QString("¥%1").arg(billUnpaid, 0, 'f', 0), "#ff4d4f", page));
        statsRow->addWidget(mkCard2(QStringLiteral("缴费率"), QString::number(billRate) + "%", "#fa8c16", page));
        statsRow->addStretch();
        layout->addLayout(statsRow);
        layout->addSpacing(12);

        // Toolbar
        auto* billToolbar = new QHBoxLayout();
        billToolbar->setSpacing(12);
        auto* billSearchEdit = new QLineEdit();
        billSearchEdit->setPlaceholderText(QStringLiteral("搜索账单编号/房屋..."));
        billSearchEdit->setMaximumWidth(260);

        billToolbar->addWidget(billSearchEdit);
        auto* billTypeCombo = new QComboBox();
        billTypeCombo->addItem(QStringLiteral("全部费用类型"), -1);
        billTypeCombo->addItem(QStringLiteral("物业费"), 1);
        billTypeCombo->addItem(QStringLiteral("水费"), 2);
        billTypeCombo->addItem(QStringLiteral("电费"), 3);
        billTypeCombo->addItem(QStringLiteral("停车费"), 4);
        billTypeCombo->addItem(QStringLiteral("综合"), 5);
        billTypeCombo->setMinimumWidth(120);
        billToolbar->addWidget(billTypeCombo);
        auto* billStatusCombo = new QComboBox();
        billStatusCombo->addItem(QStringLiteral("全部状态"), -1);
        billStatusCombo->addItem(QStringLiteral("待缴费"), 0);
        billStatusCombo->addItem(QStringLiteral("已缴费"), 1);
        billStatusCombo->addItem(QStringLiteral("逾期"), 2);
        billStatusCombo->setMinimumWidth(120);
        billToolbar->addWidget(billStatusCombo);
        billToolbar->addStretch();
        layout->insertLayout(layout->count() - 1, billToolbar);

        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({QStringLiteral("账单编号"), QStringLiteral("房屋"), QStringLiteral("费用类型"), QStringLiteral("金额"), QStringLiteral("账期"), QStringLiteral("状态")});
        table->setColumnWidth(0, 120);

        std::function<void()> loadBills = [table, billSearchEdit, billTypeCombo, billStatusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT b.bill_no, h.house_code || '-' || h.room_number AS house_info, "
                "CASE b.bill_type WHEN 1 THEN '物业费' WHEN 2 THEN '水费' WHEN 3 THEN '电费' WHEN 4 THEN '停车费' WHEN 5 THEN '综合' END AS type_name, "
                "b.amount, b.period, b.status "
                "FROM pm_bill b LEFT JOIN cm_house h ON b.house_id = h.id "
                "WHERE b.del_flag = 0";
            QString billSearch = billSearchEdit->text().trimmed();
            int billTypeFilter = billTypeCombo->currentData().toInt();
            int billStatusFilter = billStatusCombo->currentData().toInt();
            if (!billSearch.isEmpty()) sql += " AND (b.bill_no LIKE '%" + billSearch + "%' OR h.house_code LIKE '%" + billSearch + "%' OR h.room_number LIKE '%" + billSearch + "%')";
            if (billTypeFilter >= 0) sql += " AND b.bill_type = " + QString::number(billTypeFilter);
            if (billStatusFilter >= 0) sql += " AND b.status = " + QString::number(billStatusFilter);
            sql += " ORDER BY b.period DESC, b.create_time DESC";
            QSqlQuery billQ(sql);
            int bRow = 0;
        while (billQ.next()) {
            table->insertRow(bRow);
            table->setItem(bRow, 0, new QTableWidgetItem(billQ.value(0).toString()));
            table->setItem(bRow, 1, new QTableWidgetItem(billQ.value(1).toString()));
            table->setItem(bRow, 2, new QTableWidgetItem(billQ.value(2).toString()));
            table->setItem(bRow, 3, new QTableWidgetItem(QString("¥%1").arg(billQ.value(3).toDouble(), 0, 'f', 2)));
            table->setItem(bRow, 4, new QTableWidgetItem(billQ.value(4).toString()));
            int bSts = billQ.value(5).toInt();
            auto* stsItem = new QTableWidgetItem(bSts == 1 ? QStringLiteral("已缴费") : bSts == 2 ? QStringLiteral("逾期") : QStringLiteral("待缴费"));
            stsItem->setTextAlignment(Qt::AlignCenter);
            stsItem->setBackground(QColor(bSts == 1 ? "#f6ffed" : bSts == 2 ? "#fff1f0" : "#fff7e6"));
            stsItem->setForeground(QColor(bSts == 1 ? "#52c41a" : bSts == 2 ? "#ff4d4f" : "#fa8c16"));
            table->setItem(bRow, 5, stsItem);
            bRow++;
        }
        };
        loadBills();
        connect(billSearchEdit, &QLineEdit::textChanged, this, [=]() { loadBills(); });
        connect(billTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadBills(); });
        connect(billStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadBills(); });
    } else if (sub == "income") {
        // T36 公共收益公示
        auto* header = new QLabel(QStringLiteral("公共收益公示"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("业委会公共收益和支出明细公示"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索账期..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        auto* statusCombo = new QComboBox(toolbar);
        statusCombo->addItem(QStringLiteral("全部状态"), -1);
        statusCombo->addItem(QStringLiteral("已公示"), 0);
        statusCombo->addItem(QStringLiteral("待审核"), 1);
        statusCombo->setMinimumWidth(120);
        tbLayout->addWidget(statusCombo);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({QStringLiteral("账期"), QStringLiteral("收入金额"), QStringLiteral("支出金额"), QStringLiteral("结余"), QStringLiteral("公示时间"), QStringLiteral("状态")});
        std::function<void()> loadIncomes = [table, searchEdit, statusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT period, income_amount, expense_amount, balance, publish_time, status FROM oc_public_income WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int statusFilter = statusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND period LIKE '%" + searchText + "%'";
            if (statusFilter >= 0) sql += " AND status = " + QString::number(statusFilter);
            sql += " ORDER BY publish_time DESC";
            QSqlQuery incQ(sql);
            int iRow = 0;
            while (incQ.next()) {
                table->insertRow(iRow);
                table->setItem(iRow, 0, new QTableWidgetItem(incQ.value(0).toString()));
                table->setItem(iRow, 1, new QTableWidgetItem(QString("¥%1").arg(incQ.value(1).toDouble(), 0, 'f', 2)));
                table->setItem(iRow, 2, new QTableWidgetItem(QString("¥%1").arg(incQ.value(2).toDouble(), 0, 'f', 2)));
                auto* balItem = new QTableWidgetItem(QString("¥%1").arg(incQ.value(3).toDouble(), 0, 'f', 2));
                if (incQ.value(3).toDouble() > 0) balItem->setForeground(QColor("#52c41a"));
                table->setItem(iRow, 3, balItem);
                table->setItem(iRow, 4, new QTableWidgetItem(incQ.value(4).toDateTime().toString("yyyy-MM-dd")));
                auto* stsItem = new QTableWidgetItem(incQ.value(5).toInt() == 0 ? QStringLiteral("已公示") : QStringLiteral("待审核"));
                stsItem->setTextAlignment(Qt::AlignCenter);
                stsItem->setBackground(QColor(incQ.value(5).toInt() == 0 ? "#f6ffed" : "#fff7e6"));
                stsItem->setForeground(QColor(incQ.value(5).toInt() == 0 ? "#52c41a" : "#fa8c16"));
                table->setItem(iRow, 5, stsItem);
                iRow++;
            }
        };
        loadIncomes();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadIncomes(); });
        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadIncomes(); });
    }

    layout->addWidget(table);
    return page;
}

// ========== Governance Pages ==========
QWidget* MainWindow::createGovernancePage(const QString& sub) {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    auto& db = DatabaseManager::instance();

    if (sub == "event") {
        // Page header
        auto* header = new QLabel(QStringLiteral("网格事件管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("管理社区事件上报、审核分派、处理反馈、督办归档全流程"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setStyleSheet("background: transparent;");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 12);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索事件号/标题..."));
        searchEdit->setFixedWidth(220);
        tbLayout->addWidget(searchEdit);
        auto* filterCombo = new QComboBox(toolbar);
        filterCombo->addItems({QStringLiteral("全部状态"), QStringLiteral("待审核"), QStringLiteral("已审核"), QStringLiteral("已分派"), QStringLiteral("处理中"), QStringLiteral("已完成"), QStringLiteral("已归档")});
        filterCombo->setFixedWidth(130);
        tbLayout->addWidget(filterCombo);
        tbLayout->addStretch();
        auto* newBtn = new QPushButton(QStringLiteral("+ 上报事件"), toolbar);
        newBtn->setProperty("cssClass", "primary");
        newBtn->setCursor(Qt::PointingHandCursor);
        tbLayout->addWidget(newBtn);
        layout->addWidget(toolbar);

        // Table
        table->setColumnCount(8);
        table->setHorizontalHeaderLabels({QStringLiteral("事件号"), QStringLiteral("标题"), QStringLiteral("类别"), QStringLiteral("优先级"), QStringLiteral("状态"), QStringLiteral("上报人"), QStringLiteral("创建时间"), QStringLiteral("操作")});
        table->setColumnWidth(1, 180);
        table->setColumnWidth(7, 100);
        table->horizontalHeader()->setStretchLastSection(false);

        std::function<void()> loadEvents = [table, searchEdit, filterCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT id, event_no, title, event_category, priority, status, reporter_name, create_time FROM ge_event WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int filterIdx = filterCombo->currentIndex();
            if (!searchText.isEmpty()) {
                sql += " AND (event_no LIKE :search OR title LIKE :search)";
            }
            if (filterIdx > 0) {
                sql += " AND status = :status";
            }
            sql += " ORDER BY create_time DESC";
            QSqlQuery q(DatabaseManager::instance().database());
            q.prepare(sql);
            if (!searchText.isEmpty()) {
                q.bindValue(":search", "%" + searchText + "%");
            }
            if (filterIdx > 0) {
                int statusMap[] = {-1, 0, 1, 2, 3, 4, 6};
                q.bindValue(":status", statusMap[filterIdx]);
            }
            q.exec();
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 2, new QTableWidgetItem(EventCategory::label(q.value(3).toInt())));
                table->setItem(row, 3, new QTableWidgetItem(EventPriority::label(q.value(4).toInt())));
                auto* statusItem = new QTableWidgetItem(EventStatus::label(q.value(5).toInt()));
                statusItem->setTextAlignment(Qt::AlignCenter);
                { QColor tagBg(EventStatus::color(q.value(5).toInt())); tagBg.setAlpha(30); statusItem->setBackground(tagBg); }
                statusItem->setForeground(QColor(EventStatus::color(q.value(5).toInt())));
                table->setItem(row, 4, statusItem);
                table->setItem(row, 5, new QTableWidgetItem(q.value(6).toString()));
                table->setItem(row, 6, new QTableWidgetItem(q.value(7).toDateTime().toString("yyyy-MM-dd hh:mm")));
                // Action text item
                int sts = q.value(5).toInt();
                qint64 evId = q.value(0).toLongLong();
                QString actionText;
                QString actionColor;
                if (sts == 0) {
                    actionText = QStringLiteral("审核");
                    actionColor = "#1677ff";
                } else if (sts == 1 || sts == 2 || sts == 3) {
                    actionText = QStringLiteral("完成");
                    actionColor = "#52c41a";
                } else {
                    actionText = QStringLiteral("--");
                    actionColor = "#8c8c8c";
                }
                auto* actionItem = new QTableWidgetItem(actionText);
                actionItem->setTextAlignment(Qt::AlignCenter);
                actionItem->setForeground(QColor(actionColor));
                actionItem->setData(Qt::UserRole, evId);
                actionItem->setData(Qt::UserRole + 1, sts);
                actionItem->setFont(QFont("Microsoft YaHei", 12));
                table->setItem(row, 7, actionItem);
                row++;
            }
        };
        loadEvents();
        connect(table, &QTableWidget::cellClicked, this, [=](int r, int c) {
            if (c != 7) return;
            auto* item = table->item(r, c);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 1).toInt();
            if (sts == 0) {
                DatabaseManager::instance().update("ge_event", id, {{"status", 1}, {"review_time", QDateTime::currentDateTime()}});
                loadEvents();
            } else if (sts == 1 || sts == 2 || sts == 3) {
                DatabaseManager::instance().update("ge_event", id, {{"status", 4}, {"finish_time", QDateTime::currentDateTime()}});
                loadEvents();
            }
        });
        connect(table, &QTableWidget::cellEntered, this, [=](int r, int c) {
            table->viewport()->setCursor(c == 7 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        });
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadEvents(); });
        connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadEvents(); });

        // New event dialog
        connect(newBtn, &QPushButton::clicked, this, [=]() {
            QDialog dlg(this);
            dlg.setWindowTitle(QStringLiteral("上报事件"));
            dlg.setMinimumWidth(500);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);
            auto* formTitle = new QLabel(QStringLiteral("填写事件信息"), &dlg);
            formTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1f1f1f;");
            dlgLayout->addWidget(formTitle);
            dlgLayout->addSpacing(8);
            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* titleEdit = new QLineEdit(&dlg);
            titleEdit->setPlaceholderText(QStringLiteral("请简要描述事件"));
            form->addRow(QStringLiteral("标题:"), titleEdit);
            auto* catCombo = new QComboBox(&dlg);
            catCombo->addItems({QStringLiteral("民生服务"), QStringLiteral("环境卫生"), QStringLiteral("设施安全"), QStringLiteral("邻里纠纷"), QStringLiteral("特殊帮扶"), QStringLiteral("市容秩序"), QStringLiteral("突发预警")});
            form->addRow(QStringLiteral("类别:"), catCombo);
            auto* prioCombo = new QComboBox(&dlg);
            prioCombo->addItems({QStringLiteral("一般"), QStringLiteral("重要"), QStringLiteral("紧急"), QStringLiteral("特急")});
            form->addRow(QStringLiteral("优先级:"), prioCombo);
            auto* descEdit = new QTextEdit(&dlg);
            descEdit->setPlaceholderText(QStringLiteral("详细描述事件情况..."));
            descEdit->setFixedHeight(80);
            form->addRow(QStringLiteral("描述:"), descEdit);
            auto* locEdit = new QLineEdit(&dlg);
            locEdit->setPlaceholderText(QStringLiteral("事件发生地点"));
            form->addRow(QStringLiteral("地点:"), locEdit);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);
            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (titleEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题"));
                    return;
                }
                auto& d = DatabaseManager::instance();
                QString eventNo = Utils::generateEventNo();
                const auto& user = AuthService::instance().currentUser();
                d.insert("ge_event", {
                    {"event_no", eventNo}, {"title", titleEdit->text().trimmed()},
                    {"event_category", catCombo->currentIndex() + 1},
                    {"priority", prioCombo->currentIndex() + 1},
                    {"description", descEdit->toPlainText().trimmed()},
                    {"location", locEdit->text().trimmed()},
                    {"reporter_id", user.id}, {"reporter_name", user.nickname.isEmpty() ? user.username : user.nickname},
                    {"status", 0}, {"source", 1},
                    {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                });
                dlg.accept();
                loadEvents();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec();
        });
    } else if (sub == "inspection") {
        // Page header
        auto* header = new QLabel(QStringLiteral("社区巡查管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("管理巡查计划、查看巡查记录和问题发现"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Tab for plan and record
        auto* tabWidget = new QTabWidget(page);
        tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; padding: 8px 16px; }");

        // Plan tab
        auto* planPage = new QWidget();
        auto* planLayout = new QVBoxLayout(planPage);
        // Plan search toolbar
        auto* planToolbar = new QWidget(planPage);
        planToolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* planTbLayout = new QHBoxLayout(planToolbar);
        planTbLayout->setContentsMargins(0, 0, 0, 8);
        planTbLayout->setSpacing(10);
        auto* planSearchEdit = new QLineEdit(planToolbar);
        planSearchEdit->setPlaceholderText(QStringLiteral("搜索计划名称/巡查员..."));
        planSearchEdit->setMinimumWidth(200);
        planSearchEdit->setClearButtonEnabled(true);
        planTbLayout->addWidget(planSearchEdit);
        planTbLayout->addStretch();
        planLayout->addWidget(planToolbar);

        auto* planTable = new QTableWidget(planPage);
        planTable->setAlternatingRowColors(true);
        planTable->horizontalHeader()->setStretchLastSection(false);
        planTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        planTable->setStyleSheet(TABLE_STYLE);
        planTable->setShowGrid(false);
        planTable->verticalHeader()->setVisible(false);
        planTable->setColumnCount(6);
        planTable->setHorizontalHeaderLabels({QStringLiteral("计划名称"), QStringLiteral("网格"), QStringLiteral("频率"), QStringLiteral("巡查员"), QStringLiteral("起止日期"), QStringLiteral("状态")});
        std::function<void()> loadPlans = [planTable, planSearchEdit]() {
            while (planTable->rowCount() > 0) planTable->removeRow(0);
            QString sql = "SELECT ip.plan_name, g.grid_name, ip.frequency, u.real_name, ip.start_date || '~' || ip.end_date, ip.status FROM ge_inspection_plan ip LEFT JOIN cm_grid g ON ip.grid_id = g.id LEFT JOIN sys_user u ON ip.inspector_id = u.id WHERE ip.del_flag = 0";
            QString searchText = planSearchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND (ip.plan_name LIKE '%" + searchText + "%' OR u.real_name LIKE '%" + searchText + "%')";
            sql += " ORDER BY ip.create_time DESC";
            QSqlQuery planQ(sql);
            int pRow = 0;
            while (planQ.next()) {
                planTable->insertRow(pRow);
                for (int c = 0; c < 5; ++c) planTable->setItem(pRow, c, new QTableWidgetItem(planQ.value(c).toString()));
                int pSts = planQ.value(5).toInt();
                auto* planStsItem = new QTableWidgetItem(pSts == 0 ? QStringLiteral("待执行") : pSts == 1 ? QStringLiteral("进行中") : QStringLiteral("已完成"));
                planStsItem->setTextAlignment(Qt::AlignCenter);
                if (pSts == 0) {
                    planStsItem->setBackground(QColor(230, 244, 255));
                    planStsItem->setForeground(QColor(22, 119, 255));
                } else if (pSts == 1) {
                    planStsItem->setBackground(QColor(255, 247, 230));
                    planStsItem->setForeground(QColor(250, 140, 22));
                } else {
                    planStsItem->setBackground(QColor(246, 255, 237));
                    planStsItem->setForeground(QColor(82, 196, 26));
                }
                planTable->setItem(pRow, 5, planStsItem);
                pRow++;
            }
        };
        loadPlans();
        connect(planSearchEdit, &QLineEdit::textChanged, this, [=]() { loadPlans(); });
        planLayout->addWidget(planTable);
        tabWidget->addTab(planPage, QStringLiteral("巡查计划"));

        // Record tab
        auto* recordPage = new QWidget();
        auto* recordLayout = new QVBoxLayout(recordPage);
        // Record search toolbar
        auto* recToolbar = new QWidget(recordPage);
        recToolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* recTbLayout = new QHBoxLayout(recToolbar);
        recTbLayout->setContentsMargins(0, 0, 0, 8);
        recTbLayout->setSpacing(10);
        auto* recSearchEdit = new QLineEdit(recToolbar);
        recSearchEdit->setPlaceholderText(QStringLiteral("搜索巡查员..."));
        recSearchEdit->setMinimumWidth(200);
        recSearchEdit->setClearButtonEnabled(true);
        recTbLayout->addWidget(recSearchEdit);
        auto* recStatusCombo = new QComboBox(recToolbar);
        recStatusCombo->addItem(QStringLiteral("全部状态"), -1);
        recStatusCombo->addItem(QStringLiteral("进行中"), 0);
        recStatusCombo->addItem(QStringLiteral("已完成"), 1);
        recStatusCombo->setMinimumWidth(120);
        recTbLayout->addWidget(recStatusCombo);
        recTbLayout->addStretch();
        recordLayout->addWidget(recToolbar);

        auto* recTable = new QTableWidget(recordPage);
        recTable->setAlternatingRowColors(true);
        recTable->horizontalHeader()->setStretchLastSection(false);
        recTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        recTable->setStyleSheet(TABLE_STYLE);
        recTable->setShowGrid(false);
        recTable->verticalHeader()->setVisible(false);
        recTable->setColumnCount(6);
        recTable->setHorizontalHeaderLabels({QStringLiteral("巡查员"), QStringLiteral("开始时间"), QStringLiteral("结束时间"), QStringLiteral("时长(分)"), QStringLiteral("发现问题"), QStringLiteral("状态")});
        std::function<void()> loadRecords = [recTable, recSearchEdit, recStatusCombo]() {
            while (recTable->rowCount() > 0) recTable->removeRow(0);
            QString sql = "SELECT u.real_name, i.start_time, i.end_time, i.duration, i.issue_count, i.status FROM ge_inspection i LEFT JOIN sys_user u ON i.inspector_id = u.id WHERE i.del_flag = 0";
            QString searchText = recSearchEdit->text().trimmed();
            int statusFilter = recStatusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND u.real_name LIKE '%" + searchText + "%'";
            if (statusFilter >= 0) sql += " AND i.status = " + QString::number(statusFilter);
            sql += " ORDER BY i.start_time DESC";
            QSqlQuery recQ(sql);
            int rRow = 0;
            while (recQ.next()) {
                recTable->insertRow(rRow);
                recTable->setItem(rRow, 0, new QTableWidgetItem(recQ.value(0).toString()));
                recTable->setItem(rRow, 1, new QTableWidgetItem(recQ.value(1).toDateTime().toString("MM-dd hh:mm")));
                recTable->setItem(rRow, 2, new QTableWidgetItem(recQ.value(2).toDateTime().toString("MM-dd hh:mm")));
                recTable->setItem(rRow, 3, new QTableWidgetItem(recQ.value(3).toString()));
                recTable->setItem(rRow, 4, new QTableWidgetItem(recQ.value(4).toString()));
                int iSts = recQ.value(5).toInt();
                auto* stsItem = new QTableWidgetItem(iSts == 0 ? QStringLiteral("进行中") : QStringLiteral("已完成"));
                stsItem->setTextAlignment(Qt::AlignCenter);
                stsItem->setBackground(QColor(iSts == 0 ? "#e6f4ff" : "#f6ffed"));
                stsItem->setForeground(QColor(iSts == 0 ? "#1677ff" : "#52c41a"));
                recTable->setItem(rRow, 5, stsItem);
                rRow++;
            }
        };
        loadRecords();
        connect(recSearchEdit, &QLineEdit::textChanged, this, [=]() { loadRecords(); });
        connect(recStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadRecords(); });
        recordLayout->addWidget(recTable);
        tabWidget->addTab(recordPage, QStringLiteral("巡查记录"));
        layout->addWidget(tabWidget);
        return page;
    } else if (sub == "care") {
        // Page header
        auto* header = new QLabel(QStringLiteral("重点人群关怀"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("管理特殊群体信息、走访计划和关怀记录"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        auto* tabWidget = new QTabWidget(page);
        tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; padding: 8px 16px; }");

        // Group tab
        auto* groupPage = new QWidget();
        auto* groupLayout = new QVBoxLayout(groupPage);
        // Group search toolbar
        auto* grpToolbar = new QWidget(groupPage);
        grpToolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* grpTbLayout = new QHBoxLayout(grpToolbar);
        grpTbLayout->setContentsMargins(0, 0, 0, 8);
        grpTbLayout->setSpacing(10);
        auto* grpSearchEdit = new QLineEdit(grpToolbar);
        grpSearchEdit->setPlaceholderText(QStringLiteral("搜索居民姓名..."));
        grpSearchEdit->setMinimumWidth(200);
        grpSearchEdit->setClearButtonEnabled(true);
        grpTbLayout->addWidget(grpSearchEdit);
        grpTbLayout->addStretch();
        groupLayout->addWidget(grpToolbar);

        auto* groupTable = new QTableWidget(groupPage);
        groupTable->setAlternatingRowColors(true);
        groupTable->horizontalHeader()->setStretchLastSection(false);
        groupTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        groupTable->setStyleSheet(TABLE_STYLE);
        groupTable->setShowGrid(false);
        groupTable->verticalHeader()->setVisible(false);
        groupTable->setColumnCount(6);
        groupTable->setHorizontalHeaderLabels({QStringLiteral("居民姓名"), QStringLiteral("类型"), QStringLiteral("关怀等级"), QStringLiteral("走访频率"), QStringLiteral("责任人"), QStringLiteral("最近走访")});
        std::function<void()> loadGroups = [groupTable, grpSearchEdit]() {
            while (groupTable->rowCount() > 0) groupTable->removeRow(0);
            QString sql = "SELECT r.name, sg.group_type, sg.care_level, sg.care_frequency, u.real_name, sg.last_visit_time FROM cm_special_group sg JOIN cm_resident r ON sg.resident_id = r.id LEFT JOIN sys_user u ON sg.care_worker_id = u.id WHERE sg.del_flag = 0";
            QString searchText = grpSearchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND r.name LIKE '%" + searchText + "%'";
            QSqlQuery grpQ(sql);
            int gRow = 0;
            while (grpQ.next()) {
                groupTable->insertRow(gRow);
                groupTable->setItem(gRow, 0, new QTableWidgetItem(grpQ.value(0).toString()));
                groupTable->setItem(gRow, 1, new QTableWidgetItem(SpecialGroupType::label(grpQ.value(1).toInt())));
                int lvl = grpQ.value(2).toInt();
                auto* lvlItem = new QTableWidgetItem(lvl == 1 ? QStringLiteral("一般") : lvl == 2 ? QStringLiteral("重点") : QStringLiteral("特殊"));
                lvlItem->setTextAlignment(Qt::AlignCenter);
                lvlItem->setBackground(QColor(lvl == 1 ? "#e6f4ff" : lvl == 2 ? "#fff7e6" : "#fff1f0"));
                lvlItem->setForeground(QColor(lvl == 1 ? "#1677ff" : lvl == 2 ? "#fa8c16" : "#ff4d4f"));
                groupTable->setItem(gRow, 2, lvlItem);
                groupTable->setItem(gRow, 3, new QTableWidgetItem(grpQ.value(3).toString()));
                groupTable->setItem(gRow, 4, new QTableWidgetItem(grpQ.value(4).toString()));
                groupTable->setItem(gRow, 5, new QTableWidgetItem(grpQ.value(5).toDateTime().toString("yyyy-MM-dd")));
                gRow++;
            }
        };
        loadGroups();
        connect(grpSearchEdit, &QLineEdit::textChanged, this, [=]() { loadGroups(); });
        groupLayout->addWidget(groupTable);
        tabWidget->addTab(groupPage, QStringLiteral("关怀对象"));

        // Visit record tab
        auto* visitPage = new QWidget();
        auto* visitLayout = new QVBoxLayout(visitPage);
        // Visit search toolbar
        auto* visToolbar = new QWidget(visitPage);
        visToolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* visTbLayout = new QHBoxLayout(visToolbar);
        visTbLayout->setContentsMargins(0, 0, 0, 8);
        visTbLayout->setSpacing(10);
        auto* visSearchEdit = new QLineEdit(visToolbar);
        visSearchEdit->setPlaceholderText(QStringLiteral("搜索走访人..."));
        visSearchEdit->setMinimumWidth(200);
        visSearchEdit->setClearButtonEnabled(true);
        visTbLayout->addWidget(visSearchEdit);
        visTbLayout->addStretch();
        visitLayout->addWidget(visToolbar);

        auto* visitTable = new QTableWidget(visitPage);
        visitTable->setAlternatingRowColors(true);
        visitTable->horizontalHeader()->setStretchLastSection(false);
        visitTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        visitTable->setStyleSheet(TABLE_STYLE);
        visitTable->setShowGrid(false);
        visitTable->verticalHeader()->setVisible(false);
        visitTable->setColumnCount(5);
        visitTable->setHorizontalHeaderLabels({QStringLiteral("走访人"), QStringLiteral("走访时间"), QStringLiteral("类型"), QStringLiteral("发现问题"), QStringLiteral("后续跟进")});
        visitTable->setColumnWidth(3, 180);
        visitTable->setColumnWidth(4, 180);
        std::function<void()> loadVisits = [visitTable, visSearchEdit]() {
            while (visitTable->rowCount() > 0) visitTable->removeRow(0);
            QString sql = "SELECT u.real_name, vr.visit_time, vr.visit_type, vr.found_issues, vr.follow_up FROM ge_visit_record vr LEFT JOIN sys_user u ON vr.visitor_id = u.id WHERE vr.del_flag = 0";
            QString searchText = visSearchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND u.real_name LIKE '%" + searchText + "%'";
            sql += " ORDER BY vr.visit_time DESC";
            QSqlQuery visQ(sql);
            int vRow = 0;
            while (visQ.next()) {
                visitTable->insertRow(vRow);
                visitTable->setItem(vRow, 0, new QTableWidgetItem(visQ.value(0).toString()));
                visitTable->setItem(vRow, 1, new QTableWidgetItem(visQ.value(1).toDateTime().toString("yyyy-MM-dd hh:mm")));
                visitTable->setItem(vRow, 2, new QTableWidgetItem(visQ.value(2).toInt() == 1 ? QStringLiteral("定期走访") : visQ.value(2).toInt() == 2 ? QStringLiteral("临时走访") : QStringLiteral("电话慰问")));
                visitTable->setItem(vRow, 3, new QTableWidgetItem(visQ.value(3).toString()));
                visitTable->setItem(vRow, 4, new QTableWidgetItem(visQ.value(4).toString()));
                vRow++;
            }
        };
        loadVisits();
        connect(visSearchEdit, &QLineEdit::textChanged, this, [=]() { loadVisits(); });
        visitLayout->addWidget(visitTable);
        tabWidget->addTab(visitPage, QStringLiteral("走访记录"));
        layout->addWidget(tabWidget);
        return page;
    } else if (sub == "supervision") {
        // Page header
        auto* header = new QLabel(QStringLiteral("督办管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("督办任务创建、进度跟踪和催办管理"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索事件/督办人..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        auto* statusCombo = new QComboBox(toolbar);
        statusCombo->addItem(QStringLiteral("全部状态"), -1);
        statusCombo->addItem(QStringLiteral("待处理"), 0);
        statusCombo->addItem(QStringLiteral("已反馈"), 1);
        statusCombo->addItem(QStringLiteral("已关闭"), 2);
        statusCombo->setMinimumWidth(120);
        tbLayout->addWidget(statusCombo);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(7);
        table->setHorizontalHeaderLabels({QStringLiteral("关联事件"), QStringLiteral("督办人"), QStringLiteral("被督办人"), QStringLiteral("截止日期"), QStringLiteral("原因"), QStringLiteral("反馈"), QStringLiteral("状态")});
        table->setColumnWidth(4, 150);
        table->setColumnWidth(5, 150);
        std::function<void()> loadSupervisions = [table, searchEdit, statusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT e.title, u1.real_name, u2.real_name, s.deadline, s.reason, s.feedback, s.status FROM ge_supervision s LEFT JOIN ge_event e ON s.event_id = e.id LEFT JOIN sys_user u1 ON s.supervisor_id = u1.id LEFT JOIN sys_user u2 ON s.supervise_to = u2.id WHERE s.del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int statusFilter = statusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND (e.title LIKE '%" + searchText + "%' OR u1.real_name LIKE '%" + searchText + "%')";
            if (statusFilter >= 0) sql += " AND s.status = " + QString::number(statusFilter);
            sql += " ORDER BY s.create_time DESC";
            QSqlQuery supQ(sql);
            int sRow = 0;
            while (supQ.next()) {
                table->insertRow(sRow);
                table->setItem(sRow, 0, new QTableWidgetItem(supQ.value(0).toString()));
                table->setItem(sRow, 1, new QTableWidgetItem(supQ.value(1).toString()));
                table->setItem(sRow, 2, new QTableWidgetItem(supQ.value(2).toString()));
                table->setItem(sRow, 3, new QTableWidgetItem(supQ.value(3).toDateTime().toString("yyyy-MM-dd")));
                table->setItem(sRow, 4, new QTableWidgetItem(supQ.value(4).toString()));
                table->setItem(sRow, 5, new QTableWidgetItem(supQ.value(5).toString()));
                int sSts = supQ.value(6).toInt();
                auto* stsItem = new QTableWidgetItem(sSts == 0 ? QStringLiteral("待处理") : sSts == 1 ? QStringLiteral("已反馈") : QStringLiteral("已关闭"));
                stsItem->setTextAlignment(Qt::AlignCenter);
                stsItem->setBackground(QColor(sSts == 0 ? "#fff7e6" : sSts == 1 ? "#e6f4ff" : "#f0f0f0"));
                stsItem->setForeground(QColor(sSts == 0 ? "#fa8c16" : sSts == 1 ? "#1677ff" : "#595959"));
                table->setItem(sRow, 6, stsItem);
                sRow++;
            }
        };
        loadSupervisions();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadSupervisions(); });
        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadSupervisions(); });
    } else if (sub == "opinion") {
        // Page header
        auto* header = new QLabel(QStringLiteral("民意收集"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("居民意见箱、建议收集和热点问题分析"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Stats cards
        auto* statsRow = new QHBoxLayout();
        auto createMiniCard = [](const QString& label, const QString& val, const QString& color, QWidget* parent) {
            auto* card = new QFrame(parent);
            card->setFixedHeight(90);
            card->setStyleSheet(QString("QFrame{background:#fff;border-radius:8px;border:1px solid #f0f0f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
            auto* cl = new QVBoxLayout(card);
            cl->setContentsMargins(16, 10, 16, 10);
            cl->setSpacing(4);
            auto* indicator = new QFrame(card); indicator->setFixedHeight(3);
            indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
            auto* tl = new QLabel(label); tl->setStyleSheet("color:#8c8c8c;font-size:12px;");
            auto* vl = new QLabel(val); vl->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(color));
            cl->addWidget(indicator); cl->addWidget(tl); cl->addWidget(vl);
            return card;
        };
        QSqlQuery opTotalQ("SELECT COUNT(*) FROM ge_opinion WHERE create_time >= date('now','start of month') AND del_flag = 0"); int opTotal = opTotalQ.next() ? opTotalQ.value(0).toInt() : 0;
        QSqlQuery opReplyQ("SELECT COUNT(*) FROM ge_opinion WHERE status >= 1 AND del_flag = 0"); int opReply = opReplyQ.next() ? opReplyQ.value(0).toInt() : 0;
        QSqlQuery opCatQ("SELECT COUNT(DISTINCT category) FROM ge_opinion WHERE del_flag = 0"); int opCat = opCatQ.next() ? opCatQ.value(0).toInt() : 0;
        QSqlQuery opAdoptQ("SELECT COUNT(*) FROM ge_opinion WHERE status = 2 AND del_flag = 0"); int opAdopt = opAdoptQ.next() ? opAdoptQ.value(0).toInt() : 0;
        int satRate = opTotal > 0 ? qRound((opReply + opAdopt) * 100.0 / opTotal) : 0;
        statsRow->addWidget(createMiniCard(QStringLiteral("本月意见"), QString::number(opTotal), "#1677ff", page));
        statsRow->addWidget(createMiniCard(QStringLiteral("已回复"), QString::number(opReply), "#52c41a", page));
        statsRow->addWidget(createMiniCard(QStringLiteral("热点类别"), QString::number(opCat), "#fa8c16", page));
        statsRow->addWidget(createMiniCard(QStringLiteral("响应率"), QString::number(satRate) + "%", "#52c41a", page));
        layout->addLayout(statsRow);
        layout->addSpacing(12);

        // Toolbar
        auto* opToolbar = new QHBoxLayout();
        opToolbar->setSpacing(12);
        auto* opSearchEdit = new QLineEdit();
        opSearchEdit->setPlaceholderText(QStringLiteral("搜索意见内容..."));
        opSearchEdit->setMaximumWidth(260);

        opToolbar->addWidget(opSearchEdit);
        auto* opCatCombo = new QComboBox();
        opCatCombo->addItem(QStringLiteral("全部类别"), -1);
        opCatCombo->addItem(QStringLiteral("环境"), QStringLiteral("环境"));
        opCatCombo->addItem(QStringLiteral("设施"), QStringLiteral("设施"));
        opCatCombo->addItem(QStringLiteral("安全"), QStringLiteral("安全"));
        opCatCombo->addItem(QStringLiteral("服务"), QStringLiteral("服务"));
        opCatCombo->addItem(QStringLiteral("其他"), QStringLiteral("其他"));
        opCatCombo->setMinimumWidth(120);
        opToolbar->addWidget(opCatCombo);
        auto* opStatusCombo = new QComboBox();
        opStatusCombo->addItem(QStringLiteral("全部状态"), -1);
        opStatusCombo->addItem(QStringLiteral("待处理"), 0);
        opStatusCombo->addItem(QStringLiteral("已回复"), 1);
        opStatusCombo->addItem(QStringLiteral("已采纳"), 2);
        opStatusCombo->setMinimumWidth(120);
        opToolbar->addWidget(opStatusCombo);
        opToolbar->addStretch();
        layout->insertLayout(layout->count() - 1, opToolbar);

        // Opinions table
        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({QStringLiteral("提交人"), QStringLiteral("类别"), QStringLiteral("内容摘要"), QStringLiteral("提交时间"), QStringLiteral("状态")});
        table->setColumnWidth(2, 250);

        std::function<void()> loadOpinions = [table, opSearchEdit, opCatCombo, opStatusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT o.title, o.category, o.content, r.name, o.create_time, o.status "
                "FROM ge_opinion o LEFT JOIN cm_resident r ON o.resident_id = r.id "
                "WHERE o.del_flag = 0";
            QString opSearch = opSearchEdit->text().trimmed();
            QString opCatFilter = opCatCombo->currentData().toString();
            int opStatusFilter = opStatusCombo->currentData().toInt();
            if (!opSearch.isEmpty()) sql += " AND o.content LIKE '%" + opSearch + "%'";
            if (opCatFilter != "-1" && !opCatFilter.isEmpty()) sql += " AND o.category = '" + opCatFilter + "'";
            if (opStatusFilter >= 0) sql += " AND o.status = " + QString::number(opStatusFilter);
            sql += " ORDER BY o.create_time DESC";
            QSqlQuery opQ(sql);
            int oRow = 0;
        while (opQ.next()) {
            table->insertRow(oRow);
            table->setItem(oRow, 0, new QTableWidgetItem(opQ.value(3).toString()));
            table->setItem(oRow, 1, new QTableWidgetItem(opQ.value(1).toString()));
            table->setItem(oRow, 2, new QTableWidgetItem(opQ.value(2).toString()));
            table->setItem(oRow, 3, new QTableWidgetItem(opQ.value(4).toDateTime().toString("yyyy-MM-dd")));
            int oSts = opQ.value(5).toInt();
            auto* stsItem = new QTableWidgetItem(oSts == 0 ? QStringLiteral("待处理") : oSts == 1 ? QStringLiteral("已回复") : QStringLiteral("已采纳"));
            stsItem->setTextAlignment(Qt::AlignCenter);
            stsItem->setBackground(QColor(oSts == 0 ? "#fff7e6" : oSts == 1 ? "#e6f4ff" : "#f6ffed"));
            stsItem->setForeground(QColor(oSts == 0 ? "#fa8c16" : oSts == 1 ? "#1677ff" : "#52c41a"));
            table->setItem(oRow, 4, stsItem);
            oRow++;
        }
        };
        loadOpinions();
        connect(opSearchEdit, &QLineEdit::textChanged, this, [=]() { loadOpinions(); });
        connect(opCatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadOpinions(); });
        connect(opStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadOpinions(); });
    } else if (sub == "assessment") {
        // Page header
        auto* header = new QLabel(QStringLiteral("考核管理"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("考核指标设置、完成率统计和排名展示"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Stats
        auto* statsRow = new QHBoxLayout();
        auto createStatCard = [](const QString& label, const QString& val, const QString& color, QWidget* parent) {
            auto* card = new QFrame(parent);
            card->setFixedHeight(90);
            card->setStyleSheet(QString("QFrame{background:#fff;border-radius:8px;border:1px solid #f0f0f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
            auto* cl = new QVBoxLayout(card);
            cl->setContentsMargins(16, 10, 16, 10);
            cl->setSpacing(4);
            auto* indicator = new QFrame(card); indicator->setFixedHeight(3);
            indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
            auto* tl = new QLabel(label); tl->setStyleSheet("color:#8c8c8c;font-size:12px;");
            auto* vl = new QLabel(val); vl->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(color));
            cl->addWidget(indicator); cl->addWidget(tl); cl->addWidget(vl);
            return card;
        };
        QSqlQuery asmtCntQ("SELECT COUNT(DISTINCT config_id) FROM kf_assessment_config"); int asmtCnt = asmtCntQ.next() ? asmtCntQ.value(0).toInt() : 0;
        QSqlQuery avgRateQ("SELECT AVG(actual_value) FROM kf_assessment_result WHERE del_flag = 0"); double avgRate = avgRateQ.next() ? avgRateQ.value(0).toDouble() : 0;
        QSqlQuery excellentQ("SELECT COUNT(DISTINCT target_user_id) FROM kf_assessment_result WHERE del_flag = 0 AND score >= 90"); int excellent = excellentQ.next() ? excellentQ.value(0).toInt() : 0;
        QSqlQuery timeoutQ("SELECT COUNT(*) FROM ge_event WHERE del_flag = 0 AND sla_deadline IS NOT NULL AND finish_time > sla_deadline"); int timeout = timeoutQ.next() ? timeoutQ.value(0).toInt() : 0;
        statsRow->addWidget(createStatCard(QStringLiteral("考核指标"), QString::number(asmtCnt), "#1677ff", page));
        statsRow->addWidget(createStatCard(QStringLiteral("平均完成率"), QString::number(qRound(avgRate)) + "%", "#52c41a", page));
        statsRow->addWidget(createStatCard(QStringLiteral("优秀网格员"), QString::number(excellent), "#fa8c16", page));
        statsRow->addWidget(createStatCard(QStringLiteral("超时事件"), QString::number(timeout), "#ff4d4f", page));
        layout->addLayout(statsRow);
        layout->addSpacing(12);

        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索网格员姓名..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        // Ranking table
        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({QStringLiteral("排名"), QStringLiteral("网格员"), QStringLiteral("处理事件"), QStringLiteral("平均时效(h)"), QStringLiteral("完成率"), QStringLiteral("评分")});
        std::function<void()> loadAssessments = [table, searchEdit]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT ar.target_user_id, u.real_name, COUNT(*) as event_count, "
                "AVG(ar.actual_value) as avg_actual, AVG(ar.score) as avg_score, MIN(ar.rank) as best_rank "
                "FROM kf_assessment_result ar "
                "LEFT JOIN sys_user u ON ar.target_user_id = u.id "
                "WHERE ar.del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND u.real_name LIKE '%" + searchText + "%'";
            sql += " GROUP BY ar.target_user_id ORDER BY avg_score DESC";
            QSqlQuery rankQ(sql);
            int rRow = 0;
            while (rankQ.next()) {
                table->insertRow(rRow);
                auto* rankItem = new QTableWidgetItem(QString::number(rRow + 1));
                if (rRow < 3) { rankItem->setBackground(QColor("#fff7e6")); rankItem->setForeground(QColor("#fa8c16")); }
                table->setItem(rRow, 0, rankItem);
                table->setItem(rRow, 1, new QTableWidgetItem(rankQ.value(1).toString()));
                table->setItem(rRow, 2, new QTableWidgetItem(rankQ.value(2).toString()));
                table->setItem(rRow, 3, new QTableWidgetItem(QString::number(rankQ.value(3).toDouble(), 'f', 1)));
                double avgActual = rankQ.value(3).toDouble();
                table->setItem(rRow, 4, new QTableWidgetItem(QString::number(avgActual, 'f', 1) + "%"));
                int score = qRound(rankQ.value(4).toDouble());
                auto* scoreItem = new QTableWidgetItem(QString::number(score));
                if (score >= 90) { scoreItem->setBackground(QColor("#f6ffed")); scoreItem->setForeground(QColor("#52c41a")); }
                table->setItem(rRow, 5, scoreItem);
                rRow++;
            }
        };
        loadAssessments();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadAssessments(); });
    }

    layout->addWidget(table);
    return page;
}

// ========== Service Pages ==========
QWidget* MainWindow::createServicePage(const QString& sub) {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    auto& db = DatabaseManager::instance();

    if (sub == "volunteer") {
        // Page header
        auto* header = new QLabel(QStringLiteral("志愿服务 - 活动组织与报名"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("社区志愿活动发布、报名、签到和时长管理"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Stat cards
        auto* statsLayout = new QHBoxLayout();
        int totalAct = 0, recruiting = 0, totalSignup = 0, ended = 0;
        QSqlQuery sq1("SELECT COUNT(*) FROM sv_volunteer_activity WHERE del_flag = 0"); if(sq1.next()) totalAct = sq1.value(0).toInt();
        QSqlQuery sq2("SELECT COUNT(*) FROM sv_volunteer_activity WHERE status = 1 AND del_flag = 0"); if(sq2.next()) recruiting = sq2.value(0).toInt();
        QSqlQuery sq3("SELECT COUNT(*) FROM sv_volunteer_signup"); if(sq3.next()) totalSignup = sq3.value(0).toInt();
        QSqlQuery sq4("SELECT COUNT(*) FROM sv_volunteer_activity WHERE status = 3 AND del_flag = 0"); if(sq4.next()) ended = sq4.value(0).toInt();
        struct VolStat { QString label; QString val; QString color; };
        VolStat volStats[] = {
            {QStringLiteral("总活动数"), QString::number(totalAct), "#1677ff"},
            {QStringLiteral("招募中"), QString::number(recruiting), "#52c41a"},
            {QStringLiteral("报名人次"), QString::number(totalSignup), "#faad14"},
            {QStringLiteral("已结束"), QString::number(ended), "#8c8c8c"},
        };
        for (auto& s : volStats) {
            auto* card = new QFrame(page);
            card->setStyleSheet(QString("QFrame{background:#fff;border-radius:8px;border:1px solid #f0f0f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(s.color));
            card->setFixedHeight(90);
            auto* cl = new QVBoxLayout(card);
            cl->setContentsMargins(16, 10, 16, 10);
            cl->setSpacing(4);
            auto* indicator = new QFrame(card); indicator->setFixedHeight(3);
            indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(s.color));
            auto* valL = new QLabel(s.val, card);
            valL->setStyleSheet(QString("font-size:26px;font-weight:bold;color:%1;background:transparent;").arg(s.color));
            auto* nameL = new QLabel(s.label, page);
            nameL->setStyleSheet("font-size:12px;color:#8c8c8c;background:transparent;");
            cl->addWidget(indicator); cl->addWidget(valL); cl->addWidget(nameL);
            statsLayout->addWidget(card);
        }
        layout->addLayout(statsLayout);

        // Tabs
        auto* tabs = new QTabWidget(page);
        tabs->setStyleSheet("QTabWidget::pane{border:1px solid #f0f0f0;border-radius:4px;background:#fff;} QTabBar::tab{padding:8px 16px;} QTabBar::tab:selected{color:#1677ff;border-bottom:2px solid #1677ff;}");

        // Tab 1: Activity list
        auto* actWidget = new QWidget();
        auto* actLayout = new QVBoxLayout(actWidget);
        actLayout->setContentsMargins(12,12,12,12);

        // Toolbar for activity list
        auto* actToolbar = new QHBoxLayout();
        actToolbar->setSpacing(12);
        auto* actSearchEdit = new QLineEdit();
        actSearchEdit->setPlaceholderText(QStringLiteral("搜索活动标题..."));
        actSearchEdit->setMaximumWidth(260);

        actToolbar->addWidget(actSearchEdit);
        auto* actStatusCombo = new QComboBox();
        actStatusCombo->addItem(QStringLiteral("全部状态"), -1);
        actStatusCombo->addItem(QStringLiteral("草稿"), 0);
        actStatusCombo->addItem(QStringLiteral("招募中"), 1);
        actStatusCombo->addItem(QStringLiteral("进行中"), 2);
        actStatusCombo->addItem(QStringLiteral("已结束"), 3);
        actStatusCombo->addItem(QStringLiteral("已取消"), 4);
        actStatusCombo->setMinimumWidth(120);
        actToolbar->addWidget(actStatusCombo);
        actToolbar->addStretch();
        actLayout->insertLayout(0, actToolbar);

        auto* actTable = new QTableWidget(actWidget);
        actTable->setAlternatingRowColors(true);
        actTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        actTable->setStyleSheet(TABLE_STYLE);
        actTable->setShowGrid(false);
        actTable->verticalHeader()->setVisible(false);
        actTable->setColumnCount(8);
        actTable->setHorizontalHeaderLabels({QStringLiteral("活动标题"), QStringLiteral("类型"), QStringLiteral("地点"), QStringLiteral("开始时间"), QStringLiteral("结束时间"), QStringLiteral("需求/已报"), QStringLiteral("状态"), QStringLiteral("操作")});
        actTable->setColumnWidth(0, 200);
        actTable->setColumnWidth(3, 130);
        actTable->setColumnWidth(4, 130);
        actTable->setColumnWidth(7, 80);
        actTable->horizontalHeader()->setStretchLastSection(false);

        std::function<void()> loadActivities = [actTable, actSearchEdit, actStatusCombo]() {
            while (actTable->rowCount() > 0) actTable->removeRow(0);
            QString sql = "SELECT id, title, activity_type, location, start_time, end_time, need_count, enrolled_count, status FROM sv_volunteer_activity WHERE del_flag = 0";
            QString actSearch = actSearchEdit->text().trimmed();
            int actStatusFilter = actStatusCombo->currentData().toInt();
            if (!actSearch.isEmpty()) sql += " AND title LIKE '%" + actSearch + "%'";
            if (actStatusFilter >= 0) sql += " AND status = " + QString::number(actStatusFilter);
            sql += " ORDER BY start_time DESC";
            QSqlQuery actQ(sql);
            int actRow = 0;
        while (actQ.next()) {
            actTable->insertRow(actRow);
            actTable->setItem(actRow, 0, new QTableWidgetItem(actQ.value(1).toString()));
            actTable->setItem(actRow, 1, new QTableWidgetItem(VolunteerActivityType::label(actQ.value(2).toInt())));
            actTable->setItem(actRow, 2, new QTableWidgetItem(actQ.value(3).toString()));
            actTable->setItem(actRow, 3, new QTableWidgetItem(actQ.value(4).toDateTime().toString("yyyy-MM-dd hh:mm")));
            actTable->setItem(actRow, 4, new QTableWidgetItem(actQ.value(5).toDateTime().toString("yyyy-MM-dd hh:mm")));
            actTable->setItem(actRow, 5, new QTableWidgetItem(QString("%1/%2").arg(actQ.value(6).toInt()).arg(actQ.value(7).toInt())));
            int sts = actQ.value(8).toInt();
            auto* stsItem = new QTableWidgetItem(VolunteerActivityStatus::label(sts));
            stsItem->setTextAlignment(Qt::AlignCenter);
            QString stsColor = sts == 1 ? "#52c41a" : sts == 2 ? "#1677ff" : sts == 3 ? "#8c8c8c" : sts == 4 ? "#ff4d4f" : "#faad14";
            QString stsBg = sts == 1 ? "#f6ffed" : sts == 2 ? "#e6f4ff" : sts == 3 ? "#f5f5f5" : sts == 4 ? "#fff1f0" : "#fffbe6";
            stsItem->setBackground(QColor(stsBg));
            stsItem->setForeground(QColor(stsColor));
            actTable->setItem(actRow, 6, stsItem);

            // Action text item
            QString actActionText = (sts == 1) ? QStringLiteral("报名") : QStringLiteral("查看");
            QString actActionColor = (sts == 1) ? "#1677ff" : "#8c8c8c";
            auto* actActionItem = new QTableWidgetItem(actActionText);
            actActionItem->setTextAlignment(Qt::AlignCenter);
            actActionItem->setForeground(QColor(actActionColor));
            actActionItem->setData(Qt::UserRole, actQ.value(0).toLongLong());
            actActionItem->setData(Qt::UserRole + 1, sts);
            actActionItem->setFont(QFont("Microsoft YaHei", 12));
            actTable->setItem(actRow, 7, actActionItem);
            actRow++;
        }
        };
        loadActivities();
        connect(actTable, &QTableWidget::cellClicked, this, [=](int r, int c) {
            if (c != 7) return;
            auto* item = actTable->item(r, c);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 1).toInt();
            if (sts == 1) {
                const auto& user = AuthService::instance().currentUser();
                DatabaseManager::instance().insert("sv_volunteer_signup", {
                    {"activity_id", id}, {"user_id", user.id}, {"user_name", user.nickname.isEmpty() ? user.username : user.nickname},
                    {"signup_time", QDateTime::currentDateTime()}, {"status", 0}, {"create_by", user.id}
                });
                loadActivities();
            }
        });
        connect(actTable, &QTableWidget::cellEntered, this, [=](int r, int c) {
            actTable->viewport()->setCursor(c == 7 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        });
        connect(actSearchEdit, &QLineEdit::textChanged, this, [=]() { loadActivities(); });
        connect(actStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadActivities(); });
        actLayout->addWidget(actTable);
        tabs->addTab(actWidget, QStringLiteral("活动列表"));

        // Tab 2: My signups
        auto* signupWidget = new QWidget();
        auto* signupLayout = new QVBoxLayout(signupWidget);
        signupLayout->setContentsMargins(12,12,12,12);

        auto* signupTable = new QTableWidget(signupWidget);
        signupTable->setAlternatingRowColors(true);
        signupTable->horizontalHeader()->setStretchLastSection(false);
        signupTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        signupTable->setStyleSheet(TABLE_STYLE);
        signupTable->setShowGrid(false);
        signupTable->verticalHeader()->setVisible(false);
        signupTable->setColumnCount(6);
        signupTable->setHorizontalHeaderLabels({QStringLiteral("活动标题"), QStringLiteral("报名时间"), QStringLiteral("签到时间"), QStringLiteral("签退时间"), QStringLiteral("服务时长"), QStringLiteral("状态")});
        signupTable->setColumnWidth(0, 250);

        QSqlQuery sigQ("SELECT s.id, a.title, s.signup_time, s.checkin_time, s.checkout_time, s.hours, s.status FROM sv_volunteer_signup s LEFT JOIN sv_volunteer_activity a ON s.activity_id = a.id ORDER BY s.signup_time DESC");
        int sigRow = 0;
        while (sigQ.next()) {
            signupTable->insertRow(sigRow);
            signupTable->setItem(sigRow, 0, new QTableWidgetItem(sigQ.value(1).toString()));
            signupTable->setItem(sigRow, 1, new QTableWidgetItem(sigQ.value(2).toDateTime().toString("yyyy-MM-dd hh:mm")));
            signupTable->setItem(sigRow, 2, new QTableWidgetItem(sigQ.value(3).isNull() ? "-" : sigQ.value(3).toDateTime().toString("yyyy-MM-dd hh:mm")));
            signupTable->setItem(sigRow, 3, new QTableWidgetItem(sigQ.value(4).isNull() ? "-" : sigQ.value(4).toDateTime().toString("yyyy-MM-dd hh:mm")));
            double hours = sigQ.value(5).toDouble();
            signupTable->setItem(sigRow, 4, new QTableWidgetItem(hours > 0 ? QString("%1h").arg(hours) : "-"));
            int sigSts = sigQ.value(6).toInt();
            auto* sigStsItem = new QTableWidgetItem(sigSts == 0 ? QStringLiteral("待审核") : sigSts == 1 ? QStringLiteral("已确认") : sigSts == 2 ? QStringLiteral("已签到") : QStringLiteral("已完成"));
            sigStsItem->setTextAlignment(Qt::AlignCenter);
            sigStsItem->setBackground(QColor(sigSts == 1 ? "#f6ffed" : sigSts == 2 ? "#e6f4ff" : sigSts == 3 ? "#f5f5f5" : "#fffbe6"));
            sigStsItem->setForeground(QColor(sigSts == 1 ? "#52c41a" : sigSts == 2 ? "#1677ff" : sigSts == 3 ? "#8c8c8c" : "#faad14"));
            signupTable->setItem(sigRow, 5, sigStsItem);
            sigRow++;
        }
        signupLayout->addWidget(signupTable);
        tabs->addTab(signupWidget, QStringLiteral("我的报名 (%1)").arg(sigRow));

        layout->addWidget(tabs, 1);
        delete table; // Remove the unused default table
        return page;
    } else if (sub == "convenience") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索订单号/标题..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        auto* statusCombo = new QComboBox(toolbar);
        statusCombo->addItem(QStringLiteral("全部状态"), -1);
        statusCombo->addItem(QStringLiteral("待接单"), 0);
        statusCombo->addItem(QStringLiteral("已接单"), 1);
        statusCombo->addItem(QStringLiteral("服务中"), 2);
        statusCombo->addItem(QStringLiteral("已完成"), 3);
        statusCombo->addItem(QStringLiteral("已取消"), 4);
        statusCombo->setMinimumWidth(120);
        tbLayout->addWidget(statusCombo);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({QStringLiteral("订单号"), QStringLiteral("标题"), QStringLiteral("服务类型"), QStringLiteral("状态"), QStringLiteral("预约时间")});
        std::function<void()> loadConvenience = [table, searchEdit, statusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT order_no, title, service_type, status, appointment_time FROM sv_service_order WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int statusFilter = statusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND (order_no LIKE '%" + searchText + "%' OR title LIKE '%" + searchText + "%')";
            if (statusFilter >= 0) sql += " AND status = " + QString::number(statusFilter);
            sql += " ORDER BY create_time DESC";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 2, new QTableWidgetItem(ServiceType::label(q.value(2).toInt())));
                int convSts = q.value(3).toInt();
                auto* convStsItem = new QTableWidgetItem(ServiceOrderStatus::label(convSts));
                convStsItem->setTextAlignment(Qt::AlignCenter);
                if (convSts == 0) {
                    convStsItem->setBackground(QColor(230, 244, 255));
                    convStsItem->setForeground(QColor(22, 119, 255));
                } else if (convSts == 1) {
                    convStsItem->setBackground(QColor(255, 247, 230));
                    convStsItem->setForeground(QColor(250, 140, 22));
                } else if (convSts == 2) {
                    convStsItem->setBackground(QColor(246, 255, 237));
                    convStsItem->setForeground(QColor(82, 196, 26));
                } else if (convSts == 3) {
                    convStsItem->setBackground(QColor(249, 240, 255));
                    convStsItem->setForeground(QColor(114, 46, 209));
                } else {
                    convStsItem->setBackground(QColor(245, 245, 245));
                    convStsItem->setForeground(QColor(140, 140, 140));
                }
                table->setItem(row, 3, convStsItem);
                table->setItem(row, 4, new QTableWidgetItem(q.value(4).toDateTime().toString("yyyy-MM-dd hh:mm")));
                row++;
            }
        };
        loadConvenience();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadConvenience(); });
        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadConvenience(); });
    } else if (sub == "job") {
        // Page header
        auto* header = new QLabel(QStringLiteral("就业服务"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("社区岗位发布、招聘信息和求职服务"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索岗位/公司..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        auto* statusCombo = new QComboBox(toolbar);
        statusCombo->addItem(QStringLiteral("全部状态"), -1);
        statusCombo->addItem(QStringLiteral("招聘中"), 0);
        statusCombo->addItem(QStringLiteral("已截止"), 1);
        statusCombo->addItem(QStringLiteral("已关闭"), 2);
        statusCombo->setMinimumWidth(120);
        tbLayout->addWidget(statusCombo);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({QStringLiteral("岗位名称"), QStringLiteral("公司"), QStringLiteral("薪资"), QStringLiteral("人数"), QStringLiteral("截止日期"), QStringLiteral("状态")});
        table->setColumnWidth(0, 200);
        table->setColumnWidth(1, 150);
        std::function<void()> loadJobs = [table, searchEdit, statusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT title, company, salary_range, headcount, deadline, status FROM sv_job_posting WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int statusFilter = statusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND (title LIKE '%" + searchText + "%' OR company LIKE '%" + searchText + "%')";
            if (statusFilter >= 0) sql += " AND status = " + QString::number(statusFilter);
            sql += " ORDER BY create_time DESC";
            QSqlQuery jobQ(sql);
            int jRow = 0;
            while (jobQ.next()) {
                table->insertRow(jRow);
                table->setItem(jRow, 0, new QTableWidgetItem(jobQ.value(0).toString()));
                table->setItem(jRow, 1, new QTableWidgetItem(jobQ.value(1).toString()));
                table->setItem(jRow, 2, new QTableWidgetItem(jobQ.value(2).toString()));
                table->setItem(jRow, 3, new QTableWidgetItem(jobQ.value(3).toString()));
                table->setItem(jRow, 4, new QTableWidgetItem(jobQ.value(4).toString()));
                int jSts = jobQ.value(5).toInt();
                auto* stsItem = new QTableWidgetItem(jSts == 0 ? QStringLiteral("招聘中") : jSts == 1 ? QStringLiteral("已截止") : QStringLiteral("已关闭"));
                stsItem->setTextAlignment(Qt::AlignCenter);
                stsItem->setBackground(QColor(jSts == 0 ? "#e6f4ff" : jSts == 1 ? "#fff7e6" : "#f0f0f0"));
                stsItem->setForeground(QColor(jSts == 0 ? "#1677ff" : jSts == 1 ? "#fa8c16" : "#595959"));
                table->setItem(jRow, 5, stsItem);
                jRow++;
            }
        };
        loadJobs();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadJobs(); });
        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadJobs(); });
    }

    layout->addWidget(table);
    return page;
}

// ========== Report Pages ==========
QWidget* MainWindow::createReportPage(const QString& sub) {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    auto& db = DatabaseManager::instance();

    if (sub == "workorder") {
        auto* label = new QLabel(QStringLiteral("工单统计分析"), page);
        label->setStyleSheet("font-size: 18px; font-weight: bold; color: #333; margin-bottom: 16px;");
        layout->addWidget(label);

        // Pie chart for work order types
        auto* chart = new QChart();
        chart->setTitle(QStringLiteral("工单类型分布"));

        QSqlQuery q("SELECT order_type, COUNT(*) as cnt FROM wo_work_order WHERE del_flag = 0 GROUP BY order_type");
        auto* series = new QPieSeries();
        while (q.next()) {
            series->append(WorkOrderType::label(q.value(0).toInt()), q.value(1).toInt());
        }
        chart->addSeries(series);
        for (auto* slice : series->slices()) {
            slice->setLabelVisible(true);
            slice->setLabel(QString("%1 %2%").arg(slice->label()).arg(slice->percentage() * 100, 0, 'f', 1));
            slice->setLabelPosition(QPieSlice::LabelOutside);
        }
        connect(series, &QPieSeries::hovered, [](QPieSlice* slice, bool state) {
            if (state) {
                slice->setExploded(true);
                QToolTip::showText(QCursor::pos(),
                    QString("%1: %2 (%3%)").arg(slice->label()).arg(slice->value()).arg(slice->percentage() * 100, 0, 'f', 1));
            } else {
                slice->setExploded(false);
                QToolTip::hideText();
            }
        });
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);

        auto* chartView = new QChartView(chart, page);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMinimumHeight(300);
        layout->addWidget(chartView);

    } else if (sub == "event") {
        auto* label = new QLabel(QStringLiteral("事件统计分析"), page);
        label->setStyleSheet("font-size: 18px; font-weight: bold; color: #333; margin-bottom: 16px;");
        layout->addWidget(label);

        auto* chart = new QChart();
        chart->setTitle(QStringLiteral("事件类别分布"));

        QSqlQuery q("SELECT event_category, COUNT(*) as cnt FROM ge_event WHERE del_flag = 0 GROUP BY event_category");
        auto* series = new QPieSeries();
        while (q.next()) {
            series->append(EventCategory::label(q.value(0).toInt()), q.value(1).toInt());
        }
        chart->addSeries(series);
        for (auto* slice : series->slices()) {
            slice->setLabelVisible(true);
            slice->setLabel(QString("%1 %2%").arg(slice->label()).arg(slice->percentage() * 100, 0, 'f', 1));
            slice->setLabelPosition(QPieSlice::LabelOutside);
        }
        connect(series, &QPieSeries::hovered, [](QPieSlice* slice, bool state) {
            if (state) {
                slice->setExploded(true);
                QToolTip::showText(QCursor::pos(),
                    QString("%1: %2 (%3%)").arg(slice->label()).arg(slice->value()).arg(slice->percentage() * 100, 0, 'f', 1));
            } else {
                slice->setExploded(false);
                QToolTip::hideText();
            }
        });
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);

        auto* chartView = new QChartView(chart, page);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMinimumHeight(300);
        layout->addWidget(chartView);

    } else if (sub == "service") {
        auto* label = new QLabel(QStringLiteral("服务统计分析"), page);
        label->setStyleSheet("font-size: 18px; font-weight: bold; color: #333; margin-bottom: 16px;");
        layout->addWidget(label);

        auto* chart = new QChart();
        chart->setTitle(QStringLiteral("服务订单状态分布"));

        QSqlQuery q("SELECT status, COUNT(*) as cnt FROM sv_service_order WHERE del_flag = 0 GROUP BY status");
        auto* series = new QPieSeries();
        while (q.next()) {
            series->append(ServiceOrderStatus::label(q.value(0).toInt()), q.value(1).toInt());
        }
        chart->addSeries(series);
        for (auto* slice : series->slices()) {
            slice->setLabelVisible(true);
            slice->setLabel(QString("%1 %2%").arg(slice->label()).arg(slice->percentage() * 100, 0, 'f', 1));
            slice->setLabelPosition(QPieSlice::LabelOutside);
        }
        connect(series, &QPieSeries::hovered, [](QPieSlice* slice, bool state) {
            if (state) {
                slice->setExploded(true);
                QToolTip::showText(QCursor::pos(),
                    QString("%1: %2 (%3%)").arg(slice->label()).arg(slice->value()).arg(slice->percentage() * 100, 0, 'f', 1));
            } else {
                slice->setExploded(false);
                QToolTip::hideText();
            }
        });
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);

        auto* chartView = new QChartView(chart, page);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMinimumHeight(300);
        layout->addWidget(chartView);

    } else if (sub == "dashboard") {
        // Enhanced dashboard with stats + multiple charts
        auto* scrollArea = new QScrollArea(page);
        scrollArea->setWidgetResizable(true);
        scrollArea->setStyleSheet("border:none;background:transparent;");
        auto* scrollContent = new QWidget(scrollArea);
        auto* mainLay = new QVBoxLayout(scrollContent);
        mainLay->setContentsMargins(0, 0, 0, 0);

        auto* titleLabel = new QLabel(QStringLiteral("综合数据看板"), scrollContent);
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px;");
        mainLay->addWidget(titleLabel);
        auto* descLabel = new QLabel(QStringLiteral("多维度数据可视化，实时监控社区运营状态"), scrollContent);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 16px;");
        mainLay->addWidget(descLabel);

        // Summary stat cards
        auto* statsRow = new QHBoxLayout();
        statsRow->setSpacing(12);
        auto mkCard = [](const QString& title, const QString& val, const QString& color, QWidget* parent) {
            auto* card = new QFrame(parent);
            card->setFixedHeight(90);
            card->setStyleSheet(QString("QFrame{background:#fff;border-radius:8px;border:1px solid #f0f0f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
            auto* cl = new QVBoxLayout(card);
            cl->setContentsMargins(16, 12, 16, 12); cl->setSpacing(4);
            auto* indicator = new QFrame(card); indicator->setFixedHeight(3);
            indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
            auto* tl = new QLabel(title); tl->setStyleSheet("color:#8c8c8c;font-size:12px;");
            auto* vl = new QLabel(val); vl->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(color));
            cl->addWidget(indicator); cl->addWidget(tl); cl->addWidget(vl);
            return card;
        };

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

        statsRow->addWidget(mkCard(QStringLiteral("工单总数"), QString::number(woTotal), "#1677ff", scrollContent));
        statsRow->addWidget(mkCard(QStringLiteral("待处理工单"), QString::number(woPend), "#fa8c16", scrollContent));
        statsRow->addWidget(mkCard(QStringLiteral("事件总数"), QString::number(evTotal), "#52c41a", scrollContent));
        statsRow->addWidget(mkCard(QStringLiteral("待审核事件"), QString::number(evPend), "#ff4d4f", scrollContent));
        statsRow->addWidget(mkCard(QStringLiteral("服务订单"), QString::number(svcTotal), "#722ed1", scrollContent));
        statsRow->addWidget(mkCard(QStringLiteral("公告数量"), QString::number(annTotal), "#13c2c2", scrollContent));
        mainLay->addLayout(statsRow);
        mainLay->addSpacing(16);

        // Row 1: Work order trend bar chart + Work order status pie
        auto* chartRow1 = new QHBoxLayout();
        chartRow1->setSpacing(16);

        // Bar chart: 7-day trend
        auto* trendChart = new QChart();
        trendChart->setTitle(QStringLiteral("近7天工单趋势"));
        trendChart->setBackgroundVisible(false);
        QSqlQuery trendQ("SELECT DATE(create_time) as dt, COUNT(*) as cnt FROM wo_work_order WHERE del_flag = 0 AND create_time >= DATE('now', '-7 days') GROUP BY dt ORDER BY dt");
        auto* trendSeries = new QBarSeries();
        auto* trendSet = new QBarSet(QStringLiteral("工单数"));
        trendSet->setColor(QColor("#1677ff"));
        QStringList trendCats;
        while (trendQ.next()) {
            trendCats << trendQ.value(0).toString().mid(5);
            trendSet->append(trendQ.value(1).toInt());
        }
        trendSeries->append(trendSet);
        trendChart->addSeries(trendSeries);
        if (!trendCats.isEmpty()) {
            auto* tAxisX = new QCategoryAxis();
            for (int i = 0; i < trendCats.size(); ++i) tAxisX->append(trendCats.at(i), i + 1);
            trendChart->addAxis(tAxisX, Qt::AlignBottom);
            trendSeries->attachAxis(tAxisX);
        }
        auto* trendView = new QChartView(trendChart, scrollContent);
        trendView->setRenderHint(QPainter::Antialiasing);
        trendView->setMinimumHeight(280);
        trendView->setStyleSheet("QChartView{background:#fff;border-radius:8px;border:1px solid #f0f0f0;}");
        chartRow1->addWidget(trendView);

        // Pie chart: work order status
        auto* woStsChart = new QChart();
        woStsChart->setTitle(QStringLiteral("工单状态分布"));
        woStsChart->setBackgroundVisible(false);
        QSqlQuery woStsQ("SELECT status, COUNT(*) FROM wo_work_order WHERE del_flag = 0 GROUP BY status");
        auto* woStsSeries = new QPieSeries();
        while (woStsQ.next()) {
            woStsSeries->append(WorkOrderStatus::label(woStsQ.value(0).toInt()), woStsQ.value(1).toInt());
        }
        woStsChart->addSeries(woStsSeries);
        for (auto* slice : woStsSeries->slices()) {
            slice->setLabelVisible(true);
            slice->setLabel(QString("%1 %2%").arg(slice->label()).arg(slice->percentage() * 100, 0, 'f', 1));
            slice->setLabelPosition(QPieSlice::LabelOutside);
        }
        connect(woStsSeries, &QPieSeries::hovered, [](QPieSlice* slice, bool state) {
            if (state) {
                slice->setExploded(true);
                QToolTip::showText(QCursor::pos(),
                    QString("%1: %2 (%3%)").arg(slice->label()).arg(slice->value()).arg(slice->percentage() * 100, 0, 'f', 1));
            } else {
                slice->setExploded(false);
                QToolTip::hideText();
            }
        });
        woStsChart->setAnimationOptions(QChart::SeriesAnimations);
        woStsChart->legend()->setVisible(true);
        woStsChart->legend()->setAlignment(Qt::AlignRight);
        auto* woStsView = new QChartView(woStsChart, scrollContent);
        woStsView->setRenderHint(QPainter::Antialiasing);
        woStsView->setMinimumHeight(280);
        woStsView->setStyleSheet("QChartView{background:#fff;border-radius:8px;border:1px solid #f0f0f0;}");
        chartRow1->addWidget(woStsView);
        mainLay->addLayout(chartRow1);
        mainLay->addSpacing(16);

        // Row 2: Event category pie + Service status pie
        auto* chartRow2 = new QHBoxLayout();
        chartRow2->setSpacing(16);

        auto* evCatChart = new QChart();
        evCatChart->setTitle(QStringLiteral("事件类别分布"));
        evCatChart->setBackgroundVisible(false);
        QSqlQuery evCatQ("SELECT event_category, COUNT(*) FROM ge_event WHERE del_flag = 0 GROUP BY event_category");
        auto* evCatSeries = new QPieSeries();
        while (evCatQ.next()) {
            evCatSeries->append(EventCategory::label(evCatQ.value(0).toInt()), evCatQ.value(1).toInt());
        }
        evCatChart->addSeries(evCatSeries);
        for (auto* slice : evCatSeries->slices()) {
            slice->setLabelVisible(true);
            slice->setLabel(QString("%1 %2%").arg(slice->label()).arg(slice->percentage() * 100, 0, 'f', 1));
            slice->setLabelPosition(QPieSlice::LabelOutside);
        }
        connect(evCatSeries, &QPieSeries::hovered, [](QPieSlice* slice, bool state) {
            if (state) {
                slice->setExploded(true);
                QToolTip::showText(QCursor::pos(),
                    QString("%1: %2 (%3%)").arg(slice->label()).arg(slice->value()).arg(slice->percentage() * 100, 0, 'f', 1));
            } else {
                slice->setExploded(false);
                QToolTip::hideText();
            }
        });
        evCatChart->setAnimationOptions(QChart::SeriesAnimations);
        evCatChart->legend()->setVisible(true);
        evCatChart->legend()->setAlignment(Qt::AlignRight);
        auto* evCatView = new QChartView(evCatChart, scrollContent);
        evCatView->setRenderHint(QPainter::Antialiasing);
        evCatView->setMinimumHeight(280);
        evCatView->setStyleSheet("QChartView{background:#fff;border-radius:8px;border:1px solid #f0f0f0;}");
        chartRow2->addWidget(evCatView);

        auto* svcStsChart = new QChart();
        svcStsChart->setTitle(QStringLiteral("服务订单状态"));
        svcStsChart->setBackgroundVisible(false);
        QSqlQuery svcStsQ("SELECT status, COUNT(*) FROM sv_service_order WHERE del_flag = 0 GROUP BY status");
        auto* svcStsSeries = new QPieSeries();
        while (svcStsQ.next()) {
            svcStsSeries->append(ServiceOrderStatus::label(svcStsQ.value(0).toInt()), svcStsQ.value(1).toInt());
        }
        svcStsChart->addSeries(svcStsSeries);
        for (auto* slice : svcStsSeries->slices()) {
            slice->setLabelVisible(true);
            slice->setLabel(QString("%1 %2%").arg(slice->label()).arg(slice->percentage() * 100, 0, 'f', 1));
            slice->setLabelPosition(QPieSlice::LabelOutside);
        }
        connect(svcStsSeries, &QPieSeries::hovered, [](QPieSlice* slice, bool state) {
            if (state) {
                slice->setExploded(true);
                QToolTip::showText(QCursor::pos(),
                    QString("%1: %2 (%3%)").arg(slice->label()).arg(slice->value()).arg(slice->percentage() * 100, 0, 'f', 1));
            } else {
                slice->setExploded(false);
                QToolTip::hideText();
            }
        });
        svcStsChart->setAnimationOptions(QChart::SeriesAnimations);
        svcStsChart->legend()->setVisible(true);
        svcStsChart->legend()->setAlignment(Qt::AlignRight);
        auto* svcStsView = new QChartView(svcStsChart, scrollContent);
        svcStsView->setRenderHint(QPainter::Antialiasing);
        svcStsView->setMinimumHeight(280);
        svcStsView->setStyleSheet("QChartView{background:#fff;border-radius:8px;border:1px solid #f0f0f0;}");
        chartRow2->addWidget(svcStsView);
        mainLay->addLayout(chartRow2);

        scrollArea->setWidget(scrollContent);
        layout->addWidget(scrollArea);
    }

    layout->addStretch();
    return page;
}

// ========== System Pages ==========
QWidget* MainWindow::createSystemPage(const QString& sub) {
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    auto& db = DatabaseManager::instance();

    if (sub == "user") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索用户名/姓名..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        auto* statusCombo = new QComboBox(toolbar);
        statusCombo->addItem(QStringLiteral("全部状态"), -1);
        statusCombo->addItem(QStringLiteral("正常"), 1);
        statusCombo->addItem(QStringLiteral("禁用"), 0);
        statusCombo->setMinimumWidth(120);
        tbLayout->addWidget(statusCombo);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({QStringLiteral("用户名"), QStringLiteral("真实姓名"), QStringLiteral("手机号"), QStringLiteral("类型"), QStringLiteral("状态")});
        std::function<void()> loadUsers = [table, searchEdit, statusCombo]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT username, real_name, phone, user_type, status FROM sys_user WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            int statusFilter = statusCombo->currentData().toInt();
            if (!searchText.isEmpty()) sql += " AND (username LIKE '%" + searchText + "%' OR real_name LIKE '%" + searchText + "%')";
            if (statusFilter >= 0) sql += " AND status = " + QString::number(statusFilter);
            sql += " ORDER BY id";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 2, new QTableWidgetItem(Utils::maskPhone(q.value(2).toString())));
                table->setItem(row, 3, new QTableWidgetItem(UserType::label(q.value(3).toInt())));
                int userSts = q.value(4).toInt();
                auto* statusItem = new QTableWidgetItem(UserStatus::label(userSts));
                statusItem->setTextAlignment(Qt::AlignCenter);
                if (userSts == 1) {
                    statusItem->setBackground(QColor(246, 255, 237));
                    statusItem->setForeground(QColor(82, 196, 26));
                } else {
                    statusItem->setBackground(QColor(255, 242, 240));
                    statusItem->setForeground(QColor(255, 77, 79));
                }
                table->setItem(row, 4, statusItem);
                row++;
            }
        };
        loadUsers();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadUsers(); });
        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() { loadUsers(); });
    } else if (sub == "role") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索角色名称/标识..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({QStringLiteral("角色名称"), QStringLiteral("标识"), QStringLiteral("域"), QStringLiteral("数据权限")});
        std::function<void()> loadRoles = [table, searchEdit]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT role_name, role_key, role_domain, data_scope FROM sys_role WHERE del_flag = 0";
            QString searchText = searchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND (role_name LIKE '%" + searchText + "%' OR role_key LIKE '%" + searchText + "%')";
            sql += " ORDER BY sort_order";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 2, new QTableWidgetItem(RoleDomain::label(q.value(2).toString())));
                table->setItem(row, 3, new QTableWidgetItem(DataScope::label(q.value(3).toInt())));
                row++;
            }
        };
        loadRoles();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadRoles(); });
    } else if (sub == "menu") {
        auto* tree = new QTreeWidget(page);
        tree->setHeaderLabels({QStringLiteral("菜单名称"), QStringLiteral("类型"), QStringLiteral("路径"), QStringLiteral("权限标识")});
        tree->setColumnWidth(0, 200);
        tree->setColumnWidth(1, 80);
        tree->setColumnWidth(2, 200);

        QSqlQuery q("SELECT * FROM sys_menu WHERE status = 0 ORDER BY parent_id, sort_order");
        QMap<qint64, QTreeWidgetItem*> items;
        while (q.next()) {
            qint64 id = q.value("id").toLongLong();
            qint64 pid = q.value("parent_id").toLongLong();
            auto* item = new QTreeWidgetItem();
            item->setText(0, q.value("menu_name").toString());
            item->setText(1, q.value("menu_type").toInt() == 1 ? QStringLiteral("目录") : QStringLiteral("菜单"));
            item->setText(2, q.value("path").toString());
            item->setText(3, q.value("permission").toString());
            items[id] = item;
            if (pid == 0) {
                tree->addTopLevelItem(item);
            } else if (items.contains(pid)) {
                items[pid]->addChild(item);
            }
        }
        layout->addWidget(tree);
        return page;
    } else if (sub == "dict") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索字典类型/标签..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(3);
        table->setHorizontalHeaderLabels({QStringLiteral("字典类型"), QStringLiteral("标签"), QStringLiteral("值")});
        std::function<void()> loadDicts = [table, searchEdit]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT dt.dict_type, dd.dict_label, dd.dict_value FROM sys_dict_data dd JOIN sys_dict_type dt ON dd.dict_type = dt.dict_type WHERE dt.status = 0 AND dd.status = 0";
            QString searchText = searchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND (dt.dict_type LIKE '%" + searchText + "%' OR dd.dict_label LIKE '%" + searchText + "%')";
            sql += " ORDER BY dt.dict_type, dd.sort_order";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
                row++;
            }
        };
        loadDicts();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadDicts(); });
    } else if (sub == "log") {
        // Toolbar
        auto* toolbar = new QWidget(page);
        toolbar->setObjectName("filterToolbar"); setStyleSheet("#filterToolbar{background:transparent;}");
        auto* tbLayout = new QHBoxLayout(toolbar);
        tbLayout->setContentsMargins(0, 0, 0, 8);
        tbLayout->setSpacing(10);
        auto* searchEdit = new QLineEdit(toolbar);
        searchEdit->setPlaceholderText(QStringLiteral("搜索用户/模块/操作..."));
        searchEdit->setMinimumWidth(200);
        searchEdit->setClearButtonEnabled(true);
        tbLayout->addWidget(searchEdit);
        tbLayout->addStretch();
        layout->addWidget(toolbar);

        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({QStringLiteral("用户"), QStringLiteral("模块"), QStringLiteral("操作"), QStringLiteral("IP"), QStringLiteral("时间")});
        std::function<void()> loadLogs = [table, searchEdit]() {
            while (table->rowCount() > 0) table->removeRow(0);
            QString sql = "SELECT username, module, operation, ip, operation_time FROM sys_operation_log WHERE 1=1";
            QString searchText = searchEdit->text().trimmed();
            if (!searchText.isEmpty()) sql += " AND (username LIKE '%" + searchText + "%' OR module LIKE '%" + searchText + "%' OR operation LIKE '%" + searchText + "%')";
            sql += " ORDER BY operation_time DESC LIMIT 100";
            QSqlQuery q(sql);
            int row = 0;
            while (q.next()) {
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
                table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
                table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
                table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
                table->setItem(row, 4, new QTableWidgetItem(q.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
                row++;
            }
        };
        loadLogs();
        connect(searchEdit, &QLineEdit::textChanged, this, [=]() { loadLogs(); });
    } else if (sub == "ai") {
        // AI智能问答 - 交互式聊天界面
        layout->removeWidget(table);
        delete table;

        auto* header = new QLabel(QStringLiteral("智能问答助手"), page);
        header->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f1f1f; margin-bottom: 4px; background: transparent;");
        layout->addWidget(header);
        auto* descLabel = new QLabel(QStringLiteral("输入您的问题，智能助手将为您提供解答"), page);
        descLabel->setStyleSheet("font-size: 13px; color: #8c8c8c; margin-bottom: 12px; background: transparent;");
        layout->addWidget(descLabel);

        // 聊天显示区域
        auto* chatDisplay = new QTextEdit(page);
        chatDisplay->setReadOnly(true);
        chatDisplay->setStyleSheet(R"(
            QTextEdit {
                background: #f5f5f5; border: 1px solid #e8e8e8; border-radius: 8px;
                padding: 12px; font-size: 14px; color: #333;
            }
        )");
        {
            QString initHtml = QString::fromUtf8(
                "<div style='margin-bottom:12px;'>"
                "<div style='background:#1677ff;color:#fff;padding:10px 14px;border-radius:8px;display:inline-block;max-width:80%;'>"
                "\xe6\x82\xa8\xe5\xa5\xbd\xef\xbc\x81\xe6\x88\x91\xe6\x98\xaf\xe6\x99\xba\xe6\x85\xa7\xe7\xa4\xbe\xe5\x8c\xba\xe6\x99\xba\xe8\x83\xbd\xe5\x8a\xa9\xe6\x89\x8b\xef\xbc\x8c\xe6\x9c\x89\xe4\xbb\x80\xe4\xb9\x88\xe5\x8f\xaf\xe4\xbb\xa5\xe5\xb8\xae\xe6\x82\xa8\xe7\x9a\x84\xe5\x90\x97\xef\xbc\x9f</div></div>"
                "<div style='color:#8c8c8c;font-size:12px;'>\xe5\xb0\x9d\xe8\xaf\x95\xe8\xbe\x93\xe5\x85\xa5\xef\xbc\x9a\xe6\x80\x8e\xe4\xb9\x88\xe6\x8a\xa5\xe4\xbf\xae\xe3\x80\x81\xe7\x89\xa9\xe4\xb8\x9a\xe8\xb4\xb9\xe6\x80\x8e\xe4\xb9\x88\xe4\xba\xa4\xe3\x80\x81\xe5\x81\x9c\xe8\xbd\xa6\xe6\x9c\x88\xe5\x8d\xa1\xe3\x80\x81\xe5\xbf\x97\xe6\x84\xbf\xe8\x80\x85\xe3\x80\x81\xe7\xa4\xbe\xe5\x8c\xba\xe5\x9c\xa8\xe5\x93\xaa\xe9\x87\x8c\xe3\x80\x81\xe5\x8a\x9e\xe5\xb1\x85\xe4\xbd\x8f\xe8\xaf\x81</div>"
            );
            chatDisplay->setHtml(initHtml);
        }
        layout->addWidget(chatDisplay, 1);

        // 输入区域
        auto* inputLayout = new QHBoxLayout();
        auto* inputEdit = new QLineEdit(page);
        inputEdit->setPlaceholderText(QStringLiteral("请输入您的问题..."));
        inputEdit->setStyleSheet("QLineEdit { padding: 10px 14px; border: 1px solid #d9d9d9; border-radius: 8px; font-size: 14px; }"
                                 "QLineEdit:focus { border-color: #1677ff; }");
        auto* sendBtn = new QPushButton(QStringLiteral("发送"), page);
        sendBtn->setProperty("cssClass", "primary");
        sendBtn->setFixedSize(80, 40);
        sendBtn->setCursor(Qt::PointingHandCursor);
        inputLayout->addWidget(inputEdit, 1);
        inputLayout->addWidget(sendBtn);
        layout->addLayout(inputLayout);

        // 快捷问题按钮
        auto* quickLayout = new QHBoxLayout();
        quickLayout->setSpacing(8);
        auto addQuickQ = [&](const QString& text) {
            auto* btn = new QPushButton(text, page);
            btn->setStyleSheet("QPushButton{background:#f0f5ff;color:#1677ff;border:1px solid #adc6ff;border-radius:16px;padding:4px 12px;font-size:12px;}"
                               "QPushButton:hover{background:#1677ff;color:#fff;}");
            btn->setCursor(Qt::PointingHandCursor);
            connect(btn, &QPushButton::clicked, this, [inputEdit, text]() { inputEdit->setText(text); });
            quickLayout->addWidget(btn);
        };
        addQuickQ(QStringLiteral("怎么报修？"));
        addQuickQ(QStringLiteral("物业费怎么交？"));
        addQuickQ(QStringLiteral("怎么办停车月卡？"));
        addQuickQ(QStringLiteral("怎么成为志愿者？"));
        addQuickQ(QStringLiteral("社区在哪里？"));
        quickLayout->addStretch();
        layout->addLayout(quickLayout);

        // 发送逻辑
        auto sendMessage = [=]() {
            QString question = inputEdit->text().trimmed();
            if (question.isEmpty()) return;

            // 显示用户消息
            chatDisplay->append(QString("<div style='text-align:right;margin:8px 0;'><div style='background:#1677ff;color:#fff;padding:8px 12px;border-radius:8px;display:inline-block;max-width:80%;'>%1</div></div>").arg(question));

            // 搜索知识库
            QSqlQuery searchQ(DatabaseManager::instance().database());
            searchQ.prepare("SELECT id, answer FROM ai_knowledge WHERE status = 0 AND (question LIKE :q1 OR keywords LIKE :q2) ORDER BY hit_count DESC LIMIT 1");
            searchQ.bindValue(":q1", "%" + question + "%");
            searchQ.bindValue(":q2", "%" + question + "%");

            QString answer;
            if (searchQ.exec() && searchQ.next()) {
                answer = searchQ.value(1).toString();
                // 更新命中次数
                qint64 kid = searchQ.value(0).toLongLong();
                QSqlQuery updQ(DatabaseManager::instance().database());
                updQ.prepare("UPDATE ai_knowledge SET hit_count = hit_count + 1 WHERE id = :id");
                updQ.bindValue(":id", kid);
                updQ.exec();
            } else {
                // 尝试模糊匹配关键词
                QStringList keywords = {QStringLiteral("报修"), QStringLiteral("物业"), QStringLiteral("停车"), QStringLiteral("志愿"), QStringLiteral("社区"), QStringLiteral("证件"), QStringLiteral("居住")};
                for (const auto& kw : keywords) {
                    if (question.contains(kw)) {
                        QSqlQuery fuzzyQ(DatabaseManager::instance().database());
                        fuzzyQ.prepare("SELECT answer FROM ai_knowledge WHERE status = 0 AND keywords LIKE :kw ORDER BY hit_count DESC LIMIT 1");
                        fuzzyQ.bindValue(":kw", "%" + kw + "%");
                        if (fuzzyQ.exec() && fuzzyQ.next()) {
                            answer = fuzzyQ.value(0).toString();
                            break;
                        }
                    }
                }
                if (answer.isEmpty()) {
                    answer = QStringLiteral("抱歉，我暂时无法回答这个问题。您可以尝试询问：报修流程、物业费缴纳、停车月卡办理、志愿者注册、社区地址等问题。");
                }
            }

            chatDisplay->append(QString("<div style='margin:8px 0;'><div style='background:#fff;border:1px solid #e8e8e8;padding:8px 12px;border-radius:8px;display:inline-block;max-width:80%;'>%1</div></div>").arg(answer));
            inputEdit->clear();
        };

        connect(sendBtn, &QPushButton::clicked, page, sendMessage);
        connect(inputEdit, &QLineEdit::returnPressed, page, sendMessage);
        return page;
    }

    layout->addWidget(table);
    return page;
}
