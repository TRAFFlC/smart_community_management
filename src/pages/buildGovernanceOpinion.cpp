#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

BasePage *PageFactory::buildGovernanceOpinion()
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  // Page header
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_announce"), QStringLiteral("民意收集"), QStringLiteral("居民意见箱、建议收集和热点问题分析"), UiKit::moduleColor("opinion"), page));
  layout->addSpacing(12);

  // Stats cards
  auto *statsRow = new QHBoxLayout();
  QSqlQuery opTotalQ;
  opTotalQ.prepare(QStringLiteral("SELECT COUNT(*) FROM ge_opinion WHERE create_time >= date('now','start of month') AND del_flag = 0"));
  opTotalQ.exec();
  int opTotal = opTotalQ.next() ? opTotalQ.value(0).toInt() : 0;

  QSqlQuery opReplyQ;
  opReplyQ.prepare(QStringLiteral("SELECT COUNT(*) FROM ge_opinion WHERE status >= :replied AND del_flag = 0"));
  opReplyQ.bindValue(":replied", OpinionStatus::Replied);
  opReplyQ.exec();
  int opReply = opReplyQ.next() ? opReplyQ.value(0).toInt() : 0;

  QSqlQuery opCatQ;
  opCatQ.prepare(QStringLiteral("SELECT COUNT(DISTINCT category) FROM ge_opinion WHERE del_flag = 0"));
  opCatQ.exec();
  int opCat = opCatQ.next() ? opCatQ.value(0).toInt() : 0;

  QSqlQuery opAdoptQ;
  opAdoptQ.prepare(QStringLiteral("SELECT COUNT(*) FROM ge_opinion WHERE status = :adopted AND del_flag = 0"));
  opAdoptQ.bindValue(":adopted", OpinionStatus::Adopted);
  opAdoptQ.exec();
  int opAdopt = opAdoptQ.next() ? opAdoptQ.value(0).toInt() : 0;
  int satRate = opTotal > 0 ? qRound((opReply + opAdopt) * 100.0 / opTotal) : 0;
  statsRow->addWidget(UiKit::createStatCard(QStringLiteral("本月意见"), QString::number(opTotal), "#b45309", page));
  statsRow->addWidget(UiKit::createStatCard(QStringLiteral("已回复"), QString::number(opReply), "#15803d", page));
  statsRow->addWidget(UiKit::createStatCard(QStringLiteral("热点类别"), QString::number(opCat), "#d97706", page));
  statsRow->addWidget(UiKit::createStatCard(QStringLiteral("响应率"), QString::number(satRate) + "%", "#15803d", page));
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
  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  UiKit::configureTable(table);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(UiKit::TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setSortingEnabled(true);
  table->setColumnCount(6);
  table->setHorizontalHeaderLabels({QStringLiteral("提交人"), QStringLiteral("类别"), QStringLiteral("内容摘要"), QStringLiteral("提交时间"), QStringLiteral("状态"), QStringLiteral("操作")});
  table->setColumnWidth(2, 250);
  table->setColumnWidth(5, 90);
  auto *emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无民意记录"), page);

  auto *pb = new PaginationBar(page);
  layout->addWidget(pb);
  // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
  auto opActionHandlerPtr = std::make_shared<std::function<void(qint64, int, int)>>();
  std::function<void()> loadOpinions = [table, opSearchEdit, opCatCombo, opStatusCombo, opActionHandlerPtr, emptyHint, pb, page]()
  {
    table->setSortingEnabled(false);
    table->setRowCount(0);
    QString opSearch = opSearchEdit->text().trimmed();
    QString opCatFilter = opCatCombo->currentData().toString();
    int opStatusFilter = opStatusCombo->currentData().toInt();
    UiKit::PagedQuery pq("SELECT o.id, o.title, o.category, o.content, r.name, o.create_time, o.status, o.resident_id "
                  "FROM ge_opinion o LEFT JOIN cm_resident r ON o.resident_id = r.id "
                  "WHERE o.del_flag = 0");
    if (!opSearch.isEmpty())
      pq.where("o.content LIKE :search", {{"search", "%" + opSearch + "%"}});
    if (opCatFilter != "-1" && !opCatFilter.isEmpty())
      pq.where("o.category = :cat", {{"cat", opCatFilter}});
    if (opStatusFilter >= 0)
      pq.where("o.status = :status", {{"status", opStatusFilter}});
    pq.orderBy("o.create_time DESC");

    auto result = pq.execute(pb->pageSize(), pb->offset());
    pb->setTotalCount(result.totalCount);
    int oRow = 0;
    for (const auto& row : result.rows)
    {
      table->insertRow(oRow);
      table->setItem(oRow, 0, new QTableWidgetItem(row.value(4).toString()));
      table->setItem(oRow, 1, new QTableWidgetItem(row.value(2).toString()));
      table->setItem(oRow, 2, new QTableWidgetItem(row.value(3).toString()));
      table->setItem(oRow, 3, new QTableWidgetItem(row.value(5).toDateTime().toString("yyyy-MM-dd")));
      int oSts = row.value(6).toInt();
      auto *stsItem = UiKit::createTagTableItem(oSts == 0 ? QStringLiteral("待处理") : oSts == 1 ? QStringLiteral("已回复")
                                                                                          : QStringLiteral("已采纳"),
                                         QColor(oSts == 0 ? "#fff7e6" : oSts == 1 ? "#e6f4ff"
                                                                                  : "#f6ffed"),
                                         QColor(oSts == 0 ? "#d97706" : oSts == 1 ? "#b45309"
                                                                                  : "#15803d"));
      table->setItem(oRow, 4, stsItem);
      // 操作列
      qint64 opId = row.value(0).toLongLong();
      int residentId = row.value(7).toInt();
      QString actText = (oSts == 0) ? QStringLiteral("回复") : QStringLiteral("查看");
      QString actColor = (oSts == 0) ? "#15803d" : "#b45309";
      auto *actionBtn = new QPushButton(actText, table);
      actionBtn->setStyleSheet(QString("QPushButton{border:none; color:%1; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;} QPushButton:hover{text-decoration:underline;}").arg(actColor));
      actionBtn->setCursor(Qt::PointingHandCursor);
      QObject::connect(actionBtn, &QPushButton::clicked, page, [opActionHandlerPtr, opId, oSts, residentId]() {
          if (*opActionHandlerPtr) (*opActionHandlerPtr)(opId, oSts, residentId);
      });
      table->setCellWidget(oRow, 5, UiKit::createActionCell({actionBtn}, table));
      oRow++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
    table->setSortingEnabled(true);
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
      titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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
  loadOpinions();
  QObject::connect(opSearchEdit, &QLineEdit::textChanged, page, [=]()
          { loadOpinions(); });
  QObject::connect(opCatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadOpinions(); });
  QObject::connect(opStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadOpinions(); });

  layout->addWidget(table);
  layout->addWidget(emptyHint);
  return page;
}
