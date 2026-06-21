#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

BasePage *PageFactory::buildGovernanceSupervision()
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  // Page header
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_list"), QStringLiteral("督办管理"), QStringLiteral("督办任务创建、进度跟踪和催办管理"), UiKit::moduleColor("supervision"), page));
  layout->addSpacing(12);

  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
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

  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  UiKit::configureTable(table);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(UiKit::TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setSortingEnabled(true);
  table->setColumnCount(8);
  table->setHorizontalHeaderLabels({QStringLiteral("关联事件"), QStringLiteral("督办人"), QStringLiteral("被督办人"), QStringLiteral("截止日期"), QStringLiteral("原因"), QStringLiteral("反馈"), QStringLiteral("状态"), QStringLiteral("操作")});
  table->setColumnWidth(4, 150);
  table->setColumnWidth(5, 150);
  table->setColumnWidth(7, 90);
  auto *emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无督办记录"), page);
  auto *pb = new PaginationBar(page);
  layout->addWidget(pb);

  // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
  auto supActionHandlerPtr = std::make_shared<std::function<void(qint64, int)>>();
  std::function<void()> loadSupervisions = [table, searchEdit, statusCombo, supActionHandlerPtr, emptyHint, pb, page]()
  {
    table->setSortingEnabled(false);
    table->setRowCount(0);
    QString searchText = searchEdit->text().trimmed();
    int statusFilter = statusCombo->currentData().toInt();
    UiKit::PagedQuery pq("SELECT s.id, e.title, u1.real_name, u2.real_name, s.deadline, s.reason, s.feedback, s.status FROM ge_supervision s LEFT JOIN ge_event e ON s.event_id = e.id LEFT JOIN sys_user u1 ON s.supervisor_id = u1.id LEFT JOIN sys_user u2 ON s.supervise_to = u2.id WHERE s.del_flag = 0");
    if (!searchText.isEmpty())
      pq.where("(e.title LIKE :search OR u1.real_name LIKE :search)", {{"search", "%" + searchText + "%"}});
    if (statusFilter >= 0)
      pq.where("s.status = :status", {{"status", statusFilter}});
    pq.orderBy("s.create_time DESC");

    auto result = pq.execute(pb->pageSize(), pb->offset());
    pb->setTotalCount(result.totalCount);
    int sRow = 0;
    for (const auto& row : result.rows)
    {
      table->insertRow(sRow);
      qint64 supId = row.value(0).toLongLong();
      table->setItem(sRow, 0, new QTableWidgetItem(row.value(1).toString()));
      table->setItem(sRow, 1, new QTableWidgetItem(row.value(2).toString()));
      table->setItem(sRow, 2, new QTableWidgetItem(row.value(3).toString()));
      table->setItem(sRow, 3, new QTableWidgetItem(row.value(4).toDateTime().toString("yyyy-MM-dd")));
      table->setItem(sRow, 4, new QTableWidgetItem(row.value(5).toString()));
      table->setItem(sRow, 5, new QTableWidgetItem(row.value(6).toString()));
      int sSts = row.value(7).toInt();
      auto *stsItem = UiKit::createTagTableItem(sSts == 0 ? QStringLiteral("待反馈") : sSts == 1 ? QStringLiteral("已反馈")
                                                                                          : QStringLiteral("已验收"),
                                         QColor(sSts == 0 ? "#fff7e6" : sSts == 1 ? "#e6f4ff"
                                                                                  : "#f6ffed"),
                                         QColor(sSts == 0 ? "#d97706" : sSts == 1 ? "#b45309"
                                                                                  : "#15803d"));
      table->setItem(sRow, 6, stsItem);
      // 操作列: status=0 显示"反馈", status=1 显示"验收", status=2 显示"--"
      if (sSts == 0 || sSts == 1)
      {
        QString actText = (sSts == 0) ? QStringLiteral("反馈") : QStringLiteral("验收");
        QString actColor = (sSts == 0) ? "#a16207" : "#15803d";
        auto *actionBtn = new QPushButton(actText, table);
        actionBtn->setStyleSheet(QString("QPushButton{border:none; color:%1; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;} QPushButton:hover{text-decoration:underline;}").arg(actColor));
        actionBtn->setCursor(Qt::PointingHandCursor);
        QObject::connect(actionBtn, &QPushButton::clicked, page, [supActionHandlerPtr, supId, sSts]() {
            if (*supActionHandlerPtr) (*supActionHandlerPtr)(supId, sSts);
        });
        table->setCellWidget(sRow, 7, UiKit::createActionCell({actionBtn}, table));
      }
      else
      {
        table->setItem(sRow, 7, UiKit::createActionItem(QStringLiteral("--"), "#64748b", supId, sSts));
      }
      sRow++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
    table->setSortingEnabled(true);
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
      titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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
  loadSupervisions();

  // 创建督办按钮
  QObject::connect(newSupBtn, &QPushButton::clicked, page, [=]()
          {
          QDialog dlg(page);
          dlg.setWindowTitle(QStringLiteral("创建督办"));
          dlg.setMinimumWidth(500);
          auto* dlgLayout = new QVBoxLayout(&dlg);
          dlgLayout->setContentsMargins(24, 20, 24, 20);

          auto* titleLabel = new QLabel(QStringLiteral("填写督办信息"), &dlg);
          titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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

  layout->addWidget(table);
  layout->addWidget(emptyHint);
  return page;
}
