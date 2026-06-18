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
#include <QListWidget>
#include <QFrame>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QTableWidget>
#include "models/Models.h"
#include "database/DatabaseManager.h"
#include "pages/BasePage.h"
#include "pages/PageFactory.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void logoutRequested();

private:
    void setupUI();
    void buildSidebar();
    QWidget* createSidebarItem(const QString& icon, const QString& text, const QString& key, bool isHeader = false);
    void switchPage(const QString& key);
    void updateUserInfo();
    BasePage* getOrCreatePage(const QString& key);

    // 通知发送（页面通过 sendNotificationRequested 信号触发）
    void sendNotification(int userId, const QString& title, const QString& content, int type, const QString& bizType = QString(), int bizId = 0);

    // UI
    QWidget* m_sidebar = nullptr;
    QStackedWidget* m_contentStack = nullptr;
    QWidget* m_topBar = nullptr;
    QLabel* m_userInfoLabel = nullptr;
    QLabel* m_roleInfoLabel = nullptr;
    QLabel* m_breadcrumbLabel = nullptr;
    QString m_currentPage;
    QMap<QString, QWidget*> m_pageCache;
    QMap<QString, QWidget*> m_sidebarItems;
    QWidget* m_activeSidebarItem = nullptr;
    QList<QPushButton*> m_groupHeaders;
    QList<QWidget*> m_groupContainers;

    // 顶栏改造：通知角标、面包屑、刷新按钮
    QToolButton* m_notifyBtn = nullptr;
    QLabel* m_notificationBadge = nullptr;
    QWidget* m_breadcrumbContainer = nullptr;
    QHBoxLayout* m_breadcrumbLayout = nullptr;
    QToolButton* m_refreshBtn = nullptr;
    QVariantAnimation* m_refreshAnimation = nullptr;
    QPixmap m_refreshIconPix;

    // 全局搜索
    QLineEdit* m_globalSearchEdit = nullptr;
    QFrame* m_searchDropdown = nullptr;
    QListWidget* m_searchResultList = nullptr;
    QToolButton* m_userMenuBtn = nullptr;

    // 加载状态指示器
    QProgressBar* m_loadingBar = nullptr;

private:
    void performGlobalSearch(const QString& keyword);
    void hideSearchDropdown();
    void showSearchDropdown();
    void onSearchResultClicked(int row);

private slots:
    void updateNotificationBadge();
    void updateBreadcrumb(const QString& module, const QString& page);
    void refreshCurrentPage();
    void onBreadcrumbClicked(const QString& target);
    void onRefreshBtnClicked();
    void onNotifyBtnClicked();
};

#endif // MAINWINDOW_H
