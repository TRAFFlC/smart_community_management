#include "pages/PageFactory.h"
#include "PagesCommon.h"

using namespace UiKit;
// ========== Archive Pages ==========
BasePage *PageFactory::createArchivePage(const QString &sub)
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  table->horizontalHeader()->setStretchLastSection(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setSortingEnabled(true);

  auto &db = DatabaseManager::instance();

  // 空状态提示（根据 sub 定制文案）
  QString archiveEmptyText;
  if (sub == "org")
    archiveEmptyText = QStringLiteral("暂无组织记录");
  else if (sub == "estate")
    archiveEmptyText = QStringLiteral("暂无小区记录");
  else if (sub == "house")
    archiveEmptyText = QStringLiteral("暂无房屋记录");
  else if (sub == "resident")
    archiveEmptyText = QStringLiteral("暂无居民记录");
  else if (sub == "vehicle")
    archiveEmptyText = QStringLiteral("暂无车辆记录");
  else if (sub == "facility")
    archiveEmptyText = QStringLiteral("暂无设施记录");
  else if (sub == "grid")
    archiveEmptyText = QStringLiteral("暂无网格记录");
  else if (sub == "special")
    archiveEmptyText = QStringLiteral("暂无关怀对象");
  else
    archiveEmptyText = QStringLiteral("暂无数据");
  auto *emptyHint = createEmptyHintLabel(archiveEmptyText, page);

  if (sub == "org")
  {
    layout->addWidget(createPageHeader(QStringLiteral("ic_building"), QStringLiteral("组织管理"), QStringLiteral("管理街道、社区、物业、业委会等组织架构"), moduleColor(sub), page));
    layout->addSpacing(12);
    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索组织名称..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *orgTypeCombo = new QComboBox(toolbar);
    orgTypeCombo->addItem(QStringLiteral("全部类型"), -1);
    orgTypeCombo->addItem(QStringLiteral("街道"), 2);
    orgTypeCombo->addItem(QStringLiteral("社区"), 3);
    orgTypeCombo->addItem(QStringLiteral("物业"), 4);
    orgTypeCombo->addItem(QStringLiteral("业委会"), 5);
    orgTypeCombo->addItem(QStringLiteral("志愿"), 7);
    orgTypeCombo->addItem(QStringLiteral("社会"), 6);
    orgTypeCombo->setMinimumWidth(120);
    tbLayout->addWidget(orgTypeCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({QStringLiteral("组织名称"), QStringLiteral("类型"), QStringLiteral("负责人"), QStringLiteral("电话")});
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadOrgs = [table, searchEdit, orgTypeCombo, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT org_name, org_type, leader, phone FROM sys_org WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int typeFilter = orgTypeCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND org_name LIKE :search";
      if (typeFilter >= 0)
        sql += " AND org_type = :type";
      sql += " ORDER BY sort_order";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery q(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (typeFilter >= 0)
        cntBinds << ":type" << (typeFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(executeCountQuery(sql, cntBinds));

      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (typeFilter >= 0)
        q.bindValue(":type", typeFilter);
      q.bindValue(":pageSize", pb->pageSize());
      q.bindValue(":offset", pb->offset());
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(OrgType::label(q.value(1).toInt())));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
        row++;
      }
      syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadOrgs();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadOrgs(); });
    QObject::connect(orgTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadOrgs(); });
  }
  else if (sub == "estate")
  {
    layout->addWidget(createPageHeader(QStringLiteral("ic_home"), QStringLiteral("小区管理"), QStringLiteral("管理小区基本信息、楼栋和户数配置"), moduleColor(sub), page));
    layout->addSpacing(12);
    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索小区名称/编码..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *estateStatusCombo = new QComboBox(toolbar);
    estateStatusCombo->addItem(QStringLiteral("全部状态"), -1);
    estateStatusCombo->addItem(QStringLiteral("正常"), 0);
    estateStatusCombo->addItem(QStringLiteral("停用"), 1);
    estateStatusCombo->setMinimumWidth(120);
    tbLayout->addWidget(estateStatusCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("小区名称"), QStringLiteral("编码"), QStringLiteral("地址"), QStringLiteral("楼栋数"), QStringLiteral("总户数")});
    std::function<void()> loadEstates = [table, searchEdit, estateStatusCombo, emptyHint]()
    {
      table->setRowCount(0);
      QString sql = "SELECT estate_name, estate_code, address, total_buildings, total_houses FROM cm_estate WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = estateStatusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (estate_name LIKE :search OR estate_code LIKE :search)";
      if (statusFilter >= 0)
        sql += " AND status = :status";
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        q.bindValue(":status", statusFilter);
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 4, new QTableWidgetItem(q.value(4).toString()));
        row++;
      }
      syncEmptyHint(table, emptyHint);
    };
    loadEstates();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadEstates(); });
    QObject::connect(estateStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadEstates(); });
  }
  else if (sub == "house")
  {
    layout->addWidget(createPageHeader(QStringLiteral("ic_home"), QStringLiteral("房屋管理"), QStringLiteral("管理房屋信息、楼层、户型和居住状态"), moduleColor(sub), page));
    layout->addSpacing(12);
    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索房屋编号/房号..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("空置"), 0);
    statusCombo->addItem(QStringLiteral("自住"), 1);
    statusCombo->addItem(QStringLiteral("出租"), 2);
    statusCombo->addItem(QStringLiteral("出售"), 3);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("房屋编号"), QStringLiteral("楼层"), QStringLiteral("房号"), QStringLiteral("面积"), QStringLiteral("户型"), QStringLiteral("状态")});
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadHouses = [table, searchEdit, statusCombo, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT house_code, floor, room_number, area, house_type, house_status FROM cm_house WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (house_code LIKE :search OR room_number LIKE :search)";
      if (statusFilter >= 0)
        sql += " AND house_status = :status";
      sql += " ORDER BY house_code";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery q(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (statusFilter >= 0)
        cntBinds << ":status" << (statusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(executeCountQuery(sql, cntBinds));

      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        q.bindValue(":status", statusFilter);
      q.bindValue(":pageSize", pb->pageSize());
      q.bindValue(":offset", pb->offset());
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 4, new QTableWidgetItem(q.value(4).toString()));
        int houseSts = q.value(5).toInt();
        QColor hsBg, hsFg;
        QString hsText = HouseStatus::label(houseSts);
        switch (houseSts)
        {
        case HouseStatus::Vacant:
          hsBg = QColor("#f8fafc");
          hsFg = QColor("#64748b");
          break;
        case HouseStatus::OwnerOccupied:
          hsBg = QColor("#eff6ff");
          hsFg = QColor("#2563eb");
          break;
        case HouseStatus::Rented:
          hsBg = QColor("#f0fdf4");
          hsFg = QColor("#15803d");
          break;
        default:
          hsBg = QColor("#fffbeb");
          hsFg = QColor("#d97706");
          break;
        }
        auto *houseStsItem = createTagTableItem(hsText, hsBg, hsFg);
        table->setItem(row, 5, houseStsItem);
        row++;
      }
      syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadHouses();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadHouses(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadHouses(); });
  }
  else if (sub == "resident")
  {
    layout->addWidget(createPageHeader(QStringLiteral("ic_people"), QStringLiteral("居民管理"), QStringLiteral("管理居民档案、联系方式和基本信息"), moduleColor(sub), page));
    layout->addSpacing(12);
    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索姓名/电话..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *genderCombo = new QComboBox(toolbar);
    genderCombo->addItem(QStringLiteral("全部性别"), -1);
    genderCombo->addItem(QStringLiteral("男"), 1);
    genderCombo->addItem(QStringLiteral("女"), 0);
    genderCombo->setMinimumWidth(120);
    tbLayout->addWidget(genderCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("姓名"), QStringLiteral("性别"), QStringLiteral("手机号"), QStringLiteral("民族"), QStringLiteral("职业")});
    std::function<void()> loadResidents = [table, searchEdit, genderCombo, emptyHint]()
    {
      table->setRowCount(0);
      QString sql = "SELECT name, gender, phone_display, nationality, occupation FROM cm_resident WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int genderFilter = genderCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (name LIKE :search OR phone_display LIKE :search)";
      if (genderFilter >= 0)
        sql += " AND gender = :gender";
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (genderFilter >= 0)
        q.bindValue(":gender", genderFilter);
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toInt() == 1 ? QStringLiteral("男") : QStringLiteral("女")));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 4, new QTableWidgetItem(q.value(4).toString()));
        row++;
      }
      syncEmptyHint(table, emptyHint);
    };
    loadResidents();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadResidents(); });
    QObject::connect(genderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadResidents(); });
  }
  else if (sub == "vehicle")
  {
    layout->addWidget(createPageHeader(QStringLiteral("ic_car"), QStringLiteral("车辆管理"), QStringLiteral("管理小区车辆登记、类型和停车信息"), moduleColor(sub), page));
    layout->addSpacing(12);
    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索车牌号..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *typeCombo = new QComboBox(toolbar);
    typeCombo->addItem(QStringLiteral("全部类型"), -1);
    typeCombo->addItem(QStringLiteral("轿车"), 1);
    typeCombo->addItem(QStringLiteral("SUV"), 2);
    typeCombo->addItem(QStringLiteral("电动车"), 3);
    typeCombo->addItem(QStringLiteral("其他"), 4);
    typeCombo->setMinimumWidth(120);
    tbLayout->addWidget(typeCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({QStringLiteral("车牌号"), QStringLiteral("品牌"), QStringLiteral("颜色"), QStringLiteral("类型")});
    std::function<void()> loadVehicles = [table, searchEdit, typeCombo, emptyHint]()
    {
      table->setRowCount(0);
      QString sql = "SELECT plate_number, vehicle_brand, vehicle_color, vehicle_type FROM cm_vehicle WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int typeFilter = typeCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND plate_number LIKE :search";
      if (typeFilter >= 0)
        sql += " AND vehicle_type = :type";
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (typeFilter >= 0)
        q.bindValue(":type", typeFilter);
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(VehicleType::label(q.value(3).toInt())));
        row++;
      }
      syncEmptyHint(table, emptyHint);
    };
    loadVehicles();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadVehicles(); });
    QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadVehicles(); });
  }
  else if (sub == "facility")
  {
    layout->addWidget(createPageHeader(QStringLiteral("ic_tool"), QStringLiteral("设施管理"), QStringLiteral("管理电梯、消防、监控等公共设施"), moduleColor(sub), page));
    layout->addSpacing(12);
    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索设施名称/编号..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *facTypeCombo = new QComboBox(toolbar);
    facTypeCombo->addItem(QStringLiteral("全部类型"), -1);
    facTypeCombo->addItem(QStringLiteral("电梯"), 1);
    facTypeCombo->addItem(QStringLiteral("消防"), 2);
    facTypeCombo->addItem(QStringLiteral("监控"), 3);
    facTypeCombo->addItem(QStringLiteral("门禁"), 4);
    facTypeCombo->addItem(QStringLiteral("照明"), 5);
    facTypeCombo->setMinimumWidth(120);
    tbLayout->addWidget(facTypeCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("设施名称"), QStringLiteral("类型"), QStringLiteral("编号"), QStringLiteral("位置"), QStringLiteral("状态")});
    std::function<void()> loadFacilities = [table, searchEdit, facTypeCombo, emptyHint]()
    {
      table->setRowCount(0);
      QString sql = "SELECT facility_name, facility_type, facility_code, location, status FROM cm_facility WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int typeFilter = facTypeCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (facility_name LIKE :search OR facility_code LIKE :search)";
      if (typeFilter >= 0)
        sql += " AND facility_type = :type";
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (typeFilter >= 0)
        q.bindValue(":type", typeFilter);
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(FacilityType::label(q.value(1).toInt())));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
        int facSts = q.value(4).toInt();
        auto *facStsItem = facSts == 0
                               ? createTagTableItem(QStringLiteral("正常"), QColor("#f0fdf4"), QColor("#15803d"))
                               : createTagTableItem(QStringLiteral("故障"), QColor("#fef2f2"), QColor("#b91c1c"));
        table->setItem(row, 4, facStsItem);
        row++;
      }
      syncEmptyHint(table, emptyHint);
    };
    loadFacilities();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadFacilities(); });
    QObject::connect(facTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadFacilities(); });
  }
  else if (sub == "grid")
  {
    layout->addWidget(createPageHeader(QStringLiteral("ic_route"), QStringLiteral("网格管理"), QStringLiteral("管理社区网格划分和网格员配置"), moduleColor(sub), page));
    layout->addSpacing(12);
    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索网格名称/编码..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *gridStatusCombo = new QComboBox(toolbar);
    gridStatusCombo->addItem(QStringLiteral("全部状态"), -1);
    gridStatusCombo->addItem(QStringLiteral("正常"), 0);
    gridStatusCombo->addItem(QStringLiteral("停用"), 1);
    gridStatusCombo->setMinimumWidth(120);
    tbLayout->addWidget(gridStatusCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({QStringLiteral("网格名称"), QStringLiteral("编码"), QStringLiteral("描述"), QStringLiteral("网格员")});
    std::function<void()> loadGrids = [table, searchEdit, gridStatusCombo, emptyHint]()
    {
      table->setRowCount(0);
      QString sql = "SELECT g.grid_name, g.grid_code, g.description, u.real_name FROM cm_grid g LEFT JOIN sys_user u ON g.grid_worker_id = u.id WHERE g.del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = gridStatusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (g.grid_name LIKE :search OR g.grid_code LIKE :search)";
      if (statusFilter >= 0)
        sql += " AND g.status = :status";
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        q.bindValue(":status", statusFilter);
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
        row++;
      }
      syncEmptyHint(table, emptyHint);
    };
    loadGrids();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadGrids(); });
    QObject::connect(gridStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadGrids(); });
  }
  else if (sub == "special")
  {
    layout->addWidget(createPageHeader(QStringLiteral("ic_heart"), QStringLiteral("特殊群体"), QStringLiteral("管理独居老人、残疾人等特殊群体关怀"), moduleColor(sub), page));
    layout->addSpacing(12);
    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索居民姓名..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *groupTypeCombo = new QComboBox(toolbar);
    groupTypeCombo->addItem(QStringLiteral("全部类型"), -1);
    groupTypeCombo->addItem(QStringLiteral("独居老人"), 1);
    groupTypeCombo->addItem(QStringLiteral("残疾人"), 2);
    groupTypeCombo->addItem(QStringLiteral("低保户"), 3);
    groupTypeCombo->addItem(QStringLiteral("留守儿童"), 4);
    groupTypeCombo->setMinimumWidth(120);
    tbLayout->addWidget(groupTypeCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({QStringLiteral("居民"), QStringLiteral("类型"), QStringLiteral("关怀等级"), QStringLiteral("走访频率")});
    std::function<void()> loadSpecials = [table, searchEdit, groupTypeCombo, emptyHint]()
    {
      table->setRowCount(0);
      QString sql = "SELECT r.name, sg.group_type, sg.care_level, sg.care_frequency FROM cm_special_group sg JOIN cm_resident r ON sg.resident_id = r.id WHERE sg.del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int typeFilter = groupTypeCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND r.name LIKE :search";
      if (typeFilter >= 0)
        sql += " AND sg.group_type = :type";
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (typeFilter >= 0)
        q.bindValue(":type", typeFilter);
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(SpecialGroupType::label(q.value(1).toInt())));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toInt() == 1 ? QStringLiteral("一般") : q.value(2).toInt() == 2 ? QStringLiteral("重点")
                                                                                                                               : QStringLiteral("特殊")));
        table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
        row++;
      }
      syncEmptyHint(table, emptyHint);
    };
    loadSpecials();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadSpecials(); });
    QObject::connect(groupTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadSpecials(); });
  }

  layout->addWidget(table);
  layout->addWidget(emptyHint);
  return page;
}


