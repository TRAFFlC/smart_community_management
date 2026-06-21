#include "pages/PageFactory.h"

#include "PagesCommon.h"

// ========== Todo Center Page ==========
BasePage *PageFactory::createTodoPage()
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(16);

  // Page header
  layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_todo"), QStringLiteral("待办中心"), QStringLiteral("聚合展示各模块待处理事项，快速定位处理"), UiKit::moduleColor("todo"), page));

  // Stats cards
  auto *statsRow = new QHBoxLayout();
  statsRow->setSpacing(16);

  // 统计待受理工单数
  QSqlQuery woQ("SELECT COUNT(*) FROM wo_work_order WHERE status = 0 AND del_flag = 0");
  int woCount = woQ.next() ? woQ.value(0).toInt() : 0;
  // 统计待审核事件数
  QSqlQuery evQ("SELECT COUNT(*) FROM ge_event WHERE status = 0 AND del_flag = 0");
  int evCount = evQ.next() ? evQ.value(0).toInt() : 0;
  // 统计待反馈督办数
  QSqlQuery supQ("SELECT COUNT(*) FROM ge_supervision WHERE status = 0 AND del_flag = 0");
  int supCount = supQ.next() ? supQ.value(0).toInt() : 0;
  // 统计待处理民意数
  QSqlQuery opQ("SELECT COUNT(*) FROM ge_opinion WHERE status = 0 AND del_flag = 0");
  int opCount = opQ.next() ? opQ.value(0).toInt() : 0;

  statsRow->addWidget(UiKit::createStatCard(QStringLiteral("待受理工单"), QString::number(woCount), "#b45309", page));
  statsRow->addWidget(UiKit::createStatCard(QStringLiteral("待审核事件"), QString::number(evCount), "#d97706", page));
  statsRow->addWidget(UiKit::createStatCard(QStringLiteral("待反馈督办"), QString::number(supCount), "#b91c1c", page));
  statsRow->addWidget(UiKit::createStatCard(QStringLiteral("待处理民意"), QString::number(opCount), "#15803d", page));
  layout->addLayout(statsRow);

  // Toolbar with filter
  auto *toolbar = new QWidget(page);
  toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
  auto *tbLayout = new QHBoxLayout(toolbar);
  tbLayout->setContentsMargins(4, 4, 4, 4);
  auto *typeCombo = new QComboBox(toolbar);
  typeCombo->addItem(QStringLiteral("全部类型"), 0);
  typeCombo->addItem(QStringLiteral("工单"), 1);
  typeCombo->addItem(QStringLiteral("事件"), 2);
  typeCombo->addItem(QStringLiteral("督办"), 3);
  typeCombo->addItem(QStringLiteral("民意"), 4);
  typeCombo->setMinimumWidth(120);
  tbLayout->addWidget(typeCombo);
  tbLayout->addStretch();
  layout->addWidget(toolbar);

  // Table
  auto *table = new QTableWidget(page);
  table->setAlternatingRowColors(true);
  UiKit::configureTable(table);
  table->setStyleSheet(UiKit::TABLE_STYLE);
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setColumnCount(6);
  table->setHorizontalHeaderLabels({QStringLiteral("类型"), QStringLiteral("标题"), QStringLiteral("提交人"), QStringLiteral("优先级"), QStringLiteral("提交时间"), QStringLiteral("操作")});
  table->setColumnWidth(0, 100);
  table->setColumnWidth(5, 100);
  table->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(table);

  // 空状态提示
  auto *emptyHint = UiKit::createEmptyHintLabel(QStringLiteral("暂无待办事项"), page);
  layout->addWidget(emptyHint);

  // 数据加载函数
  std::function<void()> loadTodos = [table, typeCombo, emptyHint]()
  {
    table->setRowCount(0);
    int filterType = typeCombo->currentData().toInt();
    int row = 0;

    // 类型颜色映射
    auto typeTag = [](int t) -> QPair<QString, QColor>
    {
      switch (t)
      {
      case 1:
        return {QStringLiteral("工单"), QColor("#b45309")};
      case 2:
        return {QStringLiteral("事件"), QColor("#d97706")};
      case 3:
        return {QStringLiteral("督办"), QColor("#b91c1c")};
      case 4:
        return {QStringLiteral("民意"), QColor("#15803d")};
      default:
        return {QStringLiteral("未知"), QColor("#64748b")};
      }
    };
    auto priorityLabel = [](int p) -> QString
    {
      switch (p)
      {
      case 0:
        return QStringLiteral("低");
      case 1:
        return QStringLiteral("中");
      case 2:
        return QStringLiteral("高");
      case 3:
        return QStringLiteral("紧急");
      default:
        return QStringLiteral("中");
      }
    };

    // 工单待受理 (status=0)
    if (filterType == 0 || filterType == 1)
    {
      QSqlQuery q("SELECT id, order_no, title, reporter_name, priority, create_time FROM wo_work_order WHERE status = 0 AND del_flag = 0 ORDER BY create_time DESC");
      while (q.next())
      {
        table->insertRow(row);
        auto tag = typeTag(1);
        auto *typeItem = UiKit::createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 3, new QTableWidgetItem(priorityLabel(q.value(4).toInt())));
        table->setItem(row, 4, new QTableWidgetItem(q.value(5).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *actionItem = UiKit::createActionItem(QStringLiteral("去处理"), QStringLiteral("#92400E"),
                                                   0, QStringLiteral("301"));
        table->setItem(row, 5, actionItem);
        row++;
      }
    }

    // 事件待审核 (status=0)
    if (filterType == 0 || filterType == 2)
    {
      QSqlQuery q("SELECT id, event_no, title, reporter_name, priority, create_time FROM ge_event WHERE status = 0 AND del_flag = 0 ORDER BY create_time DESC");
      while (q.next())
      {
        table->insertRow(row);
        auto tag = typeTag(2);
        auto *typeItem = UiKit::createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 3, new QTableWidgetItem(priorityLabel(q.value(4).toInt())));
        table->setItem(row, 4, new QTableWidgetItem(q.value(5).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *actionItem = UiKit::createActionItem(QStringLiteral("去处理"), QStringLiteral("#92400E"),
                                                   0, QStringLiteral("401"));
        table->setItem(row, 5, actionItem);
        row++;
      }
    }

    // 督办待反馈 (status=0)
    if (filterType == 0 || filterType == 3)
    {
      QSqlQuery q("SELECT id, reason, supervise_to, deadline, status FROM ge_supervision WHERE status = 0 AND del_flag = 0 ORDER BY deadline ASC");
      while (q.next())
      {
        table->insertRow(row);
        auto tag = typeTag(3);
        auto *typeItem = UiKit::createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(QStringLiteral("高")));
        table->setItem(row, 4, new QTableWidgetItem(q.value(3).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *actionItem = UiKit::createActionItem(QStringLiteral("去处理"), QStringLiteral("#92400E"),
                                                   0, QStringLiteral("404"));
        table->setItem(row, 5, actionItem);
        row++;
      }
    }

    // 民意待处理 (status=0)
    if (filterType == 0 || filterType == 4)
    {
      QSqlQuery q("SELECT id, title, category, create_time FROM ge_opinion WHERE status = 0 AND del_flag = 0 ORDER BY create_time DESC");
      while (q.next())
      {
        table->insertRow(row);
        auto tag = typeTag(4);
        auto *typeItem = UiKit::createTagTableItem(tag.first, QColor(tag.second.red(), tag.second.green(), tag.second.blue(), 30), tag.second);
        table->setItem(row, 0, typeItem);
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(QStringLiteral("中")));
        table->setItem(row, 4, new QTableWidgetItem(q.value(3).toDateTime().toString("yyyy-MM-dd hh:mm")));
        auto *actionItem = UiKit::createActionItem(QStringLiteral("去处理"), QStringLiteral("#92400E"),
                                                   0, QStringLiteral("405"));
        table->setItem(row, 5, actionItem);
        row++;
      }
    }
    UiKit::syncEmptyHint(table, emptyHint);
  };

  loadTodos();
  QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
          { loadTodos(); });

  // 点击操作列跳转对应页面（目标页面存在 actionData / UserRole+2 中）
  QObject::connect(table, &QTableWidget::cellClicked, page, [page, table](int r, int c)
          {
        if (c != 5) return;
        auto* item = table->item(r, 5);
        if (item) {
            QString target = item->data(Qt::UserRole + 2).toString();
            if (!target.isEmpty()) page->requestNavigate(target);
        } });

  return page;
}
