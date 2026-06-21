#ifndef NAVIGATIONRAIL_H
#define NAVIGATIONRAIL_H

#include <QWidget>
#include <QMap>
#include <QList>
#include <QPair>
#include <QString>
#include "models/SysModels.h"

class QToolButton;
class QPushButton;
class QLabel;
class QVBoxLayout;

// 负责左侧双栏导航：Icon Rail (64px) + Section Panel (220px)
class NavigationRail : public QWidget {
    Q_OBJECT
public:
    explicit NavigationRail(QWidget *parent = nullptr);

    // menus: 当前用户可见的一级模块菜单
    // modulePages: 模块名 -> [(pageKey, pageTitle)]，用于构建 Section Panel
    void build(const QList<SysMenu> &menus,
               const QMap<QString, QList<QPair<QString, QString>>> &modulePages);

    void setCurrentModule(const QString &moduleKey);
    void setActivePage(const QString &pageKey);
    QString currentModule() const;

signals:
    void moduleClicked(const QString &moduleName, const QString &firstPageKey);
    void pageClicked(const QString &pageKey);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void buildIconRail(const QList<SysMenu> &menus);
    void buildSectionPanel();
    QWidget *createSectionItem(const QString &text, const QString &key);
    void setIconHighlight(QToolButton *btn, bool active);
    QString firstPageKeyForModule(const QString &moduleName) const;

    QWidget *m_iconRail = nullptr;
    QWidget *m_sectionPanel = nullptr;
    QLabel *m_sectionTitle = nullptr;
    QVBoxLayout *m_sectionLayout = nullptr;
    QMap<QString, QToolButton *> m_iconRailItems;
    QMap<QString, QWidget *> m_sectionItems;
    QToolButton *m_activeIconRailItem = nullptr;
    QPushButton *m_activeSectionItem = nullptr;
    QString m_currentModule;
    QMap<QString, QList<QPair<QString, QString>>> m_modulePages;
    QMap<QString, QString> m_moduleIcons;
};

#endif // NAVIGATIONRAIL_H
