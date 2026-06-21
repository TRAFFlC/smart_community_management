#ifndef ENUM_HELPER_H
#define ENUM_HELPER_H

#include <QMap>
#include <QString>

// ============================================================================
// 通用枚举助手：根据静态映射表返回标签/颜色，未命中时返回默认值
// ============================================================================
template<typename T>
struct EnumHelper {
    static QString label(T value, const QMap<T, QString>& labels) {
        auto it = labels.find(value);
        return it != labels.end() ? it.value() : QStringLiteral("未知");
    }
    static QString label(int value, const QMap<T, QString>& labels) {
        return label(static_cast<T>(value), labels);
    }
    static QString color(T value, const QMap<T, QString>& colors) {
        auto it = colors.find(value);
        return it != colors.end() ? it.value() : QStringLiteral("#64748b");
    }
    static QString color(int value, const QMap<T, QString>& colors) {
        return color(static_cast<T>(value), colors);
    }
};

#endif // ENUM_HELPER_H
