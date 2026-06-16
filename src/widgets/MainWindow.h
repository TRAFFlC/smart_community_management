#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QMap>
#include <QList>
#include <QPushButton>
#include "models/Models.h"

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
    QWidget* getOrCreatePage(const QString& key);

    // Pages
    QWidget* createDashboardPage();
    QWidget* createArchivePage(const QString& sub);
    QWidget* createPropertyPage(const QString& sub);
    QWidget* createGovernancePage(const QString& sub);
    QWidget* createServicePage(const QString& sub);
    QWidget* createReportPage(const QString& sub);
    QWidget* createSystemPage(const QString& sub);

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
};

#endif // MAINWINDOW_H
