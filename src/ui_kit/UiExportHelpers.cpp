#include "UiKit.h"

#include <QFile>
#include <QFileDialog>
#include <QStringConverter>
#include <QTableWidget>
#include <QTextStream>

namespace UiKit {

bool exportToCsv(const QString& filename, QTableWidget* table) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
    out << "\xEF\xBB\xBF"; // BOM for Excel
    for (int c = 0; c < table->columnCount(); ++c) {
        if (c > 0) out << ",";
        QString hdr = table->horizontalHeaderItem(c) ? table->horizontalHeaderItem(c)->text() : "";
        out << "\"" << hdr.replace("\"", "\"\"") << "\"";
    }
    out << "\n";
    for (int r = 0; r < table->rowCount(); ++r) {
        for (int c = 0; c < table->columnCount(); ++c) {
            if (c > 0) out << ",";
            QString text = table->item(r, c) ? table->item(r, c)->text() : "";
            out << "\"" << text.replace("\"", "\"\"") << "\"";
        }
        out << "\n";
    }
    file.close();
    return true;
}

void exportTableToCsv(QTableWidget* table, const QString& defaultName, QWidget* parent) {
    QString path = QFileDialog::getSaveFileName(parent, QStringLiteral("导出 CSV"),
        defaultName, QStringLiteral("CSV 文件 (*.csv)"));
    if (path.isEmpty()) return;
    if (!path.endsWith(".csv", Qt::CaseInsensitive)) path += ".csv";
    if (exportToCsv(path, table)) {
        showToast(QStringLiteral("导出成功"), parent);
    } else {
        showToast(QStringLiteral("导出失败"), parent);
    }
}

} // namespace UiKit
