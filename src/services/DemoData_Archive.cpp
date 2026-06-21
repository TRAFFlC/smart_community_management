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

void DemoDataService::initArchiveData(DatabaseManager &db)
{
    // 获取物业公司ID
    QSqlQuery pq("SELECT id FROM sys_org WHERE org_type = 4 LIMIT 1");
    qint64 propOrgId = 0;
    if (pq.next())
        propOrgId = pq.value(0).toLongLong();

    // 物业公司档案
    qint64 propCompanyId = db.insert("cm_property_company", {{"org_id", propOrgId}, {"company_name", QStringLiteral("恒达物业管理有限公司")}, {"legal_person", QStringLiteral("陈明")}, {"contact_phone", "0551-88881234"}, {"service_scope", QStringLiteral("物业管理、维修、保洁、安保")}});

    // 获取社区ID
    QSqlQuery cq("SELECT id FROM sys_org WHERE org_type = 3 ORDER BY sort_order");
    qint64 comm1Id = 0, comm2Id = 0;
    if (cq.next())
        comm1Id = cq.value(0).toLongLong();
    if (cq.next())
        comm2Id = cq.value(0).toLongLong();

    // 小区
    qint64 estate1Id = db.insert("cm_estate", {{"org_id", comm1Id}, {"property_company_id", propCompanyId}, {"estate_name", QStringLiteral("翠苑小区")}, {"estate_code", "EST-001"}, {"address", QStringLiteral("翠苑路88号")}, {"total_area", 50000.0}, {"total_buildings", 6}, {"total_houses", 360}, {"built_year", 2010}, {"green_rate", 35.0}, {"parking_spaces", 200}});
    qint64 estate2Id = db.insert("cm_estate", {{"org_id", comm2Id}, {"property_company_id", propCompanyId}, {"estate_name", QStringLiteral("金都花园")}, {"estate_code", "EST-002"}, {"address", QStringLiteral("金都大道200号")}, {"total_area", 80000.0}, {"total_buildings", 10}, {"total_houses", 600}, {"built_year", 2015}, {"green_rate", 40.0}, {"parking_spaces", 400}});

    // 楼栋
    for (int i = 1; i <= 3; i++)
    {
        qint64 bldId = db.insert("cm_building", {{"estate_id", estate1Id}, {"building_name", QString::number(i) + QStringLiteral("号楼")}, {"building_code", QString("BLD-001-%1").arg(i, 2, 10, QChar('0'))}, {"total_units", 2}, {"total_floors", 18}, {"has_elevator", 1}, {"elevator_count", 2}});

        // 单元
        for (int u = 1; u <= 2; u++)
        {
            qint64 unitId = db.insert("cm_unit", {{"building_id", bldId}, {"unit_name", QString::number(u) + QStringLiteral("单元")}, {"unit_code", QString("UNIT-001-%1-%2").arg(i).arg(u)}});

            // 房屋(每单元每层2户)
            for (int f = 1; f <= 3; f++)
            {
                for (int r = 1; r <= 2; r++)
                {
                    QString code = QString("%1-%2-%3%4").arg(i).arg(u).arg(f).arg(r);
                    int hs = (f + r) % 4;
                    db.insert("cm_house", {{"unit_id", unitId}, {"estate_id", estate1Id}, {"house_code", code}, {"floor", f}, {"room_number", QString::number(f * 100 + r)}, {"area", 80.0 + (f + r) * 5}, {"house_type", QStringLiteral("两室一厅")}, {"house_status", hs}});
                }
            }
        }
    }

    // 金都花园简化楼栋
    for (int i = 1; i <= 2; i++)
    {
        qint64 bldId = db.insert("cm_building", {{"estate_id", estate2Id}, {"building_name", QString::number(i) + QStringLiteral("号楼")}, {"building_code", QString("BLD-002-%1").arg(i, 2, 10, QChar('0'))}, {"total_units", 1}, {"total_floors", 24}, {"has_elevator", 1}, {"elevator_count", 2}});
        qint64 unitId = db.insert("cm_unit", {{"building_id", bldId}, {"unit_name", QStringLiteral("1单元")}, {"unit_code", QString("UNIT-002-%1-1").arg(i)}});
        for (int f = 1; f <= 3; f++)
        {
            for (int r = 1; r <= 2; r++)
            {
                db.insert("cm_house", {{"unit_id", unitId}, {"estate_id", estate2Id}, {"house_code", QString("B%1-%2%3").arg(i).arg(f).arg(r)}, {"floor", f}, {"room_number", QString::number(f * 100 + r)}, {"area", 100.0}, {"house_type", QStringLiteral("三室两厅")}, {"house_status", 1}});
            }
        }
    }

    // 网格
    QSqlQuery gq("SELECT id FROM sys_user WHERE username = 'wangge_zhao'");
    qint64 gridWorkerId = 0;
    if (gq.next())
        gridWorkerId = gq.value(0).toLongLong();

    db.insert("cm_grid", {{"community_org_id", comm1Id}, {"grid_name", QStringLiteral("翠苑第一网格")}, {"grid_code", "GRID-001"}, {"description", QStringLiteral("翠苑小区1-3号楼范围")}, {"grid_worker_id", gridWorkerId}, {"cover_estates", QString("[%1]").arg(estate1Id)}});
    db.insert("cm_grid", {{"community_org_id", comm2Id}, {"grid_name", QStringLiteral("金都第一网格")}, {"grid_code", "GRID-002"}, {"description", QStringLiteral("金都花园1-5号楼范围")}, {"grid_worker_id", gridWorkerId}, {"cover_estates", QString("[%1]").arg(estate2Id)}});

    // 业委会
    QSqlQuery ocq("SELECT id FROM sys_org WHERE org_type = 5 LIMIT 1");
    qint64 ocOrgId = 0;
    if (ocq.next())
        ocOrgId = ocq.value(0).toLongLong();

    db.insert("cm_owner_committee", {{"org_id", ocOrgId}, {"estate_id", estate1Id}, {"committee_name", QStringLiteral("翠苑小区业主委员会")}, {"term_start", QDate(2025, 1, 1)}, {"term_end", QDate(2028, 1, 1)}, {"member_count", 5}});

    // 设施设备
    db.insert("cm_facility", {{"estate_id", estate1Id}, {"facility_name", QStringLiteral("1号楼电梯A")}, {"facility_type", 1}, {"facility_code", "FAC-E-001"}, {"location", QStringLiteral("1号楼")}, {"status", 0}});
    db.insert("cm_facility", {{"estate_id", estate1Id}, {"facility_name", QStringLiteral("1号楼电梯B")}, {"facility_type", 1}, {"facility_code", "FAC-E-002"}, {"location", QStringLiteral("1号楼")}, {"status", 0}});
    db.insert("cm_facility", {{"estate_id", estate1Id}, {"facility_name", QStringLiteral("消防栓-001")}, {"facility_type", 2}, {"facility_code", "FAC-F-001"}, {"location", QStringLiteral("1号楼大厅")}, {"status", 0}});
    db.insert("cm_facility", {{"estate_id", estate1Id}, {"facility_name", QStringLiteral("门禁-A01")}, {"facility_type", 3}, {"facility_code", "FAC-D-001"}, {"location", QStringLiteral("小区南门")}, {"status", 0}});

    // 车位
    for (int i = 1; i <= 10; i++)
    {
        db.insert("cm_parking_space", {{"estate_id", estate1Id}, {"space_code", QString("P-A-%1").arg(i, 3, 10, QChar('0'))}, {"area_name", QStringLiteral("A区")}, {"space_type", 1}, {"status", i <= 7 ? 1 : 0}});
    }

    // 特殊群体
    QSqlQuery rq("SELECT id FROM cm_resident LIMIT 1");
    if (rq.next())
    {
        qint64 rid = rq.value(0).toLongLong();
        db.insert("cm_special_group", {{"resident_id", rid}, {"group_type", 1}, {"detail", QStringLiteral("独居老人，子女在外地")}, {"care_level", 2}, {"care_frequency", "weekly"}});
    }

    // 新增8条居民档案
    struct ResidentInfo {
        const char *name; const char *phone; const char *phoneDisplay;
        int gender; const char *nationality; const char *occupation;
        const char *birthday; const char *education; const char *idCard;
        const char *politicalStatus;
    };
    ResidentInfo residents[] = {
        {"李四", "13800001002", "138****1002", 1, "汉族", "销售经理", "1985-07-22", "本科", "340104198507220000", "群众"},
        {"王五", "13800001003", "138****1003", 1, "汉族", "建筑工人", "1992-11-08", "高中", "340104199211080000", "群众"},
        {"赵六", "13800001005", "138****1005", 0, "汉族", "小学教师", "1988-04-12", "本科", "340104198804120000", "中共党员"},
        {"孙七", "13800001006", "138****1006", 1, "回族", "医生", "1975-09-30", "硕士", "340104197509300000", "群众"},
        {"周八", "13800001007", "138****1007", 0, "汉族", "退休会计", "1958-02-18", "大专", "340104195802180000", "中共党员"},
        {"吴九", "13800001008", "138****1008", 1, "汉族", "快递员", "1995-06-25", "大专", "340104199506250000", "群众"},
        {"郑十", "13800001009", "138****1009", 0, "汉族", "家庭主妇", "1990-12-03", "高中", "340104199012030000", "群众"},
        {"陈十一", "13800001010", "138****1010", 1, "汉族", "大学生", "2002-08-15", "在读本科", "340104200208150000", "共青团员"},
    };
    QList<qint64> residentIds;
    // 张三的ID
    QSqlQuery zsRq("SELECT id FROM cm_resident WHERE name = '张三' LIMIT 1");
    if (zsRq.next()) residentIds.append(zsRq.value(0).toLongLong());

    for (int i = 0; i < 8; i++)
    {
        QVariantMap data;
        data["name"] = QString::fromUtf8(residents[i].name);
        data["phone"] = QString::fromUtf8(residents[i].phone);
        data["phone_display"] = QString::fromUtf8(residents[i].phoneDisplay);
        data["gender"] = residents[i].gender;
        data["nationality"] = QString::fromUtf8(residents[i].nationality);
        data["occupation"] = QString::fromUtf8(residents[i].occupation);
        data["birthday"] = QString::fromUtf8(residents[i].birthday);
        data["education"] = QString::fromUtf8(residents[i].education);
        data["id_card"] = QString::fromUtf8(residents[i].idCard);
        data["political_status"] = QString::fromUtf8(residents[i].politicalStatus);
        data["status"] = 0;
        qint64 rid = db.insert("cm_resident", data);
        residentIds.append(rid);
    }

    Q_UNUSED(estate1Id)
    Q_UNUSED(estate2Id)
}

void DemoDataService::initFamilyData(DatabaseManager &db)
{
    // 获取房屋ID
    QSqlQuery hq("SELECT id FROM cm_house WHERE del_flag = 0 LIMIT 6");
    QList<qint64> houseIds;
    while (hq.next()) houseIds.append(hq.value(0).toLongLong());
    if (houseIds.size() < 3) return;

    // 获取居民ID
    QSqlQuery rq("SELECT id, name FROM cm_resident WHERE del_flag = 0 ORDER BY id LIMIT 9");
    QList<QPair<qint64, QString>> residentList;
    while (rq.next()) residentList.append({rq.value(0).toLongLong(), rq.value(1).toString()});
    if (residentList.size() < 9) return;

    // 家庭1: 张家 (residentList[0]=张三, [6]=郑十, [7]=陈十一)
    qint64 fam1Id = db.insert("cm_family", {
        {"family_name", QStringLiteral("张三家庭")}, {"house_id", houseIds[0]},
        {"head_resident_id", residentList[0].first}, {"member_count", 3}
    });
    db.insert("cm_family_member", {{"family_id", fam1Id}, {"resident_id", residentList[0].first}, {"relation", QStringLiteral("户主")}, {"is_head", 1}});
    db.insert("cm_family_member", {{"family_id", fam1Id}, {"resident_id", residentList[6].first}, {"relation", QStringLiteral("配偶")}, {"is_head", 0}});
    db.insert("cm_family_member", {{"family_id", fam1Id}, {"resident_id", residentList[7].first}, {"relation", QStringLiteral("子女")}, {"is_head", 0}});
    // 关联房屋
    db.insert("cm_house_resident", {{"house_id", houseIds[0]}, {"resident_id", residentList[0].first}, {"relation_type", 1}});
    db.insert("cm_house_resident", {{"house_id", houseIds[0]}, {"resident_id", residentList[6].first}, {"relation_type", 2}});
    db.insert("cm_house_resident", {{"house_id", houseIds[0]}, {"resident_id", residentList[7].first}, {"relation_type", 3}});

    // 家庭2: 李家 (residentList[1]=李四, [3]=赵六, [4]=孙七, [5]=周八)
    qint64 fam2Id = db.insert("cm_family", {
        {"family_name", QStringLiteral("李四家庭")}, {"house_id", houseIds[1]},
        {"head_resident_id", residentList[1].first}, {"member_count", 4}
    });
    db.insert("cm_family_member", {{"family_id", fam2Id}, {"resident_id", residentList[1].first}, {"relation", QStringLiteral("户主")}, {"is_head", 1}});
    db.insert("cm_family_member", {{"family_id", fam2Id}, {"resident_id", residentList[3].first}, {"relation", QStringLiteral("配偶")}, {"is_head", 0}});
    db.insert("cm_family_member", {{"family_id", fam2Id}, {"resident_id", residentList[4].first}, {"relation", QStringLiteral("父母")}, {"is_head", 0}});
    db.insert("cm_family_member", {{"family_id", fam2Id}, {"resident_id", residentList[5].first}, {"relation", QStringLiteral("父母")}, {"is_head", 0}});
    db.insert("cm_house_resident", {{"house_id", houseIds[1]}, {"resident_id", residentList[1].first}, {"relation_type", 1}});
    db.insert("cm_house_resident", {{"house_id", houseIds[1]}, {"resident_id", residentList[3].first}, {"relation_type", 2}});
    db.insert("cm_house_resident", {{"house_id", houseIds[1]}, {"resident_id", residentList[4].first}, {"relation_type", 3}});

    // 家庭3: 王家 (residentList[2]=王五, [8]=吴九)
    qint64 fam3Id = db.insert("cm_family", {
        {"family_name", QStringLiteral("王五家庭")}, {"house_id", houseIds[2]},
        {"head_resident_id", residentList[2].first}, {"member_count", 3}
    });
    db.insert("cm_family_member", {{"family_id", fam3Id}, {"resident_id", residentList[2].first}, {"relation", QStringLiteral("户主")}, {"is_head", 1}});
    db.insert("cm_family_member", {{"family_id", fam3Id}, {"resident_id", residentList[8].first}, {"relation", QStringLiteral("兄弟")}, {"is_head", 0}});
    db.insert("cm_house_resident", {{"house_id", houseIds[2]}, {"resident_id", residentList[2].first}, {"relation_type", 1}});
    db.insert("cm_house_resident", {{"house_id", houseIds[2]}, {"resident_id", residentList[8].first}, {"relation_type", 4}});
}
