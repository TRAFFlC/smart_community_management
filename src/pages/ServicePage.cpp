#include "pages/PageFactory.h"
#include "PagesCommon.h"

// ========== Service Pages ==========
BasePage *PageFactory::createServicePage(const QString &sub)
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  table->horizontalHeader()->setStretchLastSection(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(UiKit::TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setSortingEnabled(true);

  auto &db = DatabaseManager::instance();

  QString svcEmptyText;
  if (sub == "volunteer")
    svcEmptyText = QStringLiteral("暂无志愿活动记录");
  else if (sub == "convenience")
    svcEmptyText = QStringLiteral("暂无便民服务记录");
  else if (sub == "job")
    svcEmptyText = QStringLiteral("暂无岗位记录");
  else
    svcEmptyText = QStringLiteral("暂无数据");
  auto *emptyHint = UiKit::createEmptyHintLabel(svcEmptyText, page);

  if (sub == "volunteer")
  {
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_people"), QStringLiteral("志愿服务"), QStringLiteral("社区志愿活动发布、报名、签到和时长管理"), UiKit::moduleColor("volunteer"), page));
    layout->addSpacing(12);

    // Stat cards
    auto *statsLayout = new QHBoxLayout();
    int totalAct = 0, recruiting = 0, totalSignup = 0, ended = 0;
    QSqlQuery sq1("SELECT COUNT(*) FROM sv_volunteer_activity WHERE del_flag = 0");
    if (sq1.next())
      totalAct = sq1.value(0).toInt();
    QSqlQuery sq2("SELECT COUNT(*) FROM sv_volunteer_activity WHERE status = 1 AND del_flag = 0");
    if (sq2.next())
      recruiting = sq2.value(0).toInt();
    QSqlQuery sq3("SELECT COUNT(*) FROM sv_volunteer_signup WHERE del_flag = 0");
    if (sq3.next())
      totalSignup = sq3.value(0).toInt();
    QSqlQuery sq4("SELECT COUNT(*) FROM sv_volunteer_activity WHERE status = 3 AND del_flag = 0");
    if (sq4.next())
      ended = sq4.value(0).toInt();
    struct VolStat
    {
      QString label;
      QString val;
      QString color;
    };
    VolStat volStats[] = {
        {QStringLiteral("总活动数"), QString::number(totalAct), "#b45309"},
        {QStringLiteral("招募中"), QString::number(recruiting), "#15803d"},
        {QStringLiteral("报名人次"), QString::number(totalSignup), "#a16207"},
        {QStringLiteral("已结束"), QString::number(ended), "#64748b"},
    };
    for (auto &s : volStats)
    {
      auto *card = new QFrame(page);
      card->setStyleSheet(QString("QFrame{background:#fff;border-radius:6px;border:1px solid #e2e8f0;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(s.color));
      card->setFixedHeight(90);
      auto *cl = new QVBoxLayout(card);
      cl->setContentsMargins(16, 10, 16, 10);
      cl->setSpacing(4);
      auto *indicator = new QFrame(card);
      indicator->setFixedHeight(3);
      indicator->setStyleSheet(QString("background:%1;border-radius:2px;").arg(s.color));
      auto *valL = new QLabel(s.val, card);
      valL->setStyleSheet(QString("font-size:26px;font-weight:bold;color:%1;background:transparent;").arg(s.color));
      auto *nameL = new QLabel(s.label, page);
      nameL->setStyleSheet("font-size:12px;color:#64748b;background:transparent;");
      cl->addWidget(indicator);
      cl->addWidget(valL);
      cl->addWidget(nameL);
      statsLayout->addWidget(card);
    }
    layout->addLayout(statsLayout);

    // Tabs
    auto *tabs = new QTabWidget(page);
    tabs->setStyleSheet("QTabWidget::pane{border:1px solid #e2e8f0;border-radius:4px;background:#fff;} QTabBar::tab{padding:8px 16px;} QTabBar::tab:selected{color:#b45309;border-bottom:2px solid #b45309;}");

    // Tab 1: Activity list
    auto *actWidget = new QWidget();
    auto *actLayout = new QVBoxLayout(actWidget);
    actLayout->setContentsMargins(12, 12, 12, 12);

    // Toolbar for activity list
    auto *actToolbar = new QHBoxLayout();
    actToolbar->setSpacing(12);
    auto *actSearchEdit = new QLineEdit();
    actSearchEdit->setPlaceholderText(QStringLiteral("搜索活动标题..."));
    actSearchEdit->setMaximumWidth(260);

    actToolbar->addWidget(actSearchEdit);
    auto *actStatusCombo = new QComboBox();
    actStatusCombo->addItem(QStringLiteral("全部状态"), -1);
    actStatusCombo->addItem(QStringLiteral("草稿"), 0);
    actStatusCombo->addItem(QStringLiteral("招募中"), 1);
    actStatusCombo->addItem(QStringLiteral("进行中"), 2);
    actStatusCombo->addItem(QStringLiteral("已结束"), 3);
    actStatusCombo->addItem(QStringLiteral("已取消"), 4);
    actStatusCombo->setMinimumWidth(120);
    actToolbar->addWidget(actStatusCombo);
    actToolbar->addStretch();
    auto *newActBtn = new QPushButton(QStringLiteral("+ 发布活动"));
    newActBtn->setProperty("cssClass", "primary");
    newActBtn->setCursor(Qt::PointingHandCursor);
    actToolbar->addWidget(newActBtn);
    actLayout->insertLayout(0, actToolbar);

    auto *actTable = new QTableWidget(actWidget);
    actTable->setAlternatingRowColors(true);
    actTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    actTable->setStyleSheet(UiKit::TABLE_STYLE);
    actTable->setShowGrid(false);
    actTable->verticalHeader()->setVisible(false);
    actTable->setSortingEnabled(true);
    actTable->setColumnCount(8);
    actTable->setHorizontalHeaderLabels({QStringLiteral("活动标题"), QStringLiteral("类型"), QStringLiteral("地点"), QStringLiteral("开始时间"), QStringLiteral("结束时间"), QStringLiteral("需求/已报"), QStringLiteral("状态"), QStringLiteral("操作")});
    actTable->setColumnWidth(0, 200);
    actTable->setColumnWidth(3, 130);
    actTable->setColumnWidth(4, 130);
    actTable->setColumnWidth(7, 90);
    actTable->horizontalHeader()->setStretchLastSection(true);

    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto actActionHandlerPtr = std::make_shared<std::function<void(qint64, int)>>();
    auto *actEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无志愿活动记录"), actWidget);
    std::function<void()> loadActivities = [actTable, actSearchEdit, actStatusCombo, actActionHandlerPtr, actEmptyHint, pb]()
    {
      actTable->setRowCount(0);
      QString sql = "SELECT id, title, activity_type, location, start_time, end_time, need_count, enrolled_count, status FROM sv_volunteer_activity WHERE del_flag = 0";
      QString actSearch = actSearchEdit->text().trimmed();
      int actStatusFilter = actStatusCombo->currentData().toInt();
      if (!actSearch.isEmpty())
        sql += " AND title LIKE :search";
      if (actStatusFilter >= 0)
        sql += " AND status = :status";
      sql += " ORDER BY start_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery actQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!actSearch.isEmpty())
        cntBinds << ":search" << ("%" + actSearch + "%");
      if (actStatusFilter >= 0)
        cntBinds << ":status" << (actStatusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      actQ.prepare(sql);
      if (!actSearch.isEmpty())
        actQ.bindValue(":search", "%" + actSearch + "%");
      if (actStatusFilter >= 0)
        actQ.bindValue(":status", actStatusFilter);
      actQ.bindValue(":pageSize", pb->pageSize());
      actQ.bindValue(":offset", pb->offset());
      actQ.exec();
      int actRow = 0;
      while (actQ.next())
      {
        actTable->insertRow(actRow);
        actTable->setItem(actRow, 0, new QTableWidgetItem(actQ.value(1).toString()));
        actTable->setItem(actRow, 1, new QTableWidgetItem(VolunteerActivityType::label(actQ.value(2).toInt())));
        actTable->setItem(actRow, 2, new QTableWidgetItem(actQ.value(3).toString()));
        actTable->setItem(actRow, 3, new QTableWidgetItem(actQ.value(4).toDateTime().toString("yyyy-MM-dd hh:mm")));
        actTable->setItem(actRow, 4, new QTableWidgetItem(actQ.value(5).toDateTime().toString("yyyy-MM-dd hh:mm")));
        actTable->setItem(actRow, 5, new QTableWidgetItem(QString("%1/%2").arg(actQ.value(6).toInt()).arg(actQ.value(7).toInt())));
        int sts = actQ.value(8).toInt();
        QString stsColor = sts == 1 ? "#15803d" : sts == 2 ? "#b45309"
                                              : sts == 3   ? "#64748b"
                                              : sts == 4   ? "#b91c1c"
                                                           : "#a16207";
        QString stsBg = sts == 1 ? "#f6ffed" : sts == 2 ? "#e6f4ff"
                                           : sts == 3   ? "#f5f5f5"
                                           : sts == 4   ? "#fff1f0"
                                                        : "#fffbe6";
        auto *stsItem = UiKit::createTagTableItem(VolunteerActivityStatus::label(sts), QColor(stsBg), QColor(stsColor));
        actTable->setItem(actRow, 6, stsItem);

        // Action text item
        QString actActionText = (sts == 1) ? QStringLiteral("报名") : QStringLiteral("查看");
        QString actActionColor = (sts == 1) ? "#b45309" : "#b45309";
        qint64 actId = actQ.value(0).toLongLong();
        actTable->setItem(actRow, 7, UiKit::createActionItem(actActionText, actActionColor, actId, sts));
        actRow++;
      }
      UiKit::syncEmptyHint(actTable, actEmptyHint);
      pb->refreshData();
    };
    std::function<void(qint64, int)> handleActivityAction = [page, loadActivities, actTable](qint64 id, int sts)
    {
      if (sts == 1)
      {
        // 查找活动标题（通过遍历 actTable 找到对应行）
        QString actTitle = QStringLiteral("此活动");
        for (int i = 0; i < actTable->rowCount(); ++i)
        {
          auto *w = actTable->cellWidget(i, 7);
          if (w && w->property("actId").toLongLong() == id)
          {
            if (auto *it = actTable->item(i, 0))
              actTitle = it->text();
            break;
          }
        }
        auto retSignup = QMessageBox::question(page, QStringLiteral("确认操作"),
                                               QStringLiteral("确认报名「%1」活动？").arg(actTitle),
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (retSignup != QMessageBox::Yes)
          return;
        const auto &user = AuthService::instance().currentUser();
        // Look up or create volunteer record for this user
        QSqlQuery volQ(DatabaseManager::instance().database());
        volQ.prepare("SELECT id FROM sv_volunteer WHERE user_id = :uid");
        volQ.bindValue(":uid", user.id);
        volQ.exec();
        qint64 volId = 0;
        if (volQ.next())
        {
          volId = volQ.value(0).toLongLong();
        }
        else
        {
          volId = DatabaseManager::instance().insert("sv_volunteer", {{"user_id", user.id}, {"skills", QStringLiteral("通用")}, {"available_time", QStringLiteral("不限")}, {"total_hours", 0}, {"status", 0}});
        }
        DatabaseManager::instance().insert("sv_volunteer_signup", {{"activity_id", id}, {"volunteer_id", volId}, {"signup_time", QDateTime::currentDateTime()}, {"status", 0}});
        UiKit::showToast(QStringLiteral("报名成功"), page);
        loadActivities();
      }
    };
    *actActionHandlerPtr = handleActivityAction;
    QObject::connect(actTable, &QTableWidget::cellClicked, page, [actTable, actActionHandlerPtr](int row, int col)
            {
            if (col != 7) return;
            auto* item = actTable->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*actActionHandlerPtr) (*actActionHandlerPtr)(id, sts); });
    loadActivities();
    QObject::connect(actTable, &QTableWidget::cellEntered, page, [=](int r, int c)
            { actTable->viewport()->setCursor(c == 7 ? Qt::PointingHandCursor : Qt::ArrowCursor); });
    QObject::connect(actSearchEdit, &QLineEdit::textChanged, page, [=]()
            { loadActivities(); });
    QObject::connect(actStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadActivities(); });

    // 发布活动按钮
    QObject::connect(newActBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("发布志愿活动"));
            dlg.setMinimumWidth(520);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* titleLabel = new QLabel(QStringLiteral("填写活动信息"), &dlg);
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
            dlgLayout->addWidget(titleLabel);
            dlgLayout->addSpacing(8);

            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* titleEdit = new QLineEdit(&dlg);
            titleEdit->setPlaceholderText(QStringLiteral("活动标题"));
            form->addRow(QStringLiteral("标题:"), titleEdit);
            auto* typeCombo = new QComboBox(&dlg);
            typeCombo->addItems({QStringLiteral("环保"), QStringLiteral("助老"), QStringLiteral("文教"), QStringLiteral("治安"), QStringLiteral("其他")});
            form->addRow(QStringLiteral("活动类型:"), typeCombo);
            auto* descEdit = new QTextEdit(&dlg);
            descEdit->setPlaceholderText(QStringLiteral("活动描述..."));
            descEdit->setFixedHeight(80);
            form->addRow(QStringLiteral("描述:"), descEdit);
            auto* locEdit = new QLineEdit(&dlg);
            locEdit->setPlaceholderText(QStringLiteral("活动地点"));
            form->addRow(QStringLiteral("地点:"), locEdit);
            auto* startEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1), &dlg);
            startEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
            startEdit->setCalendarPopup(true);
            form->addRow(QStringLiteral("开始时间:"), startEdit);
            auto* endEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1).addSecs(2 * 3600), &dlg);
            endEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
            endEdit->setCalendarPopup(true);
            form->addRow(QStringLiteral("结束时间:"), endEdit);
            auto* needCountSpin = new QSpinBox(&dlg);
            needCountSpin->setRange(1, 1000);
            needCountSpin->setValue(10);
            form->addRow(QStringLiteral("需要人数:"), needCountSpin);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);

            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("发布"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (titleEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写活动标题"));
                    return;
                }
                if (endEdit->dateTime() <= startEdit->dateTime()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("结束时间必须晚于开始时间"));
                    return;
                }
                auto& d = DatabaseManager::instance();
                const auto& user = AuthService::instance().currentUser();
                d.insert("sv_volunteer_activity", {
                    {"title", titleEdit->text().trimmed()},
                    {"description", descEdit->toPlainText().trimmed()},
                    {"activity_type", typeCombo->currentIndex() + 1},
                    {"location", locEdit->text().trimmed()},
                    {"start_time", startEdit->dateTime()},
                    {"end_time", endEdit->dateTime()},
                    {"need_count", needCountSpin->value()},
                    {"enrolled_count", 0},
                    {"publisher_id", user.id},
                    {"status", 1},  // 招募中
                    {"create_by", user.id},
                    {"create_time", QDateTime::currentDateTime()}
                });
                UiKit::showToast(QStringLiteral("活动发布成功"), page);
                dlg.accept();
                loadActivities();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
    actLayout->addWidget(actTable);
    actLayout->addWidget(actEmptyHint);
    tabs->addTab(actWidget, QStringLiteral("活动列表"));

    // Tab 2: My signups
    auto *signupWidget = new QWidget();
    auto *signupLayout = new QVBoxLayout(signupWidget);
    signupLayout->setContentsMargins(12, 12, 12, 12);

    auto *signupTable = new QTableWidget(signupWidget);
    signupTable->setAlternatingRowColors(true);
    signupTable->horizontalHeader()->setStretchLastSection(true);
    signupTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    signupTable->setStyleSheet(UiKit::TABLE_STYLE);
    signupTable->setShowGrid(false);
    signupTable->verticalHeader()->setVisible(false);
    signupTable->setSortingEnabled(true);
    signupTable->setColumnCount(7);
    signupTable->setHorizontalHeaderLabels({QStringLiteral("活动标题"), QStringLiteral("报名时间"), QStringLiteral("签到时间"), QStringLiteral("签退时间"), QStringLiteral("服务时长"), QStringLiteral("状态"), QStringLiteral("操作")});
    signupTable->setColumnWidth(0, 200);
    signupTable->setColumnWidth(6, 100);

    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto sigActionHandlerPtr = std::make_shared<std::function<void(qint64, int)>>();
    auto *signupEmptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无报名记录"), signupWidget);
    std::function<void()> loadSignups = [signupTable, sigActionHandlerPtr, signupEmptyHint]()
    {
      signupTable->setRowCount(0);
      QSqlQuery sigQ("SELECT s.id, a.title, s.signup_time, s.checkin_time, s.checkout_time, s.hours, s.status FROM sv_volunteer_signup s LEFT JOIN sv_volunteer_activity a ON s.activity_id = a.id WHERE s.del_flag = 0 ORDER BY s.signup_time DESC");
      int sigRow = 0;
      while (sigQ.next())
      {
        signupTable->insertRow(sigRow);
        signupTable->setItem(sigRow, 0, new QTableWidgetItem(sigQ.value(1).toString()));
        signupTable->setItem(sigRow, 1, new QTableWidgetItem(sigQ.value(2).toDateTime().toString("yyyy-MM-dd hh:mm")));
        signupTable->setItem(sigRow, 2, new QTableWidgetItem(sigQ.value(3).isNull() ? "-" : sigQ.value(3).toDateTime().toString("yyyy-MM-dd hh:mm")));
        signupTable->setItem(sigRow, 3, new QTableWidgetItem(sigQ.value(4).isNull() ? "-" : sigQ.value(4).toDateTime().toString("yyyy-MM-dd hh:mm")));
        double hours = sigQ.value(5).toDouble();
        signupTable->setItem(sigRow, 4, new QTableWidgetItem(hours > 0 ? QString("%1h").arg(hours) : "-"));
        int sigSts = sigQ.value(6).toInt();
        QString sigText = sigSts == 0 ? QStringLiteral("待审核") : sigSts == 1 ? QStringLiteral("已签到")
                                                               : sigSts == 2   ? QStringLiteral("已完成")
                                                                               : QStringLiteral("已确认");
        QColor sigBg = sigSts == 0 ? QColor("#fffbeb") : sigSts == 1 ? QColor("#eff6ff")
                                                     : sigSts == 2   ? QColor("#f8fafc")
                                                                     : QColor("#f0fdf4");
        QColor sigFg = sigSts == 0 ? QColor("#a16207") : sigSts == 1 ? QColor("#b45309")
                                                     : sigSts == 2   ? QColor("#64748b")
                                                                     : QColor("#15803d");
        auto *sigStsItem = UiKit::createTagTableItem(sigText, sigBg, sigFg);
        signupTable->setItem(sigRow, 5, sigStsItem);

        // 操作列: 签到/签退
        qint64 sigId = sigQ.value(0).toLongLong();
        QString actText;
        QString actColor;
        if (sigSts == 0)
        {
          actText = QStringLiteral("签到");
          actColor = "#15803d";
        }
        else if (sigSts == 1)
        {
          actText = QStringLiteral("签退");
          actColor = "#15803d";
        }
        else
        {
          actText = QStringLiteral("--");
          actColor = "#64748b";
        }
        signupTable->setItem(sigRow, 6, UiKit::createActionItem(actText, actColor, sigId, sigSts));
        sigRow++;
      }
      UiKit::syncEmptyHint(signupTable, signupEmptyHint);
    };
    std::function<void(qint64, int)> handleSignupAction = [page, loadSignups](qint64 sigId, int sigSts)
    {
      if (sigSts == 0)
      {
        // 签到: 更新 checkin_time=now, status=1
        auto retCheckin = QMessageBox::question(page, QStringLiteral("确认操作"),
                                                QStringLiteral("确认签到？"),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (retCheckin != QMessageBox::Yes)
          return;
        DatabaseManager::instance().update("sv_volunteer_signup", sigId, {{"checkin_time", QDateTime::currentDateTime()}, {"status", 1}});
        UiKit::showToast(QStringLiteral("签到成功"), page);
        loadSignups();
      }
      else if (sigSts == 1)
      {
        // 签退: 更新 checkout_time=now, 计算 hours, status=2
        auto retCheckout = QMessageBox::question(page, QStringLiteral("确认操作"),
                                                 QStringLiteral("确认签退？"),
                                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (retCheckout != QMessageBox::Yes)
          return;
        QSqlQuery chkQ(DatabaseManager::instance().database());
        chkQ.prepare("SELECT checkin_time FROM sv_volunteer_signup WHERE id = :id");
        chkQ.bindValue(":id", sigId);
        if (chkQ.exec() && chkQ.next() && !chkQ.value(0).isNull())
        {
          QDateTime checkin = chkQ.value(0).toDateTime();
          QDateTime checkout = QDateTime::currentDateTime();
          double hours = checkin.secsTo(checkout) / 3600.0;
          if (hours < 0)
            hours = 0;
          DatabaseManager::instance().update("sv_volunteer_signup", sigId, {{"checkout_time", checkout}, {"hours", hours}, {"status", 2}});
          UiKit::showToast(QStringLiteral("签退成功"), page);
        }
        else
        {
          DatabaseManager::instance().update("sv_volunteer_signup", sigId, {{"checkout_time", QDateTime::currentDateTime()}, {"hours", 0}, {"status", 2}});
          UiKit::showToast(QStringLiteral("签退成功"), page);
        }
        loadSignups();
      }
    };
    *sigActionHandlerPtr = handleSignupAction;
    QObject::connect(signupTable, &QTableWidget::cellClicked, page, [signupTable, sigActionHandlerPtr](int row, int col)
            {
            if (col != 6) return;
            auto* item = signupTable->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*sigActionHandlerPtr) (*sigActionHandlerPtr)(id, sts); });
    loadSignups();
    QObject::connect(signupTable, &QTableWidget::cellEntered, page, [=](int r, int c)
            {
            auto* w = signupTable->cellWidget(r, c);
            bool clickable = (c == 6 && w && w->isEnabled());
            signupTable->viewport()->setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor); });
    signupLayout->addWidget(signupTable);
    signupLayout->addWidget(signupEmptyHint);
    tabs->addTab(signupWidget, QStringLiteral("我的报名"));

    layout->addWidget(tabs, 1);
    delete table; // Remove the unused default table
    return page;
  }
  else if (sub == "convenience")
  {
    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索订单号/标题..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("待接单"), 0);
    statusCombo->addItem(QStringLiteral("已接单"), 1);
    statusCombo->addItem(QStringLiteral("服务中"), 2);
    statusCombo->addItem(QStringLiteral("已完成"), 3);
    statusCombo->addItem(QStringLiteral("已取消"), 4);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("订单号"), QStringLiteral("标题"), QStringLiteral("服务类型"), QStringLiteral("状态"), QStringLiteral("预约时间")});
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    std::function<void()> loadConvenience = [table, searchEdit, statusCombo, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT order_no, title, service_type, status, appointment_time FROM sv_service_order WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (order_no LIKE :search OR title LIKE :search)";
      if (statusFilter >= 0)
        sql += " AND status = :status";
      sql += " ORDER BY create_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery q(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (statusFilter >= 0)
        cntBinds << ":status" << (statusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

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
        table->setItem(row, 2, new QTableWidgetItem(ServiceType::label(q.value(2).toInt())));
        int convSts = q.value(3).toInt();
        QColor convBg, convFg;
        switch (convSts)
        {
        case 0:
          convBg = QColor("#eff6ff");
          convFg = QColor("#2563eb");
          break;
        case 1:
          convBg = QColor("#fffbeb");
          convFg = QColor("#d97706");
          break;
        case 2:
          convBg = QColor("#f0fdf4");
          convFg = QColor("#15803d");
          break;
        case 3:
          convBg = QColor("#f3e8ff");
          convFg = QColor("#7c3aed");
          break;
        default:
          convBg = QColor("#f8fafc");
          convFg = QColor("#64748b");
          break;
        }
        auto *convStsItem = UiKit::createTagTableItem(ServiceOrderStatus::label(convSts), convBg, convFg);
        table->setItem(row, 3, convStsItem);
        table->setItem(row, 4, new QTableWidgetItem(q.value(4).toDateTime().toString("yyyy-MM-dd hh:mm")));
        row++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadConvenience();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadConvenience(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadConvenience(); });
  }
  else if (sub == "job")
  {
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_briefcase"), QStringLiteral("就业服务"), QStringLiteral("社区岗位发布、招聘信息和求职服务"), UiKit::moduleColor("job"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索岗位/公司..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("招聘中"), 0);
    statusCombo->addItem(QStringLiteral("已截止"), 1);
    statusCombo->addItem(QStringLiteral("已关闭"), 2);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    auto *newJobBtn = new QPushButton(QStringLiteral("+ 发布岗位"), toolbar);
    newJobBtn->setProperty("cssClass", "primary");
    newJobBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(newJobBtn);
    layout->addWidget(toolbar);

    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({QStringLiteral("岗位名称"), QStringLiteral("公司"), QStringLiteral("薪资"), QStringLiteral("人数"), QStringLiteral("截止日期"), QStringLiteral("状态"), QStringLiteral("操作")});
    table->setColumnWidth(0, 200);
    table->setColumnWidth(1, 150);
    table->setColumnWidth(6, 90);
    auto *pb = new PaginationBar(page);
    layout->addWidget(pb);
    // 前向声明：用 shared_ptr 持有处理函数，避免循环依赖
    auto jobActionHandlerPtr = std::make_shared<std::function<void(qint64, int)>>();
    std::function<void()> loadJobs = [table, searchEdit, statusCombo, jobActionHandlerPtr, emptyHint, pb]()
    {
      table->setRowCount(0);
      QString sql = "SELECT id, title, company, salary_range, headcount, deadline, status FROM sv_job_posting WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (title LIKE :search OR company LIKE :search)";
      if (statusFilter >= 0)
        sql += " AND status = :status";
      sql += " ORDER BY create_time DESC";
      sql += " LIMIT :pageSize OFFSET :offset";
      QSqlQuery jobQ(DatabaseManager::instance().database());
      // 分页：查询满足条件的总记录数
      QVariantList cntBinds;
      if (!searchText.isEmpty())
        cntBinds << ":search" << ("%" + searchText + "%");
      if (statusFilter >= 0)
        cntBinds << ":status" << (statusFilter);
      cntBinds << ":pageSize" << pb->pageSize() << ":offset" << pb->offset();
      pb->setTotalCount(UiKit::executeCountQuery(sql, cntBinds));

      jobQ.prepare(sql);
      if (!searchText.isEmpty())
        jobQ.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        jobQ.bindValue(":status", statusFilter);
      jobQ.bindValue(":pageSize", pb->pageSize());
      jobQ.bindValue(":offset", pb->offset());
      jobQ.exec();
      int jRow = 0;
      while (jobQ.next())
      {
        table->insertRow(jRow);
        qint64 jobId = jobQ.value(0).toLongLong();
        table->setItem(jRow, 0, new QTableWidgetItem(jobQ.value(1).toString()));
        table->setItem(jRow, 1, new QTableWidgetItem(jobQ.value(2).toString()));
        table->setItem(jRow, 2, new QTableWidgetItem(jobQ.value(3).toString()));
        table->setItem(jRow, 3, new QTableWidgetItem(jobQ.value(4).toString()));
        table->setItem(jRow, 4, new QTableWidgetItem(jobQ.value(5).toString()));
        int jSts = jobQ.value(6).toInt();
        auto *stsItem = UiKit::createTagTableItem(jSts == 0 ? QStringLiteral("招聘中") : jSts == 1 ? QStringLiteral("已截止")
                                                                                            : QStringLiteral("已关闭"),
                                           QColor(jSts == 0 ? "#e6f4ff" : jSts == 1 ? "#fff7e6"
                                                                                    : "#e2e8f0"),
                                           QColor(jSts == 0 ? "#b45309" : jSts == 1 ? "#d97706"
                                                                                    : "#64748b"));
        table->setItem(jRow, 5, stsItem);
        // 操作列: status=0(招聘中) 显示"投递"
        QString actText = (jSts == 0) ? QStringLiteral("投递") : QStringLiteral("-");
        QString actColor = (jSts == 0) ? "#b45309" : "#64748b";
        table->setItem(jRow, 6, UiKit::createActionItem(actText, actColor, jobId, jSts));
        jRow++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadJobs(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadJobs(); });

    // 操作列点击处理 - 投递
    std::function<void(qint64, int)> handleJobAction = [page](qint64 jobId, int jSts)
    {
      if (jSts != 0)
        return;
      const auto &user = AuthService::instance().currentUser();

      // 检查是否已投递过（同一 job_id + applicant_id 不重复）
      QSqlQuery chkQ(DatabaseManager::instance().database());
      chkQ.prepare("SELECT id FROM sv_job_application WHERE job_id = :jid AND applicant_id = :aid AND del_flag = 0");
      chkQ.bindValue(":jid", jobId);
      chkQ.bindValue(":aid", user.id);
      chkQ.exec();
      if (chkQ.next())
      {
        QMessageBox::information(page, QStringLiteral("提示"), QStringLiteral("您已经投递过该岗位"));
        return;
      }

      // 投递对话框
      QDialog dlg(page);
      dlg.setWindowTitle(QStringLiteral("投递简历"));
      dlg.setMinimumWidth(450);
      auto *dlgLayout = new QVBoxLayout(&dlg);
      dlgLayout->setContentsMargins(24, 20, 24, 20);

      auto *titleLabel = new QLabel(QStringLiteral("填写投递信息"), &dlg);
      titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
      dlgLayout->addWidget(titleLabel);
      dlgLayout->addSpacing(8);

      auto *form = new QFormLayout(&dlg);
      form->setSpacing(12);
      form->setLabelAlignment(Qt::AlignRight);
      auto *nameEdit = new QLineEdit(&dlg);
      nameEdit->setPlaceholderText(QStringLiteral("请输入您的姓名"));
      // 默认填充当前用户真实姓名
      if (!user.realName.isEmpty())
        nameEdit->setText(user.realName);
      form->addRow(QStringLiteral("姓名:"), nameEdit);
      auto *phoneEdit = new QLineEdit(&dlg);
      phoneEdit->setPlaceholderText(QStringLiteral("请输入联系电话"));
      if (!user.phone.isEmpty())
        phoneEdit->setText(user.phone);
      form->addRow(QStringLiteral("电话:"), phoneEdit);
      auto *resumeEdit = new QTextEdit(&dlg);
      resumeEdit->setPlaceholderText(QStringLiteral("请输入自我介绍/简历内容..."));
      resumeEdit->setFixedHeight(150);
      form->addRow(QStringLiteral("简历内容:"), resumeEdit);
      dlgLayout->addLayout(form);
      dlgLayout->addSpacing(12);

      auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
      buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认投递"));
      buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
      QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
      QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
              {
                if (nameEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写姓名"));
                    return;
                }
                if (phoneEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写联系电话"));
                    return;
                }
                if (resumeEdit->toPlainText().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写简历内容"));
                    return;
                }
                auto retApply = QMessageBox::question(page, QStringLiteral("确认操作"),
                    QStringLiteral("确认投递此岗位？"),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (retApply != QMessageBox::Yes) return;
                QDateTime now = QDateTime::currentDateTime();
                DatabaseManager::instance().insert("sv_job_application", {
                    {"job_id", jobId},
                    {"applicant_id", user.id},
                    {"applicant_name", nameEdit->text().trimmed()},
                    {"applicant_phone", phoneEdit->text().trimmed()},
                    {"resume_content", resumeEdit->toPlainText().trimmed()},
                    {"status", 0},
                    {"apply_time", now},
                    {"create_by", user.id},
                    {"create_time", now}
                });
                UiKit::showToast(QStringLiteral("投递成功"), page);
                dlg.accept(); });
      dlgLayout->addWidget(buttons);
      dlg.exec();
    };
    *jobActionHandlerPtr = handleJobAction;
    QObject::connect(table, &QTableWidget::cellClicked, page, [table, jobActionHandlerPtr](int row, int col)
            {
            if (col != 6) return;
            auto* item = table->item(row, col);
            if (!item) return;
            qint64 id = item->data(Qt::UserRole).toLongLong();
            int sts = item->data(Qt::UserRole + 2).toInt();
            if (*jobActionHandlerPtr) (*jobActionHandlerPtr)(id, sts); });
    loadJobs();
    QObject::connect(table, &QTableWidget::cellEntered, page, [=](int r, int c)
            {
            auto* w = table->cellWidget(r, c);
            bool clickable = (c == 6 && w && w->isEnabled());
            table->viewport()->setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor); });

    // 发布岗位按钮
    QObject::connect(newJobBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("发布岗位"));
            dlg.setMinimumWidth(500);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* titleLabel = new QLabel(QStringLiteral("填写岗位信息"), &dlg);
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
            dlgLayout->addWidget(titleLabel);
            dlgLayout->addSpacing(8);

            auto* form = new QFormLayout(&dlg);
            form->setSpacing(12);
            form->setLabelAlignment(Qt::AlignRight);
            auto* titleEdit = new QLineEdit(&dlg);
            titleEdit->setPlaceholderText(QStringLiteral("请输入岗位标题"));
            form->addRow(QStringLiteral("岗位标题:"), titleEdit);
            auto* companyEdit = new QLineEdit(&dlg);
            companyEdit->setPlaceholderText(QStringLiteral("请输入公司名称"));
            form->addRow(QStringLiteral("公司名称:"), companyEdit);
            auto* salaryEdit = new QLineEdit(&dlg);
            salaryEdit->setPlaceholderText(QStringLiteral("如 5000-8000元/月"));
            form->addRow(QStringLiteral("薪资范围:"), salaryEdit);
            auto* reqEdit = new QTextEdit(&dlg);
            reqEdit->setPlaceholderText(QStringLiteral("请输入招聘要求..."));
            reqEdit->setFixedHeight(100);
            form->addRow(QStringLiteral("招聘要求:"), reqEdit);
            auto* descEdit = new QTextEdit(&dlg);
            descEdit->setPlaceholderText(QStringLiteral("请输入岗位描述..."));
            descEdit->setFixedHeight(100);
            form->addRow(QStringLiteral("岗位描述:"), descEdit);
            auto* headcountSpin = new QSpinBox(&dlg);
            headcountSpin->setRange(1, 999);
            headcountSpin->setValue(1);
            form->addRow(QStringLiteral("招聘人数:"), headcountSpin);
            auto* deadlineEdit = new QDateEdit(QDate::currentDate().addDays(30), &dlg);
            deadlineEdit->setDisplayFormat("yyyy-MM-dd");
            deadlineEdit->setCalendarPopup(true);
            form->addRow(QStringLiteral("截止日期:"), deadlineEdit);
            dlgLayout->addLayout(form);
            dlgLayout->addSpacing(12);

            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认发布"));
            buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
            QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
                if (titleEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写岗位标题"));
                    return;
                }
                if (companyEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写公司名称"));
                    return;
                }
                const auto& user = AuthService::instance().currentUser();
                QDateTime now = QDateTime::currentDateTime();
                DatabaseManager::instance().insert("sv_job_posting", {
                    {"publisher_id", user.id},
                    {"title", titleEdit->text().trimmed()},
                    {"company", companyEdit->text().trimmed()},
                    {"salary_range", salaryEdit->text().trimmed()},
                    {"requirements", reqEdit->toPlainText().trimmed()},
                    {"description", descEdit->toPlainText().trimmed()},
                    {"headcount", headcountSpin->value()},
                    {"deadline", deadlineEdit->date()},
                    {"status", 0},
                    {"create_by", user.id},
                    {"create_time", now},
                    {"update_by", user.id},
                    {"update_time", now}
                });
                UiKit::showToast(QStringLiteral("岗位发布成功"), page);
                dlg.accept();
                loadJobs();
            });
            dlgLayout->addWidget(buttons);
            dlg.exec(); });
  }

  layout->addWidget(table);
  layout->addWidget(emptyHint);
  return page;
}


