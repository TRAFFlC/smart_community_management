#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QString>

class QLabel;
class QToolButton;

// 负责通知铃铛未读数角标查询与脉冲动画
class NotificationManager : public QObject {
    Q_OBJECT
public:
    explicit NotificationManager(QObject *parent = nullptr);
    void setBadgeLabel(QLabel *label, QToolButton *button);

public slots:
    void refreshBadge();

private:
    QLabel *m_badgeLabel = nullptr;
    QToolButton *m_notifyBtn = nullptr;
};

#endif // NOTIFICATIONMANAGER_H
