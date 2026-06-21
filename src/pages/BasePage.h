#ifndef BASEPAGE_H
#define BASEPAGE_H

#include <QMouseEvent>
#include <QWidget>
#include <QString>

// BasePage: 所有业务页面的基类
// 通过信号与 MainWindow 解耦，页面不直接调用 MainWindow 方法
class BasePage : public QWidget {
    Q_OBJECT
public:
    explicit BasePage(QWidget* parent = nullptr) : QWidget(parent) {}
    virtual ~BasePage() = default;

    // 外部可调用的通知/导航请求接口（内部 emit 信号）
    // 供 PageFactory 辅助函数使用，避免直接访问 protected signals
    void requestNotification(int userId, const QString& title,
                             const QString& content, int type,
                             const QString& bizType = QString(), int bizId = 0) {
        emit sendNotificationRequested(userId, title, content, type, bizType, bizId);
    }

    void requestNavigate(const QString& pageKey) {
        emit navigateToRequested(pageKey);
    }

protected:
    // 统一处理子控件 targetPage 属性的点击导航
    // 页面中的可点击卡片/按钮设置 targetPage 属性并 installEventFilter(this) 即可
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::MouseButtonPress) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                auto* w = qobject_cast<QWidget*>(watched);
                if (w) {
                    QString target = w->property("targetPage").toString();
                    if (!target.isEmpty()) {
                        emit navigateToRequested(target);
                        return true;
                    }
                }
            }
        }
        return QWidget::eventFilter(watched, event);
    }

signals:
    // 请求发送通知（替代直接调用 MainWindow::sendNotification）
    void sendNotificationRequested(int userId, const QString& title,
                                   const QString& content, int type,
                                   const QString& bizType = QString(), int bizId = 0);

    // 请求导航到指定页面（替代直接调用 MainWindow::switchPage）
    void navigateToRequested(const QString& pageKey);
};

#endif // BASEPAGE_H
