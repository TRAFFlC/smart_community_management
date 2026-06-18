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

namespace UiKit {

// 表格统一样式表
extern const char* TABLE_STYLE;

// 卡片阴影（Qt6 兼容占位，避免 QGraphicsDropShadowEffect 冲突）
void applyCardShadow(QWidget* card);

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
QWidget* createActionCell(std::initializer_list<QPushButton*> buttons, QWidget* parent = nullptr);

// 表格操作文本项
QTableWidgetItem* createActionItem(const QString& text, const QString& color,
                                   qint64 entityId, const QVariant& actionData = QVariant());

// 空状态
QWidget* createEmptyState(const QString& iconKey, const QString& message,
                          const QString& hint, QWidget* parent);
QLabel* createEmptyHintLabel(const QString& text, QWidget* parent);
void syncEmptyHint(QTableWidget* table, QLabel* hint);

// 淡入
void fadeInWidget(QWidget* widget);

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
