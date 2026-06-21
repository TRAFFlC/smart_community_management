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

// 通过页面 key 统一创建页面（由 PageCache 使用）
BasePage* createPage(const QString& key);

// ===== Archive 子页面构建辅助函数 =====
void buildArchiveOrg(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildArchiveEstate(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildArchiveHouse(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildArchiveResident(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildArchiveVehicle(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildArchiveFacility(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildArchiveGrid(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);
void buildArchiveSpecial(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint);

// ===== Report 子页面构建辅助函数 =====
BasePage* buildReportWorkOrder();
BasePage* buildReportEvent();
BasePage* buildReportService();
BasePage* buildReportDashboard();

// ===== Service 子页面构建辅助函数 =====
BasePage* buildServiceVolunteer();
BasePage* buildServiceConvenience();
BasePage* buildServiceEmployment();

// ===== Governance 子页面构建辅助函数 =====
BasePage* buildGovernanceInspection();
BasePage* buildGovernanceCare();
BasePage* buildGovernanceSupervision();
BasePage* buildGovernanceOpinion();
BasePage* buildGovernanceAssessment();

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

// ===== System 子页面构建辅助函数 =====
void buildSystemUser(BasePage* page, QVBoxLayout* layout, DatabaseManager& db, QLabel* emptyHint);
void buildSystemRole(BasePage* page, QVBoxLayout* layout, DatabaseManager& db, QLabel* emptyHint);
void buildSystemMenu(BasePage* page, QVBoxLayout* layout, DatabaseManager& db, QLabel* emptyHint);
void buildSystemDict(BasePage* page, QVBoxLayout* layout, DatabaseManager& db, QLabel* emptyHint);
void buildSystemLog(BasePage* page, QVBoxLayout* layout, DatabaseManager& db, QLabel* emptyHint);
void buildSystemAI(BasePage* page, QVBoxLayout* layout, DatabaseManager& db, QLabel* emptyHint);

} // namespace PageFactory

#endif // PAGEFACTORY_H
