#include "pages/PageFactory.h"
#include "PagesCommon.h"

using namespace UiKit;
// ========== Governance Pages ==========
BasePage *PageFactory::createGovernancePage(const QString &sub)
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(TABLE_STYLE);
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
  auto *emptyHint = createEmptyHintLabel(govEmptyText, page);

  if (sub == "event")
  {
    // Page header
    layout->addWidget(createPageHeader(QStringLiteral("ic_route"), QStringLiteral("网格事件管理"), QStringLiteral("管理社区事件上报、审核分派、处理反馈、督办归档全流程"), moduleColor("event"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索事件号/标题..."));
    searchEdit->setFixedWidth(220);
    tbLayout->addWidget(searchEdit);
    auto *filterCombo = new QComboBox(toolbar);
    filterCombo->addItems({QStringLiteral("全部状态"), QStringLiteral("待审核"), QStringLiteral("已审核"), QStringLiteral("已分派"), QStringLiteral("处理中"), QStringLiteral("已完成"), QStringLiteral("已归档")});
    filterCombo->setFixedWidth(130);
    tbLayout->addWidget(filterCombo);

    auto *exportBtn = new QPushButton(QStringLiteral("导出"), toolbar);
    exportBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(exportBtn);

    auto *myTasksCheck = new QCheckBox(QStringLiteral("只看我的"), toolbar);
    tbLayout->addWidget(myTasksCheck);

    tbLayout->addStretch();
    auto *newBtn = new QPushButton(QStringLiteral("+ 上报事件"), toolbar);
    newBtn->setProperty("cssClass", "primary");
    newBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(newBtn);
    layout->addWidget(toolbar);

    // Table
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({QStringLiteral("事件号"), QStringLiteral("标题"), QStringLiteral("类别"), QStringLiteral("优先级"), QStringLiteral("状态"), QStringLiteral("上报人"), QStringLiteral("创建时间"), QStringLiteral("操作")});
    table->setColumnWidth(1, 180);
    table->setColumnWidth(7, 100);
    table->horizontalHeader()->setStretchLastSection(true);

    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto evActionHandlerPtr = std::make_shared<std::function<void(qint64, int)>>();
    auto loadEventsPtr = std::make_shared<std::function<void()>>();
    *loadEventsPtr = [page, table, searchEdit, filterCombo, myTasksCheck, evActionHandlerPtr, loadEventsPtr, emptyHint, pb]()
    {
      auto& loadEvents = *loadEventsPtr;
      table->setRowCount(0);
      QString sql = "SELECT id, event_no, title, event_category, priority, status, reporter_name, create_time, reporter_id, assign_to FROM ge_event WHERE del_flag = 0";
      // 注入数据权限过滤（居民只看本人，社区/街道按组织范围）
      auto scopeFilter = buildDataScopeFilter("", "community_org_id");
      sql += scopeFilter.first;
      QString searchText = searchEdit->text().trimmed();
      int filterIdx = filterCombo->currentIndex();
      bool onlyMine = myTasksCheck->isChecked();
      if (!searchText.isEmpty())
      {
        sql += " AND (event_no LIKE :search OR title LIKE :search)";
      }
      if (filterIdx > 0)
      {
        sql += " AND status = :status";
      }
      if (onlyMine)
      {
        sql += " AND assign_to = :myId";
      }
      sql += " ORDER BY create_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery q(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      // 数据权限过滤的绑定参数加入计数查询
      for (int i = 0; i + 1 < scopeFilter.second.size(); i += 2)
      {
        cntBinds << scopeFilter.second[i] << scopeFilter.second[i + 1];
      }
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (filterIdx > 0)
      {
        int sm[] = {0, 0, 1, 2, 3, 4, 5, 6};
        cntBinds << ":status" << sm[filterIdx];
      }
      if (onlyMine)
        cntBinds << ":myId" << AuthService::instance().currentUser().id;
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(executeCountQuery(sql, cntBinds));

      q.prepare(sql);
      // 数据权限过滤的绑定参数加入主查询
      for (int i = 0; i + 1 < scopeFilter.second.size(); i += 2)
      {
        q.bindValue(scopeFilter.second[i].toString(), scopeFilter.second[i + 1]);
      }
      if (!searchText.isEmpty())
      {
        q.bindValue(":search", "%" + searchText + "%");
      }
      if (filterIdx > 0)
      {
        int statusMap[] = {-1, 0, 1, 2, 3, 4, 6};
        q.bindValue(":status", statusMap[filterIdx]);
      }
      if (onlyMine)
        q.bindValue(":myId", AuthService::instance().currentUser().id);
      q.bindValue(":pageSize", pb->pageSize());
      q.bindValue(":offset", pb->offset());
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(EventCategory::label(q.value(3).toInt())));
        QColor priColor(EventPriority::color(q.value(4).toInt()));
        auto *priItem = createTagTableItem(EventPriority::label(q.value(4).toInt()), QColor(priColor.red(), priColor.green(), priColor.blue(), 30), priColor);
        table->setItem(row, 3, priItem);
        QColor evColor(EventStatus::color(q.value(5).toInt()));
        auto *statusItem = createTagTableItem(EventStatus::label(q.value(5).toInt()), QColor(evColor.red(), evColor.green(), evColor.blue(), 30), evColor);
        table->setItem(row, 4, statusItem);
        table->setItem(row, 5, new QTableWidgetItem(q.value(6).toString()));
        table->setItem(row, 6, new QTableWidgetItem(q.value(7).toDateTime().toString("yyyy-MM-dd hh:mm")));
        // Action buttons - 根据角色权限决定是否显示操作按钮
        int sts = q.value(5).toInt();
        qint64 evId = q.value(0).toLongLong();
        qint64 reporterId = q.value(8).toLongLong();
        qint64 assigneeId = q.value(9).toLongLong();
        const auto& curUser = AuthService::instance().currentUser();
        bool canEdit = (sts <= 1) && (reporterId == curUser.id);
        QList<QPushButton*> actionBtns;
        if (canEdit) {
            auto* editBtn = new QPushButton(QStringLiteral("编辑"));
            editBtn->setProperty("cssClass", "text");
            editBtn->setCursor(Qt::PointingHandCursor);
            QObject::connect(editBtn, &QPushButton::clicked, page, [page, evId, loadEventsPtr]() {
                // 查询当前事件数据
                QSqlQuery eq(DatabaseManager::instance().database());
                eq.prepare("SELECT title, event_category, priority, description, location FROM ge_event WHERE id = :id AND del_flag = 0");
                eq.bindValue(":id", evId);
                if (!eq.exec() || !eq.next()) return;
                QString curTitle = eq.value(0).toString();
                int curCat = eq.value(1).toInt();
                int curPrio = eq.value(2).toInt();
                QString curDesc = eq.value(3).toString();
                QString curLoc = eq.value(4).toString();

                QDialog dlg(page);
                dlg.setWindowTitle(QStringLiteral("编辑事件"));
                dlg.setMinimumWidth(500);
                auto* dlgLayout = new QVBoxLayout(&dlg);
                dlgLayout->setContentsMargins(24, 20, 24, 20);
                auto* formTitle = new QLabel(QStringLiteral("修改事件信息"), &dlg);
                formTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
                dlgLayout->addWidget(formTitle);
                dlgLayout->addSpacing(8);
                auto* form = new QFormLayout(&dlg);
                form->setSpacing(12);
                form->setLabelAlignment(Qt::AlignRight);
                auto* titleEdit = new QLineEdit(curTitle, &dlg);
                form->addRow(QStringLiteral("标题:"), titleEdit);
                auto* catCombo = new QComboBox(&dlg);
                catCombo->addItems({QStringLiteral("民生服务"), QStringLiteral("环境卫生"), QStringLiteral("设施安全"), QStringLiteral("邻里纠纷"), QStringLiteral("特殊帮扶"), QStringLiteral("市容秩序"), QStringLiteral("突发预警")});
                if (curCat >= 1 && curCat <= 7) catCombo->setCurrentIndex(curCat - 1);
                form->addRow(QStringLiteral("类别:"), catCombo);
                auto* prioCombo = new QComboBox(&dlg);
                prioCombo->addItems({QStringLiteral("一般"), QStringLiteral("重要"), QStringLiteral("紧急"), QStringLiteral("特急")});
                if (curPrio >= 1 && curPrio <= 4) prioCombo->setCurrentIndex(curPrio - 1);
                form->addRow(QStringLiteral("优先级:"), prioCombo);
                auto* descEdit = new QTextEdit(&dlg);
                descEdit->setPlainText(curDesc);
                descEdit->setFixedHeight(80);
                form->addRow(QStringLiteral("描述:"), descEdit);
                auto* locEdit = new QLineEdit(curLoc, &dlg);
                form->addRow(QStringLiteral("地点:"), locEdit);
                dlgLayout->addLayout(form);
                dlgLayout->addSpacing(12);
                auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
                buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("保存"));
                buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
                QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
                QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                    if (titleEdit->text().trimmed().isEmpty()) {
                        QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题"));
                        return;
                    }
                    const auto& editUser = AuthService::instance().currentUser();
                    DatabaseManager::instance().update("ge_event", evId, {
                        {"title", titleEdit->text().trimmed()},
                        {"event_category", catCombo->currentIndex() + 1},
                        {"priority", prioCombo->currentIndex() + 1},
                        {"description", descEdit->toPlainText().trimmed()},
                        {"location", locEdit->text().trimmed()},
                        {"update_by", editUser.id},
                        {"update_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("事件已更新"), page);
                    dlg.accept();
                    (*loadEventsPtr)();
                });
                dlgLayout->addWidget(buttons);
                dlg.exec();
            });
            actionBtns.append(editBtn);
        }
        if (canOperateEvent(sts, reporterId, assigneeId))
        {
          QString actionText;
          QString actionColor;
          if (sts == 0) { actionText = QStringLiteral("审核"); actionColor = "#b45309"; }
          else if (sts == 1) { actionText = QStringLiteral("分派"); actionColor = "#b45309"; }
          else if (sts == 2) { actionText = QStringLiteral("处理"); actionColor = "#a16207"; }
          else if (sts == 3) { actionText = QStringLiteral("完成"); actionColor = "#15803d"; }
          else { actionText = QStringLiteral("--"); actionColor = "#64748b"; }
          if (actionText != QStringLiteral("--")) {
            auto* mainBtn = new QPushButton(actionText);
            mainBtn->setStyleSheet(QString("QPushButton{border:none; color:%1; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;} QPushButton:hover{text-decoration:underline;}").arg(actionColor));
            mainBtn->setCursor(Qt::PointingHandCursor);
            QObject::connect(mainBtn, &QPushButton::clicked, page, [evActionHandlerPtr, evId, sts]() {
                if (*evActionHandlerPtr) (*evActionHandlerPtr)(evId, sts);
            });
            actionBtns.append(mainBtn);
          }
        }
        if (actionBtns.isEmpty()) {
            table->setItem(row, 7, createActionItem(QStringLiteral("--"), "#64748b", evId, sts));
        } else {
            table->setCellWidget(row, 7, createActionCell(actionBtns, table));
        }
        row++;
      }
      syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    std::function<void(qint64, int)> handleEventAction = [page, loadEventsPtr](qint64 id, int sts)
    {
      const auto &user = AuthService::instance().currentUser();

      // 权限校验：查询事件的 reporter_id 和 assign_to，校验当前用户是否有权操作
      qint64 reporterId = 0;
      qint64 assigneeId = 0;
      {
        QSqlQuery permQ(DatabaseManager::instance().database());
        permQ.prepare("SELECT reporter_id, assign_to FROM ge_event WHERE id = :id AND del_flag = 0");
        permQ.bindValue(":id", id);
        if (permQ.exec() && permQ.next())
        {
          reporterId = permQ.value(0).toLongLong();
          assigneeId = permQ.value(1).toLongLong();
        }
      }
      if (!canOperateEvent(sts, reporterId, assigneeId))
      {
        QMessageBox::warning(page, QStringLiteral("无权限"),
                             QStringLiteral("您没有权限执行此操作，请联系相关负责人。"));
        return;
      }

      if (sts == 0)
      {
        // 审核: status=0 -> status=1, 记录 reviewer_id 和 review_time
        auto retReview = QMessageBox::question(page, QStringLiteral("确认操作"),
                                               QStringLiteral("确认审核通过此事件？"),
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (retReview != QMessageBox::Yes)
          return;
        DatabaseManager::instance().update("ge_event", id, {{"status", 1}, {"reviewer_id", user.id}, {"review_time", QDateTime::currentDateTime()}, {"update_by", user.id}, {"update_time", QDateTime::currentDateTime()}});
        showToast(QStringLiteral("事件已审核"), page);
        (*loadEventsPtr)();
      }
      else if (sts == 1)
      {
        // 分派: 弹出对话框选择处理人, status=1 -> status=2
        QDialog dlg(page);
        dlg.setWindowTitle(QStringLiteral("分派事件"));
        dlg.setMinimumWidth(450);
        auto *dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto *titleLabel = new QLabel(QStringLiteral("选择处理人员"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(8);

        auto *form = new QFormLayout(&dlg);
        form->setSpacing(12);
        auto *workerCombo = new QComboBox(&dlg);
        // 查询社区工作人员角色用户
        QSqlQuery workerQ(DatabaseManager::instance().database());
        workerQ.prepare("SELECT u.id, u.nickname, u.username FROM sys_user u "
                        "JOIN sys_user_role ur ON u.id = ur.user_id "
                        "JOIN sys_role r ON ur.role_id = r.id "
                        "WHERE r.role_key IN ('community_worker', 'grid_worker') AND u.status = 0 AND u.del_flag = 0");
        workerQ.exec();
        while (workerQ.next())
        {
          QString name = workerQ.value(1).toString();
          if (name.isEmpty())
            name = workerQ.value(2).toString();
          workerCombo->addItem(name, workerQ.value(0).toInt());
        }
        form->addRow(QStringLiteral("处理人:"), workerCombo);
        auto *assignRemarkEdit = new QTextEdit(&dlg);
        assignRemarkEdit->setPlaceholderText(QStringLiteral("分派备注(可选)"));
        assignRemarkEdit->setFixedHeight(70);
        form->addRow(QStringLiteral("备注:"), assignRemarkEdit);
        dlgLayout->addLayout(form);
        dlgLayout->addSpacing(12);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认分派"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
                {
                    if (workerCombo->count() == 0) {
                        QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("没有可选的处理人员"));
                        return;
                    }
                    int workerId = workerCombo->currentData().toInt();
                    QString workerName = workerCombo->currentText();
                    auto retAssign = QMessageBox::question(page, QStringLiteral("确认操作"),
                        QStringLiteral("确认将事件分派给 %1？").arg(workerName),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (retAssign != QMessageBox::Yes) return;
                    DatabaseManager::instance().update("ge_event", id, {
                        {"status", 2},
                        {"assign_to", workerId},
                        {"assign_time", QDateTime::currentDateTime()},
                        {"update_by", user.id},
                        {"update_time", QDateTime::currentDateTime()}
                    });
                    // 记录流转日志
                    DatabaseManager::instance().insert("ge_event_flow", {
                        {"event_id", id}, {"action", QStringLiteral("分派")},
                        {"operator_id", user.id}, {"operator_name", user.nickname.isEmpty() ? user.username : user.nickname},
                        {"from_status", 1}, {"to_status", 2},
                        {"comment", assignRemarkEdit->toPlainText().trimmed()},
                        {"action_time", QDateTime::currentDateTime()}
                    });
                    // 发送通知给处理人
                    page->requestNotification(workerId, QStringLiteral("新事件已分派"),
                        QStringLiteral("您有一个新的社区事件待处理"), 2, "event", (int)id);
                    showToast(QStringLiteral("事件分派成功"), page);
                    dlg.accept();
                    (*loadEventsPtr)(); });
        dlgLayout->addWidget(buttons);
        dlg.exec();
      }
      else if (sts == 2)
      {
        // 处理: status=2 -> status=3
        auto retStart = QMessageBox::question(page, QStringLiteral("确认操作"),
                                              QStringLiteral("确认开始处理此事件？"),
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (retStart != QMessageBox::Yes)
          return;
        DatabaseManager::instance().update("ge_event", id, {{"status", 3}, {"update_by", user.id}, {"update_time", QDateTime::currentDateTime()}});
        DatabaseManager::instance().insert("ge_event_flow", {{"event_id", id}, {"action", QStringLiteral("开始处理")}, {"operator_id", user.id}, {"operator_name", user.nickname.isEmpty() ? user.username : user.nickname}, {"from_status", 2}, {"to_status", 3}, {"action_time", QDateTime::currentDateTime()}});
        showToast(QStringLiteral("事件已开始处理"), page);
        (*loadEventsPtr)();
      }
      else if (sts == 3)
      {
        // 完成: 弹出对话框填写处理结果, status=3 -> status=4
        QDialog dlg(page);
        dlg.setWindowTitle(QStringLiteral("填写处理结果"));
        dlg.setMinimumWidth(450);
        auto *dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto *titleLabel = new QLabel(QStringLiteral("填写事件处理结果"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(8);

        auto *form = new QFormLayout(&dlg);
        form->setSpacing(12);
        auto *resultEdit = new QTextEdit(&dlg);
        resultEdit->setPlaceholderText(QStringLiteral("请描述处理过程和结果..."));
        resultEdit->setFixedHeight(100);
        form->addRow(QStringLiteral("处理结果:"), resultEdit);
        dlgLayout->addLayout(form);
        dlgLayout->addSpacing(12);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认完成"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
                {
                    if (resultEdit->toPlainText().trimmed().isEmpty()) {
                        QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写处理结果"));
                        return;
                    }
                    auto retFinish = QMessageBox::question(page, QStringLiteral("确认操作"),
                        QStringLiteral("确认完成此事件？"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (retFinish != QMessageBox::Yes) return;
                    DatabaseManager::instance().update("ge_event", id, {
                        {"status", 4},
                        {"finish_time", QDateTime::currentDateTime()},
                        {"result_desc", resultEdit->toPlainText().trimmed()},
                        {"update_by", user.id},
                        {"update_time", QDateTime::currentDateTime()}
                    });
                    DatabaseManager::instance().insert("ge_event_flow", {
                        {"event_id", id}, {"action", QStringLiteral("完成归档")},
                        {"operator_id", user.id}, {"operator_name", user.nickname.isEmpty() ? user.username : user.nickname},
                        {"from_status", 3}, {"to_status", 4},
                        {"comment", resultEdit->toPlainText().trimmed()},
                        {"action_time", QDateTime::currentDateTime()}
                    });
                    // 发送通知给上报人
                    QSqlQuery repQ(DatabaseManager::instance().database());
                    repQ.prepare("SELECT reporter_id, title FROM ge_event WHERE id = :id");
                    repQ.bindValue(":id", id);
                    if (repQ.exec() && repQ.next()) {
                        int reporterId = repQ.value(0).toInt();
                        QString title = repQ.value(1).toString();
                        if (reporterId > 0) {
                            page->requestNotification(reporterId,
                                QStringLiteral("事件已处理: %1").arg(title),
                                QStringLiteral("您上报的事件已处理完成"), 1, "event", (int)id);
                        }
                    }
                    showToast(QStringLiteral("事件已完成"), page);
                    dlg.accept();
                    (*loadEventsPtr)(); });
        dlgLayout->addWidget(buttons);
        dlg.exec();
      }
    };
    *evActionHandlerPtr = handleEventAction;
    QObject::connect(table, &QTableWidget::cellClicked, page, [table, evActionHandlerPtr](int row, int col)
            {
            if (col != 7) return;
            auto* item = table->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*evActionHandlerPtr) (*evActionHandlerPtr)(id, sts); });
    (*loadEventsPtr)();
    QObject::connect(table, &QTableWidget::cellEntered, page, [=](int r, int c)
            { table->viewport()->setCursor(c == 7 ? Qt::PointingHandCursor : Qt::ArrowCursor); });
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { (*loadEventsPtr)(); });
    QObject::connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { (*loadEventsPtr)(); });
    QObject::connect(myTasksCheck, &QCheckBox::toggled, page, [=]()
            { (*loadEventsPtr)(); });
    QObject::connect(exportBtn, &QPushButton::clicked, page, [table, page]()
            { exportTableToCsv(table, QStringLiteral("事件列表.csv"), page); });

    // New event dialog
    QObject::connect(newBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("上报事件"));
            dlg.setMinimumWidth(500);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);
            auto* formTitle = new QLabel(QStringLiteral("填写事件信息"), &dlg);
            formTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
            dlgLayout->addWidget(formTitle);
            dlgLayout->addSpacing(8);
            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* titleEdit = new QLineEdit(&dlg);
            titleEdit->setPlaceholderText(QStringLiteral("请简要描述事件"));
            form->addRow(QStringLiteral("标题:"), titleEdit);
            auto* catCombo = new QComboBox(&dlg);
            catCombo->addItems({QStringLiteral("民生服务"), QStringLiteral("环境卫生"), QStringLiteral("设施安全"), QStringLiteral("邻里纠纷"), QStringLiteral("特殊帮扶"), QStringLiteral("市容秩序"), QStringLiteral("突发预警")});
            form->addRow(QStringLiteral("类别:"), catCombo);
            auto* prioCombo = new QComboBox(&dlg);
            prioCombo->addItems({QStringLiteral("一般"), QStringLiteral("重要"), QStringLiteral("紧急"), QStringLiteral("特急")});
            form->addRow(QStringLiteral("优先级:"), prioCombo);
            auto* descEdit = new QTextEdit(&dlg);
            descEdit->setPlaceholderText(QStringLiteral("详细描述事件情况..."));
            descEdit->setFixedHeight(80);
            form->addRow(QStringLiteral("描述:"), descEdit);
            auto* locEdit = new QLineEdit(&dlg);
            locEdit->setPlaceholderText(QStringLiteral("事件发生地点"));
            form->addRow(QStringLiteral("地点:"), locEdit);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(8);

            // 图片上传区域
            QStringList selectedImages;
            auto* imgLabel = new QLabel(QStringLiteral("附件图片（可选，最多5张）"), &dlg);
            imgLabel->setStyleSheet("font-size: 13px; color: #64748b; background: transparent;");
            dlgLayout->addWidget(imgLabel);
            auto* imgListWidget = new QWidget(&dlg);
            auto* imgListLayout = new QVBoxLayout(imgListWidget);
            imgListLayout->setContentsMargins(0, 0, 0, 0);
            imgListLayout->setSpacing(4);
            dlgLayout->addWidget(imgListWidget);

            std::function<void()> refreshImageList;
            refreshImageList = [&]() {
                QLayoutItem* child;
                while ((child = imgListLayout->takeAt(0)) != nullptr) {
                    if (child->widget()) child->widget()->deleteLater();
                    delete child;
                }
                for (int i = 0; i < selectedImages.size(); ++i) {
                    auto* row = new QWidget(imgListWidget);
                    auto* rowLay = new QHBoxLayout(row);
                    rowLay->setContentsMargins(0, 0, 0, 0);
                    auto* nameLabel = new QLabel(QFileInfo(selectedImages[i]).fileName(), row);
                    nameLabel->setStyleSheet("font-size: 12px; color: #334155; background: transparent;");
                    rowLay->addWidget(nameLabel);
                    rowLay->addStretch();
                    auto* rmBtn = new QPushButton(QStringLiteral("×"), row);
                    rmBtn->setFixedSize(20, 20);
                    rmBtn->setStyleSheet("QPushButton{border:none; color:#b91c1c; font-size:14px; font-weight:bold; background:transparent;} QPushButton:hover{background:#fee2e2; border-radius:3px;}");
                    rmBtn->setCursor(Qt::PointingHandCursor);
                    int idx = i;
                    QObject::connect(rmBtn, &QPushButton::clicked, [&, idx]() {
                        selectedImages.removeAt(idx);
                        refreshImageList();
                    });
                    rowLay->addWidget(rmBtn);
                    imgListLayout->addWidget(row);
                }
            };

            auto* addImgBtn = new QPushButton(QStringLiteral("+ 添加图片"), &dlg);
            addImgBtn->setCursor(Qt::PointingHandCursor);
            addImgBtn->setStyleSheet("QPushButton{border:1px dashed #cbd5e1; border-radius:4px; padding:6px 16px; color:#64748b; background:#f8fafc; font-size:13px;} QPushButton:hover{border-color:#b45309; color:#b45309;}");
            QObject::connect(addImgBtn, &QPushButton::clicked, [&]() {
                if (selectedImages.size() >= 5) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("最多上传5张图片"));
                    return;
                }
                QStringList files = QFileDialog::getOpenFileNames(&dlg, QStringLiteral("选择图片"), QString(),
                    QStringLiteral("图片文件 (*.jpg *.jpeg *.png *.bmp *.gif);;所有文件 (*)"));
                for (const QString& f : files) {
                    if (selectedImages.size() >= 5) break;
                    if (!selectedImages.contains(f)) selectedImages.append(f);
                }
                refreshImageList();
            });
            dlgLayout->addWidget(addImgBtn);

            dlgLayout->addSpacing(12);
            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (titleEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题"));
                    return;
                }
                auto& d = DatabaseManager::instance();
                QString eventNo = Utils::generateEventNo();
                const auto& user = AuthService::instance().currentUser();
                d.insert("ge_event", {
                    {"event_no", eventNo}, {"title", titleEdit->text().trimmed()},
                    {"event_category", catCombo->currentIndex() + 1},
                    {"priority", prioCombo->currentIndex() + 1},
                    {"description", descEdit->toPlainText().trimmed()},
                    {"location", locEdit->text().trimmed()},
                    {"images", selectedImages.isEmpty() ? QVariant() : selectedImages.join(",")},
                    {"reporter_id", user.id}, {"reporter_name", user.nickname.isEmpty() ? user.username : user.nickname},
                    {"status", 0}, {"source", 1},
                    {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                });
                showToast(QStringLiteral("事件上报成功"), page);
                dlg.accept();
                (*loadEventsPtr)();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
  }
  else if (sub == "inspection")
  {
    // Page header
    layout->addWidget(createPageHeader(QStringLiteral("ic_route"), QStringLiteral("社区巡查管理"), QStringLiteral("管理巡查计划、查看巡查记录和问题发现"), moduleColor("inspection"), page));
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
    planTable->setStyleSheet(TABLE_STYLE);
    planTable->setShowGrid(false);
    planTable->verticalHeader()->setVisible(false);
    planTable->setSortingEnabled(true);
    planTable->setColumnCount(6);
    planTable->setHorizontalHeaderLabels({QStringLiteral("计划名称"), QStringLiteral("网格"), QStringLiteral("频率"), QStringLiteral("巡查员"), QStringLiteral("起止日期"), QStringLiteral("状态")});
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    auto *planEmptyHint = createEmptyHintLabel(QStringLiteral("暂无计划记录"), planPage);
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
      pb->setTotalCount(executeCountQuery(sql, cntBinds));

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
        auto *planStsItem = createTagTableItem(planStsText, planStsBg, planStsFg);
        planTable->setItem(pRow, 5, planStsItem);
        pRow++;
      }
      syncEmptyHint(planTable, planEmptyHint);
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
    recTable->setStyleSheet(TABLE_STYLE);
    recTable->setShowGrid(false);
    recTable->verticalHeader()->setVisible(false);
    recTable->setSortingEnabled(true);
    recTable->setColumnCount(6);
    recTable->setHorizontalHeaderLabels({QStringLiteral("巡查员"), QStringLiteral("开始时间"), QStringLiteral("结束时间"), QStringLiteral("时长(分)"), QStringLiteral("发现问题"), QStringLiteral("状态")});
    auto *recEmptyHint = createEmptyHintLabel(QStringLiteral("暂无巡查记录"), recordPage);
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
      pb->setTotalCount(executeCountQuery(sql, cntBinds));

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
        auto *stsItem = createTagTableItem(iSts == 0 ? QStringLiteral("进行中") : QStringLiteral("已完成"), QColor(iSts == 0 ? "#e6f4ff" : "#f6ffed"), QColor(iSts == 0 ? "#b45309" : "#15803d"));
        recTable->setItem(rRow, 5, stsItem);
        rRow++;
      }
      syncEmptyHint(recTable, recEmptyHint);
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
    layout->addWidget(createPageHeader(QStringLiteral("ic_heart"), QStringLiteral("重点人群关怀"), QStringLiteral("管理特殊群体信息、走访计划和关怀记录"), moduleColor("care"), page));
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
    groupTable->setStyleSheet(TABLE_STYLE);
    groupTable->setShowGrid(false);
    groupTable->verticalHeader()->setVisible(false);
    groupTable->setSortingEnabled(true);
    groupTable->setColumnCount(6);
    groupTable->setHorizontalHeaderLabels({QStringLiteral("居民姓名"), QStringLiteral("类型"), QStringLiteral("关怀等级"), QStringLiteral("走访频率"), QStringLiteral("责任人"), QStringLiteral("最近走访")});
    auto *groupEmptyHint = createEmptyHintLabel(QStringLiteral("暂无重点人员记录"), groupPage);
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
        auto *lvlItem = createTagTableItem(lvl == 1 ? QStringLiteral("一般") : lvl == 2 ? QStringLiteral("重点")
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
      syncEmptyHint(groupTable, groupEmptyHint);
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
    visitTable->setStyleSheet(TABLE_STYLE);
    visitTable->setShowGrid(false);
    visitTable->verticalHeader()->setVisible(false);
    visitTable->setSortingEnabled(true);
    visitTable->setColumnCount(5);
    visitTable->setHorizontalHeaderLabels({QStringLiteral("走访人"), QStringLiteral("走访时间"), QStringLiteral("类型"), QStringLiteral("发现问题"), QStringLiteral("后续跟进")});
    visitTable->setColumnWidth(3, 180);
    visitTable->setColumnWidth(4, 180);
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    auto *visitEmptyHint = createEmptyHintLabel(QStringLiteral("暂无走访记录"), visitPage);
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
      pb->setTotalCount(executeCountQuery(sql, cntBinds));

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
      syncEmptyHint(visitTable, visitEmptyHint);
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
    layout->addWidget(createPageHeader(QStringLiteral("ic_list"), QStringLiteral("督办管理"), QStringLiteral("督办任务创建、进度跟踪和催办管理"), moduleColor("supervision"), page));
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
      pb->setTotalCount(executeCountQuery(sql, cntBinds));

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
        auto *stsItem = createTagTableItem(sSts == 0 ? QStringLiteral("待反馈") : sSts == 1 ? QStringLiteral("已反馈")
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
        table->setItem(sRow, 7, createActionItem(actText, actColor, supId, sSts));
        sRow++;
      }
      syncEmptyHint(table, emptyHint);
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
                    showToast(QStringLiteral("反馈提交成功"), page);
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
        showToast(QStringLiteral("验收成功"), page);
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
                showToast(QStringLiteral("督办创建成功"), page);
                dlg.accept();
                loadSupervisions();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
  }
  else if (sub == "opinion")
  {
    // Page header
    layout->addWidget(createPageHeader(QStringLiteral("ic_announce"), QStringLiteral("民意收集"), QStringLiteral("居民意见箱、建议收集和热点问题分析"), moduleColor("opinion"), page));
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
      applyCardShadow(card);
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
      pb->setTotalCount(executeCountQuery(sql, cntBinds));

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
        auto *stsItem = createTagTableItem(oSts == 0 ? QStringLiteral("待处理") : oSts == 1 ? QStringLiteral("已回复")
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
        table->setItem(oRow, 5, createActionItem(actText, actColor, oSts, residentId));
        oRow++;
      }
      syncEmptyHint(table, emptyHint);
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
                    showToast(QStringLiteral("回复成功"), page);
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
    layout->addWidget(createPageHeader(QStringLiteral("ic_star"), QStringLiteral("考核管理"), QStringLiteral("考核指标设置、完成率统计和排名展示"), moduleColor("assessment"), page));
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
      pb->setTotalCount(executeCountQuery(sql, cntBinds));

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
      syncEmptyHint(table, emptyHint);
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

