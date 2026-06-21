#include "NavigationRail.h"
#include "PagesCommon.h"
#include "services/AuthService.h"

NavigationRail::NavigationRail(QWidget *parent) : QWidget(parent)
{
    m_moduleIcons = {
        {QStringLiteral("工作台"), QStringLiteral("ic_dashboard")},
        {QStringLiteral("基础档案"), QStringLiteral("ic_layers")},
        {QStringLiteral("小区管理"), QStringLiteral("ic_home")},
        {QStringLiteral("社区治理"), QStringLiteral("ic_route")},
        {QStringLiteral("社区服务"), QStringLiteral("ic_heart")},
        {QStringLiteral("统计分析"), QStringLiteral("ic_chart")},
        {QStringLiteral("系统管理"), QStringLiteral("ic_settings")},
    };

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ========== Icon Rail ==========
    m_iconRail = new QWidget(this);
    m_iconRail->setFixedWidth(64);
    m_iconRail->setStyleSheet(QStringLiteral(
        "QWidget { background: #0A0A0A; }"
        "QToolButton {"
        "  background: transparent; border: none; border-radius: 0;"
        "  padding: 0; margin: 0;"
        "}"
        "QToolButton:hover { background: #1A1A1A; }"
        "QToolButton:checked { background: #141413; border-left: 3px solid #B45309; }"
        "QLabel { background: transparent; color: #FAF9F6; }"
    ));
    auto *railLayout = new QVBoxLayout(m_iconRail);
    railLayout->setContentsMargins(0, 0, 0, 0);
    railLayout->setSpacing(0);

    auto *logoWidget = new QWidget(m_iconRail);
    logoWidget->setFixedHeight(56);
    logoWidget->setStyleSheet("background: #0A0A0A; border-bottom: 1px solid #1A1A1A;");
    auto *logoLayout = new QVBoxLayout(logoWidget);
    logoLayout->setContentsMargins(0, 0, 0, 0);
    logoLayout->setAlignment(Qt::AlignCenter);
    auto *logoIcon = new QLabel(logoWidget);
    logoIcon->setPixmap(UiKit::tintSvgIcon(QStringLiteral("app"), QStringLiteral("#FAF9F6"), QSize(24, 24)));
    logoIcon->setAlignment(Qt::AlignCenter);
    logoLayout->addWidget(logoIcon);
    railLayout->addWidget(logoWidget);

    auto *railScroll = new QScrollArea(m_iconRail);
    railScroll->setWidgetResizable(true);
    railScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    railScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    railScroll->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 0; }");
    auto *railContent = new QWidget(railScroll);
    railContent->setObjectName(QStringLiteral("iconRailContent"));
    auto *railContentLayout = new QVBoxLayout(railContent);
    railContentLayout->setContentsMargins(0, 8, 0, 8);
    railContentLayout->setSpacing(4);
    railScroll->setWidget(railContent);
    railLayout->addWidget(railScroll, 1);

    mainLayout->addWidget(m_iconRail);

    // ========== Section Panel ==========
    m_sectionPanel = new QWidget(this);
    m_sectionPanel->setFixedWidth(220);
    m_sectionPanel->setStyleSheet(QStringLiteral(
        "QWidget { background: #141413; }"
        "QLabel { background: transparent; color: #FAF9F6; }"
        "QPushButton {"
        "  background: transparent; color: rgba(250, 249, 246, 0.55);"
        "  border: none; border-left: 3px solid transparent;"
        "  padding: 12px 20px; text-align: left;"
        "  font-size: 13px; min-height: 38px; font-weight: 400;"
        "}"
        "QPushButton:hover { color: #FAF9F6; background: rgba(250, 249, 246, 0.04); }"
        "QPushButton[active=\"true\"] {"
        "  color: #FAF9F6; background: rgba(180, 83, 9, 0.12);"
        "  border-left: 3px solid #B45309; font-weight: 600;"
        "}"
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { background: rgba(250, 249, 246, 0.15); border-radius: 0; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    ));

    auto *sectionLayout = new QVBoxLayout(m_sectionPanel);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    sectionLayout->setSpacing(0);

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
}

void NavigationRail::build(const QList<SysMenu> &menus,
                           const QMap<QString, QList<QPair<QString, QString>>> &modulePages)
{
    m_modulePages = modulePages;
    buildIconRail(menus);
}

void NavigationRail::buildIconRail(const QList<SysMenu> &menus)
{
    auto *railContent = m_iconRail->findChild<QWidget *>(QStringLiteral("iconRailContent"));
    if (!railContent)
        return;

    QLayoutItem *child;
    while ((child = railContent->layout()->takeAt(0)) != nullptr)
    {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    m_iconRailItems.clear();
    m_activeIconRailItem = nullptr;

    auto *railLayout = qobject_cast<QVBoxLayout *>(railContent->layout());

    for (const auto &menu : menus)
    {
        QString moduleName = menu.menuName;
        QString iconKey = m_moduleIcons.value(moduleName, QStringLiteral("ic_grid"));
        QString firstPage = firstPageKeyForModule(moduleName);
        if (menu.id == 1)
            firstPage = QStringLiteral("1");
        if (firstPage.isEmpty())
            firstPage = QString::number(menu.id);

        auto *btn = new QToolButton(railContent);
        btn->setFixedSize(64, 56);
        btn->setAutoRaise(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(moduleName);
        btn->setIcon(QIcon(UiKit::tintSvgIcon(iconKey, QStringLiteral("#9A9A9A"), QSize(22, 22))));
        btn->setIconSize(QSize(22, 22));
        btn->setCheckable(true);

        connect(btn, &QToolButton::clicked, this, [this, moduleName, firstPage, btn]()
                {
                    setCurrentModule(moduleName);
                    emit moduleClicked(moduleName, firstPage);
                });

        btn->setProperty("iconKey", iconKey);
        btn->setProperty("moduleName", moduleName);
        btn->setProperty("isIconRailItem", true);
        btn->installEventFilter(this);
        m_iconRailItems[moduleName] = btn;
        railLayout->addWidget(btn);
    }

    railLayout->addStretch();
}

QString NavigationRail::firstPageKeyForModule(const QString &moduleName) const
{
    auto it = m_modulePages.constFind(moduleName);
    if (it != m_modulePages.constEnd() && !it.value().isEmpty())
        return it.value().first().first;
    return QString();
}

void NavigationRail::setCurrentModule(const QString &moduleKey)
{
    if (moduleKey.isEmpty())
        return;

    bool changed = (moduleKey != m_currentModule);
    m_currentModule = moduleKey;

    // 同步 Icon Rail 高亮
    auto *railBtn = m_iconRailItems.value(moduleKey, nullptr);
    if (railBtn && m_activeIconRailItem != railBtn)
    {
        if (m_activeIconRailItem)
            setIconHighlight(m_activeIconRailItem, false);
        setIconHighlight(railBtn, true);
        m_activeIconRailItem = railBtn;
    }

    // 更新 Section Panel 标题与内容
    if (m_sectionTitle)
        m_sectionTitle->setText(moduleKey);
    buildSectionPanel();

    // 内容区淡入动画
    if (m_sectionLayout && m_sectionLayout->parentWidget())
        UiKit::fadeInWidget(m_sectionLayout->parentWidget(), 150);
}

void NavigationRail::buildSectionPanel()
{
    if (!m_sectionLayout)
        return;

    QLayoutItem *child;
    while ((child = m_sectionLayout->takeAt(0)) != nullptr)
    {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    m_sectionItems.clear();
    m_activeSectionItem = nullptr;

    auto entries = m_modulePages.value(m_currentModule);
    for (const auto &entry : entries)
    {
        QWidget *item = createSectionItem(entry.second, entry.first);
        m_sectionLayout->addWidget(item);
        m_sectionItems[entry.first] = item;
    }

    m_sectionLayout->addStretch();
}

QWidget *NavigationRail::createSectionItem(const QString &text, const QString &key)
{
    auto *btn = new QPushButton(text, m_sectionPanel);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setProperty("key", key);
    btn->setProperty("active", false);

    connect(btn, &QPushButton::clicked, this, [this, key, btn]()
            {
                if (m_activeSectionItem && m_activeSectionItem != btn)
                {
                    m_activeSectionItem->setProperty("active", false);
                    m_activeSectionItem->style()->unpolish(m_activeSectionItem);
                    m_activeSectionItem->style()->polish(m_activeSectionItem);
                }
                btn->setProperty("active", true);
                btn->style()->unpolish(btn);
                btn->style()->polish(btn);
                m_activeSectionItem = btn;
                emit pageClicked(key);
            });

    return btn;
}

void NavigationRail::setActivePage(const QString &pageKey)
{
    auto *w = m_sectionItems.value(pageKey, nullptr);
    auto *btn = qobject_cast<QPushButton *>(w);
    if (!btn)
        return;

    if (m_activeSectionItem && m_activeSectionItem != btn)
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

QString NavigationRail::currentModule() const
{
    return m_currentModule;
}

void NavigationRail::setIconHighlight(QToolButton *btn, bool active)
{
    if (!btn)
        return;
    QString iconKey = btn->property("iconKey").toString();
    if (iconKey.isEmpty())
        return;
    btn->setChecked(active);
    QString color = active ? QStringLiteral("#B45309") : QStringLiteral("#9A9A9A");
    btn->setIcon(QIcon(UiKit::tintSvgIcon(iconKey, color, QSize(22, 22))));
}

bool NavigationRail::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Enter || event->type() == QEvent::Leave)
    {
        auto *btn = qobject_cast<QToolButton *>(watched);
        if (btn && btn->property("isIconRailItem").toBool())
        {
            QString iconKey = btn->property("iconKey").toString();
            if (!btn->isChecked() && !iconKey.isEmpty())
            {
                if (event->type() == QEvent::Enter)
                    UiKit::animateIconColor(btn, iconKey, QColor("#9A9A9A"), QColor("#D97706"), 150);
                else
                    UiKit::animateIconColor(btn, iconKey, QColor("#D97706"), QColor("#9A9A9A"), 150);
            }
            return QWidget::eventFilter(watched, event);
        }
    }
    return QWidget::eventFilter(watched, event);
}
