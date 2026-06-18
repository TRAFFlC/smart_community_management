#include "DbHelpers.h"

#include "../database/DatabaseManager.h"

#include <QRegularExpression>
#include <QSqlQuery>

namespace UiKit {

int executeCountQuery(const QString& dataSql, const QVariantList& bindPairs) {
    QString innerSql = dataSql;
    QRegularExpression limitRe("\\s+LIMIT\\s+:\\w+(\\s+OFFSET\\s+:\\w+)?\\s*$",
                               QRegularExpression::CaseInsensitiveOption);
    innerSql.replace(limitRe, "");
    QSqlQuery cntQ(DatabaseManager::instance().database());
    cntQ.prepare("SELECT COUNT(*) FROM (" + innerSql + ")");
    for (int i = 0; i + 1 < bindPairs.size(); i += 2)
        cntQ.bindValue(bindPairs[i].toString(), bindPairs[i + 1]);
    if (!cntQ.exec()) return 0;
    return cntQ.next() ? cntQ.value(0).toInt() : 0;
}

} // namespace UiKit
