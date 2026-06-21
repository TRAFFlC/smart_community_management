#include "pages/PageFactory.h"

#include "PagesCommon.h"
#include "services/AuthService.h"
#include "ui_kit/PagedQuery.h"

// ========== Message Center Page ==========
BasePage *PageFactory::createMessagePage()
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(16);

  // Page header
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_bell"), QStringLiteral("消息中心"), QStringLiteral("查看系统通知、业务提醒和公告推送"), UiKit::moduleColor("message"), page));

  const auto &user = AuthService::instance().currentUser();

  // Stats cards
  auto *statsRow = new QHBoxLayout();
  statsRow->setSpacing(16);

  // 统计未读消息数
  QSqlQuery unreadQ;
  unreadQ.prepare("SELECT COUNT(*) FROM nt_notification WHERE user_id = :uid AND is_read = 0");
  unreadQ.bindValue(":uid", user.id);
  int unreadCount = (unreadQ.exec() && unreadQ.next()) ? unreadQ.value(0).toInt() : 0;
  // 统计今日新增数
  QSqlQuery todayQ;
  todayQ.prepare("SELECT COUNT(*) FROM nt_notification WHERE user_id = :uid AND date(create_time) = date('now')");
  todayQ.bindValue(":uid", user.id);
  int todayCount = (todayQ.exec() && todayQ.next()) ? todayQ.value(0).toInt() : 0;

  statsRow->addWidget(UiKit::createStatCard(QStringLiteral("未读消息"), QString::number(unreadCount), "#b91c1c", page));
  statsRow->addWidget(UiKit::createStatCard(QStringLiteral("今日新增"), QString::number(todayCount), "#b45309", page));
  statsRow->addStretch();
  layout->addLayout(statsRow);

  // Toolbar
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  auto *typeCombo = new QComboBox(toolbar);
  typeCombo->addItem(QStringLiteral("全部类型"), 0);
  typeCombo->addItem(QStringLiteral("公告"), 1);
  typeCombo->addItem(QStringLiteral("工单"), 2);
  typeCombo->addItem(QStringLiteral("事件"), 3);
  typeCombo->addItem(QStringLiteral("督办"), 4);
  typeCombo->addItem(QStringLiteral("志愿"), 5);
  typeCombo->addItem(QStringLiteral("系统"), 6);
  typeCombo->setMinimumWidth(120);
  tbLayout->addWidget(typeCombo);
  tbLayout->addStretch();
  auto *readAllBtn = new QPushButton(QStringLiteral("全部已读"), toolbar);
  UiKit::applyPrimaryButton(readAllBtn);
  readAllBtn->setCursor(Qt::PointingHandCursor);
  tbLayout->addWidget(readAllBtn);
  layout->addWidget(toolbar);

  // Table
  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  UiKit::configureTable(table);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setStyleSheet(UiKit::TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setColumnCount(5);
  table->setHorizontalHeaderLabels({QStringLiteral("类型"), QStringLiteral("标题"), QStringLiteral("内容"), QStringLiteral("时间"), QStringLiteral("状态")});
  table->setColumnWidth(0, 100);
  table->setColumnWidth(1, 200);
  table->setColumnWidth(4, 100);
  table->horizontalHeader()->setStretchLastSection(true);
  auto *pb = new PaginationBar(page);
  layout->addWidget(pb);
  layout->addWidget(table);

  // 空状态提示
  auto *emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无消息"), page);
  layout->addWidget(emptyHint);

  // 数据加载函数
  std::function<void()> loadMessages = [table, typeCombo, user, emptyHint, pb]()
  {
    table->setRowCount(0);
    int filterType = typeCombo->currentData().toInt();
    UiKit::PagedQuery pq("SELECT id, title, content, notification_type, is_read, create_time FROM nt_notification");
    pq.where("user_id = :uid", {{"uid", user.id}});
    if (filterType > 0)
      pq.where("notification_type = :type", {{"type", filterType}});
    pq.orderBy("is_read ASC, create_time DESC");

    auto result = pq.execute(pb->pageSize(), pb->offset());
    pb->setTotalCount(result.totalCount);
    int row = 0;
    // 类型标签
    auto typeLabel = [](int t) -> QPair<QString, QColor>
    {
      switch (t)
      {
      case 1:
        return {QStringLiteral("公告"), QColor("#b45309")};
      case 2:
        return {QStringLiteral("工单"), QColor("#d97706")};
      case 3:
        return {QStringLiteral("事件"), QColor("#7c3aed")};
      case 4:
        return {QStringLiteral("督办"), QColor("#b91c1c")};
      case 5:
        return {QStringLiteral("志愿"), QColor("#15803d")};
      case 6:
        return {QStringLiteral("系统"), QColor("#64748b")};
      default:
        return {QStringLiteral("未知"), QColor("#64748b")};
      }
    };
    for (const auto& rec : result.rows)
    {
      table->insertRow(row);
      qint64 msgId = rec.value(0).toLongLong();
      int ntype = rec.value(3).toInt();
      int isRead = rec.value(4).toInt();
      auto tag = typeLabel(ntype);
      auto *typeItem = UiKit::createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
      typeItem->setData(Qt::UserRole, msgId);
      table->setItem(row, 0, typeItem);

      auto *titleItem = new QTableWidgetItem(rec.value(1).toString());
      auto *contentItem = new QTableWidgetItem(rec.value(2).toString());
      auto *timeItem = new QTableWidgetItem(rec.value(5).toDateTime().toString("yyyy-MM-dd hh:mm"));
      // 未读消息加粗 + 浅蓝背景
      if (isRead == 0)
      {
        QFont boldFont = titleItem->font();
        boldFont.setBold(true);
        titleItem->setFont(boldFont);
        contentItem->setFont(boldFont);
        QColor lightBlue(230, 244, 255);
        titleItem->setBackground(lightBlue);
        contentItem->setBackground(lightBlue);
        timeItem->setBackground(lightBlue);
      }
      table->setItem(row, 1, titleItem);
      table->setItem(row, 2, contentItem);
      table->setItem(row, 3, timeItem);

      auto *statusItem = isRead == 0
                             ? UiKit::createTagTableItem(QStringLiteral("未读"), QColor("#fef2f2"), QColor("#b91c1c"))
                             : UiKit::createTagTableItem(QStringLiteral("已读"), QColor("#f1f5f9"), QColor("#64748b"));
      table->setItem(row, 4, statusItem);
      row++;
    }
    UiKit::syncEmptyHint(table, emptyHint);
    pb->refreshData();
  };

  loadMessages();
  QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadMessages(); });

  // 双击消息行查看详情并标记为已读
  QObject::connect(table, &QTableWidget::cellDoubleClicked, page, [page, table, loadMessages](int r, int)
          {
        auto* item = table->item(r, 0);
        if (!item) return;
        qint64 msgId = item->data(Qt::UserRole).toLongLong();
        if (msgId <= 0) return;

        // 查询完整消息内容
        QSqlQuery detailQ(DatabaseManager::instance().database());
        detailQ.prepare("SELECT title, content, notification_type, is_read, create_time FROM nt_notification WHERE id = :id");
        detailQ.bindValue(":id", msgId);
        if (!detailQ.exec() || !detailQ.next()) {
            QMessageBox::warning(page, QStringLiteral("提示"), QStringLiteral("消息内容加载失败"));
            return;
        }

        QString title = detailQ.value(0).toString();
        QString content = detailQ.value(1).toString();
        int ntype = detailQ.value(2).toInt();
        int isRead = detailQ.value(3).toInt();
        QDateTime createTime = detailQ.value(4).toDateTime();

        // 弹出详情对话框
        QDialog detailDlg(page);
        detailDlg.setWindowTitle(QStringLiteral("消息详情"));
        detailDlg.setMinimumWidth(480);
        auto* detailLayout = new QVBoxLayout(&detailDlg);
        detailLayout->setContentsMargins(24, 20, 24, 20);
        detailLayout->setSpacing(12);

        // 类型标签
        auto typeLabelFn = [](int t) -> QString {
            switch (t) {
                case 1: return QStringLiteral("公告");
                case 2: return QStringLiteral("工单");
                case 3: return QStringLiteral("事件");
                case 4: return QStringLiteral("督办");
                case 5: return QStringLiteral("志愿");
                case 6: return QStringLiteral("系统");
                default: return QStringLiteral("未知");
            }
        };
        auto* typeTag = new QLabel(typeLabelFn(ntype), &detailDlg);
        typeTag->setStyleSheet("background:#fef3c7;color:#b45309;padding:2px 10px;border-radius:3px;font-size:12px;");
        detailLayout->addWidget(typeTag);

        // 标题：加大字号、加粗字重，确保低 DPI 屏幕清晰可读
        auto* titleLabel = new QLabel(title, &detailDlg);
        titleLabel->setStyleSheet(
            "font-size:20px;font-weight:700;color:#141413;"
            "font-family:'Noto Sans SC','Source Han Sans SC','Microsoft YaHei UI',sans-serif;");
        titleLabel->setWordWrap(true);
        detailLayout->addWidget(titleLabel);

        // 时间
        auto* timeLabel = new QLabel(QStringLiteral("时间：%1").arg(createTime.toString("yyyy-MM-dd hh:mm")), &detailDlg);
        timeLabel->setStyleSheet("color:#64748b;font-size:12px;");
        detailLayout->addWidget(timeLabel);

        // 分割线
        auto* sep = new QFrame(&detailDlg);
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("color:#D4D0C8;background:#D4D0C8;max-height:1px;border:none;");
        detailLayout->addWidget(sep);

        // 内容（支持滚动）
        auto* contentScroll = new QScrollArea(&detailDlg);
        contentScroll->setWidgetResizable(true);
        contentScroll->setStyleSheet("QScrollArea{border:none;background:transparent;}");
        auto* contentLabel = new QLabel(content);
        contentLabel->setWordWrap(true);
        contentLabel->setStyleSheet("font-size:14px;color:#141413;line-height:1.6;background:transparent;");
        contentLabel->setTextFormat(Qt::PlainText);
        contentScroll->setWidget(contentLabel);
        contentScroll->setMinimumHeight(180);
        detailLayout->addWidget(contentScroll, 1);

        // 关闭按钮
        auto* closeBtn = new QPushButton(QStringLiteral("关闭"), &detailDlg);
        closeBtn->setProperty("cssClass", "primary");
        closeBtn->setCursor(Qt::PointingHandCursor);
        closeBtn->setFixedWidth(100);
        auto* btnRow = new QHBoxLayout();
        btnRow->addStretch();
        btnRow->addWidget(closeBtn);
        detailLayout->addLayout(btnRow);

        QObject::connect(closeBtn, &QPushButton::clicked, &detailDlg, &QDialog::accept);

        // 如果未读，标记为已读
        if (isRead == 0) {
            DatabaseManager::instance().update("nt_notification", msgId, {
                {"is_read", 1},
                {"read_time", QDateTime::currentDateTime()}
            });
        }

        detailDlg.exec();
        loadMessages(); });

  // 全部已读按钮
  QObject::connect(readAllBtn, &QPushButton::clicked, page, [page, user, loadMessages]()
          {
        QSqlQuery q(DatabaseManager::instance().database());
        q.prepare("UPDATE nt_notification SET is_read = 1, read_time = :now WHERE user_id = :uid AND is_read = 0");
        q.bindValue(":now", QDateTime::currentDateTime());
        q.bindValue(":uid", user.id);
        q.exec();
        UiKit::showToast(QStringLiteral("已全部标为已读"), page);
        loadMessages(); });

  return page;
}
