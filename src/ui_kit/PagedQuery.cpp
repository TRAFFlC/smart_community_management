#include "PagedQuery.h"

#include "../database/DatabaseManager.h"

#include <QSqlQuery>

namespace UiKit {

PagedQuery::PagedQuery(const QString& baseSql)
    : m_baseSql(baseSql)
{
}

PagedQuery& PagedQuery::where(const QString& clause, const QVariantMap& binds)
{
    m_whereClauses << clause;
    for (auto it = binds.begin(); it != binds.end(); ++it)
        m_binds[it.key()] = it.value();
    return *this;
}

PagedQuery& PagedQuery::orderBy(const QString& clause)
{
    m_orderBy = clause;
    return *this;
}

PagedQuery::Result PagedQuery::execute(int pageSize, int offset)
{
    Result result;

    // 构建 WHERE 子句
    QString whereSql;
    if (!m_whereClauses.isEmpty())
        whereSql = " AND " + m_whereClauses.join(" AND ");

    // 构建 ORDER BY 子句
    QString orderSql;
    if (!m_orderBy.isEmpty())
        orderSql = " ORDER BY " + m_orderBy;

    // 1. 计数查询（不含 LIMIT/OFFSET）
    QString countSql = "SELECT COUNT(*) FROM (" + m_baseSql + whereSql + ")";
    QSqlQuery cntQ(DatabaseManager::instance().database());
    cntQ.prepare(countSql);
    for (auto it = m_binds.begin(); it != m_binds.end(); ++it)
        cntQ.bindValue(":" + it.key(), it.value());
    if (cntQ.exec() && cntQ.next())
        result.totalCount = cntQ.value(0).toInt();

    // 2. 数据查询（含 LIMIT/OFFSET）
    QString dataSql = m_baseSql + whereSql + orderSql + " LIMIT :pageSize OFFSET :offset";
    QSqlQuery dataQ(DatabaseManager::instance().database());
    dataQ.prepare(dataSql);
    for (auto it = m_binds.begin(); it != m_binds.end(); ++it)
        dataQ.bindValue(":" + it.key(), it.value());
    dataQ.bindValue(":pageSize", pageSize);
    dataQ.bindValue(":offset", offset);
    if (dataQ.exec()) {
        while (dataQ.next())
            result.rows << dataQ.record();
    }

    return result;
}

} // namespace UiKit
