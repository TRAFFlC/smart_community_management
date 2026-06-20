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
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariantAnimation>

namespace UiKit {

const char* TABLE_STYLE = R"(
    QTableWidget {
        background: #FAF9F6; border: 1px solid #D4D0C8; border-radius: 0;
        gridline-color: transparent; font-size: 13px; color: #141413;
    }
    QTableWidget::item {
        padding: 10px 14px;
        border-bottom: 1px solid #E8E5DE;
        min-height: 44px;
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
    QTableWidget QPushButton, QTreeWidget QPushButton,
    QTableWidget QWidget QPushButton, QTreeWidget QWidget QPushButton {
        min-height: 24px !important; max-height: 28px !important;
        min-width: 0px; max-width: 80px;
        padding: 2px 10px !important; font-size: 12px;
        border: 1px solid #D4D0C8; border-radius: 0;
        background: transparent; color: #141413;
    }
    QTableWidget QPushButton:hover, QTreeWidget QPushButton:hover,
    QTableWidget QWidget QPushButton:hover, QTreeWidget QWidget QPushButton:hover {
        background: #141413; color: #FAF9F6; border-color: #141413;
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
    // 现代档案室设计：单一琥珀强调，分类用墨色深浅区分
    // 默认返回深琥珀；特殊语义保留少量克制色（红/绿/蓝），但仅用于状态标签
    static const QMap<QString, QString> colors = {
        // 档案/基础 — 中性墨色
        {"org", "#475569"}, {"estate", "#475569"}, {"house", "#475569"}, {"resident", "#475569"},
        {"vehicle", "#475569"}, {"facility", "#475569"}, {"grid", "#475569"}, {"special", "#475569"},
        // 工单/物业 — 琥珀（主强调色）
        {"workorder", "#92400E"}, {"complaint", "#92400E"}, {"inspection", "#92400E"},
        {"announcement", "#92400E"}, {"visitor", "#92400E"}, {"topic", "#92400E"},
        {"parking", "#92400E"}, {"billing", "#92400E"}, {"income", "#92400E"},
        // 治理/事件 — 琥珀
        {"event", "#92400E"}, {"care", "#92400E"}, {"supervision", "#92400E"},
        {"opinion", "#92400E"}, {"assessment", "#92400E"},
        // 服务/志愿 — 琥珀
        {"volunteer", "#92400E"}, {"convenience", "#92400E"}, {"job", "#92400E"},
        // 系统/AI — 中性墨色
        {"user", "#475569"}, {"role", "#475569"}, {"menu", "#475569"},
        {"dict", "#475569"}, {"log", "#475569"}, {"ai", "#475569"},
        // 聚合/看板 — 琥珀
        {"dashboard", "#92400E"}, {"todo", "#92400E"}, {"message", "#92400E"},
    };
    return colors.value(sub, "#92400E");
}

QWidget* createPageHeader(const QString& iconKey, const QString& title, const QString& desc, const QString& color, QWidget* parent) {
    auto* header = new QFrame(parent);
    header->setFixedHeight(72);
    header->setStyleSheet(QStringLiteral(
        "QFrame { background: transparent; border: none; border-bottom: 1px solid #D4D0C8; border-radius: 0; }"
        "QLabel { background: transparent; border: none; }"
    ));
    auto* hl = new QHBoxLayout(header);
    hl->setContentsMargins(0, 0, 0, 16);
    hl->setSpacing(12);

    if (!iconKey.isEmpty()) {
        auto* iconLabel = new QLabel(header);
        iconLabel->setFixedSize(24, 24);
        iconLabel->setPixmap(tintSvgIcon(iconKey, color.isEmpty() ? QStringLiteral("#92400E") : color, QSize(24, 24)));
        hl->addWidget(iconLabel, 0, Qt::AlignVCenter);
    }

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);
    auto* titleLabel = new QLabel(title, header);
    titleLabel->setStyleSheet(QStringLiteral(
        "font-size: 22px; font-weight: 600; color: #141413;"
        " font-family: 'Noto Serif SC', 'Source Han Serif SC', 'SimSun', serif;"
        " letter-spacing: 0.5px;"));
    textLayout->addWidget(titleLabel);
    if (!desc.isEmpty()) {
        auto* descLabel = new QLabel(desc, header);
        descLabel->setStyleSheet(QStringLiteral("font-size: 12px; color: #6B6B6B; letter-spacing: 0.3px;"));
        textLayout->addWidget(descLabel);
    }
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
        "QPushButton { background: #141413; color: #FAF9F6; border: 1px solid #141413; border-radius: 0; "
        "padding: 8px 20px; font-size: 13px; font-weight: 500; min-height: 36px; letter-spacing: 0.5px; }"
        "QPushButton:hover { background: #92400E; border-color: #92400E; }"
        "QPushButton:pressed { background: #0A0A0A; border-color: #0A0A0A; }"
    );
}

void applySmallButton(QPushButton* btn) {
    btn->setProperty("cssClass", "small");
    btn->setStyleSheet(
        "QPushButton { min-height: 28px; padding: 4px 12px; font-size: 12px; border-radius: 0; }"
    );
}

void applyTextButton(QPushButton* btn) {
    btn->setProperty("cssClass", "text");
    btn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #92400E; padding: 4px 8px; "
        "font-size: 13px; min-height: 28px; border-bottom: 1px solid transparent; }"
        "QPushButton:hover { color: #B45309; border-bottom-color: #B45309; }"
    );
}

void applyDangerButton(QPushButton* btn) {
    btn->setProperty("cssClass", "danger");
    btn->setStyleSheet(
        "QPushButton { background: transparent; color: #B91C1C; border: 1px solid #B91C1C; border-radius: 0; "
        "padding: 8px 20px; font-size: 13px; font-weight: 500; min-height: 36px; letter-spacing: 0.5px; }"
        "QPushButton:hover { background: #B91C1C; color: #FAF9F6; border-color: #B91C1C; }"
    );
}

QPushButton* createPillButton(const QString& text, const QString& color, QWidget* parent) {
    auto* btn = new QPushButton(text, parent);
    // 档案室风格：透明背景 + 1px 细边框 + hover 反色
    const QString accent = color.isEmpty() ? QStringLiteral("#141413") : color;
    btn->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: 1px solid %1; border-radius: 0;"
        "  padding: 2px 10px !important; font-size: 12px; font-weight: 500;"
        "  min-height: 24px !important; max-height: 24px !important;"
        "  min-width: 0px; max-width: 80px; letter-spacing: 0.3px; }"
        "QPushButton:hover { background: %1; color: #FAF9F6; }"
    ).arg(accent));
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
        painter.fillRect(img.rect(), QColor("#D4D0C8"));
        painter.end();
        iconLabel->setPixmap(QPixmap::fromImage(img).scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background: transparent; border: none;");
    layout->addWidget(iconLabel, 0, Qt::AlignCenter);
    auto* msgLabel = new QLabel(message, empty);
    msgLabel->setStyleSheet("font-size: 15px; color: #6B6B6B; background: transparent; border: none;");
    msgLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(msgLabel);
    if (!hint.isEmpty()) {
        auto* hintLabel = new QLabel(hint, empty);
        hintLabel->setStyleSheet("font-size: 12px; color: #9A9A9A; background: transparent; border: none;");
        hintLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(hintLabel);
    }
    empty->setVisible(false);
    return empty;
}

QLabel* createEmptyHintLabel(const QString& text, QWidget* parent) {
    auto* lbl = new QLabel(text, parent);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("color: #6B6B6B; font-size: 14px; padding: 40px; background: #FAF9F6; border-radius: 0; border: 1px dashed #D4D0C8;");
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
    fadeInWidget(widget, 200);
}

// 真正的 opacity 淡入：使用 QGraphicsOpacityEffect + QPropertyAnimation
// 200ms OutCubic，克制快速
void fadeInWidget(QWidget* widget, int duration) {
    if (!widget) return;
    widget->show();

    // 避免重复设置 effect 导致冲突
    auto* existingEffect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    QGraphicsOpacityEffect* opacity = existingEffect;
    if (!opacity) {
        opacity = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(opacity);
    }
    opacity->setOpacity(0.0);

    auto* anim = new QPropertyAnimation(opacity, "opacity", widget);
    anim->setDuration(duration);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    // 动画结束后清理（widget 仍持有 effect）
    QObject::connect(anim, &QPropertyAnimation::finished, anim, &QObject::deleteLater);
    anim->start();
}

// 滑入 + 淡入：先位移到 offset 位置，再动画回到原位 + 淡入
void slideInWidget(QWidget* widget, const QPoint& offset, int duration) {
    if (!widget) return;
    widget->show();

    // 位移动画（geometry 不变，用 pos）
    QPoint targetPos = widget->pos();
    QPoint startPos = targetPos + offset;
    widget->move(startPos);

    auto* moveAnim = new QPropertyAnimation(widget, "pos", widget);
    moveAnim->setDuration(duration);
    moveAnim->setStartValue(startPos);
    moveAnim->setEndValue(targetPos);
    moveAnim->setEasingCurve(QEasingCurve::OutCubic);

    // 透明度动画
    auto* existingEffect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    QGraphicsOpacityEffect* opacity = existingEffect;
    if (!opacity) {
        opacity = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(opacity);
    }
    opacity->setOpacity(0.0);
    auto* fadeAnim = new QPropertyAnimation(opacity, "opacity", widget);
    fadeAnim->setDuration(duration);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

    QObject::connect(moveAnim, &QPropertyAnimation::finished, moveAnim, &QObject::deleteLater);
    QObject::connect(fadeAnim, &QPropertyAnimation::finished, fadeAnim, &QObject::deleteLater);

    moveAnim->start();
    fadeAnim->start();
}

// 数字滚动：从 0 滚动到 target
// 使用 QVariantAnimation + 整数插值，每帧更新 label 文本
void countUpLabel(QLabel* label, int target, int duration) {
    if (!label) return;
    // 保存目标值，用于动画结束后确保最终值准确
    label->setProperty("countUpTarget", target);

    auto* anim = new QVariantAnimation(label);
    anim->setDuration(duration);
    anim->setStartValue(0);
    anim->setEndValue(target);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    QObject::connect(anim, &QVariantAnimation::valueChanged, label, [label](const QVariant& value) {
        if (label) {
            label->setText(QString::number(value.toInt()));
        }
    });
    // 动画结束后确保显示最终值（避免浮点精度导致显示 target-1）
    QObject::connect(anim, &QVariantAnimation::finished, label, [label, target]() {
        if (label) label->setText(QString::number(target));
    });
    QObject::connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
    anim->start();
}

// 脉冲：透明度 1→0.4→1 + 轻微缩放（通过 geometry）
// 用于通知角标新消息到达时
void pulseWidget(QWidget* widget, int duration) {
    if (!widget) return;

    auto* existingEffect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    QGraphicsOpacityEffect* opacity = existingEffect;
    if (!opacity) {
        opacity = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(opacity);
    }

    auto* anim = new QPropertyAnimation(opacity, "opacity", widget);
    anim->setDuration(duration);
    anim->setKeyValueAt(0.0, 1.0);
    anim->setKeyValueAt(0.5, 0.4);
    anim->setKeyValueAt(1.0, 1.0);
    anim->setEasingCurve(QEasingCurve::InOutSine);
    QObject::connect(anim, &QPropertyAnimation::finished, anim, &QObject::deleteLater);
    anim->start();
}

// 图标颜色过渡动画：在 fromColor 和 toColor 之间插值
// 每帧重新着色 SVG 图标并 setIcon，150ms OutCubic
void animateIconColor(QToolButton* btn, const QString& iconKey,
                      const QColor& from, const QColor& to, int duration) {
    if (!btn || iconKey.isEmpty()) return;

    // 停止该按钮上正在进行的颜色动画（避免冲突）
    auto* oldAnim = btn->findChild<QVariantAnimation*>();
    if (oldAnim) {
        oldAnim->stop();
        oldAnim->deleteLater();
    }

    QPixmap srcPix(QString(":/icons/%1.svg").arg(iconKey));
    if (srcPix.isNull()) return;
    QImage srcImg = srcPix.toImage();

    auto* anim = new QVariantAnimation(btn);
    anim->setDuration(duration);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    QObject::connect(anim, &QVariantAnimation::valueChanged, btn, [btn, srcImg, from, to](const QVariant& value) {
        qreal t = value.toReal();
        int r = int(from.red() + (to.red() - from.red()) * t);
        int g = int(from.green() + (to.green() - from.green()) * t);
        int b = int(from.blue() + (to.blue() - from.blue()) * t);
        QColor interpolated(r, g, b);

        QImage img = srcImg;
        QPainter painter(&img);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(img.rect(), interpolated);
        painter.end();
        btn->setIcon(QIcon(QPixmap::fromImage(img).scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    });

    QObject::connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
    anim->start();
}

void showToast(const QString& msg, QWidget* parent) {
    if (!parent) parent = qobject_cast<QWidget*>(QApplication::activeWindow());
    if (!parent) return;

    auto* toast = new QFrame(parent);
    toast->setObjectName("toastFrame");
    toast->setStyleSheet(
        "QFrame#toastFrame { background: #141413; border: 1px solid #141413; "
        "border-left: 3px solid #B45309; border-radius: 0; }"
        "QLabel { background: transparent; border: none; color: #FAF9F6; font-size: 13px; letter-spacing: 0.3px; }"
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
    p.setBrush(QColor("#B45309"));
    p.drawEllipse(0, 0, 20, 20);
    p.setPen(QPen(QColor("#FAF9F6"), 2));
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
        m_iconLabel->setPixmap(UiKit::tintSvgIcon(m_iconKey, "#B45309", QSize(32, 32)));
    } else if (event->type() == QEvent::Leave) {
        m_iconLabel->setPixmap(UiKit::tintSvgIcon(m_iconKey, "#6B6B6B", QSize(32, 32)));
    }
    return QObject::eventFilter(watched, event);
}
