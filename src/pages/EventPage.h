#ifndef EVENTPAGE_H
#define EVENTPAGE_H

#include "pages/BasePage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QVBoxLayout>

#include "models/GeModels.h"
#include "widgets/PaginationBar.h"

class EventPage : public BasePage {
    Q_OBJECT
public:
    explicit EventPage(QWidget* parent = nullptr);

private:
    void buildToolbar();
    void buildTable();
    void loadData();
    void updateRow(int row, const GeEvent& ev);
    void onNewEvent();
    void onEditEvent(qint64 eventId);
    void onActionEvent(qint64 eventId, int status);

    QVBoxLayout* m_layout = nullptr;
    QTableWidget* m_table = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QComboBox* m_filterCombo = nullptr;
    QCheckBox* m_onlyMineCheck = nullptr;
    QLabel* m_emptyHint = nullptr;
    PaginationBar* m_pagination = nullptr;
};

#endif // EVENTPAGE_H
