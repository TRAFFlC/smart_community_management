#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

BasePage *PageFactory::buildGovernanceCare()
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  // Page header
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_heart"), QStringLiteral("重点人群关怀"), QStringLiteral("管理特殊群体信息、走访计划和关怀记录"), UiKit::moduleColor("care"), page));
  layout->addSpacing(12);

  auto *tabWidget = new QTabWidget(page);
  tabWidget->setDocumentMode(true);
  tabWidget->setFocusPolicy(Qt::NoFocus);
  tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; padding: 8px 16px; }");

  // Group tab
  auto *groupPage = new QWidget();
  auto *groupLayout = new QVBoxLayout(groupPage);
  // Group search toolbar
  auto *grpToolbar = new QWidget(groupPage);
  grpToolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *grpTbLayout = new QHBoxLayout(grpToolbar);
  grpTbLayout->setContentsMargins(4, 4, 4, 4);
  grpTbLayout->setSpacing(10);
  auto *grpSearchEdit = new QLineEdit(grpToolbar);
  grpSearchEdit->setPlaceholderText(QStringLiteral("搜索居民姓名..."));
  grpSearchEdit->setMinimumWidth(200);
  grpSearchEdit->setClearButtonEnabled(true);
  grpTbLayout->addWidget(grpSearchEdit);
  auto *careLevelCombo = new QComboBox(grpToolbar);
  careLevelCombo->addItem(QStringLiteral("全部等级"), -1);
  careLevelCombo->addItem(QStringLiteral("一般"), 1);
  careLevelCombo->addItem(QStringLiteral("重点"), 2);
  careLevelCombo->addItem(QStringLiteral("特殊"), 3);
  careLevelCombo->setMinimumWidth(120);
  grpTbLayout->addWidget(careLevelCombo);
  grpTbLayout->addStretch();
  groupLayout->addWidget(grpToolbar);

  auto *groupTable = new QTableWidget(groupPage);
  groupTable->setAlternatingRowColors(true);
  UiKit::configureTable(groupTable);
  groupTable->horizontalHeader()->setStretchLastSection(true);
  groupTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  groupTable->setStyleSheet(UiKit::TABLE_STYLE);
  groupTable->setShowGrid(false);
  groupTable->verticalHeader()->setVisible(false);
  groupTable->setSortingEnabled(true);
  groupTable->setColumnCount(6);
  groupTable->setHorizontalHeaderLabels({QStringLiteral("居民姓名"), QStringLiteral("类型"), QStringLiteral("关怀等级"), QStringLiteral("走访频率"), QStringLiteral("责任人"), QStringLiteral("最近走访")});
  auto *groupPb = new PaginationBar(groupPage);
  auto *groupEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无重点人员记录"), groupPage);
  std::function<void()> loadGroups = [groupTable, grpSearchEdit, careLevelCombo, groupEmptyHint, groupPb]()
  {
    groupTable->setSortingEnabled(false);
    groupTable->setRowCount(0);
    QString searchText = grpSearchEdit->text().trimmed();
    int careLevelFilter = careLevelCombo->currentData().toInt();
    UiKit::PagedQuery pq("SELECT r.name, sg.group_type, sg.care_level, sg.care_frequency, u.real_name, sg.last_visit_time FROM cm_special_group sg JOIN cm_resident r ON sg.resident_id = r.id LEFT JOIN sys_user u ON sg.care_worker_id = u.id WHERE sg.del_flag = 0");
    if (!searchText.isEmpty())
      pq.where("r.name LIKE :search", {{"search", "%" + searchText + "%"}});
    if (careLevelFilter >= 0)
      pq.where("sg.care_level = :level", {{"level", careLevelFilter}});
    pq.orderBy("sg.create_time DESC");

    auto result = pq.execute(groupPb->pageSize(), groupPb->offset());
    groupPb->setTotalCount(result.totalCount);
    int gRow = 0;
    for (const auto& row : result.rows)
    {
      groupTable->insertRow(gRow);
      groupTable->setItem(gRow, 0, new QTableWidgetItem(row.value(0).toString()));
      groupTable->setItem(gRow, 1, new QTableWidgetItem(SpecialGroupType::label(row.value(1).toInt())));
      int lvl = row.value(2).toInt();
      auto *lvlItem = UiKit::createTagTableItem(
          SpecialGroupCareLevel::label(lvl),
          QColor(SpecialGroupCareLevel::color(lvl)),
          QColor(SpecialGroupCareLevel::fgColor(lvl)));
      groupTable->setItem(gRow, 2, lvlItem);
      groupTable->setItem(gRow, 3, new QTableWidgetItem(row.value(3).toString()));
      groupTable->setItem(gRow, 4, new QTableWidgetItem(row.value(4).toString()));
      groupTable->setItem(gRow, 5, new QTableWidgetItem(row.value(5).toDateTime().toString("yyyy-MM-dd")));
      gRow++;
    }
    UiKit::syncEmptyHint(groupTable, groupEmptyHint);
    groupPb->refreshData();
    groupTable->setSortingEnabled(true);
  };
  loadGroups();
  QObject::connect(grpSearchEdit, &QLineEdit::textChanged, page, [=]()
          { loadGroups(); });
  QObject::connect(careLevelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadGroups(); });
  QObject::connect(groupPb, &PaginationBar::pageChanged, page, [=]()
          { loadGroups(); });
  groupLayout->addWidget(groupTable);
  groupLayout->addWidget(groupEmptyHint);
  groupLayout->addWidget(groupPb);
  tabWidget->addTab(groupPage, QStringLiteral("关怀对象"));

  // Visit record tab
  auto *visitPage = new QWidget();
  auto *visitLayout = new QVBoxLayout(visitPage);
  // Visit search toolbar
  auto *visToolbar = new QWidget(visitPage);
  visToolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *visTbLayout = new QHBoxLayout(visToolbar);
  visTbLayout->setContentsMargins(4, 4, 4, 4);
  visTbLayout->setSpacing(10);
  auto *visSearchEdit = new QLineEdit(visToolbar);
  visSearchEdit->setPlaceholderText(QStringLiteral("搜索走访人..."));
  visSearchEdit->setMinimumWidth(200);
  visSearchEdit->setClearButtonEnabled(true);
  visTbLayout->addWidget(visSearchEdit);
  auto *visitTypeCombo = new QComboBox(visToolbar);
  visitTypeCombo->addItem(QStringLiteral("全部类型"), -1);
  visitTypeCombo->addItem(QStringLiteral("定期走访"), 1);
  visitTypeCombo->addItem(QStringLiteral("临时走访"), 2);
  visitTypeCombo->addItem(QStringLiteral("电话慰问"), 3);
  visitTypeCombo->setMinimumWidth(120);
  visTbLayout->addWidget(visitTypeCombo);
  visTbLayout->addStretch();
  visitLayout->addWidget(visToolbar);

  auto *visitTable = new QTableWidget(visitPage);
  visitTable->setAlternatingRowColors(true);
  UiKit::configureTable(visitTable);
  visitTable->horizontalHeader()->setStretchLastSection(true);
  visitTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  visitTable->setStyleSheet(UiKit::TABLE_STYLE);
  visitTable->setShowGrid(false);
  visitTable->verticalHeader()->setVisible(false);
  visitTable->setSortingEnabled(true);
  visitTable->setColumnCount(5);
  visitTable->setHorizontalHeaderLabels({QStringLiteral("走访人"), QStringLiteral("走访时间"), QStringLiteral("类型"), QStringLiteral("发现问题"), QStringLiteral("后续跟进")});
  visitTable->setColumnWidth(3, 180);
  visitTable->setColumnWidth(4, 180);
  auto *visitPb = new PaginationBar(visitPage);
  auto *visitEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无走访记录"), visitPage);
  std::function<void()> loadVisits = [visitTable, visSearchEdit, visitTypeCombo, visitEmptyHint, visitPb]()
  {
    visitTable->setSortingEnabled(false);
    visitTable->setRowCount(0);
    QString searchText = visSearchEdit->text().trimmed();
    int visitTypeFilter = visitTypeCombo->currentData().toInt();
    UiKit::PagedQuery pq("SELECT u.real_name, vr.visit_time, vr.visit_type, vr.found_issues, vr.follow_up FROM ge_visit_record vr LEFT JOIN sys_user u ON vr.visitor_id = u.id WHERE vr.del_flag = 0");
    if (!searchText.isEmpty())
      pq.where("u.real_name LIKE :search", {{"search", "%" + searchText + "%"}});
    if (visitTypeFilter >= 0)
      pq.where("vr.visit_type = :vtype", {{"vtype", visitTypeFilter}});
    pq.orderBy("vr.visit_time DESC");

    auto result = pq.execute(visitPb->pageSize(), visitPb->offset());
    visitPb->setTotalCount(result.totalCount);
    int vRow = 0;
    for (const auto& row : result.rows)
    {
      visitTable->insertRow(vRow);
      visitTable->setItem(vRow, 0, new QTableWidgetItem(row.value(0).toString()));
      visitTable->setItem(vRow, 1, new QTableWidgetItem(row.value(1).toDateTime().toString("yyyy-MM-dd hh:mm")));
      visitTable->setItem(vRow, 2, new QTableWidgetItem(row.value(2).toInt() == 1 ? QStringLiteral("定期走访") : row.value(2).toInt() == 2 ? QStringLiteral("临时走访")
                                                                                                                                               : QStringLiteral("电话慰问")));
      visitTable->setItem(vRow, 3, new QTableWidgetItem(row.value(3).toString()));
      visitTable->setItem(vRow, 4, new QTableWidgetItem(row.value(4).toString()));
      vRow++;
    }
    UiKit::syncEmptyHint(visitTable, visitEmptyHint);
    visitPb->refreshData();
    visitTable->setSortingEnabled(true);
  };
  loadVisits();
  QObject::connect(visSearchEdit, &QLineEdit::textChanged, page, [=]()
          { loadVisits(); });
  QObject::connect(visitTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadVisits(); });
  visitLayout->addWidget(visitTable);
  visitLayout->addWidget(visitEmptyHint);
  visitLayout->addWidget(visitPb);
  tabWidget->addTab(visitPage, QStringLiteral("走访记录"));
  layout->addWidget(tabWidget);
  return page;
}
