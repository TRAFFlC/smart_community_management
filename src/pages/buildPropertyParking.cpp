#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

void PageFactory::buildPropertyParking(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // T33 停车管理
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_car"), QStringLiteral("停车管理"), QStringLiteral("车位管理、月卡办理和临停记录查看"), UiKit::moduleColor("parking"), page));
    layout->addSpacing(12);

    // Stats
    auto *statsRow = new QHBoxLayout();
    QSqlQuery spaceQ("SELECT COUNT(*) FROM cm_parking_space WHERE del_flag = 0");
    int totalSpaces = spaceQ.next() ? spaceQ.value(0).toInt() : 0;
    QSqlQuery occQ("SELECT COUNT(*) FROM cm_parking_space WHERE del_flag = 0 AND status = 1");
    int occSpaces = occQ.next() ? occQ.value(0).toInt() : 0;
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("总车位"), QString::number(totalSpaces), "#b45309", page));
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("已占用"), QString::number(occSpaces), "#15803d", page));
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("空余车位"), QString::number(totalSpaces - occSpaces), "#d97706", page));
    QSqlQuery cardQ("SELECT COUNT(*) FROM pm_monthly_card WHERE del_flag = 0 AND status = 1");
    int cardCount = cardQ.next() ? cardQ.value(0).toInt() : 0;
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("月卡数"), QString::number(cardCount), "#7c3aed", page));
    statsRow->addStretch();
    layout->addLayout(statsRow);
    layout->addSpacing(12);

    auto *tabWidget = new QTabWidget(page);
    tabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; padding: 8px 16px; }");

    // Parking spaces tab
    auto *spacePage = new QWidget();
    auto *spaceLayout = new QVBoxLayout(spacePage);
    // Space search toolbar
    auto *spaceToolbar = new QWidget(spacePage);
    spaceToolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
    auto *spaceTbLayout = new QHBoxLayout(spaceToolbar);
    spaceTbLayout->setContentsMargins(4, 4, 4, 4);
    spaceTbLayout->setSpacing(10);
    auto *spaceSearchEdit = new QLineEdit(spaceToolbar);
    spaceSearchEdit->setPlaceholderText(QStringLiteral("搜索车位编号/区域..."));
    spaceSearchEdit->setMinimumWidth(200);
    spaceSearchEdit->setClearButtonEnabled(true);
    spaceTbLayout->addWidget(spaceSearchEdit);
    auto *spaceStatusCombo = new QComboBox(spaceToolbar);
    spaceStatusCombo->addItem(QStringLiteral("全部状态"), -1);
    spaceStatusCombo->addItem(QStringLiteral("空闲"), 0);
    spaceStatusCombo->addItem(QStringLiteral("已占用"), 1);
    spaceStatusCombo->setMinimumWidth(120);
    spaceTbLayout->addWidget(spaceStatusCombo);
    spaceTbLayout->addStretch();
    spaceLayout->addWidget(spaceToolbar);

    auto *spaceTable = new QTableWidget(spacePage);
    spaceTable->setAlternatingRowColors(true);
    UiKit::configureTable(spaceTable);
    spaceTable->horizontalHeader()->setStretchLastSection(true);
    spaceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    spaceTable->setStyleSheet(UiKit::TABLE_STYLE);
    spaceTable->setShowGrid(false);
    spaceTable->verticalHeader()->setVisible(false);
    spaceTable->setSortingEnabled(true);
    spaceTable->setColumnCount(5);
    spaceTable->setHorizontalHeaderLabels({QStringLiteral("车位编号"), QStringLiteral("区域"), QStringLiteral("类型"), QStringLiteral("关联车辆"), QStringLiteral("状态")});
    auto *spaceEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无车位记录"), spacePage);
    std::function<void()> loadSpaces = [spaceTable, spaceSearchEdit, spaceStatusCombo, spaceEmptyHint]()
    {
      spaceTable->setSortingEnabled(false);
      spaceTable->setRowCount(0);
      QString sql = "SELECT ps.space_code, ps.area_name, ps.space_type, v.plate_number, ps.status FROM cm_parking_space ps LEFT JOIN cm_vehicle v ON v.parking_space_id = ps.id WHERE ps.del_flag = 0";
      QString searchText = spaceSearchEdit->text().trimmed();
      int statusFilter = spaceStatusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (ps.space_code LIKE :search OR ps.area_name LIKE :search)";
      if (statusFilter >= 0)
        sql += " AND ps.status = :status";
      QSqlQuery spQ(DatabaseManager::instance().database());
      spQ.prepare(sql);
      if (!searchText.isEmpty())
        spQ.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        spQ.bindValue(":status", statusFilter);
      spQ.exec();
      int spRow = 0;
      while (spQ.next())
      {
        spaceTable->insertRow(spRow);
        spaceTable->setItem(spRow, 0, new QTableWidgetItem(spQ.value(0).toString()));
        spaceTable->setItem(spRow, 1, new QTableWidgetItem(spQ.value(1).toString()));
        spaceTable->setItem(spRow, 2, new QTableWidgetItem(spQ.value(2).toInt() == 1 ? QStringLiteral("产权车位") : QStringLiteral("租赁车位")));
        spaceTable->setItem(spRow, 3, new QTableWidgetItem(spQ.value(3).toString()));
        int spSts = spQ.value(4).toInt();
        auto *stsItem = UiKit::createTagTableItem(spSts == 0 ? QStringLiteral("空闲") : QStringLiteral("已占用"), QColor(spSts == 0 ? "#f6ffed" : "#e6f4ff"), QColor(spSts == 0 ? "#15803d" : "#b45309"));
        spaceTable->setItem(spRow, 4, stsItem);
        spRow++;
      }
      UiKit::syncEmptyHint(spaceTable, spaceEmptyHint);
      spaceTable->setSortingEnabled(true);
    };
    loadSpaces();
    QObject::connect(spaceSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadSpaces(); });
    QObject::connect(spaceStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadSpaces(); });
    spaceLayout->addWidget(spaceTable);
    spaceLayout->addWidget(spaceEmptyHint);
    tabWidget->addTab(spacePage, QStringLiteral("车位列表"));

    // Monthly cards tab
    auto *cardPage = new QWidget();
    auto *cardLayout = new QVBoxLayout(cardPage);
    // Card search toolbar
    auto *cardToolbar = new QWidget(cardPage);
    cardToolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
    auto *cardTbLayout = new QHBoxLayout(cardToolbar);
    cardTbLayout->setContentsMargins(4, 4, 4, 4);
    cardTbLayout->setSpacing(10);
    auto *cardSearchEdit = new QLineEdit(cardToolbar);
    cardSearchEdit->setPlaceholderText(QStringLiteral("搜索车牌号/车主..."));
    cardSearchEdit->setMinimumWidth(200);
    cardSearchEdit->setClearButtonEnabled(true);
    cardTbLayout->addWidget(cardSearchEdit);
    auto *cardStatusCombo = new QComboBox(cardToolbar);
    cardStatusCombo->addItem(QStringLiteral("全部状态"), -1);
    cardStatusCombo->addItem(QStringLiteral("有效"), 1);
    cardStatusCombo->addItem(QStringLiteral("待续费"), 2);
    cardStatusCombo->addItem(QStringLiteral("已过期"), 0);
    cardStatusCombo->setMinimumWidth(120);
    cardTbLayout->addWidget(cardStatusCombo);
    cardTbLayout->addStretch();
    auto *newCardBtn = new QPushButton(QStringLiteral("+ 办理月卡"), cardToolbar);
    newCardBtn->setProperty("cssClass", "primary");
    newCardBtn->setCursor(Qt::PointingHandCursor);
    cardTbLayout->addWidget(newCardBtn);
    cardLayout->addWidget(cardToolbar);

    auto *cardTable = new QTableWidget(cardPage);
    cardTable->setAlternatingRowColors(true);
    UiKit::configureTable(cardTable);
    cardTable->horizontalHeader()->setStretchLastSection(true);
    cardTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cardTable->setStyleSheet(UiKit::TABLE_STYLE);
    cardTable->setShowGrid(false);
    cardTable->verticalHeader()->setVisible(false);
    cardTable->setSortingEnabled(true);
    cardTable->setColumnCount(7);
    cardTable->setHorizontalHeaderLabels({QStringLiteral("车牌号"), QStringLiteral("车主"), QStringLiteral("车位"), QStringLiteral("有效期"), QStringLiteral("状态"), QStringLiteral("卡类型"), QStringLiteral("操作")});
    cardTable->setColumnWidth(6, 90);
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto cardActionHandlerPtr = std::make_shared<std::function<void(qint64, int, int)>>();
    std::function<void()> loadCards = [cardTable, cardSearchEdit, cardStatusCombo, cardActionHandlerPtr, pb, page]()
    {
      cardTable->setSortingEnabled(false);
      cardTable->setRowCount(0);
      QString searchText = cardSearchEdit->text().trimmed();
      int statusFilter = cardStatusCombo->currentData().toInt();
      UiKit::PagedQuery pq("SELECT mc.id, mc.plate_no, mc.owner_name, ps.space_code, mc.start_date, mc.end_date, mc.status, mc.card_type "
                    "FROM pm_monthly_card mc LEFT JOIN cm_parking_space ps ON mc.space_id = ps.id "
                    "WHERE mc.del_flag = 0");
      if (!searchText.isEmpty())
        pq.where("(mc.plate_no LIKE :search OR mc.owner_name LIKE :search)", {{"search", "%" + searchText + "%"}});
      if (statusFilter >= 0)
        pq.where("mc.status = :status", {{"status", statusFilter}});
      pq.orderBy("mc.end_date DESC");

      auto result = pq.execute(pb->pageSize(), pb->offset());
      pb->setTotalCount(result.totalCount);
      int cRow = 0;
      for (const auto& row : result.rows)
      {
        cardTable->insertRow(cRow);
        qint64 cardId = row.value(0).toLongLong();
        cardTable->setItem(cRow, 0, new QTableWidgetItem(row.value(1).toString()));
        cardTable->setItem(cRow, 1, new QTableWidgetItem(row.value(2).toString()));
        cardTable->setItem(cRow, 2, new QTableWidgetItem(row.value(3).toString()));
        QString period = row.value(4).toString() + " ~ " + row.value(5).toString();
        cardTable->setItem(cRow, 3, new QTableWidgetItem(period));
        int cSts = row.value(6).toInt();
        auto *stsItem = UiKit::createTagTableItem(cSts == 1 ? QStringLiteral("有效") : cSts == 2 ? QStringLiteral("待续费")
                                                                                          : QStringLiteral("已过期"),
                                           QColor(cSts == 1 ? "#f6ffed" : cSts == 2 ? "#fff7e6"
                                                                                    : "#fff1f0"),
                                           QColor(cSts == 1 ? "#15803d" : cSts == 2 ? "#d97706"
                                                                                    : "#b91c1c"));
        cardTable->setItem(cRow, 4, stsItem);
        int cType = row.value(7).toInt();
        QString typeText = (cType == 0) ? QStringLiteral("月卡") : (cType == 1) ? QStringLiteral("季卡")
                                                                                : QStringLiteral("年卡");
        cardTable->setItem(cRow, 5, new QTableWidgetItem(typeText));
        // 操作列: status=0(已过期) 或 2(待续费) 时显示"续费"
        if (cSts == 0 || cSts == 2)
        {
            auto *renewBtn = new QPushButton(QStringLiteral("续费"), cardTable);
            renewBtn->setStyleSheet(QStringLiteral(
                "QPushButton{border:none; color:#b45309; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;}"
                "QPushButton:hover{text-decoration:underline;}"));
            renewBtn->setCursor(Qt::PointingHandCursor);
            QObject::connect(renewBtn, &QPushButton::clicked, page, [cardActionHandlerPtr, cardId, cSts, cType]() {
                if (*cardActionHandlerPtr) (*cardActionHandlerPtr)(cardId, cSts, cType);
            });
            cardTable->setCellWidget(cRow, 6, UiKit::createActionCell({renewBtn}, cardTable));
        }
        cRow++;
      }
      cardTable->setSortingEnabled(true);
    };
    QObject::connect(cardSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadCards(); });
    QObject::connect(cardStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadCards(); });
    cardLayout->addWidget(cardTable);

    // 操作列点击处理 - 续费
    std::function<void(qint64, int, int)> handleCardAction = [page, loadCards](qint64 cardId, int cSts, int cType)
    {
      if (cSts != 0 && cSts != 2)
        return;
      int addDays = (cType == 0) ? 30 : (cType == 1) ? 90
                                                     : 365;
      // 查询原 end_date
      QSqlQuery origQ(DatabaseManager::instance().database());
      origQ.prepare("SELECT end_date FROM pm_monthly_card WHERE id = :id");
      origQ.bindValue(":id", cardId);
      if (!origQ.exec() || !origQ.next())
        return;
      QDate origEnd = origQ.value(0).toDate();
      QDate today = QDate::currentDate();
      QDate baseDate = (origEnd < today) ? today : origEnd;
      QDate newEnd = baseDate.addDays(addDays);
      auto ret = QMessageBox::question(page,QStringLiteral("确认续费"),
                                       QStringLiteral("确认续费该月卡？\n续费后有效期至: %1").arg(newEnd.toString("yyyy-MM-dd")),
                                       QMessageBox::Yes | QMessageBox::No);
      if (ret != QMessageBox::Yes)
        return;
      DatabaseManager::instance().update("pm_monthly_card", cardId, {{"end_date", newEnd}, {"status", 1}, {"update_time", QDateTime::currentDateTime()}});
      UiKit::showToast(QStringLiteral("月卡续费成功"), page);
      loadCards();
    };
    *cardActionHandlerPtr = handleCardAction;
    loadCards();

    // 办理月卡按钮
    QObject::connect(newCardBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("办理月卡"));
            dlg.setMinimumWidth(450);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* titleLabel = new QLabel(QStringLiteral("填写月卡信息"), &dlg);
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
            dlgLayout->addWidget(titleLabel);
            dlgLayout->addSpacing(8);

            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* plateEdit = new QLineEdit(&dlg);
            plateEdit->setPlaceholderText(QStringLiteral("请输入车牌号，如 京A12345"));
            form->addRow(QStringLiteral("车牌号:"), plateEdit);
            auto* ownerEdit = new QLineEdit(&dlg);
            ownerEdit->setPlaceholderText(QStringLiteral("请输入车主姓名"));
            form->addRow(QStringLiteral("车主姓名:"), ownerEdit);
            auto* phoneEdit = new QLineEdit(&dlg);
            phoneEdit->setPlaceholderText(QStringLiteral("请输入联系电话"));
            form->addRow(QStringLiteral("联系电话:"), phoneEdit);
            auto* typeCombo = new QComboBox(&dlg);
            typeCombo->addItem(QStringLiteral("月卡 (¥150/30天)"), 0);
            typeCombo->addItem(QStringLiteral("季卡 (¥400/90天)"), 1);
            typeCombo->addItem(QStringLiteral("年卡 (¥1440/365天)"), 2);
            form->addRow(QStringLiteral("卡类型:"), typeCombo);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);

            // 费用提示标签
            auto* feeLabel = new QLabel(&dlg);
            feeLabel->setStyleSheet("color:#64748b;font-size:12px;");
            auto updateFee = [feeLabel, typeCombo]() {
                int idx = typeCombo->currentData().toInt();
                double fee = (idx == 0) ? 150.0 : (idx == 1) ? 400.0 : 1440.0;
                int days = (idx == 0) ? 30 : (idx == 1) ? 90 : 365;
                feeLabel->setText(QStringLiteral("费用: ¥%1  有效期: %2 天").arg(fee, 0, 'f', 0).arg(days));
            };
            updateFee();
            QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), updateFee);
            dlgLayout->addWidget(feeLabel);
            dlgLayout->addSpacing(8);

            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认办理"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (plateEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写车牌号"));
                    return;
                }
                if (ownerEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写车主姓名"));
                    return;
                }
                int cType = typeCombo->currentData().toInt();
                int addDays = (cType == 0) ? 30 : (cType == 1) ? 90 : 365;
                double fee = (cType == 0) ? 150.0 : (cType == 1) ? 400.0 : 1440.0;
                QDate today = QDate::currentDate();
                const auto& user = AuthService::instance().currentUser();
                QDateTime now = QDateTime::currentDateTime();
                DatabaseManager::instance().insert("pm_monthly_card", {
                    {"plate_no", plateEdit->text().trimmed()},
                    {"owner_name", ownerEdit->text().trimmed()},
                    {"owner_phone", phoneEdit->text().trimmed()},
                    {"card_type", cType},
                    {"start_date", today},
                    {"end_date", today.addDays(addDays)},
                    {"fee", fee},
                    {"status", 1},
                    {"create_by", user.id},
                    {"create_time", now},
                    {"update_by", user.id},
                    {"update_time", now}
                });
                UiKit::showToast(QStringLiteral("月卡办理成功"), page);
                dlg.accept();
                loadCards();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
    tabWidget->addTab(cardPage, QStringLiteral("月卡管理"));
    layout->addWidget(tabWidget);
    return;
}
