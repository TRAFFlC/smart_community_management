#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include <QButtonGroup>
#include <QRadioButton>
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

void PageFactory::buildPropertyTopic(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_list"), QStringLiteral("业委会议题管理"), QStringLiteral("议题发布、投票管理和结果公示"), UiKit::moduleColor("topic"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
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
    std::function<void()> loadTopics = [table, searchEdit, statusCombo, emptyHint, topicActionHandlerPtr, pb, page]()
    {
      table->setRowCount(0);
      QString baseSql = "SELECT t.id, t.title, t.topic_type, t.vote_end, t.status, t.need_vote, "
                    "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 1) as approve_cnt, "
                    "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 2) as oppose_cnt, "
                    "(SELECT COUNT(*) FROM oc_vote v WHERE v.topic_id = t.id AND v.choice = 3) as abstain_cnt, "
                    "COALESCE((SELECT real_name FROM sys_user WHERE id = t.publisher_id), '业委会') as publisher "
                    "FROM oc_topic t WHERE t.del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      UiKit::PagedQuery pq(baseSql);
      if (!searchText.isEmpty())
        pq.where("t.title LIKE :search", {{"search", "%" + searchText + "%"}});
      if (statusFilter >= 0)
        pq.where("t.status = :status", {{"status", statusFilter}});
      pq.orderBy("t.publish_time DESC");

      auto result = pq.execute(pb->pageSize(), pb->offset());
      pb->setTotalCount(result.totalCount);
      int tRow = 0;
      for (const auto& rec : result.rows)
      {
        table->insertRow(tRow);
        table->setItem(tRow, 0, new QTableWidgetItem(rec.value(1).toString()));
        table->setItem(tRow, 1, new QTableWidgetItem(TopicType::label(rec.value(2).toInt())));
        table->setItem(tRow, 2, new QTableWidgetItem(rec.value(9).toString()));
        table->setItem(tRow, 3, new QTableWidgetItem(rec.value(3).toDateTime().toString("yyyy-MM-dd")));
        int approve = rec.value(6).toInt();
        int oppose = rec.value(7).toInt();
        int abstain = rec.value(8).toInt();
        int total = approve + oppose + abstain;
        int tSts = rec.value(4).toInt();
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
                                           QColor(tSts == 0 ? "#e6f4ff" : tSts == 1 ? "#D4D0C8"
                                                                                    : "#f6ffed"),
                                           QColor(tSts == 0 ? "#b45309" : tSts == 1 ? "#64748b"
                                                                                    : "#15803d"));
        table->setItem(tRow, 5, stsItem);
        // 操作列: status=0(投票中) 且 need_vote=1 时显示"投票"
        int needVote = rec.value(5).toInt();
        qint64 topicId = rec.value(0).toLongLong();
        QString actText = (tSts == 0 && needVote == 1) ? QStringLiteral("投票") : QStringLiteral("查看");
        QString actColor = "#b45309";
        auto *actionBtn = new QPushButton(actText, table);
        actionBtn->setStyleSheet(QString("QPushButton{border:none; color:%1; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;} QPushButton:hover{text-decoration:underline;}").arg(actColor));
        actionBtn->setCursor(Qt::PointingHandCursor);
        QObject::connect(actionBtn, &QPushButton::clicked, page, [topicActionHandlerPtr, topicId, tSts, needVote]() {
            if (*topicActionHandlerPtr) (*topicActionHandlerPtr)(topicId, tSts, needVote);
        });
        table->setCellWidget(tRow, 6, UiKit::createActionCell({actionBtn}, table));
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
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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
    loadTopics();

    // 发起议题按钮
    QObject::connect(newBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("发起议题"));
            dlg.setMinimumWidth(500);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* titleLabel = new QLabel(QStringLiteral("填写议题信息"), &dlg);
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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
