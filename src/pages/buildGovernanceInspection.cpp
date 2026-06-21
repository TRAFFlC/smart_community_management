#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

BasePage *PageFactory::buildGovernanceInspection()
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  // Page header
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_route"), QStringLiteral("社区巡查管理"), QStringLiteral("管理巡查计划、查看巡查记录和问题发现"), UiKit::moduleColor("inspection"), page));
  layout->addSpacing(12);

  // Tab for plan and record
  auto *tabWidget = new QTabWidget(page);
  tabWidget->setDocumentMode(true);
  tabWidget->setFocusPolicy(Qt::NoFocus);
  tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; padding: 8px 16px; }");

  // Plan tab
  auto *planPage = new QWidget();
  auto *planLayout = new QVBoxLayout(planPage);
  // Plan search toolbar
  auto *planToolbar = new QWidget(planPage);
  planToolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *planTbLayout = new QHBoxLayout(planToolbar);
  planTbLayout->setContentsMargins(4, 4, 4, 4);
  planTbLayout->setSpacing(10);
  auto *planSearchEdit = new QLineEdit(planToolbar);
  planSearchEdit->setPlaceholderText(QStringLiteral("搜索计划名称/巡查员..."));
  planSearchEdit->setMinimumWidth(200);
  planSearchEdit->setClearButtonEnabled(true);
  planTbLayout->addWidget(planSearchEdit);
  auto *planStatusCombo = new QComboBox(planToolbar);
  planStatusCombo->addItem(QStringLiteral("全部状态"), -1);
  planStatusCombo->addItem(QStringLiteral("待执行"), 0);
  planStatusCombo->addItem(QStringLiteral("进行中"), 1);
  planStatusCombo->addItem(QStringLiteral("已完成"), 2);
  planStatusCombo->setMinimumWidth(120);
  planTbLayout->addWidget(planStatusCombo);
  planTbLayout->addStretch();
  planLayout->addWidget(planToolbar);

  auto *planTable = new QTableWidget(planPage);
  planTable->setAlternatingRowColors(true);
  UiKit::configureTable(planTable);
  planTable->horizontalHeader()->setStretchLastSection(true);
  planTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  planTable->setStyleSheet(UiKit::TABLE_STYLE);
  planTable->setShowGrid(false);
  planTable->verticalHeader()->setVisible(false);
  planTable->setSortingEnabled(true);
  planTable->setColumnCount(6);
  planTable->setHorizontalHeaderLabels({QStringLiteral("计划名称"), QStringLiteral("网格"), QStringLiteral("频率"), QStringLiteral("巡查员"), QStringLiteral("起止日期"), QStringLiteral("状态")});
  auto *planPb = new PaginationBar(planPage);
  auto *planEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无计划记录"), planPage);
  std::function<void()> loadPlans = [planTable, planSearchEdit, planStatusCombo, planEmptyHint, planPb]()
  {
    planTable->setSortingEnabled(false);
    planTable->setRowCount(0);
    QString searchText = planSearchEdit->text().trimmed();
    int planStsFilter = planStatusCombo->currentData().toInt();
    UiKit::PagedQuery pq("SELECT ip.plan_name, g.grid_name, ip.frequency, u.real_name, ip.start_date || '~' || ip.end_date, ip.status FROM ge_inspection_plan ip LEFT JOIN cm_grid g ON ip.grid_id = g.id LEFT JOIN sys_user u ON ip.inspector_id = u.id WHERE ip.del_flag = 0");
    if (!searchText.isEmpty())
      pq.where("(ip.plan_name LIKE :search OR u.real_name LIKE :search)", {{"search", "%" + searchText + "%"}});
    if (planStsFilter >= 0)
      pq.where("ip.status = :status", {{"status", planStsFilter}});
    pq.orderBy("ip.create_time DESC");

    auto result = pq.execute(planPb->pageSize(), planPb->offset());
    planPb->setTotalCount(result.totalCount);
    int pRow = 0;
    for (const auto& row : result.rows)
    {
      planTable->insertRow(pRow);
      for (int c = 0; c < 5; ++c)
        planTable->setItem(pRow, c, new QTableWidgetItem(row.value(c).toString()));
      int pSts = row.value(5).toInt();
      auto *planStsItem = UiKit::createTagTableItem(
          InspectionPlanStatus::label(pSts),
          QColor(InspectionPlanStatus::color(pSts)),
          QColor(InspectionPlanStatus::fgColor(pSts)));
      planTable->setItem(pRow, 5, planStsItem);
      pRow++;
    }
    UiKit::syncEmptyHint(planTable, planEmptyHint);
    planPb->refreshData();
    planTable->setSortingEnabled(true);
  };
  loadPlans();
  QObject::connect(planSearchEdit, &QLineEdit::textChanged, page, [=]()
          { loadPlans(); });
  QObject::connect(planStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadPlans(); });
  planLayout->addWidget(planTable);
  planLayout->addWidget(planEmptyHint);
  planLayout->addWidget(planPb);
  tabWidget->addTab(planPage, QStringLiteral("巡查计划"));

  // Record tab
  auto *recordPage = new QWidget();
  auto *recordLayout = new QVBoxLayout(recordPage);
  // Record search toolbar
  auto *recToolbar = new QWidget(recordPage);
  recToolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *recTbLayout = new QHBoxLayout(recToolbar);
  recTbLayout->setContentsMargins(4, 4, 4, 4);
  recTbLayout->setSpacing(10);
  auto *recSearchEdit = new QLineEdit(recToolbar);
  recSearchEdit->setPlaceholderText(QStringLiteral("搜索巡查员..."));
  recSearchEdit->setMinimumWidth(200);
  recSearchEdit->setClearButtonEnabled(true);
  recTbLayout->addWidget(recSearchEdit);
  auto *recStatusCombo = new QComboBox(recToolbar);
  recStatusCombo->addItem(QStringLiteral("全部状态"), -1);
  recStatusCombo->addItem(QStringLiteral("进行中"), 0);
  recStatusCombo->addItem(QStringLiteral("已完成"), 1);
  recStatusCombo->setMinimumWidth(120);
  recTbLayout->addWidget(recStatusCombo);
  recTbLayout->addStretch();
  recordLayout->addWidget(recToolbar);

  auto *recTable = new QTableWidget(recordPage);
  recTable->setAlternatingRowColors(true);
  UiKit::configureTable(recTable);
  recTable->horizontalHeader()->setStretchLastSection(true);
  recTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  recTable->setStyleSheet(UiKit::TABLE_STYLE);
  recTable->setShowGrid(false);
  recTable->verticalHeader()->setVisible(false);
  recTable->setSortingEnabled(true);
  recTable->setColumnCount(6);
  recTable->setHorizontalHeaderLabels({QStringLiteral("巡查员"), QStringLiteral("开始时间"), QStringLiteral("结束时间"), QStringLiteral("时长(分)"), QStringLiteral("发现问题"), QStringLiteral("状态")});
  auto *recPb = new PaginationBar(recordPage);
  auto *recEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无巡查记录"), recordPage);
  std::function<void()> loadRecords = [recTable, recSearchEdit, recStatusCombo, recEmptyHint, recPb]()
  {
    recTable->setSortingEnabled(false);
    recTable->setRowCount(0);
    QString searchText = recSearchEdit->text().trimmed();
    int statusFilter = recStatusCombo->currentData().toInt();
    UiKit::PagedQuery pq("SELECT u.real_name, i.start_time, i.end_time, i.duration, i.issue_count, i.status FROM ge_inspection i LEFT JOIN sys_user u ON i.inspector_id = u.id WHERE i.del_flag = 0");
    if (!searchText.isEmpty())
      pq.where("u.real_name LIKE :search", {{"search", "%" + searchText + "%"}});
    if (statusFilter >= 0)
      pq.where("i.status = :status", {{"status", statusFilter}});
    pq.orderBy("i.start_time DESC");

    auto result = pq.execute(recPb->pageSize(), recPb->offset());
    recPb->setTotalCount(result.totalCount);
    int rRow = 0;
    for (const auto& row : result.rows)
    {
      recTable->insertRow(rRow);
      recTable->setItem(rRow, 0, new QTableWidgetItem(row.value(0).toString()));
      recTable->setItem(rRow, 1, new QTableWidgetItem(row.value(1).toDateTime().toString("MM-dd hh:mm")));
      recTable->setItem(rRow, 2, new QTableWidgetItem(row.value(2).toDateTime().toString("MM-dd hh:mm")));
      recTable->setItem(rRow, 3, new QTableWidgetItem(row.value(3).toString()));
      recTable->setItem(rRow, 4, new QTableWidgetItem(row.value(4).toString()));
      int iSts = row.value(5).toInt();
      auto *stsItem = UiKit::createTagTableItem(
          InspectionRecordStatus::label(iSts),
          QColor(InspectionRecordStatus::color(iSts)),
          QColor(InspectionRecordStatus::fgColor(iSts)));
      recTable->setItem(rRow, 5, stsItem);
      rRow++;
    }
    UiKit::syncEmptyHint(recTable, recEmptyHint);
    recPb->refreshData();
    recTable->setSortingEnabled(true);
  };
  loadRecords();
  QObject::connect(recSearchEdit, &QLineEdit::textChanged, page, [=]()
          { loadRecords(); });
  QObject::connect(recStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadRecords(); });
  recordLayout->addWidget(recTable);
  recordLayout->addWidget(recEmptyHint);
  recordLayout->addWidget(recPb);
  tabWidget->addTab(recordPage, QStringLiteral("巡查记录"));
  layout->addWidget(tabWidget);
  return page;
}
