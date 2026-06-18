#ifndef EVENTPAGE_H
#define EVENTPAGE_H

#include "pages/BasePage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QVBoxLayout>

#include "models/Models.h"
#include "widgets/PaginationBar.h"

class EventPage : public BasePage {
    Q_OBJECT
public:
    explicit EventPage(QWidget* parent = nullptr);

private:
    void buildToolbar();
    void buildTable();
    void loadData();
    void onEditEvent(qint64 eventId);
    void onActionEvent(qint64 eventId, int status);
    void updateRow(int row, const GeEvent& ev);

    QVBoxLayout* m_layout = nullptr;
    QTableWidget* m_table = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QComboBox* m_filterCombo = nullptr;
    QCheckBox* m_onlyMineCheck = nullptr;
    QLabel* m_emptyHint = nullptr;
    PaginationBar* m_pagination = nullptr;
};

#endif // EVENTPAGE_H
