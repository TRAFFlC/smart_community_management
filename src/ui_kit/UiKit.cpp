#include "UiKit.h"

#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>

namespace UiKit {

void applyCardShadow(QWidget* card) {
    // 使用样式表边框模拟阴影，避免 QGraphicsDropShadowEffect 在 Qt6 上触发
    // "A paint device can only be painted by one painter at a time" 错误
    card->setGraphicsEffect(nullptr);
}

QFrame* createStatCard(const QString& label, const QString& value,
                       const QString& color, QWidget* parent) {
    auto* card = new QFrame(parent);
    card->setFixedHeight(90);
    card->setStyleSheet(QString(
        "QFrame{background:#fff;border-radius:2px;border:1px solid #D4D0C8;}"
        "QLabel{background:transparent;border:none;}"
        "QFrame:hover{border-color:%1;}").arg(color));
    auto* cl = new QVBoxLayout(card);
    cl->setContentsMargins(16, 12, 16, 12);
    cl->setSpacing(4);
    auto* indicator = new QFrame(card);
    indicator->setFixedHeight(3);
    indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
    auto* tl = new QLabel(label);
    tl->setStyleSheet("color:#64748b;font-size:12px;");
    auto* vl = new QLabel(value);
    vl->setStyleSheet(QString("color:%1;font-size:24px;font-weight:bold;").arg(color));
    cl->addWidget(indicator);
    cl->addWidget(tl);
    cl->addWidget(vl);
    applyCardShadow(card);
    return card;
}

QPixmap tintSvgIcon(const QString& iconKey, const QString& color, const QSize& size) {
    QStringList candidates = {
        QStringLiteral(":/icons/%1.svg").arg(iconKey),
        QStringLiteral(":/%1.svg").arg(iconKey)
    };
    QPixmap pix;
    for (const QString& path : candidates) {
        pix.load(path);
        if (!pix.isNull()) break;
    }
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
    header->setFrameStyle(QFrame::NoFrame);  // 显式禁用 QFrame 默认边框，避免左上角出现意外框线
    header->setStyleSheet(QStringLiteral(
        "QFrame { background: transparent; border: none; border-bottom: 1px solid #D4D0C8; border-radius: 0; }"
        "QLabel { background: transparent; border: none; }"));
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

    QTimer::singleShot(4000, toast, [fadeOut]() { fadeOut->start(); });
    QObject::connect(fadeOut, &QPropertyAnimation::finished, toast, &QObject::deleteLater);
    fadeIn->start();
    slideIn->start();
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
