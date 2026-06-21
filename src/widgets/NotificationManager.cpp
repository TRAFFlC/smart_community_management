#include "NotificationManager.h"
#include "PagesCommon.h"
#include "database/DatabaseManager.h"
#include "services/AuthService.h"

NotificationManager::NotificationManager(QObject *parent) : QObject(parent) {}

void NotificationManager::setBadgeLabel(QLabel *label, QToolButton *button)
{
    m_badgeLabel = label;
    m_notifyBtn = button;
    if (m_badgeLabel)
        m_badgeLabel->setProperty("lastUnread", 0);
}

void NotificationManager::refreshBadge()
{
    if (!m_badgeLabel || !m_notifyBtn)
        return;

    qint64 uid = AuthService::instance().currentUser().id;
    int unreadCount = 0;
    if (uid > 0)
    {
        QSqlQuery q(DatabaseManager::instance().database());
        q.prepare("SELECT COUNT(*) FROM nt_notification WHERE user_id = :uid AND is_read = 0");
        q.bindValue(":uid", uid);
        if (q.exec() && q.next())
            unreadCount = q.value(0).toInt();
    }

    int lastUnread = m_badgeLabel->property("lastUnread").toInt();

    if (unreadCount <= 0)
    {
        m_badgeLabel->setVisible(false);
    }
    else
    {
        QString text = unreadCount > 99 ? QStringLiteral("99+") : QString::number(unreadCount);
        m_badgeLabel->setText(text);
        m_badgeLabel->setVisible(true);
        m_badgeLabel->adjustSize();

        int badgeW = m_badgeLabel->sizeHint().width();
        int badgeH = 16;
        int x = m_notifyBtn->width() - badgeW - 2;
        int y = 2;
        if (x < 0)
            x = 0;
        m_badgeLabel->setGeometry(x, y, badgeW, badgeH);
        m_badgeLabel->raise();

        if (unreadCount > lastUnread)
            UiKit::pulseWidget(m_badgeLabel, 400);
    }

    m_badgeLabel->setProperty("lastUnread", unreadCount);
}
