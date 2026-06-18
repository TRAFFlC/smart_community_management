#include "pages/PageFactory.h"
#include "PagesCommon.h"

// ========== System Pages ==========
BasePage *PageFactory::createSystemPage(const QString &sub)
{
  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  auto &db = DatabaseManager::instance();

  // 空状态提示（根据 sub 定制文案）
  QString sysEmptyText;
  if (sub == "user")
    sysEmptyText = QStringLiteral("暂无用户记录");
  else if (sub == "role")
    sysEmptyText = QStringLiteral("暂无角色记录");
  else if (sub == "dict")
    sysEmptyText = QStringLiteral("暂无字典记录");
  else if (sub == "log")
    sysEmptyText = QStringLiteral("暂无日志记录");
  else
    sysEmptyText = QStringLiteral("暂无数据");
  auto *emptyHint = UiKit::createEmptyHintLabel(sysEmptyText, page);

  // ========== 通用对话框辅助函数 ==========
  // 创建标准表单对话框（固定宽度 480px，带标题和 QFormLayout）
  auto createFormDialog = [page](const QString &title, QFormLayout *&outForm, QDialog *&outDlg) -> QVBoxLayout *
  {
    outDlg = new QDialog(page);
    outDlg->setWindowTitle(title);
    outDlg->setFixedWidth(480);
    auto *dlgLayout = new QVBoxLayout(outDlg);
    dlgLayout->setContentsMargins(24, 20, 24, 20);
    dlgLayout->setSpacing(12);

    auto *titleLabel = new QLabel(title, outDlg);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
    dlgLayout->addWidget(titleLabel);
    dlgLayout->addSpacing(4);

    outForm = new QFormLayout(outDlg);
    outForm->setSpacing(12);
    outForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    dlgLayout->addLayout(outForm);
    return dlgLayout;
  };

  // 为表单输入框设置统一样式（固定高度 36px）
  auto styleInput = [](QWidget *w)
  {
    w->setStyleSheet("QLineEdit, QComboBox, QSpinBox, QTextEdit {"
                     "  min-height: 36px; padding: 4px 8px; border: 1px solid #e2e8f0;"
                     "  border-radius: 4px; font-size: 13px; background: #ffffff;"
                     "}"
                     "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QTextEdit:focus {"
                     "  border-color: #b45309;"
                     "}");
  };

  // 添加底部按钮区（取消 + 确定），右对齐
  auto addDialogButtons = [](QVBoxLayout *dlgLayout, QDialog *dlg, const QString &okText,
                             std::function<bool()> onOk)
  {
    dlgLayout->addSpacing(8);
    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    auto *cancelBtn = new QPushButton(QStringLiteral("取消"), dlg);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    auto *okBtn = new QPushButton(okText, dlg);
    okBtn->setProperty("cssClass", "primary");
    okBtn->setCursor(Qt::PointingHandCursor);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);
    dlgLayout->addLayout(btnRow);
    QObject::connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    QObject::connect(okBtn, &QPushButton::clicked, dlg, [dlg, onOk]()
                     {
            if (onOk()) dlg->accept(); });
  };

  // 主按钮样式（工具栏用）
  auto primaryBtnStyle = []()
  {
    return QStringLiteral(
        "QPushButton { background: #b45309; color: #fff; border: none; border-radius: 4px;"
        " padding: 7px 16px; font-size: 14px; min-height: 36px; }"
        "QPushButton:hover { background: #d97706; }"
        "QPushButton:pressed { background: #92400e; }");
  };

  // ========================================================================
  // Task 11: 用户管理 CRUD
  // ========================================================================
  if (sub == "user")
  {
    auto *table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(UiKit::TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索用户名/姓名..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *statusCombo = new QComboBox(toolbar);
    statusCombo->addItem(QStringLiteral("全部状态"), -1);
    statusCombo->addItem(QStringLiteral("正常"), 0);
    statusCombo->addItem(QStringLiteral("禁用"), 1);
    statusCombo->setMinimumWidth(120);
    tbLayout->addWidget(statusCombo);
    tbLayout->addStretch();
    auto *addUserBtn = new QPushButton(QStringLiteral("+ 新增用户"), toolbar);
    addUserBtn->setStyleSheet(primaryBtnStyle());
    addUserBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(addUserBtn);
    layout->addWidget(toolbar);

    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({QStringLiteral("用户名"), QStringLiteral("真实姓名"),
                                      QStringLiteral("手机号"), QStringLiteral("类型"), QStringLiteral("状态"),
                                      QStringLiteral("角色"), QStringLiteral("操作")});
    table->setColumnWidth(6, 300);

    // 用户新增/编辑对话框
    auto openUserDialog = [page, &db, &styleInput, &createFormDialog, &addDialogButtons](
                              qint64 userId, std::function<void()> reload)
    {
      bool isEdit = (userId > 0);
      QFormLayout *form = nullptr;
      QDialog *dlg = nullptr;
      auto *dlgLayout = createFormDialog(isEdit ? QStringLiteral("编辑用户") : QStringLiteral("新增用户"), form, dlg);

      auto *usernameEdit = new QLineEdit(dlg);
      auto *realNameEdit = new QLineEdit(dlg);
      auto *phoneEdit = new QLineEdit(dlg);
      auto *userTypeCombo = new QComboBox(dlg);
      userTypeCombo->addItem(UserType::label(UserType::Admin), UserType::Admin);
      userTypeCombo->addItem(UserType::label(UserType::Staff), UserType::Staff);
      userTypeCombo->addItem(UserType::label(UserType::ServiceProvider), UserType::ServiceProvider);
      userTypeCombo->addItem(UserType::label(UserType::Resident), UserType::Resident);
      auto *passwordEdit = new QLineEdit(dlg);
      passwordEdit->setEchoMode(QLineEdit::Password);
      auto *roleCombo = new QComboBox(dlg);
      QSqlQuery rq(db.database());
      rq.exec("SELECT id, role_name FROM sys_role WHERE del_flag = 0 AND status = 0 ORDER BY sort_order");
      while (rq.next())
      {
        roleCombo->addItem(rq.value(1).toString(), rq.value(0).toLongLong());
      }
      auto *statusCombo2 = new QComboBox(dlg);
      statusCombo2->addItem(UserStatus::label(UserStatus::Normal), UserStatus::Normal);
      statusCombo2->addItem(UserStatus::label(UserStatus::Disabled), UserStatus::Disabled);

      styleInput(usernameEdit);
      styleInput(realNameEdit);
      styleInput(phoneEdit);
      styleInput(userTypeCombo);
      styleInput(passwordEdit);
      styleInput(roleCombo);
      styleInput(statusCombo2);

      int oldRoleId = 0;
      if (isEdit)
      {
        QSqlQuery uq(db.database());
        uq.prepare("SELECT username, real_name, phone, user_type, status FROM sys_user WHERE id = :id");
        uq.bindValue(":id", userId);
        uq.exec();
        if (uq.next())
        {
          usernameEdit->setText(uq.value(0).toString());
          realNameEdit->setText(uq.value(1).toString());
          phoneEdit->setText(uq.value(2).toString());
          int ut = uq.value(3).toInt();
          for (int i = 0; i < userTypeCombo->count(); ++i)
          {
            if (userTypeCombo->itemData(i).toInt() == ut)
            {
              userTypeCombo->setCurrentIndex(i);
              break;
            }
          }
          int st = uq.value(4).toInt();
          for (int i = 0; i < statusCombo2->count(); ++i)
          {
            if (statusCombo2->itemData(i).toInt() == st)
            {
              statusCombo2->setCurrentIndex(i);
              break;
            }
          }
        }
        QSqlQuery urq(db.database());
        urq.prepare("SELECT role_id FROM sys_user_role WHERE user_id = :uid LIMIT 1");
        urq.bindValue(":uid", userId);
        urq.exec();
        if (urq.next())
        {
          oldRoleId = urq.value(0).toInt();
          for (int i = 0; i < roleCombo->count(); ++i)
          {
            if (roleCombo->itemData(i).toInt() == oldRoleId)
            {
              roleCombo->setCurrentIndex(i);
              break;
            }
          }
        }
        passwordEdit->setPlaceholderText(QStringLiteral("留空表示不修改密码"));
      }
      else
      {
        passwordEdit->setPlaceholderText(QStringLiteral("至少6位"));
      }

      form->addRow(QStringLiteral("用户名 *"), usernameEdit);
      form->addRow(QStringLiteral("姓名 *"), realNameEdit);
      form->addRow(QStringLiteral("手机号 *"), phoneEdit);
      form->addRow(QStringLiteral("用户类型"), userTypeCombo);
      form->addRow(isEdit ? QStringLiteral("新密码") : QStringLiteral("密码 *"), passwordEdit);
      form->addRow(QStringLiteral("角色"), roleCombo);
      form->addRow(QStringLiteral("状态"), statusCombo2);

      addDialogButtons(dlgLayout, dlg, QStringLiteral("确定"), [=, &db]() -> bool
                       {
                QString username = usernameEdit->text().trimmed();
                QString realName = realNameEdit->text().trimmed();
                QString phone = phoneEdit->text().trimmed();
                QString password = passwordEdit->text();
                int userType = userTypeCombo->currentData().toInt();
                int status = statusCombo2->currentData().toInt();
                qint64 roleId = roleCombo->currentData().toLongLong();

                if (username.isEmpty()) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请输入用户名")); return false; }
                if (realName.isEmpty()) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请输入姓名")); return false; }
                if (phone.isEmpty() || phone.length() != 11) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请输入11位手机号")); return false; }
                if (!isEdit && password.length() < 6) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("密码至少6位")); return false; }

                // 用户名唯一性校验
                QSqlQuery cntQ(db.database());
                cntQ.prepare("SELECT COUNT(*) FROM sys_user WHERE username = :u AND del_flag = 0 AND id != :id");
                cntQ.bindValue(":u", username);
                cntQ.bindValue(":id", userId);
                cntQ.exec();
                if (cntQ.next() && cntQ.value(0).toInt() > 0) {
                    QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("用户名已存在"));
                    return false;
                }

                qint64 currentUserId = AuthService::instance().currentUser().id;
                QString currentUser = AuthService::instance().currentUser().username;
                if (!isEdit) {
                    QVariantMap data{
                        {"username", username},
                        {"password", Utils::hashPassword(password)},
                        {"real_name", realName},
                        {"nickname", realName},
                        {"phone", phone},
                        {"user_type", userType},
                        {"status", status},
                        {"create_by", currentUserId},
                        {"create_time", QDateTime::currentDateTime()},
                        {"update_time", QDateTime::currentDateTime()}
                    };
                    qint64 newId = db.insert("sys_user", data);
                    if (newId <= 0) { QMessageBox::warning(dlg, QStringLiteral("错误"), QStringLiteral("创建用户失败")); return false; }
                    if (roleId > 0) {
                        db.insert("sys_user_role", {{"user_id", newId}, {"role_id", roleId}});
                    }
                    db.insert("sys_operation_log", {
                        {"user_id", currentUserId}, {"username", currentUser},
                        {"module", QStringLiteral("用户管理")},
                        {"operation", QStringLiteral("新增用户: %1").arg(username)},
                        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("用户创建成功"), page);
                } else {
                    QVariantMap data{
                        {"username", username}, {"real_name", realName}, {"nickname", realName},
                        {"phone", phone}, {"user_type", userType}, {"status", status},
                        {"update_by", currentUserId}, {"update_time", QDateTime::currentDateTime()}
                    };
                    if (!password.isEmpty()) data["password"] = Utils::hashPassword(password);
                    db.update("sys_user", userId, data);
                    if (roleId > 0 && roleId != oldRoleId) {
                        QSqlQuery delQ(db.database());
                        delQ.prepare("DELETE FROM sys_user_role WHERE user_id = :uid");
                        delQ.bindValue(":uid", userId);
                        delQ.exec();
                        db.insert("sys_user_role", {{"user_id", userId}, {"role_id", roleId}});
                    }
                    db.insert("sys_operation_log", {
                        {"user_id", currentUserId}, {"username", currentUser},
                        {"module", QStringLiteral("用户管理")},
                        {"operation", QStringLiteral("编辑用户: %1").arg(username)},
                        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("用户更新成功"), page);
                }
                reload();
                return true; });
      dlg->exec();
      delete dlg;
    };

    // 重置密码对话框
    auto resetPassword = [page, &db, &styleInput, &createFormDialog, &addDialogButtons](
                             qint64 userId, const QString &username, std::function<void()> reload)
    {
      QFormLayout *form = nullptr;
      QDialog *dlg = nullptr;
      auto *dlgLayout = createFormDialog(QStringLiteral("重置密码 - %1").arg(username), form, dlg);
      auto *pwdEdit = new QLineEdit(dlg);
      pwdEdit->setEchoMode(QLineEdit::Password);
      pwdEdit->setPlaceholderText(QStringLiteral("请输入新密码（至少6位）"));
      auto *pwdConfirmEdit = new QLineEdit(dlg);
      pwdConfirmEdit->setEchoMode(QLineEdit::Password);
      pwdConfirmEdit->setPlaceholderText(QStringLiteral("请再次输入新密码"));
      styleInput(pwdEdit);
      styleInput(pwdConfirmEdit);
      form->addRow(QStringLiteral("新密码 *"), pwdEdit);
      form->addRow(QStringLiteral("确认密码 *"), pwdConfirmEdit);

      addDialogButtons(dlgLayout, dlg, QStringLiteral("确认重置"), [=, &db]() -> bool
                       {
                QString p1 = pwdEdit->text();
                QString p2 = pwdConfirmEdit->text();
                if (p1.length() < 6) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("密码至少6位")); return false; }
                if (p1 != p2) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("两次密码不一致")); return false; }
                auto ret = QMessageBox::question(dlg, QStringLiteral("确认操作"),
                    QStringLiteral("确认重置用户 %1 的密码？").arg(username),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (ret != QMessageBox::Yes) return false;
                qint64 currentUserId = AuthService::instance().currentUser().id;
                db.update("sys_user", userId, {
                    {"password", Utils::hashPassword(p1)},
                    {"update_by", currentUserId},
                    {"update_time", QDateTime::currentDateTime()}
                });
                db.insert("sys_operation_log", {
                    {"user_id", currentUserId},
                    {"username", AuthService::instance().currentUser().username},
                    {"module", QStringLiteral("用户管理")},
                    {"operation", QStringLiteral("重置用户密码: %1").arg(username)},
                    {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                });
                UiKit::showToast(QStringLiteral("密码重置成功"), page);
                reload();
                return true; });
      dlg->exec();
      delete dlg;
    };

    // 禁用/启用
    auto toggleUserStatus = [page, &db](qint64 userId, int currentStatus, const QString &username,
                                        std::function<void()> reload)
    {
      bool toDisable = (currentStatus == UserStatus::Normal);
      QString action = toDisable ? QStringLiteral("禁用") : QStringLiteral("启用");
      auto ret = QMessageBox::question(page, QStringLiteral("确认操作"),
                                       QStringLiteral("确认%1用户 %2 ？").arg(action).arg(username),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (ret != QMessageBox::Yes)
        return;
      qint64 currentUserId = AuthService::instance().currentUser().id;
      db.update("sys_user", userId, {{"status", toDisable ? UserStatus::Disabled : UserStatus::Normal}, {"update_by", currentUserId}, {"update_time", QDateTime::currentDateTime()}});
      db.insert("sys_operation_log", {{"user_id", currentUserId},
                                      {"username", AuthService::instance().currentUser().username},
                                      {"module", QStringLiteral("用户管理")},
                                      {"operation", QStringLiteral("%1用户: %2").arg(action).arg(username)},
                                      {"status", 0},
                                      {"operation_time", QDateTime::currentDateTime()}});
      UiKit::showToast(QStringLiteral("已%1用户").arg(action), page);
      reload();
    };

    std::function<void()> loadUsers;
    loadUsers = [table, searchEdit, statusCombo, emptyHint, &openUserDialog, &resetPassword, &toggleUserStatus, &loadUsers]()
    {
      table->setSortingEnabled(false);
      table->setRowCount(0);
      QString sql = "SELECT id, username, real_name, phone, user_type, status FROM sys_user WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      int statusFilter = statusCombo->currentData().toInt();
      if (!searchText.isEmpty())
        sql += " AND (username LIKE :search OR real_name LIKE :search)";
      if (statusFilter >= 0)
        sql += " AND status = :status";
      sql += " ORDER BY id";
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (statusFilter >= 0)
        q.bindValue(":status", statusFilter);
      q.exec();
      int row = 0;
      while (q.next())
      {
        qint64 userId = q.value(0).toLongLong();
        QString username = q.value(1).toString();
        int userSts = q.value(5).toInt();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(username));
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(Utils::maskPhone(q.value(3).toString())));
        table->setItem(row, 3, new QTableWidgetItem(UserType::label(q.value(4).toInt())));
        auto *statusItem = userSts == UserStatus::Normal
                               ? UiKit::createTagTableItem(UserStatus::label(userSts), QColor("#f0fdf4"), QColor("#15803d"))
                               : UiKit::createTagTableItem(UserStatus::label(userSts), QColor("#fef2f2"), QColor("#b91c1c"));
        table->setItem(row, 4, statusItem);
        QSqlQuery rq(DatabaseManager::instance().database());
        rq.prepare("SELECT r.role_name FROM sys_role r "
                   "INNER JOIN sys_user_role ur ON r.id = ur.role_id "
                   "WHERE ur.user_id = :uid AND r.del_flag = 0");
        rq.bindValue(":uid", userId);
        rq.exec();
        QStringList roleNames;
        while (rq.next())
          roleNames << rq.value(0).toString();
        table->setItem(row, 5, new QTableWidgetItem(roleNames.join(", ")));

        // 操作列：文本 item（替代 setCellWidget 防止溢出）
        auto *opsItem = new QTableWidgetItem(QStringLiteral("编辑 ▾"));
        opsItem->setForeground(QColor("#b45309"));
        opsItem->setTextAlignment(Qt::AlignCenter);
        QFont opsFont;
        opsFont.setPointSize(11);
        opsFont.setBold(true);
        opsItem->setFont(opsFont);
        opsItem->setData(Qt::UserRole, userId);
        opsItem->setData(Qt::UserRole + 1, userSts);
        opsItem->setData(Qt::UserRole + 2, username);
        opsItem->setFlags(opsItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 6, opsItem);
        row++;
      }
      table->setSortingEnabled(true);
      UiKit::syncEmptyHint(table, emptyHint);
    };
    loadUsers();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadUsers(); });
    QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadUsers(); });
    QObject::connect(addUserBtn, &QPushButton::clicked, page, [openUserDialog, loadUsers]()
            { openUserDialog(0, loadUsers); });
    // 操作列点击：弹出右键菜单
    QObject::connect(table, &QTableWidget::cellClicked, page, [page, table, openUserDialog, toggleUserStatus, resetPassword, loadUsers](int row, int col)
            {
            if (col != 6) return;
            auto* item = table->item(row, col);
            if (!item) return;
            qint64 userId = item->data(Qt::UserRole).toLongLong();
            int userSts = item->data(Qt::UserRole + 1).toInt();
            QString username = item->data(Qt::UserRole + 2).toString();

            QMenu menu(page);
            auto* editAct = menu.addAction(QStringLiteral("编辑用户"));
            auto* toggleAct = menu.addAction(userSts == 0 ? QStringLiteral("禁用用户") : QStringLiteral("启用用户"));
            auto* resetAct = menu.addAction(QStringLiteral("重置密码"));
            QAction* chosen = menu.exec(QCursor::pos());
            if (chosen == editAct) openUserDialog(userId, loadUsers);
            else if (chosen == toggleAct) toggleUserStatus(userId, userSts, username, loadUsers);
            else if (chosen == resetAct) resetPassword(userId, username, loadUsers); });

    layout->addWidget(table);
    layout->addWidget(emptyHint);
    return page;
  }

  // ========================================================================
  // Task 12: 角色管理 CRUD
  // ========================================================================
  if (sub == "role")
  {
    auto *table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(UiKit::TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索角色名称/标识..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *roleDomainCombo = new QComboBox(toolbar);
    roleDomainCombo->addItem(QStringLiteral("全部域"), QString());
    roleDomainCombo->addItem(QStringLiteral("居民域"), "resident");
    roleDomainCombo->addItem(QStringLiteral("物业域"), "property");
    roleDomainCombo->addItem(QStringLiteral("治理域"), "governance");
    roleDomainCombo->addItem(QStringLiteral("服务域"), "service");
    roleDomainCombo->addItem(QStringLiteral("监督域"), "supervision");
    roleDomainCombo->addItem(QStringLiteral("平台域"), "platform");
    roleDomainCombo->setMinimumWidth(120);
    tbLayout->addWidget(roleDomainCombo);
    tbLayout->addStretch();
    auto *addRoleBtn = new QPushButton(QStringLiteral("+ 新增角色"), toolbar);
    addRoleBtn->setStyleSheet(primaryBtnStyle());
    addRoleBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(addRoleBtn);
    layout->addWidget(toolbar);

    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("角色名称"), QStringLiteral("标识"),
                                      QStringLiteral("域"), QStringLiteral("数据权限"), QStringLiteral("状态"), QStringLiteral("操作")});
    table->setColumnWidth(5, 160);

    // 构建菜单树（带复选框）
    auto buildMenuTree = [&db](QTreeWidget *tree, const QList<qint64> &checkedIds = {})
    {
      tree->clear();
      QSqlQuery q(db.database());
      q.exec("SELECT id, parent_id, menu_name, menu_type FROM sys_menu WHERE status = 0 ORDER BY parent_id, sort_order");
      QMap<qint64, QTreeWidgetItem *> items;
      while (q.next())
      {
        qint64 id = q.value(0).toLongLong();
        qint64 pid = q.value(1).toLongLong();
        auto *item = new QTreeWidgetItem();
        item->setText(0, q.value(2).toString());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, checkedIds.contains(id) ? Qt::Checked : Qt::Unchecked);
        item->setData(0, Qt::UserRole, id);
        int mt = q.value(3).toInt();
        item->setText(1, mt == 1 ? QStringLiteral("目录") : (mt == 2 ? QStringLiteral("菜单") : QStringLiteral("按钮")));
        items[id] = item;
        if (pid == 0)
          tree->addTopLevelItem(item);
        else if (items.contains(pid))
          items[pid]->addChild(item);
      }
    };

    // 收集树中所有勾选的菜单ID
    auto collectCheckedMenus = [](QTreeWidget *tree) -> QList<qint64>
    {
      QList<qint64> ids;
      QTreeWidgetItemIterator it(tree);
      while (*it)
      {
        if ((*it)->checkState(0) == Qt::Checked)
        {
          ids << (*it)->data(0, Qt::UserRole).toLongLong();
        }
        ++it;
      }
      return ids;
    };

    // 角色新增/编辑对话框
    auto openRoleDialog = [page, &db, &styleInput, &createFormDialog, &addDialogButtons, &buildMenuTree, &collectCheckedMenus](
                              qint64 roleId, std::function<void()> reload)
    {
      bool isEdit = (roleId > 0);
      QFormLayout *form = nullptr;
      QDialog *dlg = nullptr;
      auto *dlgLayout = createFormDialog(isEdit ? QStringLiteral("编辑角色") : QStringLiteral("新增角色"), form, dlg);

      auto *nameEdit = new QLineEdit(dlg);
      auto *keyEdit = new QLineEdit(dlg);
      auto *domainCombo = new QComboBox(dlg);
      domainCombo->addItem(QStringLiteral("平台域"), "platform");
      domainCombo->addItem(QStringLiteral("物业域"), "property");
      domainCombo->addItem(QStringLiteral("治理域"), "governance");
      domainCombo->addItem(QStringLiteral("服务域"), "service");
      domainCombo->addItem(QStringLiteral("居民域"), "resident");
      domainCombo->addItem(QStringLiteral("监督域"), "supervision");
      auto *scopeCombo = new QComboBox(dlg);
      scopeCombo->addItem(DataScope::label(DataScope::Platform), DataScope::Platform);
      scopeCombo->addItem(DataScope::label(DataScope::Street), DataScope::Street);
      scopeCombo->addItem(DataScope::label(DataScope::Community), DataScope::Community);
      scopeCombo->addItem(DataScope::label(DataScope::Estate), DataScope::Estate);
      scopeCombo->addItem(DataScope::label(DataScope::Building), DataScope::Building);
      scopeCombo->addItem(DataScope::label(DataScope::Personal), DataScope::Personal);
      auto *statusCombo2 = new QComboBox(dlg);
      statusCombo2->addItem(QStringLiteral("正常"), 0);
      statusCombo2->addItem(QStringLiteral("禁用"), 1);
      auto *menuTree = new QTreeWidget(dlg);
      menuTree->setHeaderLabels({QStringLiteral("菜单名称"), QStringLiteral("类型")});
      menuTree->setMinimumHeight(200);

      styleInput(nameEdit);
      styleInput(keyEdit);
      styleInput(domainCombo);
      styleInput(scopeCombo);
      styleInput(statusCombo2);

      QString oldKey;
      if (isEdit)
      {
        QSqlQuery rq(db.database());
        rq.prepare("SELECT role_name, role_key, role_domain, data_scope, status FROM sys_role WHERE id = :id");
        rq.bindValue(":id", roleId);
        rq.exec();
        if (rq.next())
        {
          nameEdit->setText(rq.value(0).toString());
          oldKey = rq.value(1).toString();
          keyEdit->setText(oldKey);
          QString dom = rq.value(2).toString();
          for (int i = 0; i < domainCombo->count(); ++i)
          {
            if (domainCombo->itemData(i).toString() == dom)
            {
              domainCombo->setCurrentIndex(i);
              break;
            }
          }
          int ds = rq.value(3).toInt();
          for (int i = 0; i < scopeCombo->count(); ++i)
          {
            if (scopeCombo->itemData(i).toInt() == ds)
            {
              scopeCombo->setCurrentIndex(i);
              break;
            }
          }
          int st = rq.value(4).toInt();
          for (int i = 0; i < statusCombo2->count(); ++i)
          {
            if (statusCombo2->itemData(i).toInt() == st)
            {
              statusCombo2->setCurrentIndex(i);
              break;
            }
          }
        }
        // 查询已分配菜单
        QList<qint64> checkedIds;
        QSqlQuery rmq(db.database());
        rmq.prepare("SELECT menu_id FROM sys_role_menu WHERE role_id = :rid");
        rmq.bindValue(":rid", roleId);
        rmq.exec();
        while (rmq.next())
          checkedIds << rmq.value(0).toLongLong();
        buildMenuTree(menuTree, checkedIds);
      }
      else
      {
        buildMenuTree(menuTree);
      }

      form->addRow(QStringLiteral("角色名称 *"), nameEdit);
      form->addRow(QStringLiteral("角色标识 *"), keyEdit);
      form->addRow(QStringLiteral("角色域"), domainCombo);
      form->addRow(QStringLiteral("数据权限"), scopeCombo);
      form->addRow(QStringLiteral("状态"), statusCombo2);
      form->addRow(QStringLiteral("菜单权限"), menuTree);

      addDialogButtons(dlgLayout, dlg, QStringLiteral("确定"), [=, &db]() -> bool
                       {
                QString name = nameEdit->text().trimmed();
                QString key = keyEdit->text().trimmed();
                QString domain = domainCombo->currentData().toString();
                int scope = scopeCombo->currentData().toInt();
                int status = statusCombo2->currentData().toInt();
                if (name.isEmpty()) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请输入角色名称")); return false; }
                if (key.isEmpty()) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请输入角色标识")); return false; }

                // 角色标识唯一性校验
                QSqlQuery cntQ(db.database());
                cntQ.prepare("SELECT COUNT(*) FROM sys_role WHERE role_key = :k AND del_flag = 0 AND id != :id");
                cntQ.bindValue(":k", key);
                cntQ.bindValue(":id", roleId);
                cntQ.exec();
                if (cntQ.next() && cntQ.value(0).toInt() > 0) {
                    QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("角色标识已存在"));
                    return false;
                }

                qint64 currentUserId = AuthService::instance().currentUser().id;
                QList<qint64> menuIds = collectCheckedMenus(menuTree);

                if (!isEdit) {
                    qint64 newId = db.insert("sys_role", {
                        {"role_name", name}, {"role_key", key}, {"role_domain", domain},
                        {"data_scope", scope}, {"status", status}, {"sort_order", 0},
                        {"create_time", QDateTime::currentDateTime()},
                        {"update_time", QDateTime::currentDateTime()}
                    });
                    if (newId <= 0) { QMessageBox::warning(dlg, QStringLiteral("错误"), QStringLiteral("创建角色失败")); return false; }
                    for (qint64 mid : menuIds) {
                        db.insert("sys_role_menu", {{"role_id", newId}, {"menu_id", mid}});
                    }
                    db.insert("sys_operation_log", {
                        {"user_id", currentUserId}, {"username", AuthService::instance().currentUser().username},
                        {"module", QStringLiteral("角色管理")},
                        {"operation", QStringLiteral("新增角色: %1").arg(name)},
                        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("角色创建成功"), page);
                } else {
                    db.update("sys_role", roleId, {
                        {"role_name", name}, {"role_key", key}, {"role_domain", domain},
                        {"data_scope", scope}, {"status", status},
                        {"update_time", QDateTime::currentDateTime()}
                    });
                    // 重建菜单关联
                    QSqlQuery delQ(db.database());
                    delQ.prepare("DELETE FROM sys_role_menu WHERE role_id = :rid");
                    delQ.bindValue(":rid", roleId);
                    delQ.exec();
                    for (qint64 mid : menuIds) {
                        db.insert("sys_role_menu", {{"role_id", roleId}, {"menu_id", mid}});
                    }
                    db.insert("sys_operation_log", {
                        {"user_id", currentUserId}, {"username", AuthService::instance().currentUser().username},
                        {"module", QStringLiteral("角色管理")},
                        {"operation", QStringLiteral("编辑角色: %1").arg(name)},
                        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("角色更新成功"), page);
                }
                reload();
                return true; });
      dlg->exec();
      delete dlg;
    };

    std::function<void()> loadRoles;
    loadRoles = [table, searchEdit, roleDomainCombo, emptyHint, &openRoleDialog, &loadRoles]()
    {
      table->setSortingEnabled(false);
      table->setRowCount(0);
      QString sql = "SELECT id, role_name, role_key, role_domain, data_scope, status FROM sys_role WHERE del_flag = 0";
      QString searchText = searchEdit->text().trimmed();
      QString domainFilter = roleDomainCombo->currentData().toString();
      if (!searchText.isEmpty())
        sql += " AND (role_name LIKE :search OR role_key LIKE :search)";
      if (!domainFilter.isEmpty())
        sql += " AND role_domain = :domain";
      sql += " ORDER BY sort_order";
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (!domainFilter.isEmpty())
        q.bindValue(":domain", domainFilter);
      q.exec();
      int row = 0;
      while (q.next())
      {
        qint64 rid = q.value(0).toLongLong();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(RoleDomain::label(q.value(3).toString())));
        table->setItem(row, 3, new QTableWidgetItem(DataScope::label(q.value(4).toInt())));
        int st = q.value(5).toInt();
        auto *stItem = st == 0
                           ? UiKit::createTagTableItem(QStringLiteral("正常"), QColor("#f0fdf4"), QColor("#15803d"))
                           : UiKit::createTagTableItem(QStringLiteral("禁用"), QColor("#fef2f2"), QColor("#b91c1c"));
        table->setItem(row, 4, stItem);
        auto *editItem = new QTableWidgetItem(QStringLiteral("编辑"));
        editItem->setForeground(QColor("#b45309"));
        editItem->setTextAlignment(Qt::AlignCenter);
        QFont ef;
        ef.setPointSize(11);
        ef.setBold(true);
        editItem->setFont(ef);
        editItem->setData(Qt::UserRole, rid);
        editItem->setFlags(editItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 5, editItem);
        row++;
      }
      table->setSortingEnabled(true);
      UiKit::syncEmptyHint(table, emptyHint);
    };
    loadRoles();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadRoles(); });
    QObject::connect(roleDomainCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadRoles(); });
    QObject::connect(addRoleBtn, &QPushButton::clicked, page, [openRoleDialog, loadRoles]()
            { openRoleDialog(0, loadRoles); });
    QObject::connect(table, &QTableWidget::cellClicked, page, [table, openRoleDialog, &loadRoles](int row, int col)
            {
            if (col != 5) return;
            auto* item = table->item(row, col);
            if (!item) return;
            openRoleDialog(item->data(Qt::UserRole).toLongLong(), loadRoles); });
    QObject::connect(roleDomainCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadRoles(); });
    QObject::connect(addRoleBtn, &QPushButton::clicked, page, [openRoleDialog, loadRoles]()
            { openRoleDialog(0, loadRoles); });

    layout->addWidget(table);
    layout->addWidget(emptyHint);
    return page;
  }

  // ========================================================================
  // Task 13: 菜单管理 CRUD
  // ========================================================================
  if (sub == "menu")
  {
    auto *tree = new QTreeWidget(page);
    tree->setHeaderLabels({QStringLiteral("菜单名称"), QStringLiteral("类型"), QStringLiteral("路径"),
                           QStringLiteral("权限标识"), QStringLiteral("排序"), QStringLiteral("操作")});
    tree->setColumnWidth(0, 200);
    tree->setColumnWidth(1, 90);
    tree->setColumnWidth(2, 180);
    tree->setColumnWidth(3, 160);
    tree->setColumnWidth(4, 60);
    tree->setColumnWidth(5, 160);
    tree->setStyleSheet(UiKit::TABLE_STYLE);

    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->addStretch();
    auto *addMenuBtn = new QPushButton(QStringLiteral("+ 新增菜单"), toolbar);
    addMenuBtn->setStyleSheet(primaryBtnStyle());
    addMenuBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(addMenuBtn);
    layout->addWidget(toolbar);

    // 菜单新增/编辑对话框
    auto openMenuDialog = [page, &db, &styleInput, &createFormDialog, &addDialogButtons](
                              qint64 menuId, std::function<void()> reload)
    {
      bool isEdit = (menuId > 0);
      QFormLayout *form = nullptr;
      QDialog *dlg = nullptr;
      auto *dlgLayout = createFormDialog(isEdit ? QStringLiteral("编辑菜单") : QStringLiteral("新增菜单"), form, dlg);

      auto *parentCombo = new QComboBox(dlg);
      parentCombo->addItem(QStringLiteral("根菜单"), 0);
      QSqlQuery pq(db.database());
      pq.exec("SELECT id, menu_name, parent_id FROM sys_menu WHERE status = 0 AND menu_type IN (1,2) ORDER BY parent_id, sort_order");
      while (pq.next())
      {
        qint64 pid = pq.value(0).toLongLong();
        QString name = pq.value(1).toString();
        qint64 ppid = pq.value(2).toLongLong();
        QString prefix = ppid == 0 ? QString() : QStringLiteral("  └ ");
        parentCombo->addItem(prefix + name, pid);
      }
      auto *nameEdit = new QLineEdit(dlg);
      auto *pathEdit = new QLineEdit(dlg);
      pathEdit->setPlaceholderText(QStringLiteral("如 /workshop"));
      auto *iconEdit = new QLineEdit(dlg);
      iconEdit->setPlaceholderText(QStringLiteral("如 ic_dashboard"));
      auto *sortSpin = new QSpinBox(dlg);
      sortSpin->setRange(0, 999);
      auto *typeCombo = new QComboBox(dlg);
      typeCombo->addItem(QStringLiteral("目录"), 1);
      typeCombo->addItem(QStringLiteral("菜单"), 2);
      typeCombo->addItem(QStringLiteral("按钮"), 3);
      auto *statusCombo2 = new QComboBox(dlg);
      statusCombo2->addItem(QStringLiteral("正常"), 0);
      statusCombo2->addItem(QStringLiteral("禁用"), 1);
      auto *permEdit = new QLineEdit(dlg);
      permEdit->setPlaceholderText(QStringLiteral("如 system:user:list"));

      styleInput(parentCombo);
      styleInput(nameEdit);
      styleInput(pathEdit);
      styleInput(iconEdit);
      styleInput(sortSpin);
      styleInput(typeCombo);
      styleInput(statusCombo2);
      styleInput(permEdit);

      qint64 selfId = menuId;
      if (isEdit)
      {
        QSqlQuery mq(db.database());
        mq.prepare("SELECT parent_id, menu_name, path, icon, sort_order, menu_type, status, permission FROM sys_menu WHERE id = :id");
        mq.bindValue(":id", menuId);
        mq.exec();
        if (mq.next())
        {
          qint64 pid = mq.value(0).toLongLong();
          for (int i = 0; i < parentCombo->count(); ++i)
          {
            if (parentCombo->itemData(i).toLongLong() == pid)
            {
              parentCombo->setCurrentIndex(i);
              break;
            }
          }
          nameEdit->setText(mq.value(1).toString());
          pathEdit->setText(mq.value(2).toString());
          iconEdit->setText(mq.value(3).toString());
          sortSpin->setValue(mq.value(4).toInt());
          int mt = mq.value(5).toInt();
          for (int i = 0; i < typeCombo->count(); ++i)
          {
            if (typeCombo->itemData(i).toInt() == mt)
            {
              typeCombo->setCurrentIndex(i);
              break;
            }
          }
          int st = mq.value(6).toInt();
          for (int i = 0; i < statusCombo2->count(); ++i)
          {
            if (statusCombo2->itemData(i).toInt() == st)
            {
              statusCombo2->setCurrentIndex(i);
              break;
            }
          }
          permEdit->setText(mq.value(7).toString());
        }
      }

      form->addRow(QStringLiteral("上级菜单"), parentCombo);
      form->addRow(QStringLiteral("菜单名称 *"), nameEdit);
      form->addRow(QStringLiteral("菜单路径"), pathEdit);
      form->addRow(QStringLiteral("图标"), iconEdit);
      form->addRow(QStringLiteral("排序"), sortSpin);
      form->addRow(QStringLiteral("类型"), typeCombo);
      form->addRow(QStringLiteral("权限标识"), permEdit);
      form->addRow(QStringLiteral("状态"), statusCombo2);

      addDialogButtons(dlgLayout, dlg, QStringLiteral("确定"), [=, &db]() -> bool
                       {
                QString name = nameEdit->text().trimmed();
                if (name.isEmpty()) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请输入菜单名称")); return false; }
                qint64 parentId = parentCombo->currentData().toLongLong();
                if (isEdit && parentId == selfId) {
                    QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("上级菜单不能选择自身"));
                    return false;
                }
                QVariantMap data{
                    {"parent_id", parentId}, {"menu_name", name},
                    {"path", pathEdit->text().trimmed()}, {"icon", iconEdit->text().trimmed()},
                    {"sort_order", sortSpin->value()}, {"menu_type", typeCombo->currentData().toInt()},
                    {"status", statusCombo2->currentData().toInt()},
                    {"permission", permEdit->text().trimmed()}
                };
                qint64 currentUserId = AuthService::instance().currentUser().id;
                if (!isEdit) {
                    qint64 newId = db.insert("sys_menu", data);
                    if (newId <= 0) { QMessageBox::warning(dlg, QStringLiteral("错误"), QStringLiteral("创建菜单失败")); return false; }
                    db.insert("sys_operation_log", {
                        {"user_id", currentUserId}, {"username", AuthService::instance().currentUser().username},
                        {"module", QStringLiteral("菜单管理")},
                        {"operation", QStringLiteral("新增菜单: %1").arg(name)},
                        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("菜单创建成功"), page);
                } else {
                    // sys_menu 没有 update_time 字段，直接更新
                    QSqlQuery uq(db.database());
                    uq.prepare("UPDATE sys_menu SET parent_id=:pid, menu_name=:name, path=:path, icon=:icon, "
                               "sort_order=:sort, menu_type=:type, status=:status, permission=:perm WHERE id=:id");
                    uq.bindValue(":pid", parentId);
                    uq.bindValue(":name", name);
                    uq.bindValue(":path", pathEdit->text().trimmed());
                    uq.bindValue(":icon", iconEdit->text().trimmed());
                    uq.bindValue(":sort", sortSpin->value());
                    uq.bindValue(":type", typeCombo->currentData().toInt());
                    uq.bindValue(":status", statusCombo2->currentData().toInt());
                    uq.bindValue(":perm", permEdit->text().trimmed());
                    uq.bindValue(":id", menuId);
                    uq.exec();
                    db.insert("sys_operation_log", {
                        {"user_id", currentUserId}, {"username", AuthService::instance().currentUser().username},
                        {"module", QStringLiteral("菜单管理")},
                        {"operation", QStringLiteral("编辑菜单: %1").arg(name)},
                        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("菜单更新成功"), page);
                }
                reload();
                return true; });
      dlg->exec();
      delete dlg;
    };

    // 删除菜单（逻辑删除：将 status 置为 1，因为 sys_menu 无 del_flag 字段）
    auto deleteMenu = [page, &db](qint64 menuId, const QString &name, std::function<void()> reload)
    {
      auto ret = QMessageBox::question(page, QStringLiteral("确认删除"),
                                       QStringLiteral("确认删除菜单「%1」？删除后不可见，可重新编辑恢复。").arg(name),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (ret != QMessageBox::Yes)
        return;
      QSqlQuery uq(db.database());
      uq.prepare("UPDATE sys_menu SET status = 1 WHERE id = :id");
      uq.bindValue(":id", menuId);
      uq.exec();
      qint64 currentUserId = AuthService::instance().currentUser().id;
      db.insert("sys_operation_log", {{"user_id", currentUserId}, {"username", AuthService::instance().currentUser().username}, {"module", QStringLiteral("菜单管理")}, {"operation", QStringLiteral("删除菜单: %1").arg(name)}, {"status", 0}, {"operation_time", QDateTime::currentDateTime()}});
      UiKit::showToast(QStringLiteral("菜单已删除"), page);
      reload();
    };

    std::function<void()> loadMenus;
    loadMenus = [tree, &openMenuDialog, &deleteMenu, &loadMenus]()
    {
      tree->clear();
      QSqlQuery q(DatabaseManager::instance().database());
      q.exec("SELECT id, parent_id, menu_name, menu_type, path, permission, icon, sort_order, status FROM sys_menu WHERE status = 0 ORDER BY parent_id, sort_order");
      QMap<qint64, QTreeWidgetItem *> items;
      QList<QPair<qint64, qint64>> pending;
      while (q.next())
      {
        qint64 id = q.value(0).toLongLong();
        qint64 pid = q.value(1).toLongLong();
        QString name = q.value(2).toString();
        int mt = q.value(3).toInt();
        auto *item = new QTreeWidgetItem();
        item->setText(0, name);
        item->setText(1, mt == 1 ? QStringLiteral("目录") : (mt == 2 ? QStringLiteral("菜单") : QStringLiteral("按钮")));
        item->setText(2, q.value(4).toString());
        item->setText(3, q.value(5).toString());
        item->setText(4, QString::number(q.value(7).toInt()));
        item->setData(0, Qt::UserRole, id);
        // 操作按钮
        // 操作列：文本 item
        auto *opsItem = new QTreeWidgetItem(item);
        // 用 column 5 的文本存储操作提示
        item->setText(5, QStringLiteral("编辑 | 删除"));
        item->setForeground(5, QBrush(QColor("#b45309")));
        item->setData(5, Qt::UserRole, id);
        item->setData(5, Qt::UserRole + 1, name);

        items[id] = item;
        if (pid == 0)
          tree->addTopLevelItem(item);
        else if (items.contains(pid))
          items[pid]->addChild(item);
        else
          pending << qMakePair(id, pid);
      }
      // 处理 pending（父节点后于子节点出现的情况）
      for (auto &p : pending)
      {
        if (items.contains(p.second) && items.contains(p.first))
        {
          items[p.second]->addChild(items[p.first]);
        }
      }
      tree->expandAll();
    };
    loadMenus();
    QObject::connect(addMenuBtn, &QPushButton::clicked, page, [openMenuDialog, loadMenus]()
            { openMenuDialog(0, loadMenus); });
    // 菜单树操作列点击
    QObject::connect(tree, &QTreeWidget::itemClicked, page, [page, tree, openMenuDialog, deleteMenu, loadMenus](QTreeWidgetItem *item, int col)
            {
            if (col != 5 || !item) return;
            qint64 id = item->data(5, Qt::UserRole).toLongLong();
            QString name = item->data(5, Qt::UserRole + 1).toString();
            QMenu menu(page);
            auto* editAct = menu.addAction(QStringLiteral("编辑"));
            auto* delAct = menu.addAction(QStringLiteral("删除"));
            QAction* chosen = menu.exec(QCursor::pos());
            if (chosen == editAct) openMenuDialog(id, loadMenus);
            else if (chosen == delAct) deleteMenu(id, name, loadMenus); });

    layout->addWidget(tree);
    return page;
  }

  // ========================================================================
  // Task 14: 字典管理 CRUD
  // ========================================================================
  if (sub == "dict")
  {
    auto *table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(UiKit::TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索字典类型/标签..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *dictTypeCombo = new QComboBox(toolbar);
    dictTypeCombo->addItem(QStringLiteral("全部类型"), QString());
    QSqlQuery dtQ("SELECT dict_type, dict_name FROM sys_dict_type WHERE status = 0 ORDER BY dict_type");
    while (dtQ.next())
    {
      dictTypeCombo->addItem(dtQ.value(1).toString(), dtQ.value(0).toString());
    }
    dictTypeCombo->setMinimumWidth(140);
    tbLayout->addWidget(dictTypeCombo);
    tbLayout->addStretch();
    auto *addTypeBtn = new QPushButton(QStringLiteral("+ 新增字典类型"), toolbar);
    addTypeBtn->setStyleSheet(primaryBtnStyle());
    addTypeBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(addTypeBtn);
    auto *addItemBtn = new QPushButton(QStringLiteral("+ 新增字典项"), toolbar);
    addItemBtn->setStyleSheet(primaryBtnStyle());
    addItemBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(addItemBtn);
    layout->addWidget(toolbar);

    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("字典类型"), QStringLiteral("类型名称"),
                                      QStringLiteral("字典标签"), QStringLiteral("字典值"), QStringLiteral("排序"), QStringLiteral("操作")});
    table->setColumnWidth(5, 160);

    // 字典类型新增/编辑对话框
    auto openDictTypeDialog = [page, &db, &styleInput, &createFormDialog, &addDialogButtons](
                                  qint64 typeId, std::function<void()> reload)
    {
      bool isEdit = (typeId > 0);
      QFormLayout *form = nullptr;
      QDialog *dlg = nullptr;
      auto *dlgLayout = createFormDialog(isEdit ? QStringLiteral("编辑字典类型") : QStringLiteral("新增字典类型"), form, dlg);
      auto *nameEdit = new QLineEdit(dlg);
      auto *typeEdit = new QLineEdit(dlg);
      auto *statusCombo2 = new QComboBox(dlg);
      statusCombo2->addItem(QStringLiteral("正常"), 0);
      statusCombo2->addItem(QStringLiteral("禁用"), 1);
      auto *remarkEdit = new QTextEdit(dlg);
      remarkEdit->setFixedHeight(80);
      styleInput(nameEdit);
      styleInput(typeEdit);
      styleInput(statusCombo2);
      styleInput(remarkEdit);

      if (isEdit)
      {
        QSqlQuery tq(db.database());
        tq.prepare("SELECT dict_name, dict_type, status, remark FROM sys_dict_type WHERE id = :id");
        tq.bindValue(":id", typeId);
        tq.exec();
        if (tq.next())
        {
          nameEdit->setText(tq.value(0).toString());
          typeEdit->setText(tq.value(1).toString());
          int st = tq.value(2).toInt();
          for (int i = 0; i < statusCombo2->count(); ++i)
          {
            if (statusCombo2->itemData(i).toInt() == st)
            {
              statusCombo2->setCurrentIndex(i);
              break;
            }
          }
          remarkEdit->setText(tq.value(3).toString());
        }
      }

      form->addRow(QStringLiteral("字典名称 *"), nameEdit);
      form->addRow(QStringLiteral("字典类型 *"), typeEdit);
      form->addRow(QStringLiteral("状态"), statusCombo2);
      form->addRow(QStringLiteral("备注"), remarkEdit);

      addDialogButtons(dlgLayout, dlg, QStringLiteral("确定"), [=, &db]() -> bool
                       {
                QString name = nameEdit->text().trimmed();
                QString type = typeEdit->text().trimmed();
                if (name.isEmpty()) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请输入字典名称")); return false; }
                if (type.isEmpty()) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请输入字典类型")); return false; }
                // 唯一性校验
                QSqlQuery cntQ(db.database());
                cntQ.prepare("SELECT COUNT(*) FROM sys_dict_type WHERE dict_type = :t AND id != :id");
                cntQ.bindValue(":t", type);
                cntQ.bindValue(":id", typeId);
                cntQ.exec();
                if (cntQ.next() && cntQ.value(0).toInt() > 0) {
                    QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("字典类型已存在"));
                    return false;
                }
                qint64 currentUserId = AuthService::instance().currentUser().id;
                if (!isEdit) {
                    db.insert("sys_dict_type", {
                        {"dict_name", name}, {"dict_type", type},
                        {"status", statusCombo2->currentData().toInt()},
                        {"remark", remarkEdit->toPlainText().trimmed()},
                        {"create_time", QDateTime::currentDateTime()}
                    });
                    db.insert("sys_operation_log", {
                        {"user_id", currentUserId}, {"username", AuthService::instance().currentUser().username},
                        {"module", QStringLiteral("字典管理")},
                        {"operation", QStringLiteral("新增字典类型: %1").arg(type)},
                        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("字典类型创建成功"), page);
                } else {
                    QString oldType;
                    QSqlQuery oq(db.database());
                    oq.prepare("SELECT dict_type FROM sys_dict_type WHERE id = :id");
                    oq.bindValue(":id", typeId);
                    oq.exec();
                    if (oq.next()) oldType = oq.value(0).toString();
                    QSqlQuery uq(db.database());
                    uq.prepare("UPDATE sys_dict_type SET dict_name=:n, dict_type=:t, status=:s, remark=:r WHERE id=:id");
                    uq.bindValue(":n", name);
                    uq.bindValue(":t", type);
                    uq.bindValue(":s", statusCombo2->currentData().toInt());
                    uq.bindValue(":r", remarkEdit->toPlainText().trimmed());
                    uq.bindValue(":id", typeId);
                    uq.exec();
                    // 同步更新字典数据的 dict_type
                    if (oldType != type) {
                        QSqlQuery udq(db.database());
                        udq.prepare("UPDATE sys_dict_data SET dict_type = :nt WHERE dict_type = :ot");
                        udq.bindValue(":nt", type);
                        udq.bindValue(":ot", oldType);
                        udq.exec();
                    }
                    db.insert("sys_operation_log", {
                        {"user_id", currentUserId}, {"username", AuthService::instance().currentUser().username},
                        {"module", QStringLiteral("字典管理")},
                        {"operation", QStringLiteral("编辑字典类型: %1").arg(type)},
                        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("字典类型更新成功"), page);
                }
                reload();
                return true; });
      dlg->exec();
      delete dlg;
    };

    // 字典项新增/编辑对话框
    auto openDictItemDialog = [page, &db, &styleInput, &createFormDialog, &addDialogButtons, dictTypeCombo](
                                  qint64 itemId, std::function<void()> reload)
    {
      bool isEdit = (itemId > 0);
      QFormLayout *form = nullptr;
      QDialog *dlg = nullptr;
      auto *dlgLayout = createFormDialog(isEdit ? QStringLiteral("编辑字典项") : QStringLiteral("新增字典项"), form, dlg);
      auto *typeCombo = new QComboBox(dlg);
      QSqlQuery dtq(db.database());
      dtq.exec("SELECT dict_type, dict_name FROM sys_dict_type WHERE status = 0 ORDER BY dict_type");
      while (dtq.next())
        typeCombo->addItem(dtq.value(0).toString() + " (" + dtq.value(1).toString() + ")", dtq.value(0).toString());
      auto *labelEdit = new QLineEdit(dlg);
      auto *valueEdit = new QLineEdit(dlg);
      auto *sortSpin = new QSpinBox(dlg);
      sortSpin->setRange(0, 999);
      auto *statusCombo2 = new QComboBox(dlg);
      statusCombo2->addItem(QStringLiteral("正常"), 0);
      statusCombo2->addItem(QStringLiteral("禁用"), 1);
      styleInput(typeCombo);
      styleInput(labelEdit);
      styleInput(valueEdit);
      styleInput(sortSpin);
      styleInput(statusCombo2);

      // 默认选择当前筛选的字典类型
      QString currentTypeFilter = dictTypeCombo->currentData().toString();
      if (!currentTypeFilter.isEmpty())
      {
        for (int i = 0; i < typeCombo->count(); ++i)
        {
          if (typeCombo->itemData(i).toString() == currentTypeFilter)
          {
            typeCombo->setCurrentIndex(i);
            break;
          }
        }
      }

      if (isEdit)
      {
        QSqlQuery iq(db.database());
        iq.prepare("SELECT dict_type, dict_label, dict_value, sort_order, status FROM sys_dict_data WHERE id = :id");
        iq.bindValue(":id", itemId);
        iq.exec();
        if (iq.next())
        {
          QString dt = iq.value(0).toString();
          for (int i = 0; i < typeCombo->count(); ++i)
          {
            if (typeCombo->itemData(i).toString() == dt)
            {
              typeCombo->setCurrentIndex(i);
              break;
            }
          }
          labelEdit->setText(iq.value(1).toString());
          valueEdit->setText(iq.value(2).toString());
          sortSpin->setValue(iq.value(3).toInt());
          int st = iq.value(4).toInt();
          for (int i = 0; i < statusCombo2->count(); ++i)
          {
            if (statusCombo2->itemData(i).toInt() == st)
            {
              statusCombo2->setCurrentIndex(i);
              break;
            }
          }
        }
      }

      form->addRow(QStringLiteral("字典类型 *"), typeCombo);
      form->addRow(QStringLiteral("字典标签 *"), labelEdit);
      form->addRow(QStringLiteral("字典键值 *"), valueEdit);
      form->addRow(QStringLiteral("排序"), sortSpin);
      form->addRow(QStringLiteral("状态"), statusCombo2);

      addDialogButtons(dlgLayout, dlg, QStringLiteral("确定"), [=, &db]() -> bool
                       {
                QString dtype = typeCombo->currentData().toString();
                QString label = labelEdit->text().trimmed();
                QString value = valueEdit->text().trimmed();
                if (dtype.isEmpty()) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请选择字典类型")); return false; }
                if (label.isEmpty()) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请输入字典标签")); return false; }
                if (value.isEmpty()) { QMessageBox::warning(dlg, QStringLiteral("提示"), QStringLiteral("请输入字典键值")); return false; }
                qint64 currentUserId = AuthService::instance().currentUser().id;
                if (!isEdit) {
                    db.insert("sys_dict_data", {
                        {"dict_type", dtype}, {"dict_label", label}, {"dict_value", value},
                        {"sort_order", sortSpin->value()}, {"status", statusCombo2->currentData().toInt()}
                    });
                    db.insert("sys_operation_log", {
                        {"user_id", currentUserId}, {"username", AuthService::instance().currentUser().username},
                        {"module", QStringLiteral("字典管理")},
                        {"operation", QStringLiteral("新增字典项: %1/%2").arg(dtype).arg(label)},
                        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("字典项创建成功"), page);
                } else {
                    QSqlQuery uq(db.database());
                    uq.prepare("UPDATE sys_dict_data SET dict_type=:t, dict_label=:l, dict_value=:v, sort_order=:s, status=:st WHERE id=:id");
                    uq.bindValue(":t", dtype);
                    uq.bindValue(":l", label);
                    uq.bindValue(":v", value);
                    uq.bindValue(":s", sortSpin->value());
                    uq.bindValue(":st", statusCombo2->currentData().toInt());
                    uq.bindValue(":id", itemId);
                    uq.exec();
                    db.insert("sys_operation_log", {
                        {"user_id", currentUserId}, {"username", AuthService::instance().currentUser().username},
                        {"module", QStringLiteral("字典管理")},
                        {"operation", QStringLiteral("编辑字典项: %1/%2").arg(dtype).arg(label)},
                        {"status", 0}, {"operation_time", QDateTime::currentDateTime()}
                    });
                    UiKit::showToast(QStringLiteral("字典项更新成功"), page);
                }
                reload();
                return true; });
      dlg->exec();
      delete dlg;
    };

    std::function<void()> loadDicts;
    loadDicts = [table, searchEdit, dictTypeCombo, emptyHint, &openDictTypeDialog, &openDictItemDialog, &loadDicts]()
    {
      table->setSortingEnabled(false);
      table->setRowCount(0);
      QString sql = "SELECT dd.id, dt.dict_type, dt.dict_name, dd.dict_label, dd.dict_value, dd.sort_order, dd.status "
                    "FROM sys_dict_data dd JOIN sys_dict_type dt ON dd.dict_type = dt.dict_type "
                    "WHERE dt.status = 0 AND dd.status = 0";
      QString searchText = searchEdit->text().trimmed();
      QString dictTypeFilter = dictTypeCombo->currentData().toString();
      if (!searchText.isEmpty())
        sql += " AND (dt.dict_type LIKE :search OR dd.dict_label LIKE :search)";
      if (!dictTypeFilter.isEmpty())
        sql += " AND dt.dict_type = :dtype";
      sql += " ORDER BY dt.dict_type, dd.sort_order";
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (!dictTypeFilter.isEmpty())
        q.bindValue(":dtype", dictTypeFilter);
      q.exec();
      int row = 0;
      while (q.next())
      {
        qint64 itemId = q.value(0).toLongLong();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 3, new QTableWidgetItem(q.value(4).toString()));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(q.value(5).toInt())));
        auto *editItem = new QTableWidgetItem(QStringLiteral("编辑"));
        editItem->setForeground(QColor("#b45309"));
        editItem->setTextAlignment(Qt::AlignCenter);
        QFont ef;
        ef.setPointSize(11);
        ef.setBold(true);
        editItem->setFont(ef);
        editItem->setData(Qt::UserRole, itemId);
        editItem->setFlags(editItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 5, editItem);
        row++;
      }
      table->setSortingEnabled(true);
      UiKit::syncEmptyHint(table, emptyHint);
    };
    loadDicts();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { loadDicts(); });
    QObject::connect(dictTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { loadDicts(); });
    QObject::connect(addTypeBtn, &QPushButton::clicked, page, [openDictTypeDialog, loadDicts]()
            { openDictTypeDialog(0, loadDicts); });
    QObject::connect(addItemBtn, &QPushButton::clicked, page, [openDictItemDialog, loadDicts]()
            { openDictItemDialog(0, loadDicts); });
    QObject::connect(table, &QTableWidget::cellClicked, page, [table, openDictItemDialog, loadDicts](int row, int col)
            {
            if (col != 5) return;
            auto* item = table->item(row, col);
            if (!item) return;
            openDictItemDialog(item->data(Qt::UserRole).toLongLong(), loadDicts); });

    layout->addWidget(table);
    layout->addWidget(emptyHint);
    return page;
  }

  // ========================================================================
  // Task 15: 操作日志分页
  // ========================================================================
  if (sub == "log")
  {
    auto *table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(UiKit::TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->setSpacing(10);
    auto *searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(QStringLiteral("搜索用户/模块/操作..."));
    searchEdit->setMinimumWidth(200);
    searchEdit->setClearButtonEnabled(true);
    tbLayout->addWidget(searchEdit);
    auto *moduleCombo = new QComboBox(toolbar);
    moduleCombo->addItem(QStringLiteral("全部模块"), "");
    moduleCombo->addItem(QStringLiteral("用户管理"), QStringLiteral("用户管理"));
    moduleCombo->addItem(QStringLiteral("工单管理"), QStringLiteral("工单管理"));
    moduleCombo->addItem(QStringLiteral("事件管理"), QStringLiteral("事件管理"));
    moduleCombo->addItem(QStringLiteral("报事报修"), QStringLiteral("报事报修"));
    moduleCombo->addItem(QStringLiteral("公告管理"), QStringLiteral("公告管理"));
    moduleCombo->addItem(QStringLiteral("督办管理"), QStringLiteral("督办管理"));
    moduleCombo->addItem(QStringLiteral("系统设置"), QStringLiteral("系统设置"));
    moduleCombo->addItem(QStringLiteral("角色管理"), QStringLiteral("角色管理"));
    moduleCombo->addItem(QStringLiteral("菜单管理"), QStringLiteral("菜单管理"));
    moduleCombo->addItem(QStringLiteral("字典管理"), QStringLiteral("字典管理"));
    moduleCombo->setMinimumWidth(120);
    tbLayout->addWidget(moduleCombo);
    tbLayout->addStretch();
    layout->addWidget(toolbar);

    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({QStringLiteral("用户"), QStringLiteral("模块"),
                                      QStringLiteral("操作"), QStringLiteral("IP"), QStringLiteral("时间")});

    // 分页状态（使用 shared_ptr 以便 lambda 共享）
    struct PageState
    {
      int currentPage;
      int totalPages;
      int totalCount;
      int pageSize;
    };
    auto pageState = std::make_shared<PageState>();
    pageState->currentPage = 1;
    pageState->totalPages = 1;
    pageState->totalCount = 0;
    pageState->pageSize = 20;

    // 分页控件
    auto *pageBar = new QWidget(page);
    pageBar->setStyleSheet("background:#ffffff; border-radius:6px; padding:4px 8px; border:1px solid #e2e8f0;");
    auto *pbLayout = new QHBoxLayout(pageBar);
    pbLayout->setContentsMargins(8, 4, 8, 4);
    pbLayout->setSpacing(8);
    auto *prevBtn = new QPushButton(QStringLiteral("上一页"), pageBar);
    prevBtn->setStyleSheet("QPushButton { background: #ffffff; color: #64748b; border: 1px solid #e2e8f0; border-radius: 4px; padding: 5px 14px; min-height: 32px; }"
                           "QPushButton:hover { border-color: #d97706; color: #d97706; }"
                           "QPushButton:disabled { color: #94a3b8; border-color: #e2e8f0; }");
    auto *nextBtn = new QPushButton(QStringLiteral("下一页"), pageBar);
    nextBtn->setStyleSheet(prevBtn->styleSheet());
    auto *pageLabel = new QLabel(QStringLiteral("第 1/1 页"), pageBar);
    pageLabel->setStyleSheet("color: #64748b; font-size: 13px; padding: 0 8px;");
    auto *pageSizeLabel = new QLabel(QStringLiteral("每页"), pageBar);
    pageSizeLabel->setStyleSheet("color: #64748b; font-size: 13px;");
    auto *pageSizeCombo = new QComboBox(pageBar);
    pageSizeCombo->addItem(QStringLiteral("20"), 20);
    pageSizeCombo->addItem(QStringLiteral("50"), 50);
    pageSizeCombo->addItem(QStringLiteral("100"), 100);
    pageSizeCombo->setFixedWidth(70);
    pageSizeCombo->setStyleSheet("QComboBox { min-height: 32px; padding: 2px 6px; border: 1px solid #e2e8f0; border-radius: 4px; }");
    auto *jumpLabel = new QLabel(QStringLiteral("跳转到"), pageBar);
    jumpLabel->setStyleSheet("color: #64748b; font-size: 13px;");
    auto *jumpEdit = new QLineEdit(pageBar);
    jumpEdit->setFixedWidth(60);
    jumpEdit->setStyleSheet("QLineEdit { min-height: 32px; padding: 2px 6px; border: 1px solid #e2e8f0; border-radius: 4px; }");
    auto *jumpBtn = new QPushButton(QStringLiteral("Go"), pageBar);
    jumpBtn->setStyleSheet("QPushButton { background: #b45309; color: #fff; border: none; border-radius: 4px; padding: 5px 12px; min-height: 32px; }"
                           "QPushButton:hover { background: #d97706; }");
    auto *totalLabel = new QLabel(QStringLiteral("共 0 条"), pageBar);
    totalLabel->setStyleSheet("color: #64748b; font-size: 13px;");

    pbLayout->addWidget(totalLabel);
    pbLayout->addStretch();
    pbLayout->addWidget(prevBtn);
    pbLayout->addWidget(pageLabel);
    pbLayout->addWidget(nextBtn);
    pbLayout->addSpacing(12);
    pbLayout->addWidget(pageSizeLabel);
    pbLayout->addWidget(pageSizeCombo);
    pbLayout->addSpacing(8);
    pbLayout->addWidget(jumpLabel);
    pbLayout->addWidget(jumpEdit);
    pbLayout->addWidget(jumpBtn);

    std::function<void()> loadLogs;
    loadLogs = [table, searchEdit, moduleCombo, emptyHint, pageState, pageLabel, totalLabel, prevBtn, nextBtn]()
    {
      table->setSortingEnabled(false);
      table->setRowCount(0);
      // 构建 WHERE 条件
      QString where = "1=1";
      QString searchText = searchEdit->text().trimmed();
      QString moduleFilter = moduleCombo->currentData().toString();
      if (!searchText.isEmpty())
        where += " AND (username LIKE :search OR module LIKE :search OR operation LIKE :search)";
      if (!moduleFilter.isEmpty())
        where += " AND module = :module";

      // 查询总数
      QSqlQuery cntQ(DatabaseManager::instance().database());
      cntQ.prepare("SELECT COUNT(*) FROM sys_operation_log WHERE " + where);
      if (!searchText.isEmpty())
        cntQ.bindValue(":search", "%" + searchText + "%");
      if (!moduleFilter.isEmpty())
        cntQ.bindValue(":module", moduleFilter);
      cntQ.exec();
      int total = 0;
      if (cntQ.next())
        total = cntQ.value(0).toInt();
      pageState->totalCount = total;
      pageState->totalPages = (total + pageState->pageSize - 1) / pageState->pageSize;
      if (pageState->totalPages < 1)
        pageState->totalPages = 1;
      if (pageState->currentPage > pageState->totalPages)
        pageState->currentPage = pageState->totalPages;
      if (pageState->currentPage < 1)
        pageState->currentPage = 1;

      // 查询当前页
      int offset = (pageState->currentPage - 1) * pageState->pageSize;
      QString sql = "SELECT username, module, operation, ip, operation_time FROM sys_operation_log WHERE " + where;
      sql += " ORDER BY operation_time DESC LIMIT :pageSize OFFSET :offset";
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare(sql);
      if (!searchText.isEmpty())
        q.bindValue(":search", "%" + searchText + "%");
      if (!moduleFilter.isEmpty())
        q.bindValue(":module", moduleFilter);
      q.bindValue(":pageSize", pageState->pageSize);
      q.bindValue(":offset", offset);
      q.exec();
      int row = 0;
      while (q.next())
      {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 4, new QTableWidgetItem(q.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        row++;
      }
      table->setSortingEnabled(true);
      UiKit::syncEmptyHint(table, emptyHint);

      // 更新分页控件
      pageLabel->setText(QStringLiteral("第 %1/%2 页").arg(pageState->currentPage).arg(pageState->totalPages));
      totalLabel->setText(QStringLiteral("共 %1 条").arg(total));
      prevBtn->setEnabled(pageState->currentPage > 1);
      nextBtn->setEnabled(pageState->currentPage < pageState->totalPages);
    };
    loadLogs();
    QObject::connect(searchEdit, &QLineEdit::textChanged, page, [=]()
            { pageState->currentPage = 1; loadLogs(); });
    QObject::connect(moduleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            { pageState->currentPage = 1; loadLogs(); });
    QObject::connect(prevBtn, &QPushButton::clicked, page, [=]()
            {
            if (pageState->currentPage > 1) { pageState->currentPage--; loadLogs(); } });
    QObject::connect(nextBtn, &QPushButton::clicked, page, [=]()
            {
            if (pageState->currentPage < pageState->totalPages) { pageState->currentPage++; loadLogs(); } });
    QObject::connect(pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), page, [=]()
            {
            pageState->pageSize = pageSizeCombo->currentData().toInt();
            pageState->currentPage = 1;
            loadLogs(); });
    QObject::connect(jumpBtn, &QPushButton::clicked, page, [=]()
            {
            bool ok = false;
            int p = jumpEdit->text().toInt(&ok);
            if (ok && p >= 1 && p <= pageState->totalPages) {
                pageState->currentPage = p;
                loadLogs();
            } else {
                QMessageBox::warning(page, QStringLiteral("提示"),
                    QStringLiteral("请输入 1-%1 之间的页码").arg(pageState->totalPages));
            }
            jumpEdit->clear(); });
    QObject::connect(jumpEdit, &QLineEdit::returnPressed, page, [=]()
            {
            bool ok = false;
            int p = jumpEdit->text().toInt(&ok);
            if (ok && p >= 1 && p <= pageState->totalPages) {
                pageState->currentPage = p;
                loadLogs();
            }
            jumpEdit->clear(); });

    layout->addWidget(table);
    layout->addWidget(pageBar);
    layout->addWidget(emptyHint);
    return page;
  }

  // ========================================================================
  // AI 智能问答（保持原有实现）
  // ========================================================================
  if (sub == "ai")
  {
    layout->addWidget(UiKit::createPageHeader(QStringLiteral("ic_robot"), QStringLiteral("智能问答助手"),
                                       QStringLiteral("输入您的问题，智能助手将为您提供解答"), UiKit::moduleColor("ai"), page));
    // API 状态指示（每次进入页面重新加载配置，确保管理员配置后所有用户可见）
    auto &aiSvc = AIService::instance();
    aiSvc.reloadConfig();
    auto *statusLabel = new QLabel(page);
    if (aiSvc.isConfigured())
    {
      QString keyMask = aiSvc.apiKey().trimmed();
      if (keyMask.length() > 10)
        keyMask = keyMask.left(6) + "..." + keyMask.right(4);
      statusLabel->setText(QStringLiteral("API 已配置 | 模型：%1 | Key：%2").arg(aiSvc.model(), keyMask));
      statusLabel->setStyleSheet("color:#15803d;font-size:12px;background:transparent;");
    }
    else
    {
      statusLabel->setText(QStringLiteral("注意：API 未配置，仅使用本地知识库"));
      statusLabel->setStyleSheet("color:#a16207;font-size:12px;background:transparent;");
    }
    layout->addWidget(statusLabel);
    layout->addSpacing(12);

    auto *chatDisplay = new QTextEdit(page);
    chatDisplay->setReadOnly(true);
    chatDisplay->setStyleSheet(R"(
            QTextEdit {
                background: #f5f5f5; border: 1px solid #e2e8f0; border-radius: 4px;
                padding: 12px; font-size: 14px; color: #334155;
            }
        )");
    {
      QString initHtml = QString::fromUtf8(
          "<div style='margin-bottom:12px;'>"
          "<div style='background:#b45309;color:#fff;padding:10px 14px;border-radius:6px;display:inline-block;max-width:80%;'>"
          "\xe6\x82\xa8\xe5\xa5\xbd\xef\xbc\x81\xe6\x88\x91\xe6\x98\xaf\xe6\x99\xba\xe6\x85\xa7\xe7\xa4\xbe\xe5\x8c\xba\xe6\x99\xba\xe8\x83\xbd\xe5\x8a\xa9\xe6\x89\x8b\xef\xbc\x8c\xe6\x9c\x89\xe4\xbb\x80\xe4\xb9\x88\xe5\x8f\xaf\xe4\xbb\xa5\xe5\xb8\xae\xe6\x82\xa8\xe7\x9a\x84\xe5\x90\x97\xef\xbc\x9f</div></div>"
          "<div style='color:#64748b;font-size:12px;'>\xe5\xb0\x9d\xe8\xaf\x95\xe8\xbe\x93\xe5\x85\xa5\xef\xbc\x9a\xe6\x80\x8e\xe4\xb9\x88\xe6\x8a\xa5\xe4\xbf\xae\xe3\x80\x81\xe7\x89\xa9\xe4\xb8\x9a\xe8\xb4\xb9\xe6\x80\x8e\xe4\xb9\x88\xe4\xba\xa4\xe3\x80\x81\xe5\x81\x9c\xe8\xbd\xa6\xe6\x9c\x88\xe5\x8d\xa1\xe3\x80\x81\xe5\xbf\x97\xe6\x84\xbf\xe8\x80\x85\xe3\x80\x81\xe7\xa4\xbe\xe5\x8c\xba\xe5\x9c\xa8\xe5\x93\xaa\xe9\x87\x8c\xe3\x80\x81\xe5\x8a\x9e\xe5\xb1\x85\xe4\xbd\x8f\xe8\xaf\x81</div>");
      chatDisplay->setHtml(initHtml);
    }
    layout->addWidget(chatDisplay, 1);

    auto *inputLayout = new QHBoxLayout();
    auto *inputEdit = new QLineEdit(page);
    inputEdit->setPlaceholderText(QStringLiteral("请输入您的问题..."));
    inputEdit->setStyleSheet("QLineEdit { padding: 10px 14px; border: 1px solid #e2e8f0; border-radius: 4px; font-size: 14px; }"
                             "QLineEdit:focus { border-color: #b45309; }");
    auto *sendBtn = new QPushButton(QStringLiteral("发送"), page);
    sendBtn->setProperty("cssClass", "primary");
    sendBtn->setFixedSize(80, 40);
    sendBtn->setCursor(Qt::PointingHandCursor);
    inputLayout->addWidget(inputEdit, 1);
    inputLayout->addWidget(sendBtn);
    layout->addLayout(inputLayout);

    auto *quickLayout = new QHBoxLayout();
    quickLayout->setSpacing(8);
    auto addQuickQ = [&](const QString &text)
    {
      auto *btn = new QPushButton(text, page);
      btn->setStyleSheet("QPushButton{background:#f0f5ff;color:#b45309;border:1px solid #adc6ff;border-radius:4px;padding:4px 12px;font-size:12px;}"
                         "QPushButton:hover{background:#b45309;color:#fff;}");
      btn->setCursor(Qt::PointingHandCursor);
      // 点击快捷问题后直接填充并发送
      QObject::connect(btn, &QPushButton::clicked, page, [inputEdit, sendBtn, text]()
              {
                inputEdit->setText(text);
                sendBtn->click(); // 直接触发发送
              });
      quickLayout->addWidget(btn);
    };
    addQuickQ(QStringLiteral("怎么报修？"));
    addQuickQ(QStringLiteral("物业费怎么交？"));
    addQuickQ(QStringLiteral("怎么办停车月卡？"));
    addQuickQ(QStringLiteral("怎么成为志愿者？"));
    addQuickQ(QStringLiteral("社区在哪里？"));

    // 管理员可配置系统级 API Key
    const auto &curUser = AuthService::instance().currentUser();
    bool isAdmin = curUser.permissions.contains("*:*:*") || curUser.userType == 3;
    if (isAdmin)
    {
      auto *settingsBtn = new QPushButton(QStringLiteral("API 设置"), page);
      settingsBtn->setStyleSheet("QPushButton{background:#e2e8f0;color:#64748b;border:1px solid #e2e8f0;border-radius:4px;padding:4px 12px;font-size:12px;}"
                                 "QPushButton:hover{border-color:#b45309;color:#b45309;}");
      settingsBtn->setCursor(Qt::PointingHandCursor);
      quickLayout->addWidget(settingsBtn);
      QObject::connect(settingsBtn, &QPushButton::clicked, page, [page]()
              {
                auto& ai = AIService::instance();
                QDialog dlg(page);
                dlg.setWindowTitle(QStringLiteral("AI 服务配置（系统级）"));
                dlg.setMinimumWidth(500);
                auto* dl = new QVBoxLayout(&dlg);
                auto* tipLabel = new QLabel(QStringLiteral("配置 OpenRouter API Key，所有用户共享此配置。\n未配置时仅使用本地知识库回答。"), &dlg);
                tipLabel->setStyleSheet("color:#64748b;font-size:13px;"); tipLabel->setWordWrap(true);
                dl->addWidget(tipLabel); dl->addSpacing(8);
                auto* formLayout = new QFormLayout();
                auto* keyEdit = new QLineEdit(&dlg);
                keyEdit->setText(ai.apiKey());
                keyEdit->setPlaceholderText("sk-or-v1-xxxxx（以 sk-or- 开头）");
                keyEdit->setEchoMode(QLineEdit::Password);
                auto* modelCombo = new QComboBox(&dlg);
                for (const auto& m : AIService::availableModels()) modelCombo->addItem(m);
                modelCombo->setEditable(true);
                modelCombo->setCurrentText(ai.model());
                formLayout->addRow(QStringLiteral("API Key:"), keyEdit);
                formLayout->addRow(QStringLiteral("模型:"), modelCombo);
                dl->addLayout(formLayout);
                auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
                buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("保存"));
                buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
                QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
                QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
                dl->addWidget(buttons);
                if (dlg.exec() == QDialog::Accepted) {
                    QString newKey = keyEdit->text().trimmed();
                    if (!newKey.isEmpty() && !newKey.startsWith("sk-or-")) {
                        QMessageBox::warning(page, QStringLiteral("格式错误"),
                            QStringLiteral("API Key 应以 sk-or- 开头。\n请确认粘贴的是 OpenRouter 的 API Key，不是模型名称。"));
                        return;
                    }
                    ai.setApiKey(newKey);
                    ai.setModel(modelCombo->currentText().trimmed());
                    UiKit::showToast(QStringLiteral("AI 配置已保存到数据库"), page);
                } });
    }
    quickLayout->addStretch();
    layout->addLayout(quickLayout);

    // 发送消息：本地知识库 → LLM API 降级策略
    auto sendMessage = [=]()
    {
      QString question = inputEdit->text().trimmed();
      if (question.isEmpty())
        return;
      chatDisplay->append(QString("<div style='text-align:right;margin:8px 0;'><div style='background:#b45309;color:#fff;padding:8px 12px;border-radius:6px;display:inline-block;max-width:80%;'>%1</div></div>").arg(question));
      inputEdit->clear();
      sendBtn->setEnabled(false);

      // 1. 优先本地知识库
      QSqlQuery searchQ(DatabaseManager::instance().database());
      searchQ.prepare("SELECT id, answer FROM ai_knowledge WHERE status = 0 AND (question LIKE :q1 OR keywords LIKE :q2) ORDER BY hit_count DESC LIMIT 1");
      searchQ.bindValue(":q1", "%" + question + "%");
      searchQ.bindValue(":q2", "%" + question + "%");
      QString localAnswer;
      if (searchQ.exec() && searchQ.next())
      {
        localAnswer = searchQ.value(1).toString();
        qint64 kid = searchQ.value(0).toLongLong();
        QSqlQuery updQ(DatabaseManager::instance().database());
        updQ.prepare("UPDATE ai_knowledge SET hit_count = hit_count + 1 WHERE id = :id");
        updQ.bindValue(":id", kid);
        updQ.exec();
      }
      else
      {
        QStringList keywords = {QStringLiteral("报修"), QStringLiteral("物业"), QStringLiteral("停车"), QStringLiteral("志愿"), QStringLiteral("社区"), QStringLiteral("证件"), QStringLiteral("居住")};
        for (const auto &kw : keywords)
        {
          if (question.contains(kw))
          {
            QSqlQuery fuzzyQ(DatabaseManager::instance().database());
            fuzzyQ.prepare("SELECT answer FROM ai_knowledge WHERE status = 0 AND keywords LIKE :kw ORDER BY hit_count DESC LIMIT 1");
            fuzzyQ.bindValue(":kw", "%" + kw + "%");
            if (fuzzyQ.exec() && fuzzyQ.next())
            {
              localAnswer = fuzzyQ.value(0).toString();
              break;
            }
          }
        }
      }

      if (!localAnswer.isEmpty())
      {
        chatDisplay->append(QString("<div style='margin:8px 0;'><div style='background:#fff;border:1px solid #e2e8f0;padding:8px 12px;border-radius:6px;display:inline-block;max-width:80%;'>%1</div></div>").arg(localAnswer));
        sendBtn->setEnabled(true);
      }
      else if (AIService::instance().isConfigured())
      {
        // 2. 调用 LLM API
        chatDisplay->append(QStringLiteral("<div style='color:#64748b;font-size:12px;margin:4px 0;'>正在思考...</div>"));
        AIService::instance().ask(question, [chatDisplay, sendBtn](const QString &answer, bool success)
                                  {
                    if (success) {
                        chatDisplay->append(QString("<div style='margin:8px 0;'><div style='background:#fff;border:1px solid #e2e8f0;padding:8px 12px;border-radius:6px;display:inline-block;max-width:80%;'>%1</div></div>").arg(answer));
                    } else {
                        chatDisplay->append(QString("<div style='margin:8px 0;'><div style='background:#fff2f0;border:1px solid #ffccc7;padding:8px 12px;border-radius:6px;display:inline-block;max-width:80%;color:#b91c1c;'>%1</div></div>").arg(answer));
                    }
                    sendBtn->setEnabled(true); });
      }
      else
      {
        chatDisplay->append(QString("<div style='margin:8px 0;'><div style='background:#fff;border:1px solid #e2e8f0;padding:8px 12px;border-radius:6px;display:inline-block;max-width:80%;'>%1</div></div>")
                                .arg(QStringLiteral("抱歉，本地知识库暂无匹配答案。<br>如需 LLM 智能问答，请联系管理员在「系统管理 → 智能问答」中配置 API Key。")));
        sendBtn->setEnabled(true);
      }
    };

    QObject::connect(sendBtn, &QPushButton::clicked, page, sendMessage);
    QObject::connect(inputEdit, &QLineEdit::returnPressed, page, sendMessage);
    return page;
  }

  // 默认空页面
  layout->addWidget(emptyHint);
  return page;
}
// === END createSystemPage ===
