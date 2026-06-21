#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include <QFileDialog>
#include "services/AuthService.h"
#include "dialogs/WorkOrderDetailDialog.h"
#include "ui_kit/PagedQuery.h"

namespace {

struct WorkOrderRow {
    qint64 id;
    QString orderNo;
    QString title;
    int orderType;
    int priority;
    int status;
    QString reporterName;
    QDateTime createTime;
    qint64 reporterId;
    qint64 assigneeId;
};

class WorkOrderPageController : public QObject {
    Q_OBJECT
public:
    WorkOrderPageController(BasePage* page, QVBoxLayout* layout, QTableWidget* table, DatabaseManager& db, QLabel* emptyHint, QObject* parent = nullptr)
        : QObject(parent)
        , m_page(page)
        , m_layout(layout)
        , m_table(table)
        , m_db(db)
        , m_emptyHint(emptyHint)
        , m_searchEdit(nullptr)
        , m_filterCombo(nullptr)
        , m_myTasksCheck(nullptr)
        , m_exportBtn(nullptr)
        , m_newBtn(nullptr)
        , m_pb(nullptr)
    {
        setupUi();
        setupConnections();
    }

    void loadData();
    void handleAction(qint64 id, int status);

private:
    void setupUi();
    void setupConnections();
    void onNewWorkOrderClicked();

    UiKit::PagedQuery buildQuery();
    void populateTable(const QList<WorkOrderRow>& rows);

    BasePage* m_page;
    QVBoxLayout* m_layout;
    QTableWidget* m_table;
    DatabaseManager& m_db;
    QLabel* m_emptyHint;
    QLineEdit* m_searchEdit;
    QComboBox* m_filterCombo;
    QCheckBox* m_myTasksCheck;
    QPushButton* m_exportBtn;
    QPushButton* m_newBtn;
    PaginationBar* m_pb;
};

void WorkOrderPageController::setupUi()
{
    m_table->setMouseTracking(true);

    // Page header
    m_layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_tool"), QStringLiteral("报事报修管理"), QStringLiteral("管理居民报修工单，支持受理、派单、处理、评价全流程"), UiKit::moduleColor("workorder"), m_page));
    m_layout->addSpacing(12);

    // Toolbar
    auto *toolbar = new QWidget(m_page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);

    m_searchEdit = new QLineEdit(toolbar);
    m_searchEdit->setPlaceholderText(QStringLiteral("搜索工单号/标题..."));
    m_searchEdit->setFixedWidth(220);
    tbLayout->addWidget(m_searchEdit);

    m_filterCombo = new QComboBox(toolbar);
    m_filterCombo->addItems({QStringLiteral("全部状态"), QStringLiteral("待受理"), QStringLiteral("已受理"), QStringLiteral("已派单"), QStringLiteral("处理中"), QStringLiteral("已完成"), QStringLiteral("已关闭"), QStringLiteral("已评价")});
    m_filterCombo->setFixedWidth(130);
    tbLayout->addWidget(m_filterCombo);

    m_exportBtn = new QPushButton(QStringLiteral("导出"), toolbar);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(m_exportBtn);

    m_myTasksCheck = new QCheckBox(QStringLiteral("只看我的"), toolbar);
    tbLayout->addWidget(m_myTasksCheck);

    tbLayout->addStretch();

    m_newBtn = new QPushButton(QStringLiteral("+ 新建工单"), toolbar);
    m_newBtn->setProperty("cssClass", "primary");
    m_newBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(m_newBtn);
    m_layout->addWidget(toolbar);

    // Table
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({QStringLiteral("工单号"), QStringLiteral("标题"), QStringLiteral("类型"), QStringLiteral("优先级"), QStringLiteral("状态"), QStringLiteral("报修人"), QStringLiteral("创建时间"), QStringLiteral("操作")});
    m_table->setColumnWidth(1, 180);
    m_table->setColumnWidth(7, 100);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_layout->addWidget(m_table);

    m_pb = new PaginationBar(m_page);
    m_layout->addWidget(m_pb);
}

void WorkOrderPageController::setupConnections()
{
    QObject::connect(m_table, &QTableWidget::cellEntered, this, [this](int r, int c)
            { m_table->viewport()->setCursor(c == 7 ? Qt::PointingHandCursor : Qt::ArrowCursor); });
    // 双击工单行弹出详情对话框（操作列除外）
    QObject::connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int row, int col)
            {
            // 操作列（第7列）不触发详情对话框
            if (col == 7) return;
            if (row < 0 || row >= m_table->rowCount()) return;
            auto* item = m_table->item(row, 0);
            if (!item) return;
            qint64 woId = item->data(Qt::UserRole).toLongLong();
            if (woId > 0) {
                WorkOrderDetailDialog::show(m_page, woId);
            } });
    QObject::connect(m_searchEdit, &QLineEdit::textChanged, this, [this]()
            { loadData(); });
    QObject::connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
            { loadData(); });
    QObject::connect(m_myTasksCheck, &QCheckBox::toggled, this, [this]()
            { loadData(); });
    QObject::connect(m_exportBtn, &QPushButton::clicked, this, [this]()
            { UiKit::exportTableToCsv(m_table, QStringLiteral("工单列表.csv"), m_page); });

    // New work order dialog
    QObject::connect(m_newBtn, &QPushButton::clicked, this, [this]()
            { onNewWorkOrderClicked(); });
}

void WorkOrderPageController::loadData()
{
    auto pq = buildQuery();
    auto result = pq.execute(m_pb->pageSize(), m_pb->offset());
    m_pb->setTotalCount(result.totalCount);
    QList<WorkOrderRow> rows;
    for (const auto& rec : result.rows)
    {
        WorkOrderRow row;
        row.id = rec.value(0).toLongLong();
        row.orderNo = rec.value(1).toString();
        row.title = rec.value(2).toString();
        row.orderType = rec.value(3).toInt();
        row.priority = rec.value(4).toInt();
        row.status = rec.value(5).toInt();
        row.reporterName = rec.value(6).toString();
        row.createTime = rec.value(7).toDateTime();
        row.reporterId = rec.value(8).toLongLong();
        row.assigneeId = rec.value(9).toLongLong();
        rows.append(row);
    }
    populateTable(rows);
    UiKit::syncEmptyHint(m_table, m_emptyHint);
    m_pb->refreshData();
}

UiKit::PagedQuery WorkOrderPageController::buildQuery()
{
    UiKit::PagedQuery pq("SELECT id, order_no, title, order_type, priority, status, reporter_name, create_time, reporter_id, assign_to FROM wo_work_order WHERE del_flag = 0");
    // 注入数据权限过滤（居民只看本人，物业/社区按组织范围）
    auto scopeFilter = UiKit::buildDataScopeFilter("", "estate_id");
    if (!scopeFilter.first.isEmpty())
    {
        QString scopeSql = scopeFilter.first.trimmed();
        if (scopeSql.startsWith("AND", Qt::CaseInsensitive))
            scopeSql = scopeSql.mid(3).trimmed();
        QVariantMap scopeBinds;
        for (int i = 0; i + 1 < scopeFilter.second.size(); i += 2)
        {
            QString key = scopeFilter.second[i].toString();
            if (key.startsWith(":")) key = key.mid(1);
            scopeBinds.insert(key, scopeFilter.second[i + 1]);
        }
        pq.where(scopeSql, scopeBinds);
    }

    QString searchText = m_searchEdit->text().trimmed();
    int filterIdx = m_filterCombo->currentIndex();
    bool onlyMine = m_myTasksCheck->isChecked();
    if (!searchText.isEmpty())
    {
        pq.where("(order_no LIKE :search OR title LIKE :search)", {{"search", "%" + searchText + "%"}});
    }
    if (filterIdx > 0)
    {
        pq.where("status = :status", {{"status", WorkOrderStatus::statusFilterMap()[filterIdx]}});
    }
    if (onlyMine)
    {
        pq.where("assign_to = :myId", {{"myId", AuthService::instance().currentUser().id}});
    }
    pq.orderBy("create_time DESC");
    return pq;
}

void WorkOrderPageController::populateTable(const QList<WorkOrderRow>& rows)
{
    m_table->setRowCount(0);
    int row = 0;
    for (const auto& r : rows)
    {
        m_table->insertRow(row);
        auto *orderNoItem = new QTableWidgetItem(r.orderNo);
        // 将工单 ID 存储到 item 的 UserRole 中，供双击详情使用
        orderNoItem->setData(Qt::UserRole, r.id);
        m_table->setItem(row, 0, orderNoItem);
        m_table->setItem(row, 1, new QTableWidgetItem(r.title));
        m_table->setItem(row, 2, new QTableWidgetItem(WorkOrderType::label(r.orderType)));
        QColor priColor(WorkOrderPriority::color(r.priority));
        auto *priItem = UiKit::createTagTableItem(WorkOrderPriority::label(r.priority), QColor(priColor.red(), priColor.green(), priColor.blue(), 30), priColor);
        m_table->setItem(row, 3, priItem);
        QColor woColor(WorkOrderStatus::color(r.status));
        auto *statusItem = UiKit::createTagTableItem(WorkOrderStatus::label(r.status), QColor(woColor.red(), woColor.green(), woColor.blue(), 30), woColor);
        m_table->setItem(row, 4, statusItem);
        m_table->setItem(row, 5, new QTableWidgetItem(r.reporterName));
        m_table->setItem(row, 6, new QTableWidgetItem(r.createTime.toString("yyyy-MM-dd hh:mm")));
        // 操作列：根据角色权限渲染为真实按钮
        QList<QPushButton*> actionBtns;
        if (UiKit::canOperateWorkOrder(r.status, r.reporterId, r.assigneeId))
        {
            QString actionText;
            QString actionColor;
            if (r.status == 0)
            {
                actionText = QStringLiteral("受理");
                actionColor = "#b45309";
            }
            else if (r.status == 1)
            {
                actionText = QStringLiteral("派单");
                actionColor = "#b45309";
            }
            else if (r.status == 2)
            {
                actionText = QStringLiteral("开始");
                actionColor = "#a16207";
            }
            else if (r.status == 3)
            {
                actionText = QStringLiteral("完成");
                actionColor = "#15803d";
            }
            else if (r.status == 4)
            {
                actionText = QStringLiteral("评价");
                actionColor = "#15803d";
            }
            if (!actionText.isEmpty())
            {
                auto *actionBtn = new QPushButton(actionText);
                actionBtn->setStyleSheet(QString("QPushButton{border:none; color:%1; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;} QPushButton:hover{text-decoration:underline;}")
                                             .arg(actionColor));
                actionBtn->setCursor(Qt::PointingHandCursor);
                connect(actionBtn, &QPushButton::clicked, this, [this, id = r.id, sts = r.status]() {
                    handleAction(id, sts);
                });
                actionBtns.append(actionBtn);
            }
        }
        if (actionBtns.isEmpty())
        {
            m_table->setItem(row, 7, UiKit::createActionItem(QStringLiteral("--"), "#64748b", r.id, r.status));
        }
        else
        {
            m_table->setCellWidget(row, 7, UiKit::createActionCell(actionBtns, m_table));
        }
        row++;
    }
}

void WorkOrderPageController::handleAction(qint64 id, int sts)
{
    const auto &user = AuthService::instance().currentUser();

    // 权限校验：查询工单的 reporter_id 和 assign_to，校验当前用户是否有权操作
    qint64 reporterId = 0;
    qint64 assigneeId = 0;
    {
      QSqlQuery permQ(DatabaseManager::instance().database());
      permQ.prepare("SELECT reporter_id, assign_to FROM wo_work_order WHERE id = :id AND del_flag = 0");
      permQ.bindValue(":id", id);
      if (permQ.exec() && permQ.next())
      {
        reporterId = permQ.value(0).toLongLong();
        assigneeId = permQ.value(1).toLongLong();
      }
    }
    if (!UiKit::canOperateWorkOrder(sts, reporterId, assigneeId))
    {
      QMessageBox::warning(m_page, QStringLiteral("无权限"),
                           QStringLiteral("您没有权限执行此操作，请联系相关负责人。"));
      return;
    }

    if (sts == 0)
    {
      // 受理
      auto retAccept = QMessageBox::question(m_page, QStringLiteral("确认操作"),
                                             QStringLiteral("确认受理此工单？"),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (retAccept != QMessageBox::Yes)
        return;
      DatabaseManager::instance().update("wo_work_order", id, {{"status", 1}, {"accept_time", QDateTime::currentDateTime()}, {"accept_by", user.id}});
      UiKit::showToast(QStringLiteral("工单已受理"), m_page);
      loadData();
    }
    else if (sts == 1)
    {
      // 派单 - 弹出选择维修人员对话框
      QDialog dlg(m_page);
      dlg.setWindowTitle(QStringLiteral("派单"));
      dlg.setMinimumWidth(400);
      auto *dlgLayout = new QVBoxLayout(&dlg);
      dlgLayout->setContentsMargins(24, 20, 24, 20);

      auto *titleLabel = new QLabel(QStringLiteral("选择维修人员"), &dlg);
      titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
      dlgLayout->addWidget(titleLabel);
      dlgLayout->addSpacing(12);

      auto *combo = new QComboBox(&dlg);
      // 查询维修人员角色用户（使用参数化绑定）
      QSqlQuery workerQ;
      workerQ.prepare(QStringLiteral("SELECT u.id, u.nickname, u.username FROM sys_user u "
                                     "JOIN sys_user_role ur ON u.id = ur.user_id "
                                     "JOIN sys_role r ON ur.role_id = r.id "
                                     "WHERE r.role_key = :rk AND u.status = 0 AND u.del_flag = 0"));
      workerQ.bindValue(":rk", QStringLiteral("property_repair"));
      workerQ.exec();
      while (workerQ.next())
      {
        QString name = workerQ.value(1).toString();
        if (name.isEmpty())
          name = workerQ.value(2).toString();
        combo->addItem(name, workerQ.value(0).toInt());
      }
      // 如果没有维修人员，查询所有物业相关角色
      if (combo->count() == 0)
      {
        QSqlQuery workerQ2;
        workerQ2.prepare(QStringLiteral("SELECT u.id, u.nickname, u.username FROM sys_user u "
                                         "JOIN sys_user_role ur ON u.id = ur.user_id "
                                         "JOIN sys_role r ON ur.role_id = r.id "
                                         "WHERE r.role_key IN ('property_cs', 'property_steward', 'property_manager') "
                                         "AND u.status = 0 AND u.del_flag = 0"));
        workerQ2.exec();
        while (workerQ2.next())
        {
          QString name = workerQ2.value(1).toString();
          if (name.isEmpty())
            name = workerQ2.value(2).toString();
          combo->addItem(name, workerQ2.value(0).toInt());
        }
      }
      dlgLayout->addWidget(combo);
      dlgLayout->addSpacing(16);

      auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
      buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认派单"));
      buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
      QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
      QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
              {
                  if (combo->count() == 0) {
                      QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("没有可选的维修人员"));
                      return;
                  }
                  int workerId = combo->currentData().toInt();
                  QString workerName = combo->currentText();
                  auto retAssign = QMessageBox::question(m_page, QStringLiteral("确认操作"),
                      QStringLiteral("确认将工单派单给 %1？").arg(workerName),
                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                  if (retAssign != QMessageBox::Yes) return;
                  DatabaseManager::instance().update("wo_work_order", id, {
                      {"status", 2}, {"assign_to", workerId}, {"assign_time", QDateTime::currentDateTime()}
                  });
                  // 发送通知给维修人员
                  m_page->requestNotification(workerId, QStringLiteral("新工单已派单"), QStringLiteral("您有一个新的维修工单待处理"), 2, "work_order", (int)id);
                  UiKit::showToast(QStringLiteral("派单成功"), m_page);
                  dlg.accept();
                  loadData(); });
      dlgLayout->addWidget(buttons);
      dlg.exec();
    }
    else if (sts == 2)
    {
      // 开始处理
      auto retStart = QMessageBox::question(m_page, QStringLiteral("确认操作"),
                                            QStringLiteral("确认开始处理此工单？"),
                                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (retStart != QMessageBox::Yes)
        return;
      DatabaseManager::instance().update("wo_work_order", id, {{"status", 3}});
      UiKit::showToast(QStringLiteral("已开始处理"), m_page);
      loadData();
    }
    else if (sts == 3)
    {
      // 完成 - 弹出填写处理结果对话框
      QDialog dlg(m_page);
      dlg.setWindowTitle(QStringLiteral("填写处理结果"));
      dlg.setMinimumWidth(450);
      auto *dlgLayout = new QVBoxLayout(&dlg);
      dlgLayout->setContentsMargins(24, 20, 24, 20);

      auto *titleLabel = new QLabel(QStringLiteral("处理结果"), &dlg);
      titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
      dlgLayout->addWidget(titleLabel);
      dlgLayout->addSpacing(12);

      auto *form = new QFormLayout(&dlg);
      form->setSpacing(12);
      auto *resultEdit = new QTextEdit(&dlg);
      resultEdit->setPlaceholderText(QStringLiteral("请描述维修处理结果..."));
      resultEdit->setFixedHeight(80);
      form->addRow(QStringLiteral("处理结果:"), resultEdit);
      dlgLayout->addLayout(form);
      dlgLayout->addSpacing(16);

      auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
      buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认完成"));
      buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
      QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
      QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
              {
                  auto retFinish = QMessageBox::question(m_page, QStringLiteral("确认操作"),
                      QStringLiteral("确认完成此工单？"),
                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                  if (retFinish != QMessageBox::Yes) return;
                  DatabaseManager::instance().update("wo_work_order", id, {
                      {"status", 4}, {"finish_time", QDateTime::currentDateTime()},
                      {"result_desc", resultEdit->toPlainText().trimmed()}
                  });
                  // 查询报修人ID并发送通知
                  QSqlQuery woQ(DatabaseManager::instance().database());
                  woQ.prepare("SELECT reporter_id, title FROM wo_work_order WHERE id = :id");
                  woQ.bindValue(":id", id);
                  woQ.exec();
                  if (woQ.next()) {
                      int reporterId = woQ.value(0).toInt();
                      QString woTitle = woQ.value(1).toString();
                      m_page->requestNotification(reporterId, QStringLiteral("工单已完成"), QStringLiteral("您的报修「%1」已处理完成，请评价").arg(woTitle), 2, "work_order", (int)id);
                  }
                  UiKit::showToast(QStringLiteral("工单已完成"), m_page);
                  dlg.accept();
                  loadData(); });
      dlgLayout->addWidget(buttons);
      dlg.exec();
    }
    else if (sts == 4)
    {
      // 评价 - 弹出评价对话框
      QDialog dlg(m_page);
      dlg.setWindowTitle(QStringLiteral("评价工单"));
      dlg.setMinimumWidth(450);
      auto *dlgLayout = new QVBoxLayout(&dlg);
      dlgLayout->setContentsMargins(24, 20, 24, 20);

      auto *titleLabel = new QLabel(QStringLiteral("服务评价"), &dlg);
      titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
      dlgLayout->addWidget(titleLabel);
      dlgLayout->addSpacing(12);

      auto *form = new QFormLayout(&dlg);
      form->setSpacing(12);

      auto *ratingCombo = new QComboBox(&dlg);
      ratingCombo->addItems({QStringLiteral("1星 - 很差"), QStringLiteral("2星 - 一般"), QStringLiteral("3星 - 还行"), QStringLiteral("4星 - 满意"), QStringLiteral("5星 - 非常满意")});
      ratingCombo->setCurrentIndex(4);
      form->addRow(QStringLiteral("评分:"), ratingCombo);

      auto *contentEdit = new QTextEdit(&dlg);
      contentEdit->setPlaceholderText(QStringLiteral("请输入评价内容..."));
      contentEdit->setFixedHeight(80);
      form->addRow(QStringLiteral("评价:"), contentEdit);
      dlgLayout->addLayout(form);
      dlgLayout->addSpacing(16);

      auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
      buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交评价"));
      buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
      QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
      QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]()
              {
                  int rating = ratingCombo->currentIndex() + 1;
                  auto retEval = QMessageBox::question(m_page, QStringLiteral("确认操作"),
                      QStringLiteral("确认提交评价？"),
                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                  if (retEval != QMessageBox::Yes) return;
                  DatabaseManager::instance().insert("ev_evaluation", {
                      {"biz_type", "work_order"}, {"biz_id", (int)id},
                      {"evaluator_id", user.id}, {"rating", rating},
                      {"content", contentEdit->toPlainText().trimmed()},
                      {"eval_time", QDateTime::currentDateTime()},
                      {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
                  });
                  DatabaseManager::instance().update("wo_work_order", id, {{"status", 6}});
                  UiKit::showToast(QStringLiteral("评价提交成功"), m_page);
                  dlg.accept();
                  loadData(); });
      dlgLayout->addWidget(buttons);
      dlg.exec();
    }
}

void WorkOrderPageController::onNewWorkOrderClicked()
{
    QDialog dlg(m_page);
    dlg.setWindowTitle(QStringLiteral("新建报修工单"));
    dlg.setMinimumWidth(500);
    auto* dlgLayout = new QVBoxLayout(&dlg);
    dlgLayout->setContentsMargins(24, 20, 24, 20);

    auto* formTitle = new QLabel(QStringLiteral("填写报修信息"), &dlg);
    formTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #141413;");
    dlgLayout->addWidget(formTitle);
    dlgLayout->addSpacing(8);

    auto* form = new QFormLayout(&dlg);
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);

    auto* titleEdit = new QLineEdit(&dlg);
    titleEdit->setPlaceholderText(QStringLiteral("请简要描述问题"));
    form->addRow(QStringLiteral("标题:"), titleEdit);

    auto* typeCombo = new QComboBox(&dlg);
    typeCombo->addItems({QStringLiteral("水电维修"), QStringLiteral("公共设施"), QStringLiteral("环境卫生"), QStringLiteral("安全秩序"), QStringLiteral("其他")});
    form->addRow(QStringLiteral("类型:"), typeCombo);

    auto* prioCombo = new QComboBox(&dlg);
    prioCombo->addItems({QStringLiteral("普通"), QStringLiteral("紧急"), QStringLiteral("特急")});
    form->addRow(QStringLiteral("优先级:"), prioCombo);

    auto* descEdit = new QTextEdit(&dlg);
    descEdit->setPlaceholderText(QStringLiteral("详细描述报修问题..."));
    descEdit->setFixedHeight(80);
    form->addRow(QStringLiteral("描述:"), descEdit);

    auto* locEdit = new QLineEdit(&dlg);
    locEdit->setPlaceholderText(QStringLiteral("如: 3号楼2单元501"));
    form->addRow(QStringLiteral("位置:"), locEdit);

    dlgLayout->addLayout(form);
    dlgLayout->addSpacing(8);

    // 图片上传区域
    QStringList selectedImages;
    auto* imgLabel = new QLabel(QStringLiteral("附件图片（可选，最多5张）"), &dlg);
    imgLabel->setStyleSheet("font-size: 13px; color: #64748b; background: transparent;");
    dlgLayout->addWidget(imgLabel);
    auto* imgListWidget = new QWidget(&dlg);
    auto* imgListLayout = new QVBoxLayout(imgListWidget);
    imgListLayout->setContentsMargins(0, 0, 0, 0);
    imgListLayout->setSpacing(4);
    dlgLayout->addWidget(imgListWidget);

    std::function<void()> refreshImageList;
    refreshImageList = [&]() {
        QLayoutItem* child;
        while ((child = imgListLayout->takeAt(0)) != nullptr) {
            if (child->widget()) child->widget()->deleteLater();
            delete child;
        }
        for (int i = 0; i < selectedImages.size(); ++i) {
            auto* row = new QWidget(imgListWidget);
            auto* rowLay = new QHBoxLayout(row);
            rowLay->setContentsMargins(0, 0, 0, 0);
            auto* nameLabel = new QLabel(QFileInfo(selectedImages[i]).fileName(), row);
            nameLabel->setStyleSheet("font-size: 12px; color: #141413; background: transparent;");
            rowLay->addWidget(nameLabel);
            rowLay->addStretch();
            auto* rmBtn = new QPushButton(QStringLiteral("×"), row);
            rmBtn->setFixedSize(20, 20);
            rmBtn->setStyleSheet("QPushButton{border:none; color:#b91c1c; font-size:14px; font-weight:bold; background:transparent;} QPushButton:hover{background:#fee2e2; border-radius:3px;}");
            rmBtn->setCursor(Qt::PointingHandCursor);
            int idx = i;
            QObject::connect(rmBtn, &QPushButton::clicked, [&, idx]() {
                selectedImages.removeAt(idx);
                refreshImageList();
            });
            rowLay->addWidget(rmBtn);
            imgListLayout->addWidget(row);
        }
    };

    auto* addImgBtn = new QPushButton(QStringLiteral("+ 添加图片"), &dlg);
    addImgBtn->setCursor(Qt::PointingHandCursor);
    addImgBtn->setStyleSheet("QPushButton{border:1px dashed #cbd5e1; border-radius:4px; padding:6px 16px; color:#64748b; background:#f8fafc; font-size:13px;} QPushButton:hover{border-color:#b45309; color:#b45309;}");
    QObject::connect(addImgBtn, &QPushButton::clicked, [&]() {
        if (selectedImages.size() >= 5) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("最多上传5张图片"));
            return;
        }
        QStringList files = QFileDialog::getOpenFileNames(&dlg, QStringLiteral("选择图片"), QString(),
            QStringLiteral("图片文件 (*.jpg *.jpeg *.png *.bmp *.gif);;所有文件 (*)"));
        for (const QString& f : files) {
            if (selectedImages.size() >= 5) break;
            if (!selectedImages.contains(f)) selectedImages.append(f);
        }
        refreshImageList();
    });
    dlgLayout->addWidget(addImgBtn);

    dlgLayout->addSpacing(12);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("提交"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
        if (titleEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, QStringLiteral("提示"), QStringLiteral("请填写标题"));
            return;
        }
        auto& d = DatabaseManager::instance();
        QString orderNo = Utils::generateOrderNo();
        const auto& user = AuthService::instance().currentUser();
        d.insert("wo_work_order", {
            {"order_no", orderNo}, {"title", titleEdit->text().trimmed()},
            {"order_type", typeCombo->currentIndex() + 1},
            {"priority", prioCombo->currentIndex() + 1},
            {"description", descEdit->toPlainText().trimmed()},
            {"location_desc", locEdit->text().trimmed()},
            {"images", selectedImages.isEmpty() ? QVariant() : selectedImages.join(",")},
            {"reporter_id", user.id}, {"reporter_name", user.nickname.isEmpty() ? user.username : user.nickname},
            {"reporter_phone", user.phone},
            {"status", 0}, {"source", 0},
            {"create_by", user.id}, {"create_time", QDateTime::currentDateTime()}
        });
        UiKit::showToast(QStringLiteral("工单提交成功"), m_page);
        dlg.accept();
        loadData();
    });
    dlgLayout->addWidget(buttons);
    dlg.exec();
}

} // namespace

void PageFactory::buildPropertyWorkorder(BasePage *page, QVBoxLayout *layout, QTableWidget *table, DatabaseManager &db, QLabel *emptyHint)
{
    // 避免页面缓存复用时重复创建 Controller 导致对象堆积
    if (page->findChild<WorkOrderPageController *>(QString(), Qt::FindDirectChildrenOnly))
        return;
    auto *ctrl = new WorkOrderPageController(page, layout, table, db, emptyHint, page);
    ctrl->loadData();
}

#include "buildPropertyWorkorder.moc"
