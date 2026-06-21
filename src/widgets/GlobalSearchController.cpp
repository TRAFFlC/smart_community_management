#include "GlobalSearchController.h"
#include "PagesCommon.h"
#include "database/DatabaseManager.h"
#include "services/AuthService.h"

GlobalSearchController::GlobalSearchController(QObject *parent) : QObject(parent) {}

void GlobalSearchController::attach(QLineEdit *edit, QWidget *popupParent)
{
    m_edit = edit;
    m_popupParent = popupParent;
    if (!m_edit)
        return;

    m_dropdown = new QFrame(popupParent);
    m_dropdown->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    m_dropdown->setStyleSheet(QStringLiteral(
        "QFrame { background: #FFFFFF; border: 1px solid #141413; border-radius: 0; }"
        "QListWidget { background: transparent; border: none; outline: none; }"
        "QListWidget::item { padding: 10px 14px; border-bottom: 1px solid #E8E5DE; font-size: 13px; color: #141413; }"
        "QListWidget::item:hover { background: #F5F2EB; color: #92400E; }"
        "QListWidget::item:selected { background: #141413; color: #FAF9F6; }"));
    auto *dropdownLayout = new QVBoxLayout(m_dropdown);
    dropdownLayout->setContentsMargins(0, 0, 0, 0);
    dropdownLayout->setSpacing(0);
    m_resultList = new QListWidget(m_dropdown);
    m_resultList->setMinimumWidth(260);
    m_resultList->setMaximumHeight(360);
    dropdownLayout->addWidget(m_resultList);
    m_dropdown->hide();

    connect(m_edit, &QLineEdit::textChanged, this, &GlobalSearchController::performSearch);
    connect(m_resultList, &QListWidget::itemClicked, this, &GlobalSearchController::onItemClicked);
    connect(m_resultList, &QListWidget::itemActivated, this, &GlobalSearchController::onItemClicked);
}

void GlobalSearchController::performSearch(const QString &text)
{
    if (!m_resultList || !m_dropdown)
        return;

    m_resultList->clear();
    QString keyword = text.trimmed();
    if (keyword.isEmpty())
    {
        hideDropdown();
        return;
    }

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
                QString menuKey = QString::number(q.value(0).toLongLong());
                QString display = QStringLiteral("[菜单] %1").arg(menuName);
                auto *item = new QListWidgetItem(display);
                item->setData(Qt::UserRole, QStringLiteral("menu"));
                item->setData(Qt::UserRole + 1, menuKey);
                m_resultList->addItem(item);
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
                m_resultList->addItem(item);
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
                m_resultList->addItem(item);
            }
        }
    }

    if (m_resultList->count() == 0)
    {
        auto *emptyItem = new QListWidgetItem(QStringLiteral("未找到相关结果"));
        emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
        m_resultList->addItem(emptyItem);
    }

    showDropdown();
}

void GlobalSearchController::showDropdown()
{
    if (!m_dropdown || !m_edit)
        return;
    QPoint bottomLeft = m_edit->mapToGlobal(QPoint(0, m_edit->height()));
    int dropdownWidth = m_edit->width();
    m_dropdown->setFixedWidth(dropdownWidth);
    m_dropdown->move(bottomLeft);
    m_dropdown->show();
    m_dropdown->raise();
}

void GlobalSearchController::hideDropdown()
{
    if (m_dropdown)
        m_dropdown->hide();
}

void GlobalSearchController::onItemClicked(QListWidgetItem *item)
{
    if (!item || !m_resultList)
        return;

    QString type = item->data(Qt::UserRole).toString();

    hideDropdown();
    if (m_edit)
        m_edit->clear();

    if (type == QStringLiteral("menu"))
    {
        QString target = item->data(Qt::UserRole + 1).toString();
        if (!target.isEmpty())
            emit menuSelected(target);
    }
    else if (type == QStringLiteral("workorder"))
    {
        qint64 entityId = item->data(Qt::UserRole + 2).toLongLong();
        emit workOrderSelected(entityId);
    }
    else if (type == QStringLiteral("event"))
    {
        qint64 entityId = item->data(Qt::UserRole + 2).toLongLong();
        emit eventSelected(entityId);
    }
}
