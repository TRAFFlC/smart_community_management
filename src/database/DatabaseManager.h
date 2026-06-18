#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QSet>
#include "models/Models.h"

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager& instance() {
        static DatabaseManager inst;
        return inst;
    }

    bool initialize(const QString& dbPath = "smart_community.db") {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        m_db.setDatabaseName(dbPath);
        if (!m_db.open()) {
            qCritical() << "Failed to open database:" << m_db.lastError().text();
            return false;
        }
        qDebug() << "Database opened:" << dbPath;
        return true;
    }

    bool initSchema() {
        // 查找SQL文件
        QStringList searchPaths = {
            QApplication::applicationDirPath() + "/../sql/schema.sql",
            QApplication::applicationDirPath() + "/sql/schema.sql",
            "sql/schema.sql",
            "../sql/schema.sql",
            "../../sql/schema.sql"
        };

        QString schemaFile;
        for (const auto& path : searchPaths) {
            if (QFile::exists(path)) {
                schemaFile = path;
                break;
            }
        }

        if (schemaFile.isEmpty()) {
            qWarning() << "Schema file not found, tables may not exist";
            return false;
        }

        QFile file(schemaFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Cannot read schema file:" << schemaFile;
            return false;
        }

        QString sql = file.readAll();
        file.close();

        // 按分号分割并执行每条SQL
        QStringList statements = sql.split(';', Qt::SkipEmptyParts);
        for (const auto& stmt : statements) {
            QString trimmed = stmt.trimmed();
            if (trimmed.isEmpty()) continue;
            // 去除语句开头的注释行（如 "-- ========== ... =========="）
            while (trimmed.startsWith("--")) {
                int nl = trimmed.indexOf('\n');
                if (nl < 0) { trimmed.clear(); break; }
                trimmed = trimmed.mid(nl + 1).trimmed();
            }
            if (trimmed.isEmpty()) continue;
            QSqlQuery query;
            if (!query.exec(trimmed)) {
                qWarning() << "SQL exec warning:" << query.lastError().text()
                           << "\nStatement:" << trimmed.left(100);
            }
        }
        qDebug() << "Schema initialized successfully";
        return true;
    }

    QSqlDatabase database() { return m_db; }

    // 通用查询
    QSqlQuery query(const QString& sql, const QVariantMap& params = {}) {
        QSqlQuery q;
        q.prepare(sql);
        for (auto it = params.begin(); it != params.end(); ++it) {
            q.bindValue(it.key(), it.value());
        }
        if (!q.exec()) {
            qWarning() << "Query failed:" << q.lastError().text() << "\nSQL:" << sql;
        }
        return q;
    }

    // 通用插入，返回新ID
    qint64 insert(const QString& table, const QVariantMap& data) {
        if (!isKnownTable(table)) {
            qWarning() << "Insert rejected: unknown table" << table;
            return -1;
        }
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (!isValidIdentifier(it.key())) {
                qWarning() << "Insert rejected: invalid column name" << it.key() << "in table" << table;
                return -1;
            }
        }
        QStringList columns, placeholders;
        for (auto it = data.begin(); it != data.end(); ++it) {
            columns << it.key();
            placeholders << (":" + it.key());
        }
        QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(table, columns.join(", "), placeholders.join(", "));

        QSqlQuery q(m_db);
        q.prepare(sql);
        for (auto it = data.begin(); it != data.end(); ++it) {
            q.bindValue(":" + it.key(), it.value());
        }
        if (!q.exec()) {
            qWarning() << "Insert failed:" << q.lastError().text() << "\nSQL:" << sql;
            return -1;
        }
        return q.lastInsertId().toLongLong();
    }

    // 通用更新
    bool update(const QString& table, qint64 id, const QVariantMap& data) {
        if (!isKnownTable(table)) {
            qWarning() << "Update rejected: unknown table" << table;
            return false;
        }
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (!isValidIdentifier(it.key())) {
                qWarning() << "Update rejected: invalid column name" << it.key() << "in table" << table;
                return false;
            }
        }
        QStringList sets;
        for (auto it = data.begin(); it != data.end(); ++it) {
            sets << (it.key() + " = :" + it.key());
        }
        QString sql = QString("UPDATE %1 SET %2 WHERE id = :id").arg(table, sets.join(", "));

        QSqlQuery q(m_db);
        q.prepare(sql);
        for (auto it = data.begin(); it != data.end(); ++it) {
            q.bindValue(":" + it.key(), it.value());
        }
        q.bindValue(":id", id);
        if (!q.exec()) {
            qWarning() << "Update failed:" << q.lastError().text() << "\nSQL:" << sql;
            return false;
        }
        return q.numRowsAffected() > 0;
    }

    // 软删除
    bool softDelete(const QString& table, qint64 id) {
        return update(table, id, {{"del_flag", 1}, {"update_time", QDateTime::currentDateTime()}});
    }

    /**
     * 计数查询
     *
     * 注意：where 子句会直接拼入 SQL，调用方必须保证 where 内容由程序内部构造，
     * 严禁拼接任何来自用户输入的内容；用户可控的值应通过 params 以参数化绑定方式传入。
     */
    int count(const QString& table, const QString& where = "1=1", const QVariantMap& params = {}) {
        if (!isKnownTable(table)) {
            qWarning() << "Count rejected: unknown table" << table;
            return 0;
        }
        QString sql = QString("SELECT COUNT(*) FROM %1 WHERE %2 AND del_flag = 0").arg(table, where);
        QSqlQuery q;
        q.prepare(sql);
        for (auto it = params.begin(); it != params.end(); ++it) {
            q.bindValue(it.key(), it.value());
        }
        if (q.exec() && q.next()) return q.value(0).toInt();
        return 0;
    }

    // 检查表是否有数据
    bool tableHasData(const QString& table) {
        if (!isKnownTable(table)) {
            qWarning() << "tableHasData rejected: unknown table" << table;
            return false;
        }
        QSqlQuery q("SELECT COUNT(*) FROM " + table);
        if (q.exec() && q.next()) return q.value(0).toInt() > 0;
        return false;
    }

    void close() {
        if (m_db.isOpen()) m_db.close();
    }

private:
    // 校验标识符：仅允许字母、数字、下划线
    static bool isValidIdentifier(const QString& name) {
        if (name.isEmpty()) return false;
        for (const QChar& c : name) {
            if (!(c.isLetterOrNumber() || c == '_')) return false;
        }
        return true;
    }

    // 校验表名是否在已知白名单内（覆盖 schema.sql 中所有表）
    static bool isKnownTable(const QString& table) {
        static const QSet<QString> knownTables = {
            // sys_ 系统管理
            "sys_user", "sys_role", "sys_user_role", "sys_menu", "sys_role_menu",
            "sys_org", "sys_user_org", "sys_dict_type", "sys_dict_data",
            "sys_config", "sys_operation_log", "sys_login_log", "sys_file",
            // cm_ 社区/基础档案
            "cm_estate", "cm_building", "cm_unit", "cm_house", "cm_resident",
            "cm_family", "cm_family_member", "cm_house_resident", "cm_vehicle",
            "cm_vehicle_owner", "cm_parking_space", "cm_facility", "cm_grid",
            "cm_property_company", "cm_owner_committee", "cm_owner_committee_member",
            "cm_special_group", "cm_social_org", "cm_visitor",
            // wo_ 工单管理
            "wo_work_order",
            // ge_ 社区治理/事件
            "ge_event", "ge_event_flow", "ge_inspection", "ge_inspection_plan",
            "ge_visit_record", "ge_supervision", "ge_opinion",
            // sv_ 社区服务
            "sv_volunteer", "sv_volunteer_activity", "sv_volunteer_signup",
            "sv_service_provider", "sv_service_order", "sv_job_posting",
            "sv_job_application",
            // nt_ 通知公告
            "nt_announcement", "nt_notification",
            // oc_ 业委会/自治
            "oc_topic", "oc_vote", "oc_public_income",
            // ev_ 评价
            "ev_evaluation",
            // ai_ 智能助手
            "ai_knowledge", "ai_chat_log",
            // kf_ 考核
            "kf_assessment_config", "kf_assessment_result",
            // pm_ 物业管理
            "pm_bill", "pm_monthly_card"
        };
        return knownTables.contains(table);
    }

    DatabaseManager() = default;
    ~DatabaseManager() { close(); }
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
