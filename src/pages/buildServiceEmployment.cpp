#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

BasePage *PageFactory::buildServiceEmployment()
{
    auto *page = new BasePage();
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    auto *table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    UiKit::configureTable(table);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(UiKit::TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);
    table->setSortingEnabled(true);

    auto *emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无岗位记录"), page);

    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_briefcase"), QStringLiteral("就业服务"), QStringLiteral("社区岗位发布、招聘信息和求职服务"), UiKit::moduleColor("job"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
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
    std::function<void()> loadJobs = [table, searchEdit, statusCombo, jobActionHandlerPtr, emptyHint, pb, page]()
    {
        table->setSortingEnabled(false);
        table->setRowCount(0);
        QString searchText = searchEdit->text().trimmed();
        int statusFilter = statusCombo->currentData().toInt();
        UiKit::PagedQuery pq("SELECT id, title, company, salary_range, headcount, deadline, status FROM sv_job_posting WHERE del_flag = 0");
        if (!searchText.isEmpty())
            pq.where("(title LIKE :search OR company LIKE :search)", {{"search", "%" + searchText + "%"}});
        if (statusFilter >= 0)
            pq.where("status = :status", {{"status", statusFilter}});
        pq.orderBy("create_time DESC");

        auto result = pq.execute(pb->pageSize(), pb->offset());
        pb->setTotalCount(result.totalCount);
        int jRow = 0;
        for (const auto& row : result.rows)
        {
            table->insertRow(jRow);
            qint64 jobId = row.value(0).toLongLong();
            table->setItem(jRow, 0, new QTableWidgetItem(row.value(1).toString()));
            table->setItem(jRow, 1, new QTableWidgetItem(row.value(2).toString()));
            table->setItem(jRow, 2, new QTableWidgetItem(row.value(3).toString()));
            table->setItem(jRow, 3, new QTableWidgetItem(row.value(4).toString()));
            table->setItem(jRow, 4, new QTableWidgetItem(row.value(5).toString()));
            int jSts = row.value(6).toInt();
            auto *stsItem = UiKit::createTagTableItem(jSts == 0 ? QStringLiteral("招聘中") : jSts == 1 ? QStringLiteral("已截止")
                                                                                                : QStringLiteral("已关闭"),
                                               QColor(jSts == 0 ? "#e6f4ff" : jSts == 1 ? "#fff7e6"
                                                                                        : "#D4D0C8"),
                                               QColor(jSts == 0 ? "#b45309" : jSts == 1 ? "#d97706"
                                                                                        : "#64748b"));
            table->setItem(jRow, 5, stsItem);
            // 操作列: status=0(招聘中) 显示"投递"
            if (jSts == 0)
            {
                auto *applyBtn = new QPushButton(QStringLiteral("投递"), table);
                applyBtn->setStyleSheet(QStringLiteral(
                    "QPushButton{border:none; color:#b45309; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;}"
                    "QPushButton:hover{text-decoration:underline;}"));
                applyBtn->setCursor(Qt::PointingHandCursor);
                QObject::connect(applyBtn, &QPushButton::clicked, page, [jobActionHandlerPtr, jobId, jSts]() {
                    if (*jobActionHandlerPtr) (*jobActionHandlerPtr)(jobId, jSts);
                });
                table->setCellWidget(jRow, 6, UiKit::createActionCell({applyBtn}, table));
            }
            else
            {
                table->setItem(jRow, 6, UiKit::createActionItem(QStringLiteral("--"), "#64748b", jobId, jSts));
            }
            jRow++;
        }
        UiKit::syncEmptyHint(table, emptyHint);
        pb->refreshData();
        table->setSortingEnabled(true);
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
        titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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
                    // 移除二次确认：按钮文案"确认投递"已明确表达最终性
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
    loadJobs();

    // 发布岗位按钮
    QObject::connect(newJobBtn, &QPushButton::clicked, page, [=]()
            {
            QDialog dlg(page);
            dlg.setWindowTitle(QStringLiteral("发布岗位"));
            dlg.setMinimumWidth(500);
            auto* dlgLayout = new QVBoxLayout(&dlg);
            dlgLayout->setContentsMargins(24, 20, 24, 20);

            auto* titleLabel = new QLabel(QStringLiteral("填写岗位信息"), &dlg);
            titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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

    layout->addWidget(table);
    layout->addWidget(emptyHint);
    return page;
}
