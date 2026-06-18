#include "UiKit.h"

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QStringConverter>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

namespace UiKit {

const char* TABLE_STYLE = R"(
    QTableWidget {
        background: #ffffff; border: 1px solid #e2e8f0; border-radius: 4px;
        gridline-color: transparent; font-size: 13px;
    }
    QTableWidget::item {
        padding: 10px 14px;
        border-bottom: 1px solid #f1f5f9;
        min-height: 44px;
    }
    QTableWidget::item:alternate { background: #fafafa; }
    QTableWidget::item:hover { background: #f1f5f9; }
    QTableWidget::item:selected { background: #fff7ed; color: #b45309; }
    QHeaderView::section {
        background: #f1f5f9; color: #475569; font-weight: 600;
        padding: 10px 14px; border: none;
        border-bottom: 1px solid #e2e8f0;
        font-size: 11px;
    }
    QHeaderView::section:hover { background: #e2e8f0; }
    QHeaderView::section:pressed { background: #fff7ed; }
    QTableWidget QPushButton, QTreeWidget QPushButton,
    QTableWidget QWidget QPushButton, QTreeWidget QWidget QPushButton {
        min-height: 24px !important; max-height: 28px !important;
        min-width: 0px; max-width: 80px;
        padding: 2px 8px !important; font-size: 12px;
        border: 1px solid #e2e8f0; border-radius: 4px;
        background: #ffffff; color: #334155;
    }
    QTableWidget QPushButton:hover, QTreeWidget QPushButton:hover,
    QTableWidget QWidget QPushButton:hover, QTreeWidget QWidget QPushButton:hover {
        background: #f8fafc; border-color: #cbd5e1; color: #0f172a;
    }
    QWidget#actionCell { max-height: 32px; background: transparent; }
)";

void applyCardShadow(QWidget* card) {
    // 使用样式表边框模拟阴影，避免 QGraphicsDropShadowEffect 在 Qt6 上触发
    // "A paint device can only be painted by one painter at a time" 错误
    card->setGraphicsEffect(nullptr);
}

QPixmap tintSvgIcon(const QString& iconKey, const QString& color, const QSize& size) {
    QPixmap pix(QString(":/icons/%1.svg").arg(iconKey));
    if (!pix.isNull()) {
        QImage img = pix.toImage();
        QPainter painter(&img);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(img.rect(), QColor(color));
        painter.end();
        return QPixmap::fromImage(img).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return QPixmap();
}

QString moduleColor(const QString& sub) {
    static const QMap<QString, QString> colors = {
        // 档案/基础
        {"org", "#64748b"}, {"estate", "#64748b"}, {"house", "#64748b"}, {"resident", "#64748b"},
        {"vehicle", "#64748b"}, {"facility", "#64748b"}, {"grid", "#64748b"}, {"special", "#64748b"},
        // 工单/物业
        {"workorder", "#b45309"}, {"complaint", "#b45309"}, {"inspection", "#b45309"},
        {"announcement", "#b45309"}, {"visitor", "#b45309"}, {"topic", "#b45309"},
        {"parking", "#b45309"}, {"billing", "#b45309"}, {"income", "#b45309"},
        // 治理/事件
        {"event", "#2563eb"}, {"care", "#2563eb"}, {"supervision", "#2563eb"},
        {"opinion", "#2563eb"}, {"assessment", "#2563eb"},
        // 服务/志愿
        {"volunteer", "#15803d"}, {"convenience", "#15803d"}, {"job", "#15803d"},
        // 系统/AI
        {"user", "#475569"}, {"role", "#475569"}, {"menu", "#475569"},
        {"dict", "#475569"}, {"log", "#475569"}, {"ai", "#475569"},
        // 聚合/看板
        {"dashboard", "#b45309"}, {"todo", "#b45309"}, {"message", "#b45309"},
    };
    return colors.value(sub, "#64748b");
}

QWidget* createPageHeader(const QString& iconKey, const QString& title, const QString& desc, const QString& color, QWidget* parent) {
    auto* header = new QFrame(parent);
    header->setFixedHeight(64);
    header->setStyleSheet(QStringLiteral(
        "QFrame { background: #ffffff; border: none; border-bottom: 1px solid #e2e8f0; border-radius: 0; }"
        "QLabel { background: transparent; border: none; }"
    ));
    auto* hl = new QHBoxLayout(header);
    hl->setContentsMargins(20, 0, 20, 0);
    hl->setSpacing(8);

    if (!iconKey.isEmpty()) {
        auto* iconLabel = new QLabel(header);
        iconLabel->setFixedSize(20, 20);
        iconLabel->setPixmap(tintSvgIcon(iconKey, color, QSize(20, 20)));
        hl->addWidget(iconLabel);
    }

    if (!color.isEmpty()) {
        auto* dotLabel = new QLabel(header);
        dotLabel->setFixedSize(8, 8);
        dotLabel->setStyleSheet(QStringLiteral("background: %1; border-radius: 4px;").arg(color));
        hl->addWidget(dotLabel);
    }

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);
    auto* titleLabel = new QLabel(title, header);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 700; color: #0f172a;"));
    textLayout->addWidget(titleLabel);
    auto* descLabel = new QLabel(desc, header);
    descLabel->setStyleSheet(QStringLiteral("font-size: 13px; color: #64748b;"));
    textLayout->addWidget(descLabel);
    hl->addLayout(textLayout, 1);
    return header;
}

QTableWidgetItem* createTagTableItem(const QString& text, const QColor& bg, const QColor& fg) {
    auto* item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignCenter);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);

    QColor textColor = fg.isValid() ? fg : bg.darker(160);
    QColor bgAlpha = bg;
    bgAlpha.setAlpha(34);

    item->setForeground(QBrush(textColor));
    item->setBackground(QBrush(bgAlpha));
    return item;
}

void applyPrimaryButton(QPushButton* btn) {
    btn->setProperty("cssClass", "primary");
    btn->setStyleSheet(
        "QPushButton { background: #b45309; color: #fff; border: 1px solid #b45309; border-radius: 4px; "
        "padding: 7px 16px; font-size: 14px; font-weight: 500; min-height: 36px; }"
        "QPushButton:hover { background: #d97706; border-color: #d97706; }"
        "QPushButton:pressed { background: #92400e; border-color: #92400e; }"
    );
}

void applySmallButton(QPushButton* btn) {
    btn->setProperty("cssClass", "small");
    btn->setStyleSheet(
        "QPushButton { min-height: 28px; padding: 4px 12px; font-size: 13px; border-radius: 4px; }"
    );
}

void applyTextButton(QPushButton* btn) {
    btn->setProperty("cssClass", "text");
    btn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #b45309; padding: 4px 8px; "
        "font-size: 13px; min-height: 28px; }"
        "QPushButton:hover { background: #e6f4ff; }"
    );
}

void applyDangerButton(QPushButton* btn) {
    btn->setProperty("cssClass", "danger");
    btn->setStyleSheet(
        "QPushButton { background: #b91c1c; color: #fff; border: 1px solid #b91c1c; border-radius: 4px; "
        "padding: 7px 16px; font-size: 14px; font-weight: 500; min-height: 36px; }"
        "QPushButton:hover { background: #dc2626; border-color: #dc2626; }"
    );
}

QPushButton* createPillButton(const QString& text, const QString& color, QWidget* parent) {
    auto* btn = new QPushButton(text, parent);
    const QString accent = color.isEmpty() ? QStringLiteral("#334155") : color;
    const QString border = color.isEmpty() ? QStringLiteral("#e2e8f0") : accent;
    const QString hoverBg = color.isEmpty() ? QStringLiteral("#f8fafc") : (accent + QStringLiteral("0D"));
    const QString hoverBorder = color.isEmpty() ? QStringLiteral("#cbd5e1") : accent;
    btn->setStyleSheet(QString(
        "QPushButton { background: #ffffff; color: %1; border: 1px solid %2; border-radius: 4px;"
        "  padding: 2px 8px !important; font-size: 12px; font-weight: 500;"
        "  min-height: 24px !important; max-height: 24px !important;"
        "  min-width: 0px; max-width: 80px; }"
        "QPushButton:hover { background: %3; border-color: %4; }"
    ).arg(accent, border, hoverBg, hoverBorder));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(24);
    btn->setMaximumWidth(80);
    btn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    return btn;
}

QWidget* createActionCell(const QList<QPushButton*>& buttons, QWidget* parent) {
    auto* container = new QWidget(parent);
    container->setStyleSheet("background: transparent; border: none;");
    container->setObjectName("actionCell");
    auto* lay = new QHBoxLayout(container);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(8);
    lay->setAlignment(Qt::AlignCenter);
    for (auto* btn : buttons) { btn->setParent(container); lay->addWidget(btn); }
    container->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    container->setFixedHeight(32);
    return container;
}

QTableWidgetItem* createActionItem(const QString& text, const QString& color,
                                   qint64 entityId, const QVariant& actionData) {
    auto* item = new QTableWidgetItem(text);
    item->setForeground(QColor(color));
    item->setTextAlignment(Qt::AlignCenter);
    QFont f; f.setPointSize(12); f.setBold(false); f.setWeight(QFont::Medium); item->setFont(f);
    item->setData(Qt::UserRole, entityId);
    if (actionData.isValid()) item->setData(Qt::UserRole + 2, actionData);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

QWidget* createEmptyState(const QString& iconKey, const QString& message, const QString& hint, QWidget* parent) {
    auto* empty = new QWidget(parent);
    empty->setMinimumHeight(200);
    auto* layout = new QVBoxLayout(empty);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(12);
    auto* iconLabel = new QLabel(empty);
    iconLabel->setFixedSize(48, 48);
    QPixmap iconPix(QString(":/icons/%1.svg").arg(iconKey));
    if (!iconPix.isNull()) {
        QImage img = iconPix.toImage();
        QPainter painter(&img);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(img.rect(), QColor("#cbd5e1"));
        painter.end();
        iconLabel->setPixmap(QPixmap::fromImage(img).scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background: transparent; border: none;");
    layout->addWidget(iconLabel, 0, Qt::AlignCenter);
    auto* msgLabel = new QLabel(message, empty);
    msgLabel->setStyleSheet("font-size: 15px; color: #64748b; background: transparent; border: none;");
    msgLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(msgLabel);
    if (!hint.isEmpty()) {
        auto* hintLabel = new QLabel(hint, empty);
        hintLabel->setStyleSheet("font-size: 13px; color: #94a3b8; background: transparent; border: none;");
        hintLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(hintLabel);
    }
    empty->setVisible(false);
    return empty;
}

QLabel* createEmptyHintLabel(const QString& text, QWidget* parent) {
    auto* lbl = new QLabel(text, parent);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("color: #64748b; font-size: 14px; padding: 40px; background: #ffffff; border-radius: 6px; border: 1px solid #f5f5f5;");
    lbl->setMinimumHeight(160);
    lbl->setVisible(false);
    return lbl;
}

void syncEmptyHint(QTableWidget* table, QLabel* hint) {
    bool empty = (table->rowCount() == 0);
    table->setVisible(!empty);
    hint->setVisible(empty);
}

void fadeInWidget(QWidget* widget) {
    if (widget) widget->show();
}

void showToast(const QString& msg, QWidget* parent) {
    if (!parent) parent = qobject_cast<QWidget*>(QApplication::activeWindow());
    if (!parent) return;

    auto* toast = new QFrame(parent);
    toast->setObjectName("toastFrame");
    toast->setStyleSheet(
        "QFrame#toastFrame { background: #f6ffed; border: 1px solid #b7eb8f; "
        "border-radius: 6px; }"
        "QLabel { background: transparent; border: none; color: #14532d; font-size: 14px; }"
    );

    auto* layout = new QHBoxLayout(toast);
    layout->setContentsMargins(16, 10, 20, 10);
    layout->setSpacing(8);

    auto* iconLbl = new QLabel;
    QPixmap pix(20, 20);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#15803d"));
    p.drawEllipse(0, 0, 20, 20);
    p.setPen(QPen(QColor("white"), 2));
    p.drawLine(5, 10, 9, 14);
    p.drawLine(9, 14, 15, 6);
    p.end();
    iconLbl->setPixmap(pix);
    layout->addWidget(iconLbl);

    auto* textLbl = new QLabel(msg);
    layout->addWidget(textLbl);

    toast->adjustSize();
    int x = parent->width() - toast->width() - 24;
    int y = 80;
    toast->move(x, y);
    toast->show();
    toast->raise();

    auto* opacity = new QGraphicsOpacityEffect(toast);
    toast->setGraphicsEffect(opacity);
    QPoint endPos = toast->pos();
    QPoint startPos = QPoint(endPos.x() + 30, endPos.y());

    auto* slideIn = new QPropertyAnimation(toast, "pos", toast);
    slideIn->setDuration(200);
    slideIn->setStartValue(startPos);
    slideIn->setEndValue(endPos);
    slideIn->setEasingCurve(QEasingCurve::OutCubic);

    auto* fadeIn = new QPropertyAnimation(opacity, "opacity", toast);
    fadeIn->setDuration(200);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    auto* fadeOut = new QPropertyAnimation(opacity, "opacity", toast);
    fadeOut->setDuration(300);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    QTimer::singleShot(2000, toast, [fadeOut]() { fadeOut->start(); });
    QObject::connect(fadeOut, &QPropertyAnimation::finished, toast, &QObject::deleteLater);
    fadeIn->start();
    slideIn->start();
}

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

bool QuickHoverFilter::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Enter) {
        m_iconLabel->setPixmap(UiKit::tintSvgIcon(m_iconKey, "#b45309", QSize(32, 32)));
    } else if (event->type() == QEvent::Leave) {
        m_iconLabel->setPixmap(UiKit::tintSvgIcon(m_iconKey, "#64748b", QSize(32, 32)));
    }
    return QObject::eventFilter(watched, event);
}
