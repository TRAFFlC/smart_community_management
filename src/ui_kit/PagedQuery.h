#pragma once

#include <QList>
#include <QSqlRecord>
#include <QString>
#include <QVariantMap>

namespace UiKit {

/// 分页查询工具类
/// 消除手写 SQL + LIMIT/OFFSET + 计数 SQL 的重复与 Bug
///
/// 用法：
///   PagedQuery q("SELECT id, title FROM sv_job_posting WHERE del_flag = 0");
///   if (!search.isEmpty())
///       q.where("(title LIKE :search OR company LIKE :search)", {{"search", "%" + search + "%"}});
///   if (status >= 0)
///       q.where("status = :status", {{"status", status}});
///   q.orderBy("create_time DESC");
///   auto result = q.execute(pageSize, offset);
///   pb->setTotalCount(result.totalCount);
///   for (const auto& row : result.rows) { /* 填表 */ }
class PagedQuery {
public:
    struct Result {
        QList<QSqlRecord> rows;
        int totalCount = 0;
    };

    explicit PagedQuery(const QString& baseSql);

    /// 添加 WHERE 条件（自动加 AND 前缀）
    /// clause 中的占位符以 :开头，binds 是 占位符名(不含:) -> 值 的映射
    PagedQuery& where(const QString& clause, const QVariantMap& binds = {});

    /// 设置 ORDER BY 子句（不含 "ORDER BY" 关键字）
    PagedQuery& orderBy(const QString& clause);

    /// 执行查询，返回数据行和总计数
    /// pageSize: 每页条数; offset: 偏移量
    Result execute(int pageSize, int offset);

private:
    QString m_baseSql;        // 不含 WHERE/ORDER BY/LIMIT/OFFSET
    QStringList m_whereClauses;
    QVariantMap m_binds;
    QString m_orderBy;
};

} // namespace UiKit
