#ifndef DEMODATASERVICE_H
#define DEMODATASERVICE_H

#include "database/DatabaseManager.h"
#include "utils/Utils.h"
#include <QDebug>
#include <QRandomGenerator>
#include <cstdlib>

class DemoDataService
{
public:
    static void initIfEmpty();

private:
    static void initRoles(DatabaseManager &db);
    static void initMenus(DatabaseManager &db);
    static void initOrganizations(DatabaseManager &db);
    static void initUsers(DatabaseManager &db);
    static void initDictData(DatabaseManager &db);
    static void initArchiveData(DatabaseManager &db);
    static void initFamilyData(DatabaseManager &db);
    static void initWorkOrders(DatabaseManager &db);
    static void initEvents(DatabaseManager &db);
    static void initAnnouncements(DatabaseManager &db);
    static void initVolunteerData(DatabaseManager &db);
    static void initServiceData(DatabaseManager &db);
    static void initParkingData(DatabaseManager &db);
    static void initVehicleData(DatabaseManager &db);
    static void initPublicIncomeData(DatabaseManager &db);
    static void initTopicData(DatabaseManager &db);
    static void initKnowledgeBase(DatabaseManager &db);
    static void initEvaluationData(DatabaseManager &db);
    static void initAssessmentData(DatabaseManager &db);
    static void initVisitorData(DatabaseManager &db);
    static void initBillData(DatabaseManager &db);
    static void initMonthlyCardData(DatabaseManager &db);
    static void initOpinionData(DatabaseManager &db);
    static void initInspectionData(DatabaseManager &db);
    static void initVisitRecordData(DatabaseManager &db);
    static void initSupervisionData(DatabaseManager &db);
    static void initOperationLogData(DatabaseManager &db);
    static void initVoteData(DatabaseManager &db);
    static void initNotificationData(DatabaseManager &db);
};

#endif // DEMODATASERVICE_H
