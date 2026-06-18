#ifndef PAGEFACTORY_H
#define PAGEFACTORY_H

#include "pages/BasePage.h"
#include <QString>

// 前置声明
class QVBoxLayout;
class QTableWidget;
class QLabel;
class DatabaseManager;

// 页面工厂：独立函数，不属于 MainWindow
// 每个函数创建并返回一个 BasePage 子类实例
namespace PageFactory {

// ===== 页面创建函数 =====
BasePage* createDashboardPage();
BasePage* createTodoPage();
BasePage* createMessagePage();
BasePage* createArchivePage(const QString& sub);
BasePage* createPropertyPage(const QString& sub);
BasePage* createGovernancePage(const QString& sub);
BasePage* createServicePage(const QString& sub);
BasePage* createReportPage(const QString& sub);
BasePage* createSystemPage(const QString& sub);
BasePage* createEventPage();

// ===== Property 子页面构建辅助函数 =====
// 这些函数在 createPropertyPage 内部被调用，用于构建不同子类型的页面内容
// page 参数为 BasePage*，用于通过 requestNotification/requestNavigate 与 MainWindow 通信
void buildPropertyWorkorder(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildPropertyComplaint(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildPropertyInspection(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildPropertyAnnouncement(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildPropertyVisitor(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildPropertyTopic(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildPropertyParking(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildPropertyBilling(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildPropertyIncome(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);

} // namespace PageFactory

#endif // PAGEFACTORY_H
