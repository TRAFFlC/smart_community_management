#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

BasePage *PageFactory::buildServiceVolunteer()
{
    auto *page = new BasePage();
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

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
        card->setStyleSheet(QString("QFrame{background:#fff;border-radius:2px;border:1px solid #D4D0C8;} QLabel{background:transparent;border:none;} QFrame:hover{border-color:%1;}").arg(s.color));
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
    tabs->setStyleSheet("QTabWidget::pane{border:1px solid #D4D0C8;border-radius:4px;background:#fff;} QTabBar::tab{padding:8px 16px;} QTabBar::tab:selected{color:#b45309;border-bottom:2px solid #b45309;}");

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
    UiKit::configureTable(actTable);
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
    std::function<void()> loadActivities = [actTable, actSearchEdit, actStatusCombo, actActionHandlerPtr, actEmptyHint, pb, page]()
    {
        actTable->setSortingEnabled(false);
        actTable->setRowCount(0);
        QString actSearch = actSearchEdit->text().trimmed();
        int actStatusFilter = actStatusCombo->currentData().toInt();
        UiKit::PagedQuery pq("SELECT id, title, activity_type, location, start_time, end_time, need_count, enrolled_count, status FROM sv_volunteer_activity WHERE del_flag = 0");
        if (!actSearch.isEmpty())
            pq.where("title LIKE :search", {{"search", "%" + actSearch + "%"}});
        if (actStatusFilter >= 0)
            pq.where("status = :status", {{"status", actStatusFilter}});
        pq.orderBy("start_time DESC");

        auto result = pq.execute(pb->pageSize(), pb->offset());
        pb->setTotalCount(result.totalCount);
        int actRow = 0;
        for (const auto &row : result.rows)
        {
            actTable->insertRow(actRow);
            actTable->setItem(actRow, 0, new QTableWidgetItem(row.value(1).toString()));
            actTable->setItem(actRow, 1, new QTableWidgetItem(VolunteerActivityType::label(row.value(2).toInt())));
            actTable->setItem(actRow, 2, new QTableWidgetItem(row.value(3).toString()));
            actTable->setItem(actRow, 3, new QTableWidgetItem(row.value(4).toDateTime().toString("yyyy-MM-dd hh:mm")));
            actTable->setItem(actRow, 4, new QTableWidgetItem(row.value(5).toDateTime().toString("yyyy-MM-dd hh:mm")));
            actTable->setItem(actRow, 5, new QTableWidgetItem(QString("%1/%2").arg(row.value(6).toInt()).arg(row.value(7).toInt())));
            int sts = row.value(8).toInt();
            QString stsColor = sts == 1 ? "#15803d" : sts == 2 ? "#b45309"
                                                  : sts == 3   ? "#64748b"
                                                  : sts == 4   ? "#b91c1c"
                                                               : "#a16207";
            QString stsBg = sts == 1 ? "#f6ffed" : sts == 2 ? "#e6f4ff"
                                               : sts == 3   ? "#FAF9F6"
                                               : sts == 4   ? "#fff1f0"
                                                            : "#fffbe6";
            auto *stsItem = UiKit::createTagTableItem(VolunteerActivityStatus::label(sts), QColor(stsBg), QColor(stsColor));
            actTable->setItem(actRow, 6, stsItem);

            // Action button
            QString actActionText = (sts == 1) ? QStringLiteral("报名") : QStringLiteral("查看");
            QString actActionColor = "#b45309";
            qint64 actId = row.value(0).toLongLong();
            auto *actionBtn = new QPushButton(actActionText, actTable);
            actionBtn->setStyleSheet(QString("QPushButton{border:none; color:%1; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;} QPushButton:hover{text-decoration:underline;}").arg(actActionColor));
            actionBtn->setCursor(Qt::PointingHandCursor);
            QObject::connect(actionBtn, &QPushButton::clicked, page, [actActionHandlerPtr, actId, sts]()
                             {
                if (*actActionHandlerPtr) (*actActionHandlerPtr)(actId, sts); });
            auto *actionCell = UiKit::createActionCell({actionBtn}, actTable);
            actionCell->setProperty("actId", actId);
            actTable->setCellWidget(actRow, 7, actionCell);
            actRow++;
        }
        UiKit::syncEmptyHint(actTable, actEmptyHint);
        pb->refreshData();
        actTable->setSortingEnabled(true);
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
    loadActivities();
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
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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
    UiKit::configureTable(signupTable);
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
    std::function<void()> loadSignups = [signupTable, sigActionHandlerPtr, signupEmptyHint, page]()
    {
        signupTable->setSortingEnabled(false);
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
            if (sigSts == 0 || sigSts == 1)
            {
                QString actText = (sigSts == 0) ? QStringLiteral("签到") : QStringLiteral("签退");
                auto *actionBtn = new QPushButton(actText, signupTable);
                actionBtn->setStyleSheet(QStringLiteral(
                    "QPushButton{border:none; color:#15803d; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;}"
                    "QPushButton:hover{text-decoration:underline;}"));
                actionBtn->setCursor(Qt::PointingHandCursor);
                QObject::connect(actionBtn, &QPushButton::clicked, page, [sigActionHandlerPtr, sigId, sigSts]()
                                 {
                    if (*sigActionHandlerPtr) (*sigActionHandlerPtr)(sigId, sigSts); });
                signupTable->setCellWidget(sigRow, 6, UiKit::createActionCell({actionBtn}, signupTable));
            }
            sigRow++;
        }
        UiKit::syncEmptyHint(signupTable, signupEmptyHint);
        signupTable->setSortingEnabled(true);
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
    loadSignups();
    signupLayout->addWidget(signupTable);
    signupLayout->addWidget(signupEmptyHint);
    tabs->addTab(signupWidget, QStringLiteral("我的报名"));

    layout->addWidget(tabs, 1);
    return page;
}
