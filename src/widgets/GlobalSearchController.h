#ifndef GLOBALSEARCHCONTROLLER_H
#define GLOBALSEARCHCONTROLLER_H

#include <QObject>
#include <QString>
#include <QListWidgetItem>

class QLineEdit;
class QFrame;
class QListWidget;
class QWidget;

// 负责全局搜索框：下拉结果列表、搜索逻辑、结果跳转通知
class GlobalSearchController : public QObject {
    Q_OBJECT
public:
    explicit GlobalSearchController(QObject *parent = nullptr);
    void attach(QLineEdit *edit, QWidget *popupParent);

signals:
    void menuSelected(const QString &pageKey);
    void workOrderSelected(qint64 id);
    void eventSelected(qint64 id);

public slots:
    void hideDropdown();

private slots:
    void performSearch(const QString &text);
    void onItemClicked(QListWidgetItem *item);
    void showDropdown();

private:
    QLineEdit *m_edit = nullptr;
    QWidget *m_popupParent = nullptr;
    QFrame *m_dropdown = nullptr;
    QListWidget *m_resultList = nullptr;
};

#endif // GLOBALSEARCHCONTROLLER_H
