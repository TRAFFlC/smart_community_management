#include "DemoDataService.h"

#include <QChar>
#include <QDate>
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QPair>
#include <QSqlQuery>
#include <QStringList>
#include <QVariantMap>

void DemoDataService::initVolunteerData(DatabaseManager &db)
{
    QSqlQuery uq("SELECT id FROM sys_user WHERE username = 'zhangsan'");
    qint64 userId = 0;
    if (uq.next())
        userId = uq.value(0).toLongLong();

    qint64 volId = db.insert("sv_volunteer", {{"user_id", userId}, {"skills", QStringLiteral("教学,翻译")}, {"available_time", QStringLiteral("周末")}, {"total_hours", 12.5}, {"status", 0}});

    QSqlQuery cq("SELECT id FROM sys_org WHERE org_type = 3 LIMIT 1");
    qint64 commId = 0;
    if (cq.next())
        commId = cq.value(0).toLongLong();

    QSqlQuery pq("SELECT id FROM sys_user WHERE username = 'shequ_lifang'");
    qint64 pubId = 0;
    if (pq.next())
        pubId = pq.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();

    qint64 actId = db.insert("sv_volunteer_activity", {{"community_org_id", commId}, {"title", QStringLiteral("端午节关爱独居老人活动")}, {"description", QStringLiteral("走访慰问小区独居老人，送去节日祝福和粽子")}, {"activity_type", 2}, {"location", QStringLiteral("翠苑小区")}, {"start_time", now.addDays(5)}, {"end_time", now.addDays(5).addSecs(4 * 3600)}, {"need_count", 10}, {"enrolled_count", 3}, {"publisher_id", pubId}, {"status", 1}});

    db.insert("sv_volunteer_signup", {{"activity_id", actId}, {"volunteer_id", volId}, {"status", 1}});

    Q_UNUSED(actId)

    // More activities
    qint64 act2 = db.insert("sv_volunteer_activity", {{"community_org_id", commId}, {"title", QStringLiteral("社区环境清洁日")}, {"description", QStringLiteral("每月一次的社区环境清洁志愿活动")}, {"activity_type", 1}, {"location", QStringLiteral("翠苑社区广场")}, {"start_time", now.addDays(-2)}, {"end_time", now.addDays(-2).addSecs(3 * 3600)}, {"need_count", 20}, {"enrolled_count", 15}, {"publisher_id", pubId}, {"status", 3}});
    qint64 act3 = db.insert("sv_volunteer_activity", {{"community_org_id", commId}, {"title", QStringLiteral("暑期少儿阅读辅导")}, {"description", QStringLiteral("为社区少儿提供免费阅读指导和绘本分享")}, {"activity_type", 3}, {"location", QStringLiteral("社区图书室")}, {"start_time", now.addDays(10)}, {"end_time", now.addDays(10).addSecs(2 * 3600)}, {"need_count", 8}, {"enrolled_count", 5}, {"publisher_id", pubId}, {"status", 1}});
    qint64 act4 = db.insert("sv_volunteer_activity", {{"community_org_id", commId}, {"title", QStringLiteral("社区安全巡逻")}, {"description", QStringLiteral("夜间社区安全巡逻志愿活动")}, {"activity_type", 4}, {"location", QStringLiteral("翠苑全区域")}, {"start_time", now.addDays(3)}, {"end_time", now.addDays(3).addSecs(3 * 3600)}, {"need_count", 6}, {"enrolled_count", 4}, {"publisher_id", pubId}, {"status", 1}});
    db.insert("sv_volunteer_activity", {{"community_org_id", commId}, {"title", QStringLiteral("重阳节文艺汇演")}, {"description", QStringLiteral("组织社区老人文艺表演和互动游戏")}, {"activity_type", 5}, {"location", QStringLiteral("社区活动中心")}, {"start_time", now.addDays(-30)}, {"end_time", now.addDays(-30).addSecs(4 * 3600)}, {"need_count", 12}, {"enrolled_count", 12}, {"publisher_id", pubId}, {"status", 3}});

    // More signups
    db.insert("sv_volunteer_signup", {{"activity_id", act2}, {"volunteer_id", volId}, {"status", 3}, {"hours", 3.0}});
    db.insert("sv_volunteer_signup", {{"activity_id", act3}, {"volunteer_id", volId}, {"status", 1}});
    db.insert("sv_volunteer_signup", {{"activity_id", act4}, {"volunteer_id", volId}, {"status", 0}});
}

void DemoDataService::initServiceData(DatabaseManager &db)
{
    QSqlQuery sq("SELECT id FROM sys_org WHERE org_type = 6 LIMIT 1");
    qint64 orgId = 0;
    if (sq.next())
        orgId = sq.value(0).toLongLong();

    QSqlQuery uq("SELECT id FROM sys_user WHERE username = 'fuwu_sun'");
    qint64 userId = 0;
    if (uq.next())
        userId = uq.value(0).toLongLong();

    // Service providers
    db.insert("sv_service_provider", {{"org_id", orgId}, {"user_id", userId}, {"provider_name", QStringLiteral("好帮手家政服务")}, {"service_types", QStringLiteral("家政,保洁,维修")}, {"contact_person", QStringLiteral("孙丽")}, {"contact_phone", "13800005001"}, {"rating", 4.8}, {"total_orders", 32}, {"status", 0}});
    db.insert("sv_service_provider", {{"org_id", orgId},
                                      {"provider_name", QStringLiteral("快修家电服务")},
                                      {"service_types", QStringLiteral("家电维修,水电安装")},
                                      {"contact_person", QStringLiteral("周师傅")},
                                      {"contact_phone", "13800005002"},
                                      {"rating", 4.6},
                                      {"total_orders", 58},
                                      {"status", 0}});
    db.insert("sv_service_provider", {{"org_id", orgId},
                                      {"provider_name", QStringLiteral("爱心养老护理")},
                                      {"service_types", QStringLiteral("老人陪护,日间照料")},
                                      {"contact_person", QStringLiteral("李护理")},
                                      {"contact_phone", "13800005003"},
                                      {"rating", 4.9},
                                      {"total_orders", 21},
                                      {"status", 0}});

    // Service orders
    QSqlQuery rq("SELECT id FROM sys_user WHERE username = 'zhangsan'");
    qint64 applicantId = 0;
    if (rq.next())
        applicantId = rq.value(0).toLongLong();

    QDateTime now = QDateTime::currentDateTime();
    db.insert("sv_service_order", {{"order_no", "SV20260601001"}, {"applicant_id", applicantId}, {"service_type", 2}, {"title", QStringLiteral("家庭保洁服务")}, {"description", QStringLiteral("需要一次全面家庭保洁")}, {"appointment_time", now.addDays(2)}, {"address", QStringLiteral("翠苑小区1号楼")}, {"status", 0}});
    db.insert("sv_service_order", {{"order_no", "SV20260602001"}, {"applicant_id", applicantId}, {"service_type", 1}, {"title", QStringLiteral("水龙头维修")}, {"description", QStringLiteral("厨房水龙头漏水")}, {"appointment_time", now.addDays(-3)}, {"address", QStringLiteral("翠苑小区1号楼")}, {"status", 3}});
    db.insert("sv_service_order", {{"order_no", "SV20260603001"}, {"applicant_id", applicantId}, {"service_type", 4}, {"title", QStringLiteral("老人日间照料")}, {"description", QStringLiteral("需要每周3次的日间照料服务")}, {"appointment_time", now.addDays(5)}, {"address", QStringLiteral("翠苑小区2号楼")}, {"status", 1}});

    // Job postings
    QSqlQuery cq("SELECT id FROM sys_org WHERE org_type = 3 LIMIT 1");
    qint64 commOrgId = 0;
    if (cq.next())
        commOrgId = cq.value(0).toLongLong();

    db.insert("sv_job_posting", {{"publisher_id", userId}, {"community_org_id", commOrgId}, {"title", QStringLiteral("物业保安招聘")}, {"company", QStringLiteral("恒达物业")}, {"salary_range", QStringLiteral("4000-6000元/月")}, {"requirements", QStringLiteral("年龄25-50岁，身体健康，有责任心")}, {"description", QStringLiteral("负责小区安全巡逻、门岗值守等工作")}, {"headcount", 3}, {"deadline", QDate::currentDate().addDays(30)}, {"status", 0}});
    db.insert("sv_job_posting", {{"publisher_id", userId}, {"community_org_id", commOrgId}, {"title", QStringLiteral("社区保洁员")}, {"company", QStringLiteral("恒达物业")}, {"salary_range", QStringLiteral("3500-4500元/月")}, {"requirements", QStringLiteral("年龄30-55岁，吃苦耐劳")}, {"description", QStringLiteral("负责小区公共区域保洁工作")}, {"headcount", 2}, {"deadline", QDate::currentDate().addDays(20)}, {"status", 0}});
    db.insert("sv_job_posting", {{"publisher_id", userId}, {"community_org_id", commOrgId}, {"title", QStringLiteral("养老护理员")}, {"company", QStringLiteral("爱心养老护理")}, {"salary_range", QStringLiteral("5000-8000元/月")}, {"requirements", QStringLiteral("有护理经验优先，有耐心和爱心")}, {"description", QStringLiteral("为社区独居老人提供日常照料服务")}, {"headcount", 5}, {"deadline", QDate::currentDate().addDays(45)}, {"status", 0}});
}

void DemoDataService::initParkingData(DatabaseManager &db)
{
    // 查询小区ID
    QSqlQuery eq("SELECT id FROM cm_estate WHERE del_flag = 0 LIMIT 1");
    qint64 estateId = 0;
    if (eq.next())
        estateId = eq.value(0).toLongLong();

    // 创建停车位
    struct ParkingSpace
    {
        const char *code;
        const char *area;
        int type;
        int status;
    };
    ParkingSpace spaces[] = {
        {"A-001", "A区地下", 1, 1},
        {"A-002", "A区地下", 1, 1},
        {"A-003", "A区地下", 1, 0},
        {"A-004", "A区地下", 1, 1},
        {"A-005", "A区地下", 2, 0},
        {"B-001", "B区地下", 1, 1},
        {"B-002", "B区地下", 1, 0},
        {"B-003", "B区地下", 2, 1},
        {"C-001", "C区地面", 2, 1},
        {"C-002", "C区地面", 2, 0},
        {"C-003", "C区地面", 2, 0},
        {"C-004", "C区地面", 2, 1},
    };
    for (const auto &s : spaces)
    {
        db.insert("cm_parking_space", {{"estate_id", estateId}, {"space_code", s.code}, {"area_name", s.area}, {"space_type", s.type}, {"status", s.status}});
    }

    // 将已有车辆关联到部分车位
    QSqlQuery vq("SELECT id FROM cm_vehicle WHERE del_flag = 0 LIMIT 6");
    QSqlQuery pq("SELECT id FROM cm_parking_space WHERE status = 1 AND del_flag = 0 LIMIT 6");
    while (vq.next() && pq.next())
    {
        db.update("cm_vehicle", vq.value(0).toLongLong(), {{"parking_space_id", pq.value(0).toLongLong()}});
    }
}

void DemoDataService::initVehicleData(DatabaseManager &db)
{
    QSqlQuery eq("SELECT id FROM cm_estate WHERE del_flag = 0 LIMIT 1");
    qint64 estateId = eq.next() ? eq.value(0).toLongLong() : 0;

    QSqlQuery psq("SELECT id FROM cm_parking_space WHERE del_flag = 0 AND status = 1 LIMIT 5");
    QList<qint64> parkingIds;
    while (psq.next()) parkingIds.append(psq.value(0).toLongLong());

    QSqlQuery rq("SELECT id FROM cm_resident WHERE del_flag = 0 LIMIT 5");
    QList<qint64> residentIds;
    while (rq.next()) residentIds.append(rq.value(0).toLongLong());

    struct VehicleInfo {
        const char *plate; const char *brand; const char *color;
        int type;
    };
    VehicleInfo vehicles[] = {
        {"皖A·12345", "比亚迪", "白色", 1},  // 轿车
        {"皖A·23456", "大众", "黑色", 2},     // SUV
        {"皖A·34567", "丰田", "银色", 1},     // 轿车
        {"皖A·45678", "宝马", "红色", 2},     // SUV
        {"皖A·56789", "本田", "白色", 4},     // MPV (其他)
    };

    for (int i = 0; i < 5; i++)
    {
        qint64 psId = (i < parkingIds.size()) ? parkingIds[i] : 0;
        QVariantMap vData;
        vData["plate_number"] = QString(vehicles[i].plate);
        vData["vehicle_brand"] = QString(vehicles[i].brand);
        vData["vehicle_color"] = QString(vehicles[i].color);
        vData["vehicle_type"] = vehicles[i].type;
        vData["estate_id"] = estateId;
        if (psId > 0) vData["parking_space_id"] = psId;
        vData["status"] = 0;
        qint64 vid = db.insert("cm_vehicle", vData);

        // 关联车辆和居民
        if (i < residentIds.size() && vid > 0) {
            db.insert("cm_vehicle_owner", {{"vehicle_id", vid}, {"resident_id", residentIds[i]}, {"relation", QStringLiteral("车主")}});
        }
    }
}

void DemoDataService::initBillData(DatabaseManager &db)
{
    QSqlQuery estateQ("SELECT id FROM cm_estate LIMIT 1");
    qint64 estateId = estateQ.next() ? estateQ.value(0).toLongLong() : 1;

    QSqlQuery houseQ("SELECT id FROM cm_house LIMIT 10");
    QList<qint64> houseIds;
    while (houseQ.next())
        houseIds.append(houseQ.value(0).toLongLong());

    QSqlQuery residentQ("SELECT id FROM cm_resident LIMIT 10");
    QList<qint64> residentIds;
    while (residentQ.next())
        residentIds.append(residentQ.value(0).toLongLong());

    // Bill types: 1=物业费, 2=水费, 3=电费, 4=停车费, 5=综合
    struct BData
    {
        int type;
        double amount;
        int sts;
        const char *period;
        const char *remark;
    };
    BData bList[] = {
        {1, 380.00, 1, "2026-06", "6月物业费"},
        {1, 420.00, 1, "2026-06", "6月物业费"},
        {5, 530.00, 0, "2026-06", "物业费+水费"},
        {5, 680.00, 0, "2026-06", "物业费+停车费"},
        {1, 350.00, 1, "2026-06", "6月物业费"},
        {5, 720.00, 1, "2026-06", "物业费+电费"},
        {1, 380.00, 1, "2026-05", "5月物业费"},
        {2, 45.50, 1, "2026-05", "5月水费"},
        {3, 128.30, 0, "2026-05", "5月电费"},
        {4, 200.00, 1, "2026-05", "5月停车费"},
        {1, 380.00, 2, "2026-04", "4月物业费(逾期)"},
        {1, 420.00, 1, "2026-04", "4月物业费"},
        {5, 560.00, 1, "2026-04", "物业费+水费"},
        {1, 350.00, 1, "2026-03", "3月物业费"},
        {4, 200.00, 1, "2026-03", "3月停车费"},
    };

    for (int i = 0; i < 15; i++)
    {
        int hIdx = i % houseIds.size();
        int rIdx = i % residentIds.size();
        QVariantMap bData;
        bData["bill_no"] = QString("FY%1-%2").arg(bList[i].period).arg(i + 1, 3, 10, QChar('0'));
        bData["estate_id"] = estateId;
        bData["house_id"] = houseIds[hIdx];
        bData["resident_id"] = residentIds[rIdx];
        bData["bill_type"] = bList[i].type;
        bData["amount"] = bList[i].amount;
        bData["period"] = QString(bList[i].period);
        bData["status"] = bList[i].sts;
        bData["remark"] = QString(bList[i].remark);
        if (bList[i].sts == 1)
        {
            bData["pay_time"] = QDateTime::currentDateTime().addDays(-i * 3).toString("yyyy-MM-dd HH:mm:ss");
            bData["pay_method"] = (i % 3) + 1;
        }
        db.insert("pm_bill", bData);
    }
}

void DemoDataService::initMonthlyCardData(DatabaseManager &db)
{
    QSqlQuery estateQ("SELECT id FROM cm_estate LIMIT 1");
    qint64 estateId = estateQ.next() ? estateQ.value(0).toLongLong() : 1;

    QSqlQuery spaceQ("SELECT id, space_code FROM cm_parking_space LIMIT 6");
    QList<QPair<qint64, QString>> spaces;
    while (spaceQ.next())
    {
        spaces.append({spaceQ.value(0).toLongLong(), spaceQ.value(1).toString()});
    }

    struct MCData
    {
        const char *plate;
        const char *owner;
        const char *phone;
        int cardType;
        int sts;
        int monthOffset;
    };
    MCData mcList[] = {
        {"京A88888", "张三", "13800001001", 2, 1, 0},  // 年卡, 有效
        {"京B66666", "李四", "13800001002", 1, 1, 0},  // 季卡, 有效
        {"京C12345", "王五", "13800001003", 0, 1, 0},  // 月卡, 有效
        {"京D99999", "赵六", "13800001005", 0, 0, -1}, // 月卡, 已过期
        {"京E55555", "钱七", "13800001011", 2, 1, 0},  // 年卡, 有效
        {"京F77777", "孙八", "13800001012", 1, 2, 0},  // 季卡, 待续费
    };

    double fees[] = {150.0, 400.0, 1500.0}; // 月卡/季卡/年卡
    for (int i = 0; i < 6; i++)
    {
        int sIdx = i % spaces.size();
        QVariantMap mcData;
        mcData["estate_id"] = estateId;
        mcData["plate_no"] = QString(mcList[i].plate);
        mcData["owner_name"] = QString(mcList[i].owner);
        mcData["owner_phone"] = QString(mcList[i].phone);
        mcData["space_id"] = spaces[sIdx].first;
        mcData["card_type"] = mcList[i].cardType;
        mcData["fee"] = fees[mcList[i].cardType];

        QDate startDate = QDate::currentDate().addMonths(mcList[i].monthOffset);
        QDate endDate;
        switch (mcList[i].cardType)
        {
        case 0:
            endDate = startDate.addMonths(1);
            break;
        case 1:
            endDate = startDate.addMonths(3);
            break;
        case 2:
            endDate = startDate.addYears(1);
            break;
        }
        if (mcList[i].sts == 0)
        {
            // 已过期 - 结束日期在过去
            startDate = startDate.addMonths(-2);
            endDate = endDate.addMonths(-2);
        }
        mcData["start_date"] = startDate.toString("yyyy-MM-dd");
        mcData["end_date"] = endDate.toString("yyyy-MM-dd");
        mcData["status"] = mcList[i].sts;
        db.insert("pm_monthly_card", mcData);
    }
}
