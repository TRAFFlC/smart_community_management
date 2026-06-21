#include "UiKit.h"

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QToolButton>
#include <QVariantAnimation>
#include <QWidget>

namespace UiKit {

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

} // namespace UiKit
