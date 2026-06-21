#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

// 结构化 SQL 条件，用于安全地构建 WHERE 子句（杜绝字符串拼接）
struct SqlCondition {
    QString column; // 列名，仅允许字母/数字/下划线
    QString op;     // 操作符，仅允许 = != <> < > <= >= LIKE
    QVariant value; // 值，通过参数化绑定传入

    SqlCondition() = default;
    SqlCondition(const QString& col, const QString& oper, const QVariant& val)
        : column(col), op(oper), value(val) {}
};

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager& instance();

    bool initialize(const QString& dbPath = "smart_community.db");
    bool initSchema();
    QSqlDatabase database();

    // 通用查询
    QSqlQuery query(const QString& sql, const QVariantMap& params = {});

    // 通用插入，返回新ID
    qint64 insert(const QString& table, const QVariantMap& data);

    // 通用更新
    bool update(const QString& table, qint64 id, const QVariantMap& data);

    // 软删除
    bool softDelete(const QString& table, qint64 id);

    // 计数查询（全量非删除记录）
    int count(const QString& table);

    // 计数查询（结构化条件，自动参数化绑定，杜绝 SQL 注入）
    int count(const QString& table, const QList<SqlCondition>& conditions);

    // 检查表是否有数据（表名经白名单校验）
    bool tableHasData(const QString& table);

    void close();

private:
    // 校验标识符：仅允许字母、数字、下划线
    static bool isValidIdentifier(const QString& name);

    // 校验表名是否在已知白名单内（覆盖 schema.sql 中所有表）
    static bool isKnownTable(const QString& table);

    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
