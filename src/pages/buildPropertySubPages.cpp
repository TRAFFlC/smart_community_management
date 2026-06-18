#include "pages/PageFactory.h"
#include "PagesCommon.h"

void PageFactory::buildPropertyWorkorder(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_tool"), QStringLiteral("报事报修管理"), QStringLiteral("管理居民报修工单，支持受理、派单、处理、评价全流程"), UiKit::moduleColor("workorder"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);

    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索工单号/标题..."));
    searchEdit->setFixedWidth(220);
    tbLayout->addWidget(searchEdit);

    auto *filterCombo = new QComboBox(toolbar);
    filterCombo->addItems({QStringLiteral("全部状态"), QStringLiteral("待受理"), QStringLiteral("已受理"), QStringLiteral("已派单"), QStringLiteral("处理中"), QStringLiteral("已完成"), QStringLiteral("已关闭"), QStringLiteral("已评价")});
    filterCombo->setFixedWidth(130);
    tbLayout->addWidget(filterCombo);

    auto *exportBtn = new QPushButton(QStringLiteral("导出"), toolbar);
    exportBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(exportBtn);

    auto *myTasksCheck = new QCheckBox(QStringLiteral("只看我的"), toolbar);
    tbLayout->addWidget(myTasksCheck);

    tbLayout->addStretch();

    auto *newBtn = new QPushButton(QStringLiteral("+ 新建工单"), toolbar);
    newBtn->setProperty("cssClass", "primary");
    newBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(newBtn);
    layout->addWidget(toolbar);

    // Table
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({QStringLiteral("工单号"), QStringLiteral("标题"), QStringLiteral("类型"), QStringLiteral("优先级"), QStringLiteral("状态"), QStringLiteral("报修人"), QStringLiteral("创建时间"), QStringLiteral("操作")});
    table->setColumnWidth(1, 180);
    table->setColumnWidth(7, 100);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);

    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto actionHandlerPtr = std::make_shared<std::function<void(qint64, int)>>();
    std::function<void()> loadWorkOrders = [table, searchEdit, filterCombo, myTasksCheck, actionHandlerPtr, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT id, order_no, title, order_type, priority, status, reporter_name, create_time, reporter_id, assign_to FROM wo_work_order WHERE del_flag = 0";
      // 注入数据权限过滤（居民只看本人，物业/社区按组织范围）
      auto scopeFilter = UiKit::buildDataScopeFilter("", "estate_id");
      sql += scopeFilter.first;
      QString searchText = searchEdit->text().trimmed();
      int filterIdx = filterCombo->currentIndex();
      bool onlyMine = myTasksCheck->isChecked();
      if (!searchText.isEmpty())
      {
        sql += " AND (order_no LIKE :search OR title LIKE :search)";
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
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

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
        int statusMap[] = {0, 0, 1, 2, 3, 4, 5, 6};
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
        qint64 woId = q.value(0).toLongLong();
        auto *orderNoItem = new QTableWidgetItem(q.value(1).toString());
        // 将工单 ID 存储到 item 的 UserRole 中，供双击详情使用
        orderNoItem->setData(Qt::UserRole, woId);
        table->setItem(row, 0, orderNoItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(WorkOrderType::label(q.value(3).toInt())));
        QColor priColor(WorkOrderPriority::color(q.value(4).toInt()));
        auto *priItem = UiKit::createTagTableItem(WorkOrderPriority::label(q.value(4).toInt()), QColor(priColor.red(), priColor.green(), priColor.blue(), 30), priColor);
        table->setItem(row, 3, priItem);
        QColor woColor(WorkOrderStatus::color(q.value(5).toInt()));
        auto *statusItem = UiKit::createTagTableItem(WorkOrderStatus::label(q.value(5).toInt()), QColor(woColor.red(), woColor.green(), woColor.blue(), 30), woColor);
        table->setItem(row, 4, statusItem);
        table->setItem(row, 5, new QTableWidgetItem(q.value(6).toString()));
        table->setItem(row, 6, new QTableWidgetItem(q.value(7).toDateTime().toString("yyyy-MM-dd hh:mm")));
        // Action text item - 根据角色权限决定是否显示操作按钮
        int sts = q.value(5).toInt();
        qint64 reporterId = q.value(8).toLongLong();
        qint64 assigneeId = q.value(9).toLongLong();
        QString actionText;
        QString actionColor;
        if (UiKit::canOperateWorkOrder(sts, reporterId, assigneeId))
        {
          if (sts == 0)
          {
            actionText = QStringLiteral("受理");
            actionColor = "#b45309";
          }
          else if (sts == 1)
          {
            actionText = QStringLiteral("派单");
            actionColor = "#b45309";
          }
          else if (sts == 2)
          {
            actionText = QStringLiteral("开始");
            actionColor = "#a16207";
          }
          else if (sts == 3)
          {
            actionText = QStringLiteral("完成");
            actionColor = "#15803d";
          }
          else if (sts == 4)
          {
            actionText = QStringLiteral("评价");
            actionColor = "#15803d";
          }
          else
          {
            actionText = QStringLiteral("--");
            actionColor = "#64748b";
          }
        }
        else
        {
          actionText = QStringLiteral("--");
          actionColor = "#64748b";
        }
        table->setItem(row, 7, UiKit::createActionItem(actionText, actionColor, woId, sts));
        row++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    // 操作列点击处理 - 通过按钮 clicked 信号触发
    std::function<void(qint64, int)> handleWorkOrderAction = [page, loadWorkOrders](qint64 id, int sts)
    {
      const auto &user = AuthService::instance().currentUser();

      // 权限校验：查询工单的 reporter_id 和 assign_to，校验当前用户是否有权操作
      qint64 reporterId = 0;
      qint64 assigneeId = 0;
      {
        QSqlQuery permQ(DatabaseManager::instance().database());
        permQ.prepare("SELECT reporter_id, assign_to FROM wo_work_order WHERE id = :id AND del_flag = 0");
        permQ.bindValue(":id", id);
        if (permQ.exec() && permQ.next())
        {
          reporterId = permQ.value(0).toLongLong();
          assigneeId = permQ.value(1).toLongLong();
        }
      }
      if (!UiKit::canOperateWorkOrder(sts, reporterId, assigneeId))
      {
        QMessageBox::warning(page,QStringLiteral("无权限"),
                             QStringLiteral("您没有权限执行此操作，请联系相关负责人。"));
        return;
      }

      if (sts == 0)
      {
        // 受理
        auto retAccept = QMessageBox::question(page,QStringLiteral("确认操作"),
                                               QStringLiteral("确认受理此工单？"),
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (retAccept != QMessageBox::Yes)
          return;
        DatabaseManager::instance().update("wo_work_order", id, {{"status", 1}, {"accept_time", QDateTime::currentDateTime()}, {"accept_by", user.id}});
        UiKit::showToast(QStringLiteral("工单已受理"), page);
        loadWorkOrders();
      }
      else if (sts == 1)
      {
        // 派单 - 弹出选择维修人员对话框
        QDialog dlg(page);
        dlg.setWindowTitle(QStringLiteral("派单"));
        dlg.setMinimumWidth(400);
        auto *dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto *titleLabel = new QLabel(QStringLiteral("选择维修人员"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(12);

        auto *combo = new QComboBox(&dlg);
        // 查询维修人员角色用户
        QSqlQuery workerQ("SELECT u.id, u.nickname, u.username FROM sys_user u JOIN sys_user_role ur ON u.id = ur.user_id JOIN sys_role r ON ur.role_id = r.id WHERE r.role_key = 'property_repair' AND u.status = 0 AND u.del_flag = 0");
        while (workerQ.next())
        {
          QString name = workerQ.value(1).toString();
          if (name.isEmpty())
            name = workerQ.value(2).toString();
          combo->addItem(name, workerQ.value(0).toInt());
        }
        // 如果没有维修人员，查询所有物业相关角色
        if (combo->count() == 0)
        {
          QSqlQuery workerQ2("SELECT u.id, u.nickname, u.username FROM sys_user u JOIN sys_user_role ur ON u.id = ur.user_id JOIN sys_role r ON ur.role_id = r.id WHERE r.role_key IN ('property_cs', 'property_steward', 'property_manager') AND u.status = 0 AND u.del_flag = 0");
          while (workerQ2.next())
          {
            QString name = workerQ2.value(1).toString();
            if (name.isEmpty())
              name = workerQ2.value(2).toString();
            combo->addItem(name, workerQ2.value(0).toInt());
          }
        }
        dlgLayout->addWidget(combo);
        dlgLayout->addSpacing(16);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认派单"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
                {
                    if (combo->count() == 0) {
                        QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("没有可选的维修人员"));
                        return;
                    }
                    int workerId = combo->currentData().toInt();
                    QString workerName = combo->currentText();
                    auto retAssign = QMessageBox::question(page,QStringLiteral("确认操作"),
                        QStringLiteral("确认将工单派单给 %1？").arg(workerName),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (retAssign != QMessageBox::Yes) return;
                    DatabaseManager::instance().update("wo_work_order", id, {
                        {"status", 2}, {"assign_to", workerId}, {"assign_time", QDateTime::currentDateTime()}
                    });
                    // 发送通知给维修人员
                    page->requestNotification(workerId, QStringLiteral("新工单已派单"), QStringLiteral("您有一个新的维修工单待处理"), 2, "work_order", (int)id);
                    UiKit::showToast(QStringLiteral("派单成功"), page);
                    dlg.accept();
                    loadWorkOrders(); });
        dlgLayout->addWidget(buttons);
        dlg.exec();
      }
      else if (sts == 2)
      {
        // 开始处理
        auto retStart = QMessageBox::question(page,QStringLiteral("确认操作"),
                                              QStringLiteral("确认开始处理此工单？"),
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (retStart != QMessageBox::Yes)
          return;
        DatabaseManager::instance().update("wo_work_order", id, {{"status", 3}});
        UiKit::showToast(QStringLiteral("已开始处理"), page);
        loadWorkOrders();
      }
      else if (sts == 3)
      {
        // 完成 - 弹出填写处理结果对话框
        QDialog dlg(page);
        dlg.setWindowTitle(QStringLiteral("填写处理结果"));
        dlg.setMinimumWidth(450);
        auto *dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto *titleLabel = new QLabel(QStringLiteral("处理结果"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(12);

        auto *form = new QFormLayout(&dlg);
        form->setSpacing(12);
        auto *resultEdit = new QTextEdit(&dlg);
        resultEdit->setPlaceholderText(QStringLiteral("请描述维修处理结果..."));
        resultEdit->setFixedHeight(80);
        form->addRow(QStringLiteral("处理结果:"), resultEdit);
        dlgLayout->addLayout(form);
        dlgLayout->addSpacing(16);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认完成"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
                {
                    auto retFinish = QMessageBox::question(page,QStringLiteral("确认操作"),
                        QStringLiteral("确认完成此工单？"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (retFinish != QMessageBox::Yes) return;
                    DatabaseManager::instance().update("wo_work_order", id, {
                        {"status", 4}, {"finish_time", QDateTime::currentDateTime()},
                        {"result_desc", resultEdit->toPlainText().trimmed()}
                    });
                    // 查询报修人ID并发送通知
                    QSqlQuery woQ(DatabaseManager::instance().database());
                    woQ.prepare("SELECT reporter_id, title FROM wo_work_order WHERE id = :id");
                    woQ.bindValue(":id", id);
                    woQ.exec();
                    if (woQ.next()) {
                        int reporterId = woQ.value(0).toInt();
                        QString woTitle = woQ.value(1).toString();
                        page->requestNotification(reporterId, QStringLiteral("工单已完成"), QStringLiteral("您的报修「%1」已处理完成，请评价").arg(woTitle), 2, "work_order", (int)id);
                    }
                    UiKit::showToast(QStringLiteral("工单已完成"), page);
                    dlg.accept();
                    loadWorkOrders(); });
        dlgLayout->addWidget(buttons);
        dlg.exec();
      }
      else if (sts == 4)
      {
        // 评价 - 弹出评价对话框
        QDialog dlg(page);
        dlg.setWindowTitle(QStringLiteral("评价工单"));
        dlg.setMinimumWidth(450);
        auto *dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto *titleLabel = new QLabel(QStringLiteral("服务评价"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(12);

        auto *form = new QFormLayout(&dlg);
        form->setSpacing(12);

        auto *ratingCombo = new QComboBox(&dlg);
        ratingCombo->addItems({QStringLiteral("1星 - 很差"), QStringLiteral("2星 - 一般"), QStringLiteral("3星 - 还行"), QStringLiteral("4星 - 满意"), QStringLiteral("5星 - 非常满意")});
        ratingCombo->setCurrentIndex(4);
        form->addRow(QStringLiteral("评分:"), ratingCombo);

        auto *contentEdit = new QTextEdit(&dlg);
        contentEdit->setPlaceholderText(QStringLiteral("请输入评价内容..."));
        contentEdit->setFixedHeight(80);
        form->addRow(QStringLiteral("评价:"), contentEdit);
        dlgLayout->addLayout(form);
        dlgLayout->addSpacing(16);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交评价"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
                {
                    int rating = ratingCombo->currentIndex() + 1;
                    auto retEval = QMessageBox::question(page,QStringLiteral("确认操作"),
                        QStringLiteral("确认提交评价？"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (retEval != QMessageBox::Yes) return;
                    DatabaseManager::instance().insert("ev_evaluation", {
                        {"biz_type", "work_order"}, {"biz_id", (int)id},
                        {"evaluator_id", user.id}, {"rating", rating},
                        {"content", contentEdit->toPlainText().trimmed()},
                        {"eval_time", QDateTime::currentDateTime()},
                        {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                    });
                    DatabaseManager::instance().update("wo_work_order", id, {{"status", 6}});
                    UiKit::showToast(QStringLiteral("评价提交成功"), page);
                    dlg.accept();
                    loadWorkOrders(); });
        dlgLayout->addWidget(buttons);
        dlg.exec();
      }
    };
    *actionHandlerPtr = handleWorkOrderAction;
    loadWorkOrders();
    QObject::connect(table, &QTableWidget::cellEntered, page, [=](int r, int c)
            { table->viewport()->setCursor(c == 7 ? Qt::PointingHandCursor : Qt::ArrowCursor); });
    // 双击工单行弹出详情对话框（操作列除外）
    QObject::connect(table, &QTableWidget::cellDoubleClicked, page, [page, table](int row, int col)
            {
            // 操作列（第7列）不触发详情对话框
            if (col == 7) return;
            if (row < 0 || row >= table->rowCount()) return;
            auto* item = table->item(row, 0);
            if (!item) return;
            qint64 woId = item->data(Qt::UserRole).toLongLong();
            if (woId > 0) {
                WorkOrderDetailDialog::show(page,woId);
            } });
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadWorkOrders(); });
    QObject::connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadWorkOrders(); });
    QObject::connect(myTasksCheck, &QCheckBox::toggled, page, [=]()
            { loadWorkOrders(); });
    QObject::connect(exportBtn, &QPushButton::clicked, page, [table, page]()
            { UiKit::exportTableToCsv(table, QStringLiteral("工单列表.csv"), page); });

    // New work order dialog
    QObject::connect(newBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("新建报修工单"));
            dlg.setMinimumWidth(500);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* formTitle = new QLabel(QStringLiteral("填写报修信息"), &dlg);
            formTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
            dlgLayout->addWidget(formTitle);
            dlgLayout->addSpacing(8);

            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);

            auto* titleEdit = new QLineEdit(&dlg);
            titleEdit->setPlaceholderText(QStringLiteral("请简要描述问题"));
            form->addRow(QStringLiteral("标题:"), titleEdit);

            auto* typeCombo = new QComboBox(&dlg);
            typeCombo->addItems({QStringLiteral("水电维修"), QStringLiteral("公共设施"), QStringLiteral("环境卫生"), QStringLiteral("安全秩序"), QStringLiteral("其他")});
            form->addRow(QStringLiteral("类型:"), typeCombo);

            auto* prioCombo = new QComboBox(&dlg);
            prioCombo->addItems({QStringLiteral("普通"), QStringLiteral("紧急"), QStringLiteral("特急")});
            form->addRow(QStringLiteral("优先级:"), prioCombo);

            auto* descEdit = new QTextEdit(&dlg);
            descEdit->setPlaceholderText(QStringLiteral("详细描述报修问题..."));
            descEdit->setFixedHeight(80);
            form->addRow(QStringLiteral("描述:"), descEdit);

            auto* locEdit = new QLineEdit(&dlg);
            locEdit->setPlaceholderText(QStringLiteral("如: 3号楼2单元501"));
            form->addRow(QStringLiteral("位置:"), locEdit);

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
                QString orderNo = Utils::generateOrderNo();
                const auto& user = AuthService::instance().currentUser();
                d.insert("wo_work_order", {
                    {"order_no", orderNo}, {"title", titleEdit->text().trimmed()},
                    {"order_type", typeCombo->currentIndex() + 1},
                    {"priority", prioCombo->currentIndex() + 1},
                    {"description", descEdit->toPlainText().trimmed()},
                    {"location_desc", locEdit->text().trimmed()},
                    {"images", selectedImages.isEmpty() ? QVariant() : selectedImages.join(",")},
                    {"reporter_id", user.id}, {"reporter_name", user.nickname.isEmpty() ? user.username : user.nickname},
                    {"reporter_phone", user.phone},
                    {"status", 0}, {"source", 0},
                    {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                });
                UiKit::showToast(QStringLiteral("工单提交成功"), page);
                dlg.accept();
                loadWorkOrders();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
}

void PageFactory::buildPropertyComplaint(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_chat"), QStringLiteral("投诉建议管理"), QStringLiteral("管理居民投诉和建议，跟踪处理进度和满意度评价"), UiKit::moduleColor("complaint"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索标题/编号..."));
    searchEdit->setMaximumWidth(220);

    tbLayout->addWidget(searchEdit);
    auto *typeCombo = new QComboBox(toolbar);
    typeCombo->addItem(QStringLiteral("全部类型"), -1);
    typeCombo->addItem(QStringLiteral("投诉"), 0);
    typeCombo->addItem(QStringLiteral("建议"), 1);
    typeCombo->setMinimumWidth(120);
    tbLayout->addWidget(typeCombo);
    auto *filterCombo = new QComboBox(toolbar);
    filterCombo->addItems({QStringLiteral("全部状态"), QStringLiteral("待受理"), QStringLiteral("已受理"), QStringLiteral("已派单"), QStringLiteral("处理中"), QStringLiteral("已完成"), QStringLiteral("已关闭"), QStringLiteral("已评价")});
    filterCombo->setMinimumWidth(120);
    tbLayout->addWidget(filterCombo);
    tbLayout->addStretch();
    auto *newBtn = new QPushButton(QStringLiteral("+ 提交投诉"), toolbar);
    newBtn->setProperty("cssClass", "primary");
    newBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(newBtn);
    layout->addWidget(toolbar);

    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({QStringLiteral("编号"), QStringLiteral("标题"), QStringLiteral("类型"), QStringLiteral("投诉人"), QStringLiteral("时间"), QStringLiteral("状态"), QStringLiteral("操作")});
    table->setColumnWidth(1, 200);
    table->setColumnWidth(6, 90);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);

    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto cmpActionHandlerPtr = std::make_shared<std::function<void(qint64, int)>>();
    std::function<void()> loadComplaints = [table, searchEdit, typeCombo, filterCombo, cmpActionHandlerPtr, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT id, order_no, title, order_type, reporter_name, create_time, status FROM wo_work_order WHERE del_flag = 0 AND source = 2";
      QString searchText = searchEdit->text().trimmed();
      int filterType = typeCombo->currentData().toInt();
      int filterIdx = filterCombo->currentIndex();
      if (!searchText.isEmpty())
        sql += " AND (order_no LIKE :search OR title LIKE :search)";
      if (filterType >= 0)
        sql += " AND order_type = :otype";
      if (filterIdx > 0)
        sql += " AND status = :status";
      sql += " ORDER BY create_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery q(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (filterType >= 0)
        cntBinds << ":otype" << (filterType);
      if (filterIdx > 0)
      {
        int sm[] = {0, 0, 1, 2, 3, 4, 5, 6};
        cntBinds << ":status" << sm[filterIdx];
      }
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (filterType >= 0)
        q.bindValue(":otype", filterType);
      if (filterIdx > 0)
      {
        int statusMap[] = {0, 0, 1, 2, 3, 4, 5, 6};
        q.bindValue(":status", statusMap[filterIdx]);
      }
      q.bindValue(":pageSize", pb->pageSize());
      q.bindValue(":offset", pb->offset());
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(3).toInt() == 0 ? QStringLiteral("投诉") : QStringLiteral("建议")));
        table->setItem(row, 3, new QTableWidgetItem(q.value(4).toString()));
        table->setItem(row, 4, new QTableWidgetItem(q.value(5).toDateTime().toString("yyyy-MM-dd")));
        QColor cmpColor(WorkOrderStatus::color(q.value(6).toInt()));
        auto *stsItem = UiKit::createTagTableItem(WorkOrderStatus::label(q.value(6).toInt()), QColor(cmpColor.red(), cmpColor.green(), cmpColor.blue(), 30), cmpColor);
        table->setItem(row, 5, stsItem);
        qint64 cmpId = q.value(0).toLongLong();
        int cmpSts = q.value(6).toInt();
        QString actText;
        QString actColor;
        if (cmpSts == 0)
        {
          actText = QStringLiteral("受理");
          actColor = "#b45309";
        }
        else if (cmpSts == 1)
        {
          actText = QStringLiteral("回复");
          actColor = "#15803d";
        }
        else
        {
          actText = QStringLiteral("查看");
          actColor = "#b45309";
        }
        table->setItem(row, 6, UiKit::createActionItem(actText, actColor, cmpId, cmpSts));
        row++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadComplaints(); });
    QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadComplaints(); });
    QObject::connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadComplaints(); });

    std::function<void(qint64, int)> handleComplaintAction = [page, loadComplaints](qint64 id, int sts)
    {
      const auto &user = AuthService::instance().currentUser();

      if (sts == 0)
      {
        // 受理: 弹出对话框填写处理意见, status=0 -> status=1
        QDialog dlg(page);
        dlg.setWindowTitle(QStringLiteral("受理投诉建议"));
        dlg.setMinimumWidth(450);
        auto *dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto *titleLabel = new QLabel(QStringLiteral("受理投诉建议"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(8);

        auto *form = new QFormLayout(&dlg);
        form->setSpacing(12);
        auto *opinionEdit = new QTextEdit(&dlg);
        opinionEdit->setPlaceholderText(QStringLiteral("请填写受理意见..."));
        opinionEdit->setFixedHeight(100);
        form->addRow(QStringLiteral("受理意见:"), opinionEdit);
        dlgLayout->addLayout(form);
        dlgLayout->addSpacing(12);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认受理"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
                {
                    if (opinionEdit->toPlainText().trimmed().isEmpty()) {
                        QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写受理意见"));
                        return;
                    }
                    auto retAccept = QMessageBox::question(page,QStringLiteral("确认操作"),
                        QStringLiteral("确认受理此投诉？"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (retAccept != QMessageBox::Yes) return;
                    DatabaseManager::instance().update("wo_work_order", id, {
                        {"status", 1},
                        {"accept_by", user.id},
                        {"accept_time", QDateTime::currentDateTime()},
                        {"result_desc", opinionEdit->toPlainText().trimmed()},
                        {"update_by", user.id},
                        {"update_time", QDateTime::currentDateTime()}
                    });
                    // 通知上报人
                    QSqlQuery repQ(DatabaseManager::instance().database());
                    repQ.prepare("SELECT reporter_id, title FROM wo_work_order WHERE id = :id");
                    repQ.bindValue(":id", id);
                    if (repQ.exec() && repQ.next()) {
                        int reporterId = repQ.value(0).toInt();
                        QString title = repQ.value(1).toString();
                        if (reporterId > 0) {
                            page->requestNotification(reporterId,
                                QStringLiteral("投诉建议已受理: %1").arg(title),
                                QStringLiteral("您的投诉建议已受理，正在处理中"), 1, "complaint", (int)id);
                        }
                    }
                    UiKit::showToast(QStringLiteral("受理成功"), page);
                    dlg.accept();
                    loadComplaints(); });
        dlgLayout->addWidget(buttons);
        dlg.exec();
      }
      else if (sts == 1)
      {
        // 回复: 弹出对话框填写回复内容, status=1 -> status=4 (已完成)
        QDialog dlg(page);
        dlg.setWindowTitle(QStringLiteral("回复投诉建议"));
        dlg.setMinimumWidth(450);
        auto *dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto *titleLabel = new QLabel(QStringLiteral("回复投诉建议"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(8);

        auto *form = new QFormLayout(&dlg);
        form->setSpacing(12);
        auto *replyEdit = new QTextEdit(&dlg);
        replyEdit->setPlaceholderText(QStringLiteral("请填写回复内容..."));
        replyEdit->setFixedHeight(120);
        form->addRow(QStringLiteral("回复内容:"), replyEdit);
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
                    auto retReply = QMessageBox::question(page,QStringLiteral("确认操作"),
                        QStringLiteral("确认提交回复？"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (retReply != QMessageBox::Yes) return;
                    DatabaseManager::instance().update("wo_work_order", id, {
                        {"status", 4},
                        {"finish_time", QDateTime::currentDateTime()},
                        {"result_desc", replyEdit->toPlainText().trimmed()},
                        {"update_by", user.id},
                        {"update_time", QDateTime::currentDateTime()}
                    });
                    // 通知上报人
                    QSqlQuery repQ(DatabaseManager::instance().database());
                    repQ.prepare("SELECT reporter_id, title FROM wo_work_order WHERE id = :id");
                    repQ.bindValue(":id", id);
                    if (repQ.exec() && repQ.next()) {
                        int reporterId = repQ.value(0).toInt();
                        QString title = repQ.value(1).toString();
                        if (reporterId > 0) {
                            page->requestNotification(reporterId,
                                QStringLiteral("投诉建议已回复: %1").arg(title),
                                QStringLiteral("您的投诉建议已处理回复，请查看"), 1, "complaint", (int)id);
                        }
                    }
                    UiKit::showToast(QStringLiteral("回复成功"), page);
                    dlg.accept();
                    loadComplaints(); });
        dlgLayout->addWidget(buttons);
        dlg.exec();
      }
      else
      {
        // 查看: 显示详情
        QSqlQuery detail(DatabaseManager::instance().database());
        detail.prepare("SELECT title, description, result_desc FROM wo_work_order WHERE id = :id");
        detail.bindValue(":id", id);
        if (detail.exec() && detail.next())
        {
          QString info = QStringLiteral("标题: %1\n\n描述: %2").arg(detail.value(0).toString(), detail.value(1).toString());
          if (!detail.value(2).isNull() && !detail.value(2).toString().isEmpty())
          {
            info += QStringLiteral("\n\n回复/处理: %1").arg(detail.value(2).toString());
          }
          QMessageBox::information(page,QStringLiteral("投诉建议详情"), info);
        }
      }
    };
    *cmpActionHandlerPtr = handleComplaintAction;
    QObject::connect(table, &QTableWidget::cellClicked, page, [table, cmpActionHandlerPtr](int row, int col)
            {
            if (col != 6) return;
            auto* item = table->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*cmpActionHandlerPtr) (*cmpActionHandlerPtr)(id, sts); });
    loadComplaints();
    QObject::connect(table, &QTableWidget::cellEntered, page, [=](int r, int c)
            { table->viewport()->setCursor(c == 6 ? Qt::PointingHandCursor : Qt::ArrowCursor); });

    QObject::connect(newBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("提交投诉建议"));
            dlg.setMinimumWidth(480);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);
            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* titleEdit = new QLineEdit(&dlg); titleEdit->setPlaceholderText(QStringLiteral("请简要描述"));
            form->addRow(QStringLiteral("标题:"), titleEdit);
            auto* typeCombo = new QComboBox(&dlg);
            typeCombo->addItems({QStringLiteral("投诉"), QStringLiteral("建议"), QStringLiteral("表扬")});
            form->addRow(QStringLiteral("类型:"), typeCombo);
            auto* descEdit = new QTextEdit(&dlg); descEdit->setPlaceholderText(QStringLiteral("详细内容...")); descEdit->setFixedHeight(100);
            form->addRow(QStringLiteral("内容:"), descEdit);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);
            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (titleEdit->text().trimmed().isEmpty()) { QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题")); return; }
                auto& d = DatabaseManager::instance();
                const auto& user = AuthService::instance().currentUser();
                d.insert("wo_work_order", {
                    {"order_no", Utils::generateOrderNo()}, {"title", titleEdit->text().trimmed()},
                    {"order_type", typeCombo->currentIndex() + 1},
                    {"description", descEdit->toPlainText().trimmed()},
                    {"reporter_id", user.id}, {"reporter_name", user.nickname.isEmpty() ? user.username : user.nickname},
                    {"reporter_phone", user.phone}, {"status", 0}, {"source", 2},
                    {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                });
                UiKit::showToast(QStringLiteral("提交成功"), page);
                dlg.accept(); loadComplaints();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
}

void PageFactory::buildPropertyInspection(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Property inspection - reuse from governance
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_search"), QStringLiteral("物业巡检管理"), QStringLiteral("物业巡检计划和记录查看"), UiKit::moduleColor("inspection"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索巡检员..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("进行中"), 0);
    statusCombo->addItem(QStringLiteral("已完成"), 1);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("巡检员"), QStringLiteral("开始时间"), QStringLiteral("时长(分)"), QStringLiteral("发现问题"), QStringLiteral("状态")});
    layout->addWidget(table);
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadInspections = [table, searchEdit, statusCombo, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT u.real_name, i.start_time, i.duration, i.issue_count, i.status FROM ge_inspection i LEFT JOIN sys_user u ON i.inspector_id = u.id WHERE i.del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND u.real_name LIKE :search";
      if (statusFilter >= 0)
        sql += " AND i.status = :status";
      sql += " ORDER BY i.start_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery inspQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (statusFilter >= 0)
        cntBinds << ":status" << (statusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      inspQ.prepare(sql);
      if (!searchText.isEmpty())
        inspQ.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        inspQ.bindValue(":status", statusFilter);
      inspQ.bindValue(":pageSize", pb->pageSize());
      inspQ.bindValue(":offset", pb->offset());
      inspQ.exec();
      int iRow = 0;
      while (inspQ.next())
      {
        table->insertRow(iRow);
        table->setItem(iRow, 0, new QTableWidgetItem(inspQ.value(0).toString()));
        table->setItem(iRow, 1, new QTableWidgetItem(inspQ.value(1).toDateTime().toString("MM-dd hh:mm")));
        table->setItem(iRow, 2, new QTableWidgetItem(inspQ.value(2).toString()));
        table->setItem(iRow, 3, new QTableWidgetItem(inspQ.value(3).toString()));
        int iSts = inspQ.value(4).toInt();
        auto *stsItem = UiKit::createTagTableItem(iSts == 0 ? QStringLiteral("进行中") : QStringLiteral("已完成"), QColor(iSts == 0 ? "#e6f4ff" : "#f6ffed"), QColor(iSts == 0 ? "#b45309" : "#15803d"));
        table->setItem(iRow, 4, stsItem);
        iRow++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadInspections();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadInspections(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadInspections(); });
}

void PageFactory::buildPropertyAnnouncement(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_announce"), QStringLiteral("公告通知管理"), QStringLiteral("发布和管理小区公告、社区通知、物业公告和系统公告"), UiKit::moduleColor("announcement"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索公告标题..."));
    searchEdit->setMaximumWidth(260);

    tbLayout->addWidget(searchEdit);
    auto *typeCombo = new QComboBox(toolbar);
    typeCombo->addItem(QStringLiteral("全部类型"), -1);
    typeCombo->addItem(QStringLiteral("通知"), 1);
    typeCombo->addItem(QStringLiteral("公告"), 2);
    typeCombo->addItem(QStringLiteral("制度"), 3);
    typeCombo->addItem(QStringLiteral("其他"), 4);
    typeCombo->setMinimumWidth(120);
    tbLayout->addWidget(typeCombo);
    tbLayout->addStretch();
    auto *newBtn = new QPushButton(QStringLiteral("+ 发布公告"), toolbar);
    newBtn->setProperty("cssClass", "primary");
    newBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(newBtn);
    layout->addWidget(toolbar);

    // Table
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("标题"), QStringLiteral("类型"), QStringLiteral("发布人"), QStringLiteral("发布时间"), QStringLiteral("阅读数"), QStringLiteral("操作")});
    table->setColumnWidth(0, 250);
    table->setColumnWidth(5, 90);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);

    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto annActionHandlerPtr = std::make_shared<std::function<void(qint64, QString)>>();

    std::function<void()> loadAnnouncements = [table, searchEdit, typeCombo, emptyHint, annActionHandlerPtr, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT id, title, announcement_type, publisher_id, publish_time, read_count FROM nt_announcement WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int filterType = typeCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND title LIKE :search";
      if (filterType >= 0)
        sql += " AND announcement_type = :type";
      sql += " ORDER BY publish_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery q(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (filterType >= 0)
        cntBinds << ":type" << (filterType);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (filterType >= 0)
        q.bindValue(":type", filterType);
      q.bindValue(":pageSize", pb->pageSize());
      q.bindValue(":offset", pb->offset());
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 1, new QTableWidgetItem(AnnouncementType::label(q.value(2).toInt())));
        table->setItem(row, 2, new QTableWidgetItem(QStringLiteral("管理员")));
        table->setItem(row, 3, new QTableWidgetItem(q.value(4).toDateTime().toString("yyyy-MM-dd")));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(q.value(5).toInt())));
        qint64 annId = q.value(0).toLongLong();
        QString annTitle = q.value(1).toString();
        table->setItem(row, 5, UiKit::createActionItem(QStringLiteral("查看"), "#b45309", annId, annTitle));
        row++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    std::function<void(qint64, QString)> handleAnnouncementAction = [page](qint64 id, QString title)
    {
      QSqlQuery detail(DatabaseManager::instance().database());
      detail.prepare("SELECT content FROM nt_announcement WHERE id = :id");
      detail.bindValue(":id", id);
      if (detail.exec() && detail.next())
      {
        QMessageBox::information(page,title, detail.value(0).toString());
      }
    };
    *annActionHandlerPtr = handleAnnouncementAction;
    QObject::connect(table, &QTableWidget::cellClicked, page, [table, annActionHandlerPtr](int row, int col)
            {
            if (col != 5) return;
            auto* item = table->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*annActionHandlerPtr) (*annActionHandlerPtr)(id, item->text()); });
    loadAnnouncements();
    QObject::connect(table, &QTableWidget::cellEntered, page, [=](int r, int c)
            { table->viewport()->setCursor(c == 5 ? Qt::PointingHandCursor : Qt::ArrowCursor); });
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadAnnouncements(); });
    QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadAnnouncements(); });

    // New announcement dialog
    QObject::connect(newBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("发布公告"));
            dlg.setMinimumWidth(520);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);
            auto* formTitle = new QLabel(QStringLiteral("填写公告信息"), &dlg);
            formTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
            dlgLayout->addWidget(formTitle);
            dlgLayout->addSpacing(8);
            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* titleEdit = new QLineEdit(&dlg);
            titleEdit->setPlaceholderText(QStringLiteral("公告标题"));
            form->addRow(QStringLiteral("标题:"), titleEdit);
            auto* typeCombo = new QComboBox(&dlg);
            typeCombo->addItems({QStringLiteral("小区公告"), QStringLiteral("社区公告"), QStringLiteral("物业公告"), QStringLiteral("系统公告")});
            form->addRow(QStringLiteral("类型:"), typeCombo);
            auto* contentEdit = new QTextEdit(&dlg);
            contentEdit->setPlaceholderText(QStringLiteral("公告内容..."));
            contentEdit->setFixedHeight(160);
            form->addRow(QStringLiteral("内容:"), contentEdit);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);
            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("发布"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (titleEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题"));
                    return;
                }
                auto& d = DatabaseManager::instance();
                const auto& user = AuthService::instance().currentUser();
                qint64 annId = d.insert("nt_announcement", {
                    {"title", titleEdit->text().trimmed()},
                    {"content", contentEdit->toPlainText().trimmed()},
                    {"announcement_type", typeCombo->currentIndex() + 1},
                    {"publisher_id", user.id},
                    {"publish_time", QDateTime::currentDateTime()},
                    {"status", 1}, {"read_count", 0},
                    {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                });
                // 公告发布后推送通知给所有启用用户
                if (annId > 0) {
                    QSqlQuery userQ("SELECT id FROM sys_user WHERE status = 0 AND del_flag = 0");
                    while (userQ.next()) {
                        page->requestNotification(userQ.value(0).toInt(),
                            QStringLiteral("新公告: %1").arg(titleEdit->text().trimmed()),
                            QStringLiteral("您有一条新的社区公告，请查看"),
                            1, "announcement", static_cast<int>(annId));
                    }
                }
                UiKit::showToast(QStringLiteral("公告发布成功"), page);
                dlg.accept();
                loadAnnouncements();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
}

void PageFactory::buildPropertyVisitor(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_visitor"), QStringLiteral("访客管理"), QStringLiteral("访客登记、临时通行码管理和访客记录查看"), UiKit::moduleColor("visitor"), page));
    layout->addSpacing(12);

    // Stats
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
    QSqlQuery todayVisQ("SELECT COUNT(*) FROM cm_visitor WHERE date(arrive_time) = date('now') AND del_flag = 0");
    int todayVis = todayVisQ.next() ? todayVisQ.value(0).toInt() : 0;
    QSqlQuery weekVisQ("SELECT COUNT(*) FROM cm_visitor WHERE arrive_time >= date('now', '-7 days') AND del_flag = 0");
    int weekVis = weekVisQ.next() ? weekVisQ.value(0).toInt() : 0;
    QSqlQuery activeVisQ("SELECT COUNT(*) FROM cm_visitor WHERE status = 0 AND del_flag = 0");
    int activeVis = activeVisQ.next() ? activeVisQ.value(0).toInt() : 0;
    statsRow->addWidget(createMiniCard(QStringLiteral("今日访客"), QString::number(todayVis), "#b45309", page));
    statsRow->addWidget(createMiniCard(QStringLiteral("本周访客"), QString::number(weekVis), "#15803d", page));
    statsRow->addWidget(createMiniCard(QStringLiteral("在访中"), QString::number(activeVis), "#d97706", page));
    statsRow->addStretch();
    layout->addLayout(statsRow);
    layout->addSpacing(12);

    // Toolbar
    auto *visToolbar = new QHBoxLayout();
    visToolbar->setSpacing(12);
    auto *visSearchEdit = new QLineEdit();
    visSearchEdit->setPlaceholderText(QStringLiteral("搜索姓名/手机号..."));
    visSearchEdit->setMaximumWidth(260);

    visToolbar->addWidget(visSearchEdit);
    auto *visStatusCombo = new QComboBox();
    visStatusCombo->addItem(QStringLiteral("全部状态"), -1);
    visStatusCombo->addItem(QStringLiteral("在访"), 0);
    visStatusCombo->addItem(QStringLiteral("已离开"), 1);
    visStatusCombo->setMinimumWidth(120);
    visToolbar->addWidget(visStatusCombo);
    visToolbar->addStretch();
    auto *visRegisterBtn = new QPushButton(QStringLiteral("+ 访客登记"), page);
    visRegisterBtn->setProperty("cssClass", "primary");
    visRegisterBtn->setCursor(Qt::PointingHandCursor);
    visToolbar->addWidget(visRegisterBtn);
    layout->insertLayout(layout->count() - 1, visToolbar);

    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({QStringLiteral("访客姓名"), QStringLiteral("手机号"), QStringLiteral("拜访业主"), QStringLiteral("来访时间"), QStringLiteral("离开时间"), QStringLiteral("状态"), QStringLiteral("操作")});
    table->setColumnWidth(6, 90);
    layout->addWidget(table);

    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto visActionHandlerPtr = std::make_shared<std::function<void(qint64, int)>>();
    std::function<void()> loadVisitors = [table, visSearchEdit, visStatusCombo, emptyHint, visActionHandlerPtr, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT id, visitor_name, phone, host_name, arrive_time, leave_time, status FROM cm_visitor WHERE del_flag = 0";
      QString visSearch = visSearchEdit->text().trimmed();
      int visFilter = visStatusCombo->currentData().toInt();
      if (!visSearch.isEmpty())
        sql += " AND (visitor_name LIKE :search OR phone LIKE :search)";
      if (visFilter >= 0)
        sql += " AND status = :status";
      sql += " ORDER BY arrive_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery visQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!visSearch.isEmpty())
        cntBinds << ":search" << ("%" + visSearch + "%");
      if (visFilter >= 0)
        cntBinds << ":status" << (visFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      visQ.prepare(sql);
      if (!visSearch.isEmpty())
        visQ.bindValue(":search", "%" + visSearch + "%");
      if (visFilter >= 0)
        visQ.bindValue(":status", visFilter);
      visQ.bindValue(":pageSize", pb->pageSize());
      visQ.bindValue(":offset", pb->offset());
      visQ.exec();
      int vRow = 0;
      while (visQ.next())
      {
        table->insertRow(vRow);
        qint64 visId = visQ.value(0).toLongLong();
        table->setItem(vRow, 0, new QTableWidgetItem(visQ.value(1).toString()));
        table->setItem(vRow, 1, new QTableWidgetItem(visQ.value(2).toString()));
        table->setItem(vRow, 2, new QTableWidgetItem(visQ.value(3).toString()));
        table->setItem(vRow, 3, new QTableWidgetItem(visQ.value(4).toDateTime().toString("yyyy-MM-dd HH:mm")));
        table->setItem(vRow, 4, new QTableWidgetItem(visQ.value(5).isNull() ? "-" : visQ.value(5).toDateTime().toString("yyyy-MM-dd HH:mm")));
        int vSts = visQ.value(6).toInt();
        auto *stsItem = UiKit::createTagTableItem(vSts == 0 ? QStringLiteral("在访") : QStringLiteral("已离开"), QColor(vSts == 0 ? "#e6f4ff" : "#e2e8f0"), QColor(vSts == 0 ? "#b45309" : "#64748b"));
        table->setItem(vRow, 5, stsItem);
        // 操作列
        QString actText = (vSts == 0) ? QStringLiteral("签离") : QStringLiteral("-");
        QString actColor = (vSts == 0) ? "#b45309" : "#64748b";
        table->setItem(vRow, 6, UiKit::createActionItem(actText, actColor, visId, vSts));
        vRow++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    QObject::connect(visSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadVisitors(); });
    QObject::connect(visStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadVisitors(); });

    // 操作列点击处理 - 签离
    std::function<void(qint64, int)> handleVisitorAction = [page, loadVisitors](qint64 visId, int vSts)
    {
      if (vSts != 0)
        return;
      auto ret = QMessageBox::question(page,QStringLiteral("确认"), QStringLiteral("确认该访客已离开？"), QMessageBox::Yes | QMessageBox::No);
      if (ret != QMessageBox::Yes)
        return;
      DatabaseManager::instance().update("cm_visitor", visId, {{"status", 1}, {"leave_time", QDateTime::currentDateTime()}, {"update_time", QDateTime::currentDateTime()}});
      UiKit::showToast(QStringLiteral("访客已签离"), page);
      loadVisitors();
    };
    *visActionHandlerPtr = handleVisitorAction;
    QObject::connect(table, &QTableWidget::cellClicked, page, [table, visActionHandlerPtr](int row, int col)
            {
            if (col != 6) return;
            auto* item = table->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*visActionHandlerPtr) (*visActionHandlerPtr)(id, sts); });
    loadVisitors();
    QObject::connect(table, &QTableWidget::cellEntered, page, [=](int r, int c)
            {
            auto* w = table->cellWidget(r, c);
            bool clickable = (c == 6 && w && w->isEnabled());
            table->viewport()->setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor); });

    // 访客登记按钮
    QObject::connect(visRegisterBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("访客登记"));
            dlg.setMinimumWidth(450);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* titleLabel = new QLabel(QStringLiteral("填写访客信息"), &dlg);
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
            dlgLayout->addWidget(titleLabel);
            dlgLayout->addSpacing(8);

            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* nameEdit = new QLineEdit(&dlg);
            nameEdit->setPlaceholderText(QStringLiteral("请输入访客姓名"));
            form->addRow(QStringLiteral("访客姓名:"), nameEdit);
            auto* phoneEdit = new QLineEdit(&dlg);
            phoneEdit->setPlaceholderText(QStringLiteral("请输入联系电话"));
            form->addRow(QStringLiteral("联系电话:"), phoneEdit);
            auto* idCardEdit = new QLineEdit(&dlg);
            idCardEdit->setPlaceholderText(QStringLiteral("选填"));
            form->addRow(QStringLiteral("身份证号:"), idCardEdit);
            auto* hostNameEdit = new QLineEdit(&dlg);
            hostNameEdit->setPlaceholderText(QStringLiteral("请输入被访人姓名"));
            form->addRow(QStringLiteral("被访人姓名:"), hostNameEdit);
            auto* hostPhoneEdit = new QLineEdit(&dlg);
            hostPhoneEdit->setPlaceholderText(QStringLiteral("请输入被访人电话"));
            form->addRow(QStringLiteral("被访人电话:"), hostPhoneEdit);
            auto* purposeEdit = new QLineEdit(&dlg);
            purposeEdit->setPlaceholderText(QStringLiteral("请输入来访目的"));
            form->addRow(QStringLiteral("来访目的:"), purposeEdit);
            auto* countSpin = new QSpinBox(&dlg);
            countSpin->setRange(1, 99);
            countSpin->setValue(1);
            form->addRow(QStringLiteral("来访人数:"), countSpin);
            auto* leaveEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(2 * 3600), &dlg);
            leaveEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
            leaveEdit->setCalendarPopup(true);
            form->addRow(QStringLiteral("预计离开时间:"), leaveEdit);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);

            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认登记"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (nameEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写访客姓名"));
                    return;
                }
                if (phoneEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写联系电话"));
                    return;
                }
                if (hostNameEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写被访人姓名"));
                    return;
                }
                const auto& user = AuthService::instance().currentUser();
                QDateTime now = QDateTime::currentDateTime();
                DatabaseManager::instance().insert("cm_visitor", {
                    {"visitor_name", nameEdit->text().trimmed()},
                    {"phone", phoneEdit->text().trimmed()},
                    {"id_card", idCardEdit->text().trimmed()},
                    {"host_name", hostNameEdit->text().trimmed()},
                    {"host_phone", hostPhoneEdit->text().trimmed()},
                    {"purpose", purposeEdit->text().trimmed()},
                    {"visitor_count", countSpin->value()},
                    {"arrive_time", now},
                    {"leave_time", leaveEdit->dateTime()},
                    {"status", 0},
                    {"create_by", user.id},
                    {"create_time", now},
                    {"update_by", user.id},
                    {"update_time", now}
                });
                UiKit::showToast(QStringLiteral("访客登记成功"), page);
                dlg.accept();
                loadVisitors();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
}

void PageFactory::buildPropertyTopic(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_list"), QStringLiteral("业委会议题管理"), QStringLiteral("议题发布、投票管理和结果公示"), UiKit::moduleColor("topic"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索议题标题..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("投票中"), 0);
    statusCombo->addItem(QStringLiteral("已结束"), 1);
    statusCombo->addItem(QStringLiteral("已通过"), 2);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    auto *newBtn = new QPushButton(QStringLiteral("+ 发起议题"), toolbar);
    newBtn->setProperty("cssClass", "primary");
    newBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(newBtn);
    layout->addWidget(toolbar);

    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({QStringLiteral("议题标题"), QStringLiteral("类型"), QStringLiteral("发起人"), QStringLiteral("截止日期"), QStringLiteral("投票结果"), QStringLiteral("状态"), QStringLiteral("操作")});
    table->setColumnWidth(0, 200);
    table->setColumnWidth(6, 90);
    layout->addWidget(table);
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto topicActionHandlerPtr = std::make_shared<std::function<void(qint64, int, int)>>();
    std::function<void()> loadTopics = [table, searchEdit, statusCombo, emptyHint, topicActionHandlerPtr, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT t.id, t.title, t.topic_type, t.vote_end, t.status, t.need_vote, "
                    "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 1) as approve_cnt, "
                    "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 2) as oppose_cnt, "
                    "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 3) as abstain_cnt, "
                    "COALESCE((SELECT real_name FROM sys_user WHERE id = t.publisher_id), '业委会') as publisher "
                    "FROM oc_topic t WHERE t.del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND t.title LIKE :search";
      if (statusFilter >= 0)
        sql += " AND t.status = :status";
      sql += " ORDER BY t.publish_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery topicQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (statusFilter >= 0)
        cntBinds << ":status" << (statusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      topicQ.prepare(sql);
      if (!searchText.isEmpty())
        topicQ.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        topicQ.bindValue(":status", statusFilter);
      topicQ.bindValue(":pageSize", pb->pageSize());
      topicQ.bindValue(":offset", pb->offset());
      topicQ.exec();
      int tRow = 0;
      while (topicQ.next())
      {
        table->insertRow(tRow);
        table->setItem(tRow, 0, new QTableWidgetItem(topicQ.value(1).toString()));
        table->setItem(tRow, 1, new QTableWidgetItem(TopicType::label(topicQ.value(2).toInt())));
        table->setItem(tRow, 2, new QTableWidgetItem(topicQ.value(9).toString()));
        table->setItem(tRow, 3, new QTableWidgetItem(topicQ.value(3).toDateTime().toString("yyyy-MM-dd")));
        int approve = topicQ.value(6).toInt();
        int oppose = topicQ.value(7).toInt();
        int abstain = topicQ.value(8).toInt();
        int total = approve + oppose + abstain;
        int tSts = topicQ.value(4).toInt();
        QString resultText;
        if (tSts == 0)
        {
          resultText = QStringLiteral("投票中");
        }
        else if (total > 0)
        {
          resultText = QStringLiteral("赞成:%1% 反对:%2% 弃权:%3%")
                           .arg(approve * 100 / total)
                           .arg(oppose * 100 / total)
                           .arg(abstain * 100 / total);
        }
        else
        {
          resultText = QStringLiteral("已结束");
        }
        table->setItem(tRow, 4, new QTableWidgetItem(resultText));
        auto *stsItem = UiKit::createTagTableItem(tSts == 0 ? QStringLiteral("投票中") : tSts == 1 ? QStringLiteral("已结束")
                                                                                            : QStringLiteral("已通过"),
                                           QColor(tSts == 0 ? "#e6f4ff" : tSts == 1 ? "#e2e8f0"
                                                                                    : "#f6ffed"),
                                           QColor(tSts == 0 ? "#b45309" : tSts == 1 ? "#64748b"
                                                                                    : "#15803d"));
        table->setItem(tRow, 5, stsItem);
        // 操作列: status=1(投票中) 且 need_vote=1 时显示"投票"
        int needVote = topicQ.value(5).toInt();
        qint64 topicId = topicQ.value(0).toLongLong();
        QString actText;
        QString actColor;
        if (tSts == 0 && needVote == 1)
        {
          actText = QStringLiteral("投票");
          actColor = "#b45309";
        }
        else
        {
          actText = QStringLiteral("查看");
          actColor = "#b45309";
        }
        table->setItem(tRow, 6, UiKit::createActionItem(actText, actColor, tSts, needVote));
        tRow++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadTopics(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadTopics(); });

    // 操作列点击处理
    std::function<void(qint64, int, int)> handleTopicAction = [page, loadTopics](qint64 topicId, int tSts, int needVote)
    {
      const auto &user = AuthService::instance().currentUser();

      if (tSts == 0 && needVote == 1)
      {
        // 投票: 检查是否已投过票
        QSqlQuery chkQ(DatabaseManager::instance().database());
        chkQ.prepare("SELECT id FROM oc_vote WHERE topic_id = :tid AND voter_id = :vid");
        chkQ.bindValue(":tid", topicId);
        chkQ.bindValue(":vid", user.id);
        chkQ.exec();
        if (chkQ.next())
        {
          QMessageBox::information(page,QStringLiteral("提示"), QStringLiteral("您已经投过票了"));
          return;
        }

        QDialog dlg(page);
        dlg.setWindowTitle(QStringLiteral("议题投票"));
        dlg.setMinimumWidth(400);
        auto *dlgLayout = new QVBoxLayout(&dlg);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto *titleLabel = new QLabel(QStringLiteral("请选择您的投票"), &dlg);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
        dlgLayout->addWidget(titleLabel);
        dlgLayout->addSpacing(12);

        auto *choiceGroup = new QButtonGroup(&dlg);
        auto *approveRadio = new QRadioButton(QStringLiteral("赞成"), &dlg);
        auto *opposeRadio = new QRadioButton(QStringLiteral("反对"), &dlg);
        auto *abstainRadio = new QRadioButton(QStringLiteral("弃权"), &dlg);
        approveRadio->setChecked(true);
        choiceGroup->addButton(approveRadio, 1);
        choiceGroup->addButton(opposeRadio, 2);
        choiceGroup->addButton(abstainRadio, 3);
        dlgLayout->addWidget(approveRadio);
        dlgLayout->addWidget(opposeRadio);
        dlgLayout->addWidget(abstainRadio);
        dlgLayout->addSpacing(16);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认投票"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
                {
                    int choice = choiceGroup->checkedId();
                    if (choice <= 0) {
                        QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请选择投票选项"));
                        return;
                    }
                    auto retVote = QMessageBox::question(page,QStringLiteral("确认操作"),
                        QStringLiteral("确认提交投票？"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                    if (retVote != QMessageBox::Yes) return;
                    DatabaseManager::instance().insert("oc_vote", {
                        {"topic_id", topicId},
                        {"voter_id", user.id},
                        {"choice", choice},
                        {"vote_time", QDateTime::currentDateTime()}
                    });
                    // 票数统计通过 oc_vote 表聚合查询实现，无需更新 oc_topic 字段
                    // 但可更新 vote_result 字段记录赞成数（可选）
                    QSqlQuery cntQ(DatabaseManager::instance().database());
                    cntQ.prepare("SELECT COUNT(*) FROM oc_vote WHERE topic_id = :tid AND choice = 1");
                    cntQ.bindValue(":tid", topicId);
                    if (cntQ.exec() && cntQ.next()) {
                        int approveCnt = cntQ.value(0).toInt();
                        DatabaseManager::instance().update("oc_topic", topicId, {
                            {"vote_result", approveCnt},
                            {"update_by", user.id},
                            {"update_time", QDateTime::currentDateTime()}
                        });
                    }
                    UiKit::showToast(QStringLiteral("投票成功"), page);
                    dlg.accept();
                    loadTopics(); });
        dlgLayout->addWidget(buttons);
        dlg.exec();
      }
      else
      {
        // 查看: 显示详情
        QSqlQuery detail(DatabaseManager::instance().database());
        detail.prepare("SELECT t.title, t.content, t.topic_type, t.vote_end, t.status, "
                       "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 1) as approve_cnt, "
                       "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 2) as oppose_cnt, "
                       "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 3) as abstain_cnt "
                       "FROM oc_topic t WHERE t.id = :id");
        detail.bindValue(":id", topicId);
        if (detail.exec() && detail.next())
        {
          QString info = QStringLiteral("标题: %1\n\n内容: %2\n\n类型: %3\n\n截止: %4\n\n状态: %5\n\n赞成: %6  反对: %7  弃权: %8")
                             .arg(detail.value(0).toString())
                             .arg(detail.value(1).toString().isEmpty() ? QStringLiteral("无") : detail.value(1).toString())
                             .arg(TopicType::label(detail.value(2).toInt()))
                             .arg(detail.value(3).toDateTime().toString("yyyy-MM-dd"))
                             .arg(detail.value(4).toInt() == 0 ? QStringLiteral("投票中") : detail.value(4).toInt() == 1 ? QStringLiteral("已结束")
                                                                                                                         : QStringLiteral("已通过"))
                             .arg(detail.value(5).toInt())
                             .arg(detail.value(6).toInt())
                             .arg(detail.value(7).toInt());
          QMessageBox::information(page,QStringLiteral("议题详情"), info);
        }
      }
    };
    *topicActionHandlerPtr = handleTopicAction;
    QObject::connect(table, &QTableWidget::cellClicked, page, [table, topicActionHandlerPtr](int row, int col)
            {
            if (col != 6) return;
            auto* item = table->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*topicActionHandlerPtr) (*topicActionHandlerPtr)(id, sts, item->data(Qt::UserRole + 3).toInt()); });
    loadTopics();
    QObject::connect(table, &QTableWidget::cellEntered, page, [=](int r, int c)
            { table->viewport()->setCursor(c == 6 ? Qt::PointingHandCursor : Qt::ArrowCursor); });

    // 发起议题按钮
    QObject::connect(newBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("发起议题"));
            dlg.setMinimumWidth(500);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* titleLabel = new QLabel(QStringLiteral("填写议题信息"), &dlg);
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
            dlgLayout->addWidget(titleLabel);
            dlgLayout->addSpacing(8);

            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* titleEdit = new QLineEdit(&dlg);
            titleEdit->setPlaceholderText(QStringLiteral("请输入议题标题"));
            form->addRow(QStringLiteral("标题:"), titleEdit);
            auto* typeCombo = new QComboBox(&dlg);
            typeCombo->addItems({QStringLiteral("公共收益"), QStringLiteral("物业监督"), QStringLiteral("设施改造"), QStringLiteral("其他")});
            form->addRow(QStringLiteral("议题类型:"), typeCombo);
            auto* contentEdit = new QTextEdit(&dlg);
            contentEdit->setPlaceholderText(QStringLiteral("请输入议题内容..."));
            contentEdit->setFixedHeight(120);
            form->addRow(QStringLiteral("内容:"), contentEdit);
            auto* voteCheck = new QCheckBox(QStringLiteral("需要投票"), &dlg);
            voteCheck->setChecked(true);
            form->addRow(QStringLiteral("投票:"), voteCheck);
            auto* voteEndEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(7), &dlg);
            voteEndEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
            voteEndEdit->setCalendarPopup(true);
            form->addRow(QStringLiteral("投票截止:"), voteEndEdit);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);

            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("发布"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (titleEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题"));
                    return;
                }
                auto& d = DatabaseManager::instance();
                const auto& user = AuthService::instance().currentUser();
                int needVote = voteCheck->isChecked() ? 1 : 0;
                QDateTime now = QDateTime::currentDateTime();
                QVariantMap data;
                data.insert("title", titleEdit->text().trimmed());
                data.insert("content", contentEdit->toPlainText().trimmed());
                data.insert("topic_type", typeCombo->currentIndex() + 1);
                data.insert("publisher_id", user.id);
                data.insert("publish_time", now);
                data.insert("need_vote", needVote);
                data.insert("vote_result", 0);
                data.insert("status", 1);  // 投票中
                data.insert("create_by", user.id);
                data.insert("create_time", now);
                data.insert("update_by", user.id);
                data.insert("update_time", now);
                if (needVote == 1) {
                    data.insert("vote_start", now);
                    data.insert("vote_end", voteEndEdit->dateTime());
                }
                d.insert("oc_topic", data);
                UiKit::showToast(QStringLiteral("议题发布成功"), page);
                dlg.accept();
                loadTopics();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
}

void PageFactory::buildPropertyParking(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // T33 停车管理
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_car"), QStringLiteral("停车管理"), QStringLiteral("车位管理、月卡办理和临停记录查看"), UiKit::moduleColor("parking"), page));
    layout->addSpacing(12);

    // Stats
    auto *statsRow = new QHBoxLayout();
    auto mkCard = [](const QString &label, const QString &val, const QString &color, QWidget *parent)
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
    QSqlQuery spaceQ("SELECT COUNT(*) FROM cm_parking_space WHERE del_flag = 0");
    int totalSpaces = spaceQ.next() ? spaceQ.value(0).toInt() : 0;
    QSqlQuery occQ("SELECT COUNT(*) FROM cm_parking_space WHERE del_flag = 0 AND status = 1");
    int occSpaces = occQ.next() ? occQ.value(0).toInt() : 0;
    statsRow->addWidget(mkCard(QStringLiteral("总车位"), QString::number(totalSpaces), "#b45309", page));
    statsRow->addWidget(mkCard(QStringLiteral("已占用"), QString::number(occSpaces), "#15803d", page));
    statsRow->addWidget(mkCard(QStringLiteral("空余车位"), QString::number(totalSpaces - occSpaces), "#d97706", page));
    QSqlQuery cardQ("SELECT COUNT(*) FROM pm_monthly_card WHERE del_flag = 0 AND status = 1");
    int cardCount = cardQ.next() ? cardQ.value(0).toInt() : 0;
    statsRow->addWidget(mkCard(QStringLiteral("月卡数"), QString::number(cardCount), "#7c3aed", page));
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
    spaceToolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
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
    cardToolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
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
    std::function<void()> loadCards = [cardTable, cardSearchEdit, cardStatusCombo, cardActionHandlerPtr, pb]()
    {
      cardTable->setRowCount(0);
      QString sql = "SELECT mc.id, mc.plate_no, mc.owner_name, ps.space_code, mc.start_date, mc.end_date, mc.status, mc.card_type "
                    "FROM pm_monthly_card mc LEFT JOIN cm_parking_space ps ON mc.space_id = ps.id "
                    "WHERE mc.del_flag = 0";
      QString searchText = cardSearchEdit->text().trimmed();
      int statusFilter = cardStatusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (mc.plate_no LIKE :search OR mc.owner_name LIKE :search)";
      if (statusFilter >= 0)
        sql += " AND mc.status = :status";
      sql += " ORDER BY mc.end_date DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery cardQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (statusFilter >= 0)
        cntBinds << ":status" << (statusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      cardQ.prepare(sql);
      if (!searchText.isEmpty())
        cardQ.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        cardQ.bindValue(":status", statusFilter);
      cardQ.bindValue(":pageSize", pb->pageSize());
      cardQ.bindValue(":offset", pb->offset());
      cardQ.exec();
      int cRow = 0;
      while (cardQ.next())
      {
        cardTable->insertRow(cRow);
        qint64 cardId = cardQ.value(0).toLongLong();
        cardTable->setItem(cRow, 0, new QTableWidgetItem(cardQ.value(1).toString()));
        cardTable->setItem(cRow, 1, new QTableWidgetItem(cardQ.value(2).toString()));
        cardTable->setItem(cRow, 2, new QTableWidgetItem(cardQ.value(3).toString()));
        QString period = cardQ.value(4).toString() + " ~ " + cardQ.value(5).toString();
        cardTable->setItem(cRow, 3, new QTableWidgetItem(period));
        int cSts = cardQ.value(6).toInt();
        auto *stsItem = UiKit::createTagTableItem(cSts == 1 ? QStringLiteral("有效") : cSts == 2 ? QStringLiteral("待续费")
                                                                                          : QStringLiteral("已过期"),
                                           QColor(cSts == 1 ? "#f6ffed" : cSts == 2 ? "#fff7e6"
                                                                                    : "#fff1f0"),
                                           QColor(cSts == 1 ? "#15803d" : cSts == 2 ? "#d97706"
                                                                                    : "#b91c1c"));
        cardTable->setItem(cRow, 4, stsItem);
        int cType = cardQ.value(7).toInt();
        QString typeText = (cType == 0) ? QStringLiteral("月卡") : (cType == 1) ? QStringLiteral("季卡")
                                                                                : QStringLiteral("年卡");
        cardTable->setItem(cRow, 5, new QTableWidgetItem(typeText));
        // 操作列: status=0(已过期) 或 2(待续费) 时显示"续费"
        QString actText = (cSts == 0 || cSts == 2) ? QStringLiteral("续费") : QStringLiteral("-");
        QString actColor = (cSts == 0 || cSts == 2) ? "#b45309" : "#64748b";
        cardTable->setItem(cRow, 6, UiKit::createActionItem(actText, actColor, cSts, cType));
        cRow++;
      }
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
    QObject::connect(cardTable, &QTableWidget::cellClicked, page, [cardTable, cardActionHandlerPtr](int row, int col)
            {
            if (col != 6) return;
            auto* item = cardTable->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*cardActionHandlerPtr) (*cardActionHandlerPtr)(id, sts, item->data(Qt::UserRole + 3).toInt()); });
    loadCards();
    QObject::connect(cardTable, &QTableWidget::cellEntered, page, [=](int r, int c)
            {
            auto* w = cardTable->cellWidget(r, c);
            bool clickable = (c == 6 && w && w->isEnabled());
            cardTable->viewport()->setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor); });

    // 办理月卡按钮
    QObject::connect(newCardBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("办理月卡"));
            dlg.setMinimumWidth(450);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* titleLabel = new QLabel(QStringLiteral("填写月卡信息"), &dlg);
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
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

void PageFactory::buildPropertyBilling(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // T31 物业缴费
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_money"), QStringLiteral("物业缴费"), QStringLiteral("物业费账单生成、缴费记录查看（演示级，不涉及真实支付）"), UiKit::moduleColor("billing"), page));
    layout->addSpacing(12);

    // Stats
    auto *statsRow = new QHBoxLayout();
    auto mkCard2 = [](const QString &label, const QString &val, const QString &color, QWidget *parent)
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
    QSqlQuery billTotalQ("SELECT COALESCE(SUM(amount),0) FROM pm_bill WHERE period = strftime('%Y-%m','now') AND del_flag = 0");
    double billTotal = billTotalQ.next() ? billTotalQ.value(0).toDouble() : 0;
    QSqlQuery billPaidQ("SELECT COALESCE(SUM(amount),0) FROM pm_bill WHERE period = strftime('%Y-%m','now') AND status = 1 AND del_flag = 0");
    double billPaid = billPaidQ.next() ? billPaidQ.value(0).toDouble() : 0;
    double billUnpaid = billTotal - billPaid;
    int billRate = billTotal > 0 ? qRound(billPaid / billTotal * 100) : 0;
    QString curMonth = QDate::currentDate().toString("M月");
    statsRow->addWidget(mkCard2(QStringLiteral("本月应收(%1)").arg(curMonth), QString("¥%1").arg(billTotal, 0, 'f', 0), "#b45309", page));
    statsRow->addWidget(mkCard2(QStringLiteral("本月已缴(%1)").arg(curMonth), QString("¥%1").arg(billPaid, 0, 'f', 0), "#15803d", page));
    statsRow->addWidget(mkCard2(QStringLiteral("本月未缴(%1)").arg(curMonth), QString("¥%1").arg(billUnpaid, 0, 'f', 0), "#b91c1c", page));
    statsRow->addWidget(mkCard2(QStringLiteral("本月缴费率(%1)").arg(curMonth), QString::number(billRate) + "%", "#d97706", page));
    statsRow->addStretch();
    layout->addLayout(statsRow);
    layout->addSpacing(12);

    // Toolbar
    auto *billToolbar = new QHBoxLayout();
    billToolbar->setSpacing(12);
    auto *billSearchEdit = new QLineEdit();
    billSearchEdit->setPlaceholderText(QStringLiteral("搜索账单编号/房屋..."));
    billSearchEdit->setMaximumWidth(260);

    billToolbar->addWidget(billSearchEdit);
    auto *billTypeCombo = new QComboBox();
    billTypeCombo->addItem(QStringLiteral("全部费用类型"), -1);
    billTypeCombo->addItem(QStringLiteral("物业费"), 1);
    billTypeCombo->addItem(QStringLiteral("水费"), 2);
    billTypeCombo->addItem(QStringLiteral("电费"), 3);
    billTypeCombo->addItem(QStringLiteral("停车费"), 4);
    billTypeCombo->addItem(QStringLiteral("综合"), 5);
    billTypeCombo->setMinimumWidth(120);
    billToolbar->addWidget(billTypeCombo);
    auto *billStatusCombo = new QComboBox();
    billStatusCombo->addItem(QStringLiteral("全部状态"), -1);
    billStatusCombo->addItem(QStringLiteral("待缴费"), 0);
    billStatusCombo->addItem(QStringLiteral("已缴费"), 1);
    billStatusCombo->addItem(QStringLiteral("逾期"), 2);
    billStatusCombo->setMinimumWidth(120);
    billToolbar->addWidget(billStatusCombo);
    // Month filter
    auto *billPeriodCombo = new QComboBox();
    billPeriodCombo->addItem(QStringLiteral("全部月份"), "");
    billPeriodCombo->addItem(QStringLiteral("本月"), QDate::currentDate().toString("yyyy-MM"));
    QSqlQuery periodQ("SELECT DISTINCT period FROM pm_bill WHERE del_flag = 0 ORDER BY period DESC");
    while (periodQ.next())
    {
      QString p = periodQ.value(0).toString();
      if (p != QDate::currentDate().toString("yyyy-MM"))
        billPeriodCombo->addItem(p, p);
    }
    billPeriodCombo->setMinimumWidth(120);
    billToolbar->addWidget(billPeriodCombo);
    billToolbar->addStretch();
    layout->insertLayout(layout->count() - 1, billToolbar);

    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("账单编号"), QStringLiteral("房屋"), QStringLiteral("费用类型"), QStringLiteral("金额"), QStringLiteral("账期"), QStringLiteral("状态")});
    table->setColumnWidth(0, 120);
    layout->addWidget(table);

    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadBills = [table, billSearchEdit, billTypeCombo, billStatusCombo, billPeriodCombo, emptyHint, pb]()
    {
      table->setSortingEnabled(false);
      table->setRowCount(0);
      QString sql = "SELECT b.bill_no, h.house_code || '-' || h.room_number AS house_info, "
                    "CASE b.bill_type WHEN 1 THEN '物业费' WHEN 2 THEN '水费' WHEN 3 THEN '电费' WHEN 4 THEN '停车费' WHEN 5 THEN '综合' END AS type_name, "
                    "b.amount, b.period, b.status "
                    "FROM pm_bill b LEFT JOIN cm_house h ON b.house_id = h.id "
                    "WHERE b.del_flag = 0";
      QString billSearch = billSearchEdit->text().trimmed();
      int billTypeFilter = billTypeCombo->currentData().toInt();
      int billStatusFilter = billStatusCombo->currentData().toInt();
      if (!billSearch.isEmpty())
        sql += " AND (b.bill_no LIKE :search OR h.house_code LIKE :search OR h.room_number LIKE :search)";
      if (billTypeFilter >= 0)
        sql += " AND b.bill_type = :btype";
      if (billStatusFilter >= 0)
        sql += " AND b.status = :status";
      QString periodFilter = billPeriodCombo->currentData().toString();
      if (!periodFilter.isEmpty())
        sql += " AND b.period = :period";
      sql += " ORDER BY b.period DESC, b.create_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery billQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!billSearch.isEmpty())
        cntBinds << ":search" << ("%" + billSearch + "%");
      if (billTypeFilter >= 0)
        cntBinds << ":btype" << (billTypeFilter);
      if (billStatusFilter >= 0)
        cntBinds << ":status" << (billStatusFilter);
      if (!periodFilter.isEmpty())
        cntBinds << ":period" << (periodFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      billQ.prepare(sql);
      if (!billSearch.isEmpty())
        billQ.bindValue(":search", "%" + billSearch + "%");
      if (billTypeFilter >= 0)
        billQ.bindValue(":btype", billTypeFilter);
      if (billStatusFilter >= 0)
        billQ.bindValue(":status", billStatusFilter);
      if (!periodFilter.isEmpty())
        billQ.bindValue(":period", periodFilter);
      billQ.bindValue(":pageSize", pb->pageSize());
      billQ.bindValue(":offset", pb->offset());
      billQ.exec();
      int bRow = 0;
      while (billQ.next())
      {
        table->insertRow(bRow);
        table->setItem(bRow, 0, new QTableWidgetItem(billQ.value(0).toString()));
        table->setItem(bRow, 1, new QTableWidgetItem(billQ.value(1).toString()));
        table->setItem(bRow, 2, new QTableWidgetItem(billQ.value(2).toString()));
        table->setItem(bRow, 3, new NumericSortTableWidgetItem(QString("¥%1").arg(billQ.value(3).toDouble(), 0, 'f', 2)));
        table->setItem(bRow, 4, new QTableWidgetItem(billQ.value(4).toString()));
        int bSts = billQ.value(5).toInt();
        auto *stsItem = UiKit::createTagTableItem(bSts == 1 ? QStringLiteral("已缴费") : bSts == 2 ? QStringLiteral("逾期")
                                                                                            : QStringLiteral("待缴费"),
                                           QColor(bSts == 1 ? "#f6ffed" : bSts == 2 ? "#fff1f0"
                                                                                    : "#fff7e6"),
                                           QColor(bSts == 1 ? "#15803d" : bSts == 2 ? "#b91c1c"
                                                                                    : "#d97706"));
        table->setItem(bRow, 5, stsItem);
        bRow++;
      }
      table->setSortingEnabled(true);
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadBills();
    QObject::connect(billSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadBills(); });
    QObject::connect(billTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadBills(); });
    QObject::connect(billStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadBills(); });
    QObject::connect(billPeriodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadBills(); });
}

void PageFactory::buildPropertyIncome(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // T36 公共收益公示
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_chart"), QStringLiteral("公共收益公示"), QStringLiteral("业委会公共收益和支出明细公示"), UiKit::moduleColor("income"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索账期..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("已公示"), 0);
    statusCombo->addItem(QStringLiteral("待审核"), 1);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("账期"), QStringLiteral("收入金额"), QStringLiteral("支出金额"), QStringLiteral("结余"), QStringLiteral("公示时间"), QStringLiteral("状态")});
    layout->addWidget(table);
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadIncomes = [table, searchEdit, statusCombo, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT period, income_amount, expense_amount, balance, publish_time, status FROM oc_public_income WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND period LIKE :search";
      if (statusFilter >= 0)
        sql += " AND status = :status";
      sql += " ORDER BY publish_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery incQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (statusFilter >= 0)
        cntBinds << ":status" << (statusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      incQ.prepare(sql);
      if (!searchText.isEmpty())
        incQ.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        incQ.bindValue(":status", statusFilter);
      incQ.bindValue(":pageSize", pb->pageSize());
      incQ.bindValue(":offset", pb->offset());
      incQ.exec();
      int iRow = 0;
      while (incQ.next())
      {
        table->insertRow(iRow);
        table->setItem(iRow, 0, new QTableWidgetItem(incQ.value(0).toString()));
        table->setItem(iRow, 1, new QTableWidgetItem(QString("¥%1").arg(incQ.value(1).toDouble(), 0, 'f', 2)));
        table->setItem(iRow, 2, new QTableWidgetItem(QString("¥%1").arg(incQ.value(2).toDouble(), 0, 'f', 2)));
        auto *balItem = new QTableWidgetItem(QString("¥%1").arg(incQ.value(3).toDouble(), 0, 'f', 2));
        if (incQ.value(3).toDouble() > 0)
          balItem->setForeground(QColor("#15803d"));
        table->setItem(iRow, 3, balItem);
        table->setItem(iRow, 4, new QTableWidgetItem(incQ.value(4).toDateTime().toString("yyyy-MM-dd")));
        int incSts = incQ.value(5).toInt();
        auto *stsItem = incSts == 0
                            ? UiKit::createTagTableItem(QStringLiteral("已公示"), QColor("#f0fdf4"), QColor("#15803d"))
                            : UiKit::createTagTableItem(QStringLiteral("待审核"), QColor("#fffbeb"), QColor("#d97706"));
        table->setItem(iRow, 5, stsItem);
        iRow++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadIncomes();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadIncomes(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadIncomes(); });
}

