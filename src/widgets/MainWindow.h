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
    void buildIconRail();
    void buildSectionPanel(const QString& moduleKey);
    QWidget* createSectionItem(const QString& text, const QString& key);
    void switchModule(const QString& moduleKey);
    void switchPage(const QString& key);
    void updateUserInfo();
    BasePage* getOrCreatePage(const QString& key);

    // 通知发送（页面通过 sendNotificationRequested 信号触发）
    void sendNotification(int userId, const QString& title, const QString& content, int type, const QString& bizType = QString(), int bizId = 0);

    // UI — 双栏导航：Icon Rail (64px) + Section Panel (220px)
    QWidget* m_iconRail = nullptr;
    QWidget* m_sectionPanel = nullptr;
    QLabel* m_sectionTitle = nullptr;
    QVBoxLayout* m_sectionLayout = nullptr;
    QMap<QString, QToolButton*> m_iconRailItems;   // 一级模块图标
    QMap<QString, QWidget*> m_sectionItems;        // 二级面板项（key=pageKey）
    QToolButton* m_activeIconRailItem = nullptr;
    QPushButton* m_activeSectionItem = nullptr;
    QString m_currentModule;                        // 当前一级模块名

    QStackedWidget* m_contentStack = nullptr;
    QWidget* m_topBar = nullptr;
    QString m_currentPage;
    QMap<QString, QWidget*> m_pageCache;

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
