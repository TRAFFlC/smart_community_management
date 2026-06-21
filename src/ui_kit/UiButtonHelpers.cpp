#include "UiKit.h"

#include <QPushButton>

namespace UiKit {

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

} // namespace UiKit
