#ifndef BREADCRUMBBAR_H
#define BREADCRUMBBAR_H

#include <QWidget>
#include <QMap>
#include <QString>

class QHBoxLayout;

// 负责顶部面包屑可点击导航
class BreadcrumbBar : public QWidget {
    Q_OBJECT
public:
    explicit BreadcrumbBar(QWidget *parent = nullptr);

    // 注册模块名对应的首页 pageKey（用于点击模块跳转）
    void registerModuleTarget(const QString &module, const QString &pageKey);

    // 设置面包屑路径：module 为空表示根页面（工作台）
    void setPath(const QString &module, const QString &page);

signals:
    // 点击模块名时发出
    void moduleClicked(const QString &moduleName);
    // 点击任意可导航项（首页/模块）时发出，参数为对应 pageKey
    void itemClicked(const QString &targetPageKey);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QHBoxLayout *m_layout = nullptr;
    QMap<QString, QString> m_moduleTargets;
    QString m_homeTarget = QStringLiteral("1");
};

#endif // BREADCRUMBBAR_H
