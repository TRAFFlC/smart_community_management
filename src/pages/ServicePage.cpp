#include "pages/PageFactory.h"
#include "PagesCommon.h"

BasePage *PageFactory::createServicePage(const QString &sub)
{
    if (sub == "volunteer")
        return buildServiceVolunteer();
    if (sub == "convenience")
        return buildServiceConvenience();
    if (sub == "job")
        return buildServiceEmployment();

    // Fallback for unknown sub page
    auto *page = new BasePage();
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    auto *table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    UiKit::configureTable(table);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(UiKit::TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);
    table->setSortingEnabled(true);

    auto *emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无数据"), page);

    layout->addWidget(table);
    layout->addWidget(emptyHint);
    return page;
}
