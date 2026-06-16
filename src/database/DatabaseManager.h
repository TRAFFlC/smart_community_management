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
        QStringList columns, placeholders;
        for (auto it = data.begin(); it != data.end(); ++it) {
            columns << it.key();
            placeholders << (":" + it.key());
        }
        QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(table, columns.join(", "), placeholders.join(", "));

        QSqlQuery q;
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
        QStringList sets;
        for (auto it = data.begin(); it != data.end(); ++it) {
            sets << (it.key() + " = :" + it.key());
        }
        QString sql = QString("UPDATE %1 SET %2 WHERE id = :id").arg(table, sets.join(", "));

        QSqlQuery q;
        q.prepare(sql);
        for (auto it = data.begin(); it != data.end(); ++it) {
            q.bindValue(":" + it.key(), it.value());
        }
        q.bindValue(":id", id);
        if (!q.exec()) {
            qWarning() << "Update failed:" << q.lastError().text() << "\nSQL:" << sql;
            return false;
        }
        return true;
    }

    // 软删除
    bool softDelete(const QString& table, qint64 id) {
        return update(table, id, {{"del_flag", 1}, {"update_time", QDateTime::currentDateTime()}});
    }

    // 计数查询
    int count(const QString& table, const QString& where = "1=1", const QVariantMap& params = {}) {
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
        QSqlQuery q("SELECT COUNT(*) FROM " + table);
        if (q.exec() && q.next()) return q.value(0).toInt() > 0;
        return false;
    }

    void close() {
        if (m_db.isOpen()) m_db.close();
    }

private:
    DatabaseManager() = default;
    ~DatabaseManager() { close(); }
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
