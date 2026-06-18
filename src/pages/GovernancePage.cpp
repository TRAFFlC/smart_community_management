#include "pages/PageFactory.h"
#include "PagesCommon.h"

// ========== Governance Pages ==========
BasePage *PageFactory::createGovernancePage(const QString &sub)
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(UiKit::TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setSortingEnabled(true);

  auto &db = DatabaseManager::instance();

  QString govEmptyText;
  if (sub == "event")
    govEmptyText = QStringLiteral("暂无事件记录");
  else if (sub == "plan")
    govEmptyText = QStringLiteral("暂无计划记录");
  else if (sub == "record")
    govEmptyText = QStringLiteral("暂无巡查记录");
  else if (sub == "group")
    govEmptyText = QStringLiteral("暂无重点人员记录");
  else if (sub == "visit")
    govEmptyText = QStringLiteral("暂无走访记录");
  else if (sub == "supervision")
    govEmptyText = QStringLiteral("暂无督办记录");
  else if (sub == "opinion")
    govEmptyText = QStringLiteral("暂无民意记录");
  else if (sub == "assessment")
    govEmptyText = QStringLiteral("暂无考核记录");
  else
    govEmptyText = QStringLiteral("暂无数据");
  auto *emptyHint = UiKit::createEmptyHintLabel(govEmptyText, page);

  if (sub == "event")
  {
    return PageFactory::createEventPage();
  }
  else if (sub == "inspection")
  {
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_route"), QStringLiteral("社区巡查管理"), QStringLiteral("管理巡查计划、查看巡查记录和问题发现"), UiKit::moduleColor("inspection"), page));
    layout->addSpacing(12);

    // Tab for plan and record
    auto *tabWidget = new QTabWidget(page);
    tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; padding: 8px 16px; }");

    // Plan tab
    auto *planPage = new QWidget();
    auto *planLayout = new QVBoxLayout(planPage);
    // Plan search toolbar
    auto *planToolbar = new QWidget(planPage);
    planToolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
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
    planTable->horizontalHeader()->setStretchLastSection(true);
    planTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    planTable->setStyleSheet(UiKit::TABLE_STYLE);
    planTable->setShowGrid(false);
    planTable->verticalHeader()->setVisible(false);
    planTable->setSortingEnabled(true);
    planTable->setColumnCount(6);
    planTable->setHorizontalHeaderLabels({QStringLiteral("计划名称"), QStringLiteral("网格"), QStringLiteral("频率"), QStringLiteral("巡查员"), QStringLiteral("起止日期"), QStringLiteral("状态")});
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    auto *planEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无计划记录"), planPage);
    std::function<void()> loadPlans = [planTable, planSearchEdit, planStatusCombo, planEmptyHint, pb]()
    {
      planTable->setRowCount(0);
      QString sql = "SELECT ip.plan_name, g.grid_name, ip.frequency, u.real_name, ip.start_date || '~' || ip.end_date, ip.status FROM ge_inspection_plan ip LEFT JOIN cm_grid g ON ip.grid_id = g.id LEFT JOIN sys_user u ON ip.inspector_id = u.id WHERE ip.del_flag = 0";
      QString searchText = planSearchEdit->text().trimmed();
      int planStsFilter = planStatusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (ip.plan_name LIKE :search OR u.real_name LIKE :search)";
      if (planStsFilter >= 0)
        sql += " AND ip.status = :status";
      sql += " ORDER BY ip.create_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery planQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (planStsFilter >= 0)
        cntBinds << ":status" << (planStsFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      planQ.prepare(sql);
      if (!searchText.isEmpty())
        planQ.bindValue(":search", "%" + searchText + "%");
      if (planStsFilter >= 0)
        planQ.bindValue(":status", planStsFilter);
      planQ.bindValue(":pageSize", pb->pageSize());
      planQ.bindValue(":offset", pb->offset());
      planQ.exec();
      int pRow = 0;
      while (planQ.next())
      {
        planTable->insertRow(pRow);
        for (int c = 0; c < 5; ++c)
          planTable->setItem(pRow, c, new QTableWidgetItem(planQ.value(c).toString()));
        int pSts = planQ.value(5).toInt();
        QString planStsText = pSts == 0 ? QStringLiteral("待执行") : pSts == 1 ? QStringLiteral("进行中")
                                                                               : QStringLiteral("已完成");
        QColor planStsBg = pSts == 0 ? QColor("#eff6ff") : pSts == 1 ? QColor("#fffbeb")
                                                                     : QColor("#f0fdf4");
        QColor planStsFg = pSts == 0 ? QColor("#2563eb") : pSts == 1 ? QColor("#d97706")
                                                                     : QColor("#15803d");
        auto *planStsItem = UiKit::createTagTableItem(planStsText, planStsBg, planStsFg);
        planTable->setItem(pRow, 5, planStsItem);
        pRow++;
      }
      UiKit::syncEmptyHint(planTable, planEmptyHint);
      pb->refreshData();
    };
    loadPlans();
    QObject::connect(planSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadPlans(); });
    QObject::connect(planStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadPlans(); });
    planLayout->addWidget(planTable);
    planLayout->addWidget(planEmptyHint);
    tabWidget->addTab(planPage, QStringLiteral("巡查计划"));

    // Record tab
    auto *recordPage = new QWidget();
    auto *recordLayout = new QVBoxLayout(recordPage);
    // Record search toolbar
    auto *recToolbar = new QWidget(recordPage);
    recToolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
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
    recTable->horizontalHeader()->setStretchLastSection(true);
    recTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recTable->setStyleSheet(UiKit::TABLE_STYLE);
    recTable->setShowGrid(false);
    recTable->verticalHeader()->setVisible(false);
    recTable->setSortingEnabled(true);
    recTable->setColumnCount(6);
    recTable->setHorizontalHeaderLabels({QStringLiteral("巡查员"), QStringLiteral("开始时间"), QStringLiteral("结束时间"), QStringLiteral("时长(分)"), QStringLiteral("发现问题"), QStringLiteral("状态")});
    auto *recEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无巡查记录"), recordPage);
    std::function<void()> loadRecords = [recTable, recSearchEdit, recStatusCombo, recEmptyHint, pb]()
    {
      recTable->setRowCount(0);
      QString sql = "SELECT u.real_name, i.start_time, i.end_time, i.duration, i.issue_count, i.status FROM ge_inspection i LEFT JOIN sys_user u ON i.inspector_id = u.id WHERE i.del_flag = 0";
      QString searchText = recSearchEdit->text().trimmed();
      int statusFilter = recStatusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND u.real_name LIKE :search";
      if (statusFilter >= 0)
        sql += " AND i.status = :status";
      sql += " ORDER BY i.start_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery recQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (statusFilter >= 0)
        cntBinds << ":status" << (statusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      recQ.prepare(sql);
      if (!searchText.isEmpty())
        recQ.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        recQ.bindValue(":status", statusFilter);
      recQ.bindValue(":pageSize", pb->pageSize());
      recQ.bindValue(":offset", pb->offset());
      recQ.exec();
      int rRow = 0;
      while (recQ.next())
      {
        recTable->insertRow(rRow);
        recTable->setItem(rRow, 0, new QTableWidgetItem(recQ.value(0).toString()));
        recTable->setItem(rRow, 1, new QTableWidgetItem(recQ.value(1).toDateTime().toString("MM-dd hh:mm")));
        recTable->setItem(rRow, 2, new QTableWidgetItem(recQ.value(2).toDateTime().toString("MM-dd hh:mm")));
        recTable->setItem(rRow, 3, new QTableWidgetItem(recQ.value(3).toString()));
        recTable->setItem(rRow, 4, new QTableWidgetItem(recQ.value(4).toString()));
        int iSts = recQ.value(5).toInt();
        auto *stsItem = UiKit::createTagTableItem(iSts == 0 ? QStringLiteral("进行中") : QStringLiteral("已完成"), QColor(iSts == 0 ? "#e6f4ff" : "#f6ffed"), QColor(iSts == 0 ? "#b45309" : "#15803d"));
        recTable->setItem(rRow, 5, stsItem);
        rRow++;
      }
      UiKit::syncEmptyHint(recTable, recEmptyHint);
      pb->refreshData();
    };
    loadRecords();
    QObject::connect(recSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadRecords(); });
    QObject::connect(recStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadRecords(); });
    recordLayout->addWidget(recTable);
    recordLayout->addWidget(recEmptyHint);
    tabWidget->addTab(recordPage, QStringLiteral("巡查记录"));
    layout->addWidget(tabWidget);
    return page;
  }
  else if (sub == "care")
  {
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_heart"), QStringLiteral("重点人群关怀"), QStringLiteral("管理特殊群体信息、走访计划和关怀记录"), UiKit::moduleColor("care"), page));
    layout->addSpacing(12);

    auto *tabWidget = new QTabWidget(page);
    tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; padding: 8px 16px; }");

    // Group tab
    auto *groupPage = new QWidget();
    auto *groupLayout = new QVBoxLayout(groupPage);
    // Group search toolbar
    auto *grpToolbar = new QWidget(groupPage);
    grpToolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
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
    groupTable->horizontalHeader()->setStretchLastSection(true);
    groupTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    groupTable->setStyleSheet(UiKit::TABLE_STYLE);
    groupTable->setShowGrid(false);
    groupTable->verticalHeader()->setVisible(false);
    groupTable->setSortingEnabled(true);
    groupTable->setColumnCount(6);
    groupTable->setHorizontalHeaderLabels({QStringLiteral("居民姓名"), QStringLiteral("类型"), QStringLiteral("关怀等级"), QStringLiteral("走访频率"), QStringLiteral("责任人"), QStringLiteral("最近走访")});
    auto *groupEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无重点人员记录"), groupPage);
    std::function<void()> loadGroups = [groupTable, grpSearchEdit, careLevelCombo, groupEmptyHint]()
    {
      groupTable->setRowCount(0);
      QString sql = "SELECT r.name, sg.group_type, sg.care_level, sg.care_frequency, u.real_name, sg.last_visit_time FROM cm_special_group sg JOIN cm_resident r ON sg.resident_id = r.id LEFT JOIN sys_user u ON sg.care_worker_id = u.id WHERE sg.del_flag = 0";
      QString searchText = grpSearchEdit->text().trimmed();
      int careLevelFilter = careLevelCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND r.name LIKE :search";
      if (careLevelFilter >= 0)
        sql += " AND sg.care_level = :level";
      QSqlQuery grpQ(DatabaseManager::instance().database());
      grpQ.prepare(sql);
      if (!searchText.isEmpty())
        grpQ.bindValue(":search", "%" + searchText + "%");
      if (careLevelFilter >= 0)
        grpQ.bindValue(":level", careLevelFilter);
      grpQ.exec();
      int gRow = 0;
      while (grpQ.next())
      {
        groupTable->insertRow(gRow);
        groupTable->setItem(gRow, 0, new QTableWidgetItem(grpQ.value(0).toString()));
        groupTable->setItem(gRow, 1, new QTableWidgetItem(SpecialGroupType::label(grpQ.value(1).toInt())));
        int lvl = grpQ.value(2).toInt();
        auto *lvlItem = UiKit::createTagTableItem(lvl == 1 ? QStringLiteral("一般") : lvl == 2 ? QStringLiteral("重点")
                                                                                        : QStringLiteral("特殊"),
                                           QColor(lvl == 1 ? "#e6f4ff" : lvl == 2 ? "#fff7e6"
                                                                                  : "#fff1f0"),
                                           QColor(lvl == 1 ? "#b45309" : lvl == 2 ? "#d97706"
                                                                                  : "#b91c1c"));
        groupTable->setItem(gRow, 2, lvlItem);
        groupTable->setItem(gRow, 3, new QTableWidgetItem(grpQ.value(3).toString()));
        groupTable->setItem(gRow, 4, new QTableWidgetItem(grpQ.value(4).toString()));
        groupTable->setItem(gRow, 5, new QTableWidgetItem(grpQ.value(5).toDateTime().toString("yyyy-MM-dd")));
        gRow++;
      }
      UiKit::syncEmptyHint(groupTable, groupEmptyHint);
    };
    loadGroups();
    QObject::connect(grpSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadGroups(); });
    QObject::connect(careLevelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadGroups(); });
    groupLayout->addWidget(groupTable);
    groupLayout->addWidget(groupEmptyHint);
    tabWidget->addTab(groupPage, QStringLiteral("关怀对象"));

    // Visit record tab
    auto *visitPage = new QWidget();
    auto *visitLayout = new QVBoxLayout(visitPage);
    // Visit search toolbar
    auto *visToolbar = new QWidget(visitPage);
    visToolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
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
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    auto *visitEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无走访记录"), visitPage);
    std::function<void()> loadVisits = [visitTable, visSearchEdit, visitTypeCombo, visitEmptyHint, pb]()
    {
      visitTable->setRowCount(0);
      QString sql = "SELECT u.real_name, vr.visit_time, vr.visit_type, vr.found_issues, vr.follow_up FROM ge_visit_record vr LEFT JOIN sys_user u ON vr.visitor_id = u.id WHERE vr.del_flag = 0";
      QString searchText = visSearchEdit->text().trimmed();
      int visitTypeFilter = visitTypeCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND u.real_name LIKE :search";
      if (visitTypeFilter >= 0)
        sql += " AND vr.visit_type = :vtype";
      sql += " ORDER BY vr.visit_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery visQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (visitTypeFilter >= 0)
        cntBinds << ":vtype" << (visitTypeFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      visQ.prepare(sql);
      if (!searchText.isEmpty())
        visQ.bindValue(":search", "%" + searchText + "%");
      if (visitTypeFilter >= 0)
        visQ.bindValue(":vtype", visitTypeFilter);
      visQ.bindValue(":pageSize", pb->pageSize());
      visQ.bindValue(":offset", pb->offset());
      visQ.exec();
      int vRow = 0;
      while (visQ.next())
      {
        visitTable->insertRow(vRow);
        visitTable->setItem(vRow, 0, new QTableWidgetItem(visQ.value(0).toString()));
        visitTable->setItem(vRow, 1, new QTableWidgetItem(visQ.value(1).toDateTime().toString("yyyy-MM-dd hh:mm")));
        visitTable->setItem(vRow, 2, new QTableWidgetItem(visQ.value(2).toInt() == 1 ? QStringLiteral("定期走访") : visQ.value(2).toInt() == 2 ? QStringLiteral("临时走访")
                                                                                                                                               : QStringLiteral("电话慰问")));
        visitTable->setItem(vRow, 3, new QTableWidgetItem(visQ.value(3).toString()));
        visitTable->setItem(vRow, 4, new QTableWidgetItem(visQ.value(4).toString()));
        vRow++;
      }
      UiKit::syncEmptyHint(visitTable, visitEmptyHint);
      pb->refreshData();
    };
    loadVisits();
    QObject::connect(visSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadVisits(); });
    QObject::connect(visitTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadVisits(); });
    visitLayout->addWidget(visitTable);
    visitLayout->addWidget(visitEmptyHint);
    tabWidget->addTab(visitPage, QStringLiteral("走访记录"));
    layout->addWidget(tabWidget);
    return page;
  }
  else if (sub == "supervision")
  {
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_list"), QStringLiteral("督办管理"), QStringLiteral("督办任务创建、进度跟踪和催办管理"), UiKit::moduleColor("supervision"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索事件/督办人..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("待反馈"), 0);
    statusCombo->addItem(QStringLiteral("已反馈"), 1);
    statusCombo->addItem(QStringLiteral("已验收"), 2);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    auto *newSupBtn = new QPushButton(QStringLiteral("+ 创建督办"), toolbar);
    newSupBtn->setProperty("cssClass", "primary");
    newSupBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(newSupBtn);
    layout->addWidget(toolbar);

    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({QStringLiteral("关联事件"), QStringLiteral("督办人"), QStringLiteral("被督办人"), QStringLiteral("截止日期"), QStringLiteral("原因"), QStringLiteral("反馈"), QStringLiteral("状态"), QStringLiteral("操作")});
    table->setColumnWidth(4, 150);
    table->setColumnWidth(5, 150);
    table->setColumnWidth(7, 90);
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto supActionHandlerPtr = std::make_shared<std::function<void(qint64, int)>>();
    std::function<void()> loadSupervisions = [table, searchEdit, statusCombo, supActionHandlerPtr, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT s.id, e.title, u1.real_name, u2.real_name, s.deadline, s.reason, s.feedback, s.status FROM ge_supervision s LEFT JOIN ge_event e ON s.event_id = e.id LEFT JOIN sys_user u1 ON s.supervisor_id = u1.id LEFT JOIN sys_user u2 ON s.supervise_to = u2.id WHERE s.del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (e.title LIKE :search OR u1.real_name LIKE :search)";
      if (statusFilter >= 0)
        sql += " AND s.status = :status";
      sql += " ORDER BY s.create_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery supQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (statusFilter >= 0)
        cntBinds << ":status" << (statusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      supQ.prepare(sql);
      if (!searchText.isEmpty())
        supQ.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        supQ.bindValue(":status", statusFilter);
      supQ.bindValue(":pageSize", pb->pageSize());
      supQ.bindValue(":offset", pb->offset());
      supQ.exec();
      int sRow = 0;
      while (supQ.next())
      {
        table->insertRow(sRow);
        qint64 supId = supQ.value(0).toLongLong();
        table->setItem(sRow, 0, new QTableWidgetItem(supQ.value(1).toString()));
        table->setItem(sRow, 1, new QTableWidgetItem(supQ.value(2).toString()));
        table->setItem(sRow, 2, new QTableWidgetItem(supQ.value(3).toString()));
        table->setItem(sRow, 3, new QTableWidgetItem(supQ.value(4).toDateTime().toString("yyyy-MM-dd")));
        table->setItem(sRow, 4, new QTableWidgetItem(supQ.value(5).toString()));
        table->setItem(sRow, 5, new QTableWidgetItem(supQ.value(6).toString()));
        int sSts = supQ.value(7).toInt();
        auto *stsItem = UiKit::createTagTableItem(sSts == 0 ? QStringLiteral("待反馈") : sSts == 1 ? QStringLiteral("已反馈")
                                                                                            : QStringLiteral("已验收"),
                                           QColor(sSts == 0 ? "#fff7e6" : sSts == 1 ? "#e6f4ff"
                                                                                    : "#f6ffed"),
                                           QColor(sSts == 0 ? "#d97706" : sSts == 1 ? "#b45309"
                                                                                    : "#15803d"));
        table->setItem(sRow, 6, stsItem);
        // 操作列: status=0 显示"反馈", status=1 显示"验收"
        QString actText;
        QString actColor;
        if (sSts == 0)
        {
          actText = QStringLiteral("反馈");
          actColor = "#a16207";
        }
        else if (sSts == 1)
        {
          actText = QStringLiteral("验收");
          actColor = "#15803d";
        }
        else
        {
          actText = QStringLiteral("-");
          actColor = "#64748b";
        }
        table->setItem(sRow, 7, UiKit::createActionItem(actText, actColor, supId, sSts));
        sRow++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadSupervisions(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadSupervisions(); });

    // 操作列点击处理 - 反馈/验收
    std::function<void(qint64, int)> handleSupervisionAction = [page, loadSupervisions](qint64 supId, int sSts)
    {
      if (sSts != 0 && sSts != 1)
        return;

      if (sSts == 0)
      {
        // 反馈对话框
        QDialog dlg(page);
        dlg.setWindowTitle(QStringLiteral("督办反馈"));
        dlg.setMinimumWidth(450);
        auto *dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto *titleLabel = new QLabel(QStringLiteral("填写反馈内容"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(8);

        auto *form = new QFormLayout(&dlg);
        form->setSpacing(12);
        auto *feedbackEdit = new QTextEdit(&dlg);
        feedbackEdit->setPlaceholderText(QStringLiteral("请输入反馈内容..."));
        feedbackEdit->setFixedHeight(150);
        form->addRow(QStringLiteral("反馈内容:"), feedbackEdit);
        dlgLayout->addLayout(form);
        dlgLayout->addSpacing(12);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交反馈"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
                {
                    if (feedbackEdit->toPlainText().trimmed().isEmpty()) {
                        QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写反馈内容"));
                        return;
                    }
                    auto retFeedback = QMessageBox::question(page, QStringLiteral("确认操作"),
                        QStringLiteral("确认提交反馈？"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (retFeedback != QMessageBox::Yes) return;
                    DatabaseManager::instance().update("ge_supervision", supId, {
                        {"feedback", feedbackEdit->toPlainText().trimmed()},
                        {"feedback_time", QDateTime::currentDateTime()},
                        {"status", 1},
                        {"update_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("反馈提交成功"), page);
                    dlg.accept();
                    loadSupervisions(); });
        dlgLayout->addWidget(buttons);
        dlg.exec();
      }
      else if (sSts == 1)
      {
        // 验收确认
        auto ret = QMessageBox::question(page, QStringLiteral("确认验收"),
                                         QStringLiteral("确认验收该督办任务？验收后状态将变为已验收。"),
                                         QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes)
          return;
        DatabaseManager::instance().update("ge_supervision", supId, {{"status", 2}, {"update_time", QDateTime::currentDateTime()}});
        UiKit::showToast(QStringLiteral("验收成功"), page);
        loadSupervisions();
      }
    };
    *supActionHandlerPtr = handleSupervisionAction;
    QObject::connect(table, &QTableWidget::cellClicked, page, [table, supActionHandlerPtr](int row, int col)
            {
            if (col != 7) return;
            auto* item = table->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*supActionHandlerPtr) (*supActionHandlerPtr)(id, sts); });
    loadSupervisions();
    QObject::connect(table, &QTableWidget::cellEntered, page, [=](int r, int c)
            {
            auto* w = table->cellWidget(r, c);
            bool clickable = (c == 7 && w);
            table->viewport()->setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor); });

    // 创建督办按钮
    QObject::connect(newSupBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("创建督办"));
            dlg.setMinimumWidth(500);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* titleLabel = new QLabel(QStringLiteral("填写督办信息"), &dlg);
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
            dlgLayout->addWidget(titleLabel);
            dlgLayout->addSpacing(8);

            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            // 关联事件下拉框: 查询 ge_event 表中 status<4 的事件
            auto* eventCombo = new QComboBox(&dlg);
            QSqlQuery evtQ(DatabaseManager::instance().database());
            evtQ.exec("SELECT id, title FROM ge_event WHERE status < 4 AND del_flag = 0 ORDER BY create_time DESC");
            while (evtQ.next()) {
                eventCombo->addItem(evtQ.value(1).toString(), evtQ.value(0).toLongLong());
            }
            if (eventCombo->count() == 0) {
                QMessageBox::information(page, QStringLiteral("提示"), QStringLiteral("当前没有可督办的事件（需 status<4）"));
                return;
            }
            form->addRow(QStringLiteral("关联事件:"), eventCombo);
            // 督办对象下拉框: 查询社区工作人员（user_type=1 为工作人员，或通过角色查询）
            auto* targetCombo = new QComboBox(&dlg);
            QSqlQuery workerQ(DatabaseManager::instance().database());
            workerQ.exec("SELECT id, COALESCE(real_name, nickname, username) AS name FROM sys_user WHERE del_flag = 0 AND status = 0 ORDER BY id");
            while (workerQ.next()) {
                targetCombo->addItem(workerQ.value(1).toString(), workerQ.value(0).toLongLong());
            }
            form->addRow(QStringLiteral("督办对象:"), targetCombo);
            auto* reasonEdit = new QTextEdit(&dlg);
            reasonEdit->setPlaceholderText(QStringLiteral("请输入督办原因..."));
            reasonEdit->setFixedHeight(100);
            form->addRow(QStringLiteral("督办原因:"), reasonEdit);
            auto* deadlineEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(3), &dlg);
            deadlineEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
            deadlineEdit->setCalendarPopup(true);
            form->addRow(QStringLiteral("截止时间:"), deadlineEdit);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);

            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认创建"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (reasonEdit->toPlainText().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写督办原因"));
                    return;
                }
                const auto& user = AuthService::instance().currentUser();
                QDateTime now = QDateTime::currentDateTime();
                DatabaseManager::instance().insert("ge_supervision", {
                    {"event_id", eventCombo->currentData().toLongLong()},
                    {"supervisor_id", user.id},
                    {"supervise_to", targetCombo->currentData().toLongLong()},
                    {"deadline", deadlineEdit->dateTime()},
                    {"reason", reasonEdit->toPlainText().trimmed()},
                    {"status", 0},
                    {"create_by", user.id},
                    {"create_time", now},
                    {"update_by", user.id},
                    {"update_time", now}
                });
                UiKit::showToast(QStringLiteral("督办创建成功"), page);
                dlg.accept();
                loadSupervisions();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
  }
  else if (sub == "opinion")
  {
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_announce"), QStringLiteral("民意收集"), QStringLiteral("居民意见箱、建议收集和热点问题分析"), UiKit::moduleColor("opinion"), page));
    layout->addSpacing(12);

    // Stats cards
    auto *statsRow = new QHBoxLayout();
    auto createMiniCard = [](const QString &label, const QString &val, const QString &color, QWidget *parent)
    {
      auto *card = new QFrame(parent);
      card->setFixedHeight(90);
      card->setStyleSheet(QString("QFrame{background:#fff;border-radius:6px;border:1px solid #e2e8f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
      auto *cl = new QVBoxLayout(card);
      cl->setContentsMargins(16, 10, 16, 10);
      cl->setSpacing(4);
      auto *indicator = new QFrame(card);
      indicator->setFixedHeight(3);
      indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
      auto *tl = new QLabel(label);
      tl->setStyleSheet("color:#64748b;font-size:12px;");
      auto *vl = new QLabel(val);
      vl->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(color));
      cl->addWidget(indicator);
      cl->addWidget(tl);
      cl->addWidget(vl);
      UiKit::applyCardShadow(card);
      return card;
    };
    QSqlQuery opTotalQ("SELECT COUNT(*) FROM ge_opinion WHERE create_time >= date('now','start of month') AND del_flag = 0");
    int opTotal = opTotalQ.next() ? opTotalQ.value(0).toInt() : 0;
    QSqlQuery opReplyQ("SELECT COUNT(*) FROM ge_opinion WHERE status >= 1 AND del_flag = 0");
    int opReply = opReplyQ.next() ? opReplyQ.value(0).toInt() : 0;
    QSqlQuery opCatQ("SELECT COUNT(DISTINCT category) FROM ge_opinion WHERE del_flag = 0");
    int opCat = opCatQ.next() ? opCatQ.value(0).toInt() : 0;
    QSqlQuery opAdoptQ("SELECT COUNT(*) FROM ge_opinion WHERE status = 2 AND del_flag = 0");
    int opAdopt = opAdoptQ.next() ? opAdoptQ.value(0).toInt() : 0;
    int satRate = opTotal > 0 ? qRound((opReply + opAdopt) * 100.0 / opTotal) : 0;
    statsRow->addWidget(createMiniCard(QStringLiteral("本月意见"), QString::number(opTotal), "#b45309", page));
    statsRow->addWidget(createMiniCard(QStringLiteral("已回复"), QString::number(opReply), "#15803d", page));
    statsRow->addWidget(createMiniCard(QStringLiteral("热点类别"), QString::number(opCat), "#d97706", page));
    statsRow->addWidget(createMiniCard(QStringLiteral("响应率"), QString::number(satRate) + "%", "#15803d", page));
    layout->addLayout(statsRow);
    layout->addSpacing(12);

    // Toolbar
    auto *opToolbar = new QHBoxLayout();
    opToolbar->setSpacing(12);
    auto *opSearchEdit = new QLineEdit();
    opSearchEdit->setPlaceholderText(QStringLiteral("搜索意见内容..."));
    opSearchEdit->setMaximumWidth(260);

    opToolbar->addWidget(opSearchEdit);
    auto *opCatCombo = new QComboBox();
    opCatCombo->addItem(QStringLiteral("全部类别"), -1);
    opCatCombo->addItem(QStringLiteral("环境"), QStringLiteral("环境"));
    opCatCombo->addItem(QStringLiteral("设施"), QStringLiteral("设施"));
    opCatCombo->addItem(QStringLiteral("安全"), QStringLiteral("安全"));
    opCatCombo->addItem(QStringLiteral("服务"), QStringLiteral("服务"));
    opCatCombo->addItem(QStringLiteral("其他"), QStringLiteral("其他"));
    opCatCombo->setMinimumWidth(120);
    opToolbar->addWidget(opCatCombo);
    auto *opStatusCombo = new QComboBox();
    opStatusCombo->addItem(QStringLiteral("全部状态"), -1);
    opStatusCombo->addItem(QStringLiteral("待处理"), 0);
    opStatusCombo->addItem(QStringLiteral("已回复"), 1);
    opStatusCombo->addItem(QStringLiteral("已采纳"), 2);
    opStatusCombo->setMinimumWidth(120);
    opToolbar->addWidget(opStatusCombo);
    opToolbar->addStretch();
    layout->insertLayout(layout->count() - 1, opToolbar);

    // Opinions table
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("提交人"), QStringLiteral("类别"), QStringLiteral("内容摘要"), QStringLiteral("提交时间"), QStringLiteral("状态"), QStringLiteral("操作")});
    table->setColumnWidth(2, 250);
    table->setColumnWidth(5, 90);

    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto opActionHandlerPtr = std::make_shared<std::function<void(qint64, int, int)>>();
    std::function<void()> loadOpinions = [table, opSearchEdit, opCatCombo, opStatusCombo, opActionHandlerPtr, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT o.id, o.title, o.category, o.content, r.name, o.create_time, o.status, o.resident_id "
                    "FROM ge_opinion o LEFT JOIN cm_resident r ON o.resident_id = r.id "
                    "WHERE o.del_flag = 0";
      QString opSearch = opSearchEdit->text().trimmed();
      QString opCatFilter = opCatCombo->currentData().toString();
      int opStatusFilter = opStatusCombo->currentData().toInt();
      if (!opSearch.isEmpty())
        sql += " AND o.content LIKE :search";
      if (opCatFilter != "-1" && !opCatFilter.isEmpty())
        sql += " AND o.category = :cat";
      if (opStatusFilter >= 0)
        sql += " AND o.status = :status";
      sql += " ORDER BY o.create_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery opQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!opSearch.isEmpty())
        cntBinds << ":search" << ("%" + opSearch + "%");
      if (opCatFilter != "-1" && !opCatFilter.isEmpty())
        cntBinds << ":cat" << (opCatFilter);
      if (opStatusFilter >= 0)
        cntBinds << ":status" << (opStatusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      opQ.prepare(sql);
      if (!opSearch.isEmpty())
        opQ.bindValue(":search", "%" + opSearch + "%");
      if (opCatFilter != "-1" && !opCatFilter.isEmpty())
        opQ.bindValue(":cat", opCatFilter);
      if (opStatusFilter >= 0)
        opQ.bindValue(":status", opStatusFilter);
      opQ.bindValue(":pageSize", pb->pageSize());
      opQ.bindValue(":offset", pb->offset());
      opQ.exec();
      int oRow = 0;
      while (opQ.next())
      {
        table->insertRow(oRow);
        table->setItem(oRow, 0, new QTableWidgetItem(opQ.value(4).toString()));
        table->setItem(oRow, 1, new QTableWidgetItem(opQ.value(2).toString()));
        table->setItem(oRow, 2, new QTableWidgetItem(opQ.value(3).toString()));
        table->setItem(oRow, 3, new QTableWidgetItem(opQ.value(5).toDateTime().toString("yyyy-MM-dd")));
        int oSts = opQ.value(6).toInt();
        auto *stsItem = UiKit::createTagTableItem(oSts == 0 ? QStringLiteral("待处理") : oSts == 1 ? QStringLiteral("已回复")
                                                                                            : QStringLiteral("已采纳"),
                                           QColor(oSts == 0 ? "#fff7e6" : oSts == 1 ? "#e6f4ff"
                                                                                    : "#f6ffed"),
                                           QColor(oSts == 0 ? "#d97706" : oSts == 1 ? "#b45309"
                                                                                    : "#15803d"));
        table->setItem(oRow, 4, stsItem);
        // 操作列
        qint64 opId = opQ.value(0).toLongLong();
        int residentId = opQ.value(7).toInt();
        QString actText;
        QString actColor;
        if (oSts == 0)
        {
          actText = QStringLiteral("回复");
          actColor = "#15803d";
        }
        else
        {
          actText = QStringLiteral("查看");
          actColor = "#b45309";
        }
        table->setItem(oRow, 5, UiKit::createActionItem(actText, actColor, oSts, residentId));
        oRow++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    std::function<void(qint64, int, int)> handleOpinionAction = [page, loadOpinions](qint64 opId, int oSts, int residentId)
    {
      const auto &user = AuthService::instance().currentUser();

      if (oSts == 0)
      {
        // 回复: 弹出对话框，包含回复内容输入框和"采纳"复选框
        QDialog dlg(page);
        dlg.setWindowTitle(QStringLiteral("回复民意"));
        dlg.setMinimumWidth(450);
        auto *dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto *titleLabel = new QLabel(QStringLiteral("回复民意意见"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(8);

        auto *form = new QFormLayout(&dlg);
        form->setSpacing(12);
        auto *replyEdit = new QTextEdit(&dlg);
        replyEdit->setPlaceholderText(QStringLiteral("请填写回复内容..."));
        replyEdit->setFixedHeight(120);
        form->addRow(QStringLiteral("回复内容:"), replyEdit);
        auto *adoptCheck = new QCheckBox(QStringLiteral("采纳此建议"), &dlg);
        form->addRow(QStringLiteral("采纳:"), adoptCheck);
        dlgLayout->addLayout(form);
        dlgLayout->addSpacing(12);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认回复"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
                {
                    if (replyEdit->toPlainText().trimmed().isEmpty()) {
                        QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写回复内容"));
                        return;
                    }
                    auto retReply = QMessageBox::question(page, QStringLiteral("确认操作"),
                        QStringLiteral("确认提交回复？"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (retReply != QMessageBox::Yes) return;
                    int newSts = adoptCheck->isChecked() ? 2 : 1;
                    DatabaseManager::instance().update("ge_opinion", opId, {
                        {"reply_content", replyEdit->toPlainText().trimmed()},
                        {"reply_by", user.id},
                        {"reply_time", QDateTime::currentDateTime()},
                        {"status", newSts},
                        {"update_by", user.id},
                        {"update_time", QDateTime::currentDateTime()}
                    });
                    // 如果有 resident_id，发送通知给提交居民
                    if (residentId > 0) {
                        QSqlQuery userQ(DatabaseManager::instance().database());
                        userQ.prepare("SELECT user_id FROM cm_resident WHERE id = :rid");
                        userQ.bindValue(":rid", residentId);
                        if (userQ.exec() && userQ.next()) {
                            int userId = userQ.value(0).toInt();
                            if (userId > 0) {
                                page->requestNotification(userId,
                                    QStringLiteral("您的意见已回复"),
                                    QStringLiteral("您提交的民意意见已%1").arg(adoptCheck->isChecked() ? QStringLiteral("被采纳") : QStringLiteral("收到回复")),
                                    1, "opinion", (int)opId);
                            }
                        }
                    }
                    UiKit::showToast(QStringLiteral("回复成功"), page);
                    dlg.accept();
                    loadOpinions(); });
        dlgLayout->addWidget(buttons);
        dlg.exec();
      }
      else
      {
        // 查看: 显示详情
        QSqlQuery detail(DatabaseManager::instance().database());
        detail.prepare("SELECT title, content, reply_content, status FROM ge_opinion WHERE id = :id");
        detail.bindValue(":id", opId);
        if (detail.exec() && detail.next())
        {
          QString info = QStringLiteral("标题: %1\n\n内容: %2").arg(detail.value(0).toString(), detail.value(1).toString());
          if (!detail.value(2).isNull() && !detail.value(2).toString().isEmpty())
          {
            info += QStringLiteral("\n\n回复: %1").arg(detail.value(2).toString());
          }
          int s = detail.value(3).toInt();
          info += QStringLiteral("\n\n状态: %1").arg(s == 0 ? QStringLiteral("待处理") : s == 1 ? QStringLiteral("已回复")
                                                                                                : QStringLiteral("已采纳"));
          QMessageBox::information(page, QStringLiteral("民意详情"), info);
        }
      }
    };
    *opActionHandlerPtr = handleOpinionAction;
    QObject::connect(table, &QTableWidget::cellClicked, page, [table, opActionHandlerPtr](int row, int col)
            {
            if (col != 5) return;
            auto* item = table->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*opActionHandlerPtr) (*opActionHandlerPtr)(id, sts, item->data(Qt::UserRole + 3).toInt()); });
    loadOpinions();
    QObject::connect(table, &QTableWidget::cellEntered, page, [=](int r, int c)
            { table->viewport()->setCursor(c == 5 ? Qt::PointingHandCursor : Qt::ArrowCursor); });
    QObject::connect(opSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadOpinions(); });
    QObject::connect(opCatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadOpinions(); });
    QObject::connect(opStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadOpinions(); });
  }
  else if (sub == "assessment")
  {
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_star"), QStringLiteral("考核管理"), QStringLiteral("考核指标设置、完成率统计和排名展示"), UiKit::moduleColor("assessment"), page));
    layout->addSpacing(12);

    // Stats
    auto *statsRow = new QHBoxLayout();
    auto createStatCard = [](const QString &label, const QString &val, const QString &color, QWidget *parent)
    {
      auto *card = new QFrame(parent);
      card->setFixedHeight(90);
      card->setStyleSheet(QString("QFrame{background:#fff;border-radius:6px;border:1px solid #e2e8f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(color));
      auto *cl = new QVBoxLayout(card);
      cl->setContentsMargins(16, 10, 16, 10);
      cl->setSpacing(4);
      auto *indicator = new QFrame(card);
      indicator->setFixedHeight(3);
      indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(color));
      auto *tl = new QLabel(label);
      tl->setStyleSheet("color:#64748b;font-size:12px;");
      auto *vl = new QLabel(val);
      vl->setStyleSheet(QString("color:%1;font-size:26px;font-weight:bold;").arg(color));
      cl->addWidget(indicator);
      cl->addWidget(tl);
      cl->addWidget(vl);
      return card;
    };
    QSqlQuery asmtCntQ("SELECT COUNT(DISTINCT config_id) FROM kf_assessment_config");
    int asmtCnt = asmtCntQ.next() ? asmtCntQ.value(0).toInt() : 0;
    QSqlQuery avgRateQ("SELECT AVG(actual_value) FROM kf_assessment_result WHERE del_flag = 0");
    double avgRate = avgRateQ.next() ? avgRateQ.value(0).toDouble() : 0;
    QSqlQuery excellentQ("SELECT COUNT(DISTINCT target_user_id) FROM kf_assessment_result WHERE del_flag = 0 AND score >= 90");
    int excellent = excellentQ.next() ? excellentQ.value(0).toInt() : 0;
    QSqlQuery timeoutQ("SELECT COUNT(*) FROM ge_event WHERE del_flag = 0 AND sla_deadline IS NOT NULL AND finish_time > sla_deadline");
    int timeout = timeoutQ.next() ? timeoutQ.value(0).toInt() : 0;
    statsRow->addWidget(createStatCard(QStringLiteral("考核指标"), QString::number(asmtCnt), "#b45309", page));
    statsRow->addWidget(createStatCard(QStringLiteral("平均完成率"), QString::number(qRound(avgRate)) + "%", "#15803d", page));
    statsRow->addWidget(createStatCard(QStringLiteral("优秀网格员"), QString::number(excellent), "#d97706", page));
    statsRow->addWidget(createStatCard(QStringLiteral("超时事件"), QString::number(timeout), "#b91c1c", page));
    layout->addLayout(statsRow);
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索网格员姓名..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *periodCombo = new QComboBox(toolbar);
    periodCombo->addItem(QStringLiteral("全部周期"), QString());
    // Task 19: 动态查询周期 - 从 kf_assessment_config.assessment_period 查询
    {
      QSqlQuery periodQ(DatabaseManager::instance().database());
      periodQ.prepare("SELECT DISTINCT assessment_period FROM kf_assessment_config "
                      "WHERE assessment_period IS NOT NULL AND assessment_period != '' AND del_flag = 0 "
                      "ORDER BY assessment_period DESC");
      periodQ.exec();
      bool hasData = false;
      while (periodQ.next())
      {
        QString p = periodQ.value(0).toString();
        // 转换 "2026-06" 为 "2026年6月"
        QString display;
        if (p.length() >= 7)
        {
          int year = p.mid(0, 4).toInt();
          int month = p.mid(5, 2).toInt();
          if (year > 0 && month > 0 && month <= 12)
          {
            display = QStringLiteral("%1年%2月").arg(year).arg(month);
          }
        }
        if (display.isEmpty())
          display = p;
        periodCombo->addItem(display, p);
        hasData = true;
      }
      // 如果没有数据，显示当前月份和前两个月作为默认
      if (!hasData)
      {
        QDate today = QDate::currentDate();
        for (int i = 0; i < 3; ++i)
        {
          QDate d = today.addMonths(-i);
          QString p = QString("%1-%2").arg(d.year()).arg(d.month(), 2, 10, QChar('0'));
          QString display = QStringLiteral("%1年%2月").arg(d.year()).arg(d.month());
          periodCombo->addItem(display, p);
        }
      }
    }
    periodCombo->setMinimumWidth(120);
    tbLayout->addWidget(periodCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    // Ranking table
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("排名"), QStringLiteral("网格员"), QStringLiteral("处理事件"), QStringLiteral("平均时效(h)"), QStringLiteral("完成率"), QStringLiteral("评分")});
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadAssessments = [table, searchEdit, periodCombo, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT ar.target_user_id, u.real_name, COUNT(*) as event_count, "
                    "AVG(ar.actual_value) as avg_actual, AVG(ar.score) as avg_score, MIN(ar.rank) as best_rank "
                    "FROM kf_assessment_result ar "
                    "LEFT JOIN sys_user u ON ar.target_user_id = u.id "
                    "WHERE ar.del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      QString periodFilter = periodCombo->currentData().toString();
      if (!searchText.isEmpty())
        sql += " AND u.real_name LIKE :search";
      if (!periodFilter.isEmpty())
        sql += " AND ar.period = :period";
      sql += " GROUP BY ar.target_user_id ORDER BY avg_score DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery rankQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (!periodFilter.isEmpty())
        cntBinds << ":period" << (periodFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      rankQ.prepare(sql);
      if (!searchText.isEmpty())
        rankQ.bindValue(":search", "%" + searchText + "%");
      if (!periodFilter.isEmpty())
        rankQ.bindValue(":period", periodFilter);
      rankQ.bindValue(":pageSize", pb->pageSize());
      rankQ.bindValue(":offset", pb->offset());
      rankQ.exec();
      int rRow = 0;
      while (rankQ.next())
      {
        table->insertRow(rRow);
        auto *rankItem = new QTableWidgetItem(QString::number(rRow + 1));
        if (rRow < 3)
        {
          rankItem->setBackground(QColor("#fff7e6"));
          rankItem->setForeground(QColor("#d97706"));
        }
        table->setItem(rRow, 0, rankItem);
        table->setItem(rRow, 1, new QTableWidgetItem(rankQ.value(1).toString()));
        table->setItem(rRow, 2, new QTableWidgetItem(rankQ.value(2).toString()));
        table->setItem(rRow, 3, new QTableWidgetItem(QString::number(rankQ.value(3).toDouble(), 'f', 1)));
        double avgActual = rankQ.value(3).toDouble();
        table->setItem(rRow, 4, new QTableWidgetItem(QString::number(avgActual, 'f', 1) + "%"));
        int score = qRound(rankQ.value(4).toDouble());
        auto *scoreItem = new QTableWidgetItem(QString::number(score));
        if (score >= 90)
        {
          scoreItem->setBackground(QColor("#f6ffed"));
          scoreItem->setForeground(QColor("#15803d"));
        }
        table->setItem(rRow, 5, scoreItem);
        rRow++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadAssessments();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadAssessments(); });
    QObject::connect(periodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadAssessments(); });
  }

  layout->addWidget(table);
  layout->addWidget(emptyHint);
  return page;
}

