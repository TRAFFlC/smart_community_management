#ifndef UIKIT_DB_HELPERS_H
#define UIKIT_DB_HELPERS_H

#include <QString>
#include <QVariantList>

namespace UiKit {

// 统计查询辅助：剥离 LIMIT/OFFSET，绑定过滤参数，返回行数
int executeCountQuery(const QString& dataSql, const QVariantList& bindPairs);

} // namespace UiKit

#endif // UIKIT_DB_HELPERS_H
