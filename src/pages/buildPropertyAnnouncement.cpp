#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

void PageFactory::buildPropertyAnnouncement(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // Page header
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_announce"), QStringLiteral("公告通知管理"), QStringLiteral("发布和管理小区公告、社区通知、物业公告和系统公告"), UiKit::moduleColor("announcement"), page));
    layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
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

    std::function<void()> loadAnnouncements = [table, searchEdit, typeCombo, emptyHint, pb, page]()
    {
      table->setRowCount(0);
      QString searchText = searchEdit->text().trimmed();
      int filterType = typeCombo->currentData().toInt();
      UiKit::PagedQuery pq("SELECT id, title, announcement_type, publisher_id, publish_time, read_count FROM nt_announcement WHERE del_flag = 0");
      if (!searchText.isEmpty())
        pq.where("title LIKE :search", {{"search", "%" + searchText + "%"}});
      if (filterType >= 0)
        pq.where("announcement_type = :type", {{"type", filterType}});
      pq.orderBy("publish_time DESC");

      auto result = pq.execute(pb->pageSize(), pb->offset());
      pb->setTotalCount(result.totalCount);
      int row = 0;
      for (const auto& rec : result.rows)
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(rec.value(1).toString()));
        table->setItem(row, 1, new QTableWidgetItem(AnnouncementType::label(rec.value(2).toInt())));
        table->setItem(row, 2, new QTableWidgetItem(QStringLiteral("管理员")));
        table->setItem(row, 3, new QTableWidgetItem(rec.value(4).toDateTime().toString("yyyy-MM-dd")));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(rec.value(5).toInt())));
        qint64 annId = rec.value(0).toLongLong();
        QString annTitle = rec.value(1).toString();
        auto *viewBtn = new QPushButton(QStringLiteral("查看"), table);
        viewBtn->setStyleSheet(QStringLiteral(
            "QPushButton{border:none; color:#b45309; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;}"
            "QPushButton:hover{text-decoration:underline;}"));
        viewBtn->setCursor(Qt::PointingHandCursor);
        QObject::connect(viewBtn, &QPushButton::clicked, page, [page, annId, annTitle]() {
            QSqlQuery detail(DatabaseManager::instance().database());
            detail.prepare("SELECT content FROM nt_announcement WHERE id = :id");
            detail.bindValue(":id", annId);
            if (detail.exec() && detail.next())
                QMessageBox::information(page, annTitle, detail.value(0).toString());
        });
        table->setCellWidget(row, 5, UiKit::createActionCell({viewBtn}, table));
        row++;
      }
      UiKit::syncEmptyHint(table, emptyHint);
      pb->refreshData();
    };
    loadAnnouncements();
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
            formTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
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
