#include "UiKit.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPalette>
#include <QPushButton>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QWidget>

namespace {

// 操作链接项：覆盖 data() 保证前景色/下划线字体不被 QTableWidget 的 QSS 覆盖
class ActionLinkItem : public QTableWidgetItem {
public:
    ActionLinkItem(const QString& text, const QColor& fg)
        : QTableWidgetItem(text), m_fg(fg) {
        setTextAlignment(Qt::AlignCenter);
        QFont f = font();
        f.setPointSize(12);
        f.setBold(false);
        f.setWeight(QFont::Medium);
        f.setUnderline(true);
        setFont(f);
        setFlags(flags() & ~Qt::ItemIsEditable);
    }

    QVariant data(int role) const override {
        if (role == Qt::ForegroundRole && m_fg.isValid())
            return QVariant(QBrush(m_fg));
        return QTableWidgetItem::data(role);
    }

private:
    QColor m_fg;
};

} // namespace

namespace UiKit {

const char* TABLE_STYLE = R"(
    QTableWidget {
        background: #FAF9F6; border: 1px solid #D4D0C8; border-radius: 0;
        gridline-color: transparent; font-size: 13px; color: #141413;
    }
    QTableWidget::item {
        padding: 10px 14px;
        border-bottom: 1px solid #E8E5DE;
    }
    QTableWidget::item:alternate { background: #F5F2EB; }
    QTableWidget::item:hover { background: #F5F2EB; }
    QTableWidget::item:selected { background: #141413; color: #FAF9F6; }
    QHeaderView::section {
        background: #141413; color: #FAF9F6; font-weight: 600;
        padding: 10px 14px; border: none;
        border-right: 1px solid #2A2A2A;
        font-size: 11px; letter-spacing: 1px;
    }
    QHeaderView::section:hover { background: #2A2A2A; }
    QHeaderView::section:pressed { background: #0A0A0A; }
    QWidget#actionCell { background: transparent; border: none; }
)";

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

QWidget* createActionCell(const QList<QPushButton*>& buttons, QWidget* parent) {
    auto* container = new QWidget(parent);
    container->setObjectName("actionCell");
    auto* lay = new QHBoxLayout(container);
    lay->setContentsMargins(4, 0, 4, 0);
    lay->setSpacing(8);
    lay->addStretch();
    for (auto* btn : buttons) {
        // 从按钮内联 stylesheet 中提取颜色
        QString color = "#b45309";
        QRegularExpression re("color:(#[0-9a-fA-F]+)");
        QRegularExpressionMatch match = re.match(btn->styleSheet());
        if (match.hasMatch()) color = match.captured(1);

        // 清除 stylesheet，用 setFlat + QPalette 代替
        // （QSS 的 border:none + background:transparent 在 QStyleSheetStyle 下可能导致文字不渲染）
        btn->setStyleSheet("");
        btn->setFlat(true);
        btn->setParent(container);

        // 用 palette 设置文字颜色
        QPalette pal = btn->palette();
        pal.setColor(QPalette::ButtonText, QColor(color));
        pal.setColor(QPalette::Text, QColor(color));
        btn->setPalette(pal);

        lay->addWidget(btn, 0, Qt::AlignVCenter);
    }
    lay->addStretch();
    return container;
}

QTableWidgetItem* createActionItem(const QString& text, const QString& color,
                                   qint64 entityId, const QVariant& actionData) {
    auto* item = new ActionLinkItem(text, QColor(color));
    item->setData(Qt::UserRole, entityId);
    if (actionData.isValid()) item->setData(Qt::UserRole + 2, actionData);
    return item;
}

void syncEmptyHint(QTableWidget* table, QLabel* hint) {
    bool empty = (table->rowCount() == 0);
    table->setVisible(!empty);
    hint->setVisible(empty);
}

void configureTable(QTableWidget* table) {
    if (!table) return;
    // 禁止单元格直接编辑，所有编辑通过弹窗/面板处理
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    // 统一行高：确保 setCellWidget 的按钮有足够空间完整显示
    table->verticalHeader()->setDefaultSectionSize(56);
    table->verticalHeader()->setVisible(false);
    // 启用 mouse tracking 以支持 cellEntered 信号（用于操作列 hover 光标）
    table->setMouseTracking(true);
    // 操作列 hover 光标：当鼠标进入有 actionData 的单元格时，显示手型光标
    QObject::connect(table, &QTableWidget::cellEntered, table, [table](int r, int c) {
        auto* item = table->item(r, c);
        if (item && item->data(Qt::UserRole + 2).isValid()) {
            table->setCursor(Qt::PointingHandCursor);
        } else {
            table->unsetCursor();
        }
    });
}

} // namespace UiKit
