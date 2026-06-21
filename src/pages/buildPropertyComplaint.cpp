#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

void PageFactory::buildPropertyComplaint(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_chat"), QStringLiteral("投诉建议管理"), QStringLiteral("管理居民投诉和建议，跟踪处理进度和满意度评价"), UiKit::moduleColor("complaint"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
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
    std::function<void()> loadComplaints = [table, searchEdit, typeCombo, filterCombo, cmpActionHandlerPtr, emptyHint, pb, page]()
    {
      table->setRowCount(0);
      // 注入数据权限过滤（居民只看本人，物业/社区按组织范围）
      auto scopeFilter = UiKit::buildDataScopeFilter("", "estate_id");
      UiKit::PagedQuery pq("SELECT id, order_no, title, order_type, reporter_name, create_time, status FROM wo_work_order WHERE del_flag = 0 AND source = 2");
      // 数据权限过滤的 SQL 以 " AND " 开头，需去除前缀；binds 为 [":ph", val, ...] 对，需转为 QVariantMap
      if (!scopeFilter.first.isEmpty())
      {
        QString scopeSql = scopeFilter.first.trimmed();
        if (scopeSql.startsWith("AND", Qt::CaseInsensitive))
          scopeSql = scopeSql.mid(3).trimmed();
        QVariantMap scopeBinds;
        for (int i = 0; i + 1 < scopeFilter.second.size(); i += 2)
        {
          QString key = scopeFilter.second[i].toString();
          if (key.startsWith(":")) key = key.mid(1);
          scopeBinds.insert(key, scopeFilter.second[i + 1]);
        }
        pq.where(scopeSql, scopeBinds);
      }
      QString searchText = searchEdit->text().trimmed();
      int filterType = typeCombo->currentData().toInt();
      int filterIdx = filterCombo->currentIndex();
      if (!searchText.isEmpty())
        pq.where("(order_no LIKE :search OR title LIKE :search)", {{"search", "%" + searchText + "%"}});
      if (filterType >= 0)
        pq.where("order_type = :otype", {{"otype", filterType}});
      if (filterIdx > 0)
        pq.where("status = :status", {{"status", WorkOrderStatus::statusFilterMap()[filterIdx]}});
      pq.orderBy("create_time DESC");

      auto result = pq.execute(pb->pageSize(), pb->offset());
      pb->setTotalCount(result.totalCount);
      int row = 0;
      for (const auto& rec : result.rows)
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(rec.value(1).toString()));
        table->setItem(row, 1, new QTableWidgetItem(rec.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(rec.value(3).toInt() == 0 ? QStringLiteral("投诉") : QStringLiteral("建议")));
        table->setItem(row, 3, new QTableWidgetItem(rec.value(4).toString()));
        table->setItem(row, 4, new QTableWidgetItem(rec.value(5).toDateTime().toString("yyyy-MM-dd")));
        QColor cmpColor(WorkOrderStatus::color(rec.value(6).toInt()));
        auto *stsItem = UiKit::createTagTableItem(WorkOrderStatus::label(rec.value(6).toInt()), QColor(cmpColor.red(), cmpColor.green(), cmpColor.blue(), 30), cmpColor);
        table->setItem(row, 5, stsItem);
        qint64 cmpId = rec.value(0).toLongLong();
        int cmpSts = rec.value(6).toInt();
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
        auto *actionBtn = new QPushButton(actText, table);
        actionBtn->setStyleSheet(QString("QPushButton{border:none; color:%1; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;} QPushButton:hover{text-decoration:underline;}").arg(actColor));
        actionBtn->setCursor(Qt::PointingHandCursor);
        QObject::connect(actionBtn, &QPushButton::clicked, page, [cmpActionHandlerPtr, cmpId, cmpSts]() {
            if (*cmpActionHandlerPtr) (*cmpActionHandlerPtr)(cmpId, cmpSts);
        });
        table->setCellWidget(row, 6, UiKit::createActionCell({actionBtn}, table));
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
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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
    loadComplaints();

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
