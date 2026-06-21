#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

void PageFactory::buildPropertyVisitor(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_visitor"), QStringLiteral("访客管理"), QStringLiteral("访客登记、临时通行码管理和访客记录查看"), UiKit::moduleColor("visitor"), page));
    layout->addSpacing(12);

    // Stats
    auto *statsRow = new QHBoxLayout();
    QSqlQuery todayVisQ("SELECT COUNT(*) FROM cm_visitor WHERE date(arrive_time) = date('now') AND del_flag = 0");
    int todayVis = todayVisQ.next() ? todayVisQ.value(0).toInt() : 0;
    QSqlQuery weekVisQ("SELECT COUNT(*) FROM cm_visitor WHERE arrive_time >= date('now', '-7 days') AND del_flag = 0");
    int weekVis = weekVisQ.next() ? weekVisQ.value(0).toInt() : 0;
    QSqlQuery activeVisQ("SELECT COUNT(*) FROM cm_visitor WHERE status = 0 AND del_flag = 0");
    int activeVis = activeVisQ.next() ? activeVisQ.value(0).toInt() : 0;
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("今日访客"), QString::number(todayVis), "#b45309", page));
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("本周访客"), QString::number(weekVis), "#15803d", page));
    statsRow->addWidget(UiKit::createStatCard(QStringLiteral("在访中"), QString::number(activeVis), "#d97706", page));
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

    std::function<void()> loadVisitors = [table, visSearchEdit, visStatusCombo, emptyHint, pb, page, visActionHandlerPtr]()
    {
      table->setRowCount(0);
      QString visSearch = visSearchEdit->text().trimmed();
      int visFilter = visStatusCombo->currentData().toInt();
      UiKit::PagedQuery pq("SELECT id, visitor_name, phone, host_name, arrive_time, leave_time, status FROM cm_visitor WHERE del_flag = 0");
      if (!visSearch.isEmpty())
        pq.where("(visitor_name LIKE :search OR phone LIKE :search)", {{"search", "%" + visSearch + "%"}});
      if (visFilter >= 0)
        pq.where("status = :status", {{"status", visFilter}});
      pq.orderBy("arrive_time DESC");

      auto result = pq.execute(pb->pageSize(), pb->offset());
      pb->setTotalCount(result.totalCount);
      int vRow = 0;
      for (const auto& row : result.rows)
      {
        table->insertRow(vRow);
        qint64 visId = row.value(0).toLongLong();
        table->setItem(vRow, 0, new QTableWidgetItem(row.value(1).toString()));
        table->setItem(vRow, 1, new QTableWidgetItem(row.value(2).toString()));
        table->setItem(vRow, 2, new QTableWidgetItem(row.value(3).toString()));
        table->setItem(vRow, 3, new QTableWidgetItem(row.value(4).toDateTime().toString("yyyy-MM-dd HH:mm")));
        table->setItem(vRow, 4, new QTableWidgetItem(row.value(5).isNull() ? "-" : row.value(5).toDateTime().toString("yyyy-MM-dd HH:mm")));
        int vSts = row.value(6).toInt();
        auto *stsItem = UiKit::createTagTableItem(vSts == 0 ? QStringLiteral("在访") : QStringLiteral("已离开"), QColor(vSts == 0 ? "#e6f4ff" : "#D4D0C8"), QColor(vSts == 0 ? "#b45309" : "#64748b"));
        table->setItem(vRow, 5, stsItem);
        // 操作列
        if (vSts == 0)
        {
            auto *signOutBtn = new QPushButton(QStringLiteral("签离"), table);
            signOutBtn->setStyleSheet(QStringLiteral(
                "QPushButton{border:none; color:#b45309; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;}"
                "QPushButton:hover{text-decoration:underline;}"));
            signOutBtn->setCursor(Qt::PointingHandCursor);
            QObject::connect(signOutBtn, &QPushButton::clicked, page, [visActionHandlerPtr, visId, vSts]() {
                if (*visActionHandlerPtr) (*visActionHandlerPtr)(visId, vSts);
            });
            table->setCellWidget(vRow, 6, UiKit::createActionCell({signOutBtn}, table));
        }
        else
        {
            table->setItem(vRow, 6, UiKit::createActionItem(QStringLiteral("--"), "#64748b", visId, vSts));
        }
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
    loadVisitors();

    // 访客登记按钮
    QObject::connect(visRegisterBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("访客登记"));
            dlg.setMinimumWidth(450);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* titleLabel = new QLabel(QStringLiteral("填写访客信息"), &dlg);
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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
