#include "DemoDataService.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

void DemoDataService::initIfEmpty()
{
    auto &db = DatabaseManager::instance();

    // 检查是否已有数据
    QSqlQuery q("SELECT COUNT(*) FROM sys_user");
    if (q.exec() && q.next() && q.value(0).toInt() > 0)
        return;

    qDebug() << "Initializing demo data...";

    QSqlDatabase database = db.database();
    database.transaction();

    initRoles(db);
    initMenus(db);
    initOrganizations(db);
    initUsers(db);
    initDictData(db);
    initArchiveData(db);
    initFamilyData(db);
    initWorkOrders(db);
    initEvents(db);
    initAnnouncements(db);
    initVolunteerData(db);
    initServiceData(db);
    initParkingData(db);
    initVehicleData(db);
    initPublicIncomeData(db);
    initTopicData(db);
    initKnowledgeBase(db);
    initEvaluationData(db);
    initAssessmentData(db);
    initVisitorData(db);
    initBillData(db);
    initMonthlyCardData(db);
    initOpinionData(db);
    initInspectionData(db);
    initVisitRecordData(db);
    initSupervisionData(db);
    initOperationLogData(db);
    initVoteData(db);
    initNotificationData(db);

    if (database.commit()) {
        qDebug() << "Demo data initialized successfully";
    } else {
        database.rollback();
        qWarning() << "Demo data initialization failed, rolled back:" << database.lastError().text();
    }
}
