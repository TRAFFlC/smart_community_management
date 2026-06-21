#include "DatabaseManager.h"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QResource>
#include <QSet>
#include <QSqlError>

// 校验 WHERE 操作符白名单，防止结构化条件被绕过
static bool isAllowedOperator(const QString& op) {
    static const QSet<QString> allowedOps = {"=", "!=", "<>", "<", ">", "<=", ">=", "LIKE"};
    return allowedOps.contains(op);
}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager() = default;
DatabaseManager::~DatabaseManager() { close(); }

bool DatabaseManager::initialize(const QString& dbPath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qCritical() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    qDebug() << "Database opened:" << dbPath;
    return true;
}

bool DatabaseManager::initSchema() {
    // 优先从 Qt 资源文件读取，消除部署时相对路径依赖
    QResource resSchema(":/schema.sql");
    QString sql;
    if (resSchema.isValid() && resSchema.size() > 0) {
        QByteArray rawData;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        rawData = resSchema.uncompressedData();
#else
        rawData = QByteArray(reinterpret_cast<const char*>(resSchema.data()),
                             static_cast<int>(resSchema.size()));
#endif
        sql = QString::fromUtf8(rawData);
        qDebug() << "Schema loaded from resource :/schema.sql (" << rawData.size() << " bytes)";
    } else {
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

        sql = file.readAll();
        file.close();
    }

    // 更健壮地拆分 SQL：忽略字符串字面量中的分号和行内注释
    QStringList statements;
    QString current;
    bool inString = false;
    for (int i = 0; i < sql.size(); ++i) {
        QChar c = sql.at(i);
        if (c == '\'' && (i == 0 || sql.at(i - 1) != '\\')) {
            inString = !inString;
        }
        if (!inString && c == ';') {
            statements.append(current.trimmed());
            current.clear();
        } else {
            current.append(c);
        }
    }
    if (!current.trimmed().isEmpty())
        statements.append(current.trimmed());

    for (const auto& stmt : statements) {
        QString trimmed = stmt;
        if (trimmed.isEmpty()) continue;
        // 跳过纯注释块
        QStringList lines = trimmed.split('\n');
        QStringList useful;
        for (const QString& line : lines) {
            QString l = line.trimmed();
            if (!l.isEmpty() && !l.startsWith("--"))
                useful.append(line);
        }
        if (useful.isEmpty()) continue;
        trimmed = useful.join("\n").trimmed();
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

QSqlDatabase DatabaseManager::database() { return m_db; }

QSqlQuery DatabaseManager::query(const QString& sql, const QVariantMap& params) {
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

qint64 DatabaseManager::insert(const QString& table, const QVariantMap& data) {
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

bool DatabaseManager::update(const QString& table, qint64 id, const QVariantMap& data) {
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

bool DatabaseManager::softDelete(const QString& table, qint64 id) {
    return update(table, id, QVariantMap{{"del_flag", 1}, {"update_time", QDateTime::currentDateTime()}});
}

int DatabaseManager::count(const QString& table) {
    return count(table, QList<SqlCondition>{});
}

int DatabaseManager::count(const QString& table, const QList<SqlCondition>& conditions) {
    if (!isKnownTable(table)) {
        qWarning() << "Count rejected: unknown table" << table;
        return 0;
    }

    // 无 del_flag 字段的关联表/日志表白名单
    static const QSet<QString> noDelFlagTables = {
        "sys_user_role", "sys_role_menu", "sys_user_org",
        "ge_event_action", "wo_work_order_log",
        "sv_volunteer_signup", "sv_job_application",
        "oc_vote", "oc_vote_record",
        "pm_payment", "ev_evaluation"
    };

    QStringList whereParts;
    QVariantMap params;
    for (int i = 0; i < conditions.size(); ++i) {
        const auto& c = conditions[i];
        if (!isValidIdentifier(c.column)) {
            qWarning() << "Count rejected: invalid column name" << c.column;
            return 0;
        }
        if (!isAllowedOperator(c.op)) {
            qWarning() << "Count rejected: invalid operator" << c.op;
            return 0;
        }
        const QString key = QStringLiteral(":c%1").arg(i);
        whereParts << QStringLiteral("%1 %2 %3").arg(c.column, c.op, key);
        params[key] = c.value;
    }

    // 仅对有 del_flag 字段的表添加软删除过滤
    QString sql;
    if (noDelFlagTables.contains(table)) {
        sql = QStringLiteral("SELECT COUNT(*) FROM %1 WHERE 1=1").arg(table);
    } else {
        sql = QStringLiteral("SELECT COUNT(*) FROM %1 WHERE del_flag = 0").arg(table);
    }
    if (!whereParts.isEmpty()) {
        sql += QStringLiteral(" AND ") + whereParts.join(QStringLiteral(" AND "));
    }

    QSqlQuery q;
    q.prepare(sql);
    for (auto it = params.begin(); it != params.end(); ++it) {
        q.bindValue(it.key(), it.value());
    }
    if (q.exec() && q.next()) return q.value(0).toInt();
    return 0;
}

bool DatabaseManager::tableHasData(const QString& table) {
    if (!isKnownTable(table)) {
        qWarning() << "tableHasData rejected: unknown table" << table;
        return false;
    }
    return count(table) > 0;
}

void DatabaseManager::close() {
    if (m_db.isOpen()) m_db.close();
}

bool DatabaseManager::isValidIdentifier(const QString& name) {
    if (name.isEmpty()) return false;
    for (const QChar& c : name) {
        if (!(c.isLetterOrNumber() || c == '_')) return false;
    }
    return true;
}

bool DatabaseManager::isKnownTable(const QString& table) {
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
