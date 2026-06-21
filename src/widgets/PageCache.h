#ifndef PAGECACHE_H
#define PAGECACHE_H

#include <QObject>
#include <QMap>
#include <QString>

class BasePage;
class QStackedWidget;
class QWidget;

// 负责页面缓存与生命周期管理
class PageCache : public QObject {
    Q_OBJECT
public:
    explicit PageCache(QStackedWidget *stack, QObject *parent = nullptr);

    // 获取或创建页面；placeholder 返回 nullptr，但 widget() 可获取占位控件
    BasePage *getOrCreatePage(const QString &key);
    QWidget *pageWidget(const QString &key) const;

    // 强制刷新指定页面（删除旧实例并重新创建）
    BasePage *refreshPage(const QString &key);

    void clear();

signals:
    // 新建 BasePage 时发出，便于外部连接页面信号
    void pageCreated(BasePage *page);

private:
    QWidget *createPlaceholder();

    QStackedWidget *m_stack = nullptr;
    QMap<QString, QWidget *> m_pages;
};

#endif // PAGECACHE_H
