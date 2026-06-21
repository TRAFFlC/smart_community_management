#ifndef UIKIT_H
#define UIKIT_H

#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariant>
#include <QVariantList>
#include <QWidget>
#include <QFrame>
#include <initializer_list>

class QTableWidget;
class QToolButton;

namespace UiKit {

// 表格统一样式表
extern const char* TABLE_STYLE;

// 卡片阴影（Qt6 兼容占位，避免 QGraphicsDropShadowEffect 冲突）
void applyCardShadow(QWidget* card);

// 统计卡片（数值+标题+顶部色条），消除 6+ 处重复的 mkCard/createMiniCard lambda
QFrame* createStatCard(const QString& label, const QString& value,
                       const QString& color, QWidget* parent);

// SVG 图标着色
QPixmap tintSvgIcon(const QString& iconKey, const QString& color, const QSize& size);

// 模块颜色映射（用于页面头部小色点）
QString moduleColor(const QString& sub);

// 页面头部
QWidget* createPageHeader(const QString& iconKey, const QString& title,
                          const QString& desc, const QString& color, QWidget* parent);

// 表格标签项
QTableWidgetItem* createTagTableItem(const QString& text, const QColor& bg, const QColor& fg);

// 按钮样式
void applyPrimaryButton(QPushButton* btn);
void applySmallButton(QPushButton* btn);
void applyTextButton(QPushButton* btn);
void applyDangerButton(QPushButton* btn);

// 胶囊按钮
QPushButton* createPillButton(const QString& text, const QString& color = QString(),
                              QWidget* parent = nullptr);

// 表格操作列容器
QWidget* createActionCell(const QList<QPushButton*>& buttons, QWidget* parent = nullptr);

// 表格操作文本项
QTableWidgetItem* createActionItem(const QString& text, const QString& color,
                                   qint64 entityId, const QVariant& actionData = QVariant());

// 统一表格配置：禁用行内编辑、设置常用选择行为等
void configureTable(QTableWidget* table);

// 空状态
QWidget* createEmptyState(const QString& iconKey, const QString& message,
                          const QString& hint, QWidget* parent);
QLabel* createEmptyHintLabel(const QString& text, QWidget* parent);
void syncEmptyHint(QTableWidget* table, QLabel* hint);

// 淡入
void fadeInWidget(QWidget* widget);

// ===== 动效基础设施（克制风格：150-300ms，位移≤8px）=====

// 真正的 opacity 淡入（duration 默认 200ms，OutCubic）
void fadeInWidget(QWidget* widget, int duration);

// 滑入 + 淡入：从 offset 偏移位置滑到原位
// offset = QPoint(6, 0) 表示从右侧 6px 滑入
void slideInWidget(QWidget* widget, const QPoint& offset, int duration = 200);

// 数字滚动：从 0 滚动到 target（用于大数字指标）
void countUpLabel(QLabel* label, int target, int duration = 600);

// 脉冲：轻微缩放 + 透明度（用于通知角标）
void pulseWidget(QWidget* widget, int duration = 300);

// 图标颜色过渡动画：在 fromColor 和 toColor 之间插值，每帧重新着色图标
// 用于 Icon Rail 按钮 hover 时的颜色平滑过渡
void animateIconColor(QToolButton* btn, const QString& iconKey,
                      const QColor& from, const QColor& to, int duration = 150);

// Toast 提示
void showToast(const QString& msg, QWidget* parent = nullptr);

// CSV 导出
bool exportToCsv(const QString& filename, QTableWidget* table);
void exportTableToCsv(QTableWidget* table, const QString& defaultName, QWidget* parent);

} // namespace UiKit

// 侧边栏图标 hover 效果过滤器
class QuickHoverFilter : public QObject {
public:
    QuickHoverFilter(QLabel* iconLabel, const QString& iconKey, QObject* parent = nullptr)
        : QObject(parent), m_iconLabel(iconLabel), m_iconKey(iconKey) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QLabel* m_iconLabel;
    QString m_iconKey;
};

#endif // UIKIT_H
