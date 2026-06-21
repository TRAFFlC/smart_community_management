#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QMap>
#include <QList>
#include <QPushButton>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVariantAnimation>
#include <QLineEdit>
#include <QProgressBar>
#include <QString>

class BasePage;
class NavigationRail;
class BreadcrumbBar;
class GlobalSearchController;
class NotificationManager;
class PageCache;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void logoutRequested();

private:
    void setupUI();
    void switchPage(const QString& key);
    void updateUserInfo();
    void sendNotification(int userId, const QString& title, const QString& content,
                          int type, const QString& bizType = QString(), int bizId = 0);
    void refreshCurrentPage();
    void onRefreshBtnClicked();
    void onNotifyBtnClicked();

    // 顶部框架
    QWidget* m_topBar = nullptr;
    QToolButton* m_refreshBtn = nullptr;
    QVariantAnimation* m_refreshAnimation = nullptr;
    QPixmap m_refreshIconPix;
    QLineEdit* m_globalSearchEdit = nullptr;
    QToolButton* m_userMenuBtn = nullptr;
    QToolButton* m_notifyBtn = nullptr;

    // 内容区
    QStackedWidget* m_contentStack = nullptr;
    QString m_currentPage;

    // 拆分出的辅助类（协调者模式）
    NavigationRail* m_navigationRail = nullptr;
    BreadcrumbBar* m_breadcrumbBar = nullptr;
    GlobalSearchController* m_searchController = nullptr;
    NotificationManager* m_notificationManager = nullptr;
    PageCache* m_pageCache = nullptr;

    QProgressBar* m_loadingBar = nullptr;
};

#endif // MAINWINDOW_H
