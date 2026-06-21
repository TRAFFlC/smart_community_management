#include "pages/PageFactory.h"
#include "pages/SystemPageHelpers.h"
#include "PagesCommon.h"
#include "services/AuthService.h"

namespace PageFactory {

void buildSystemUser(BasePage *page, QVBoxLayout *layout, DatabaseManager &db, QLabel *emptyHint)
{
    auto *table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    UiKit::configureTable(table);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(UiKit::TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    // Toolbar
    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
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
    addUserBtn->setStyleSheet(SystemPageHelpers::primaryBtnStyle());
    addUserBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(addUserBtn);
    layout->addWidget(toolbar);

    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({QStringLiteral("用户名"), QStringLiteral("真实姓名"),
                                      QStringLiteral("手机号"), QStringLiteral("类型"), QStringLiteral("状态"),
                                      QStringLiteral("角色"), QStringLiteral("操作")});
    table->setColumnWidth(6, 300);

    // 用户新增/编辑对话框
    auto openUserDialog = [page, &db](qint64 userId, std::function<void()> reload)
    {
        bool isEdit = (userId > 0);
        QFormLayout *form = nullptr;
        QDialog *dlg = nullptr;
        auto *dlgLayout = SystemPageHelpers::createFormDialog(page, isEdit ? QStringLiteral("编辑用户") : QStringLiteral("新增用户"), form, dlg);

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

        SystemPageHelpers::styleInput(usernameEdit);
        SystemPageHelpers::styleInput(realNameEdit);
        SystemPageHelpers::styleInput(phoneEdit);
        SystemPageHelpers::styleInput(userTypeCombo);
        SystemPageHelpers::styleInput(passwordEdit);
        SystemPageHelpers::styleInput(roleCombo);
        SystemPageHelpers::styleInput(statusCombo2);

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

        SystemPageHelpers::addDialogButtons(dlgLayout, dlg, QStringLiteral("确定"), [=, &db]() -> bool
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
            return true;
        });
        dlg->exec();
        delete dlg;
    };

    // 重置密码对话框
    auto resetPassword = [page, &db](qint64 userId, const QString &username, std::function<void()> reload)
    {
        QFormLayout *form = nullptr;
        QDialog *dlg = nullptr;
        auto *dlgLayout = SystemPageHelpers::createFormDialog(page, QStringLiteral("重置密码 - %1").arg(username), form, dlg);
        auto *pwdEdit = new QLineEdit(dlg);
        pwdEdit->setEchoMode(QLineEdit::Password);
        pwdEdit->setPlaceholderText(QStringLiteral("请输入新密码（至少6位）"));
        auto *pwdConfirmEdit = new QLineEdit(dlg);
        pwdConfirmEdit->setEchoMode(QLineEdit::Password);
        pwdConfirmEdit->setPlaceholderText(QStringLiteral("请再次输入新密码"));
        SystemPageHelpers::styleInput(pwdEdit);
        SystemPageHelpers::styleInput(pwdConfirmEdit);
        form->addRow(QStringLiteral("新密码 *"), pwdEdit);
        form->addRow(QStringLiteral("确认密码 *"), pwdConfirmEdit);

        SystemPageHelpers::addDialogButtons(dlgLayout, dlg, QStringLiteral("确认重置"), [=, &db]() -> bool
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
            return true;
        });
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
    loadUsers = [table, searchEdit, statusCombo, emptyHint, &openUserDialog, &resetPassword, &toggleUserStatus, &loadUsers, page]()
    {
        table->setSortingEnabled(false);
        table->setRowCount(0);
        QString sql = "SELECT u.id, u.username, u.real_name, u.phone, u.user_type, u.status, "
                      "GROUP_CONCAT(r.role_name) AS role_names "
                      "FROM sys_user u "
                      "LEFT JOIN sys_user_role ur ON u.id = ur.user_id "
                      "LEFT JOIN sys_role r ON r.id = ur.role_id AND r.del_flag = 0 "
                      "WHERE u.del_flag = 0";
        QString searchText = searchEdit->text().trimmed();
        int statusFilter = statusCombo->currentData().toInt();
        if (!searchText.isEmpty())
            sql += " AND (u.username LIKE :search OR u.real_name LIKE :search)";
        if (statusFilter >= 0)
            sql += " AND u.status = :status";
        sql += " GROUP BY u.id ORDER BY u.id";
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
            table->setItem(row, 5, new QTableWidgetItem(q.value(6).toString()));

            // 操作列：下拉菜单按钮
            auto *opsBtn = new QPushButton(QStringLiteral("编辑 ▾"), table);
            opsBtn->setStyleSheet(QStringLiteral(
                "QPushButton{border:none; color:#b45309; font-size:12px; font-weight:500; background:transparent; padding:2px 6px;}"
                "QPushButton:hover{text-decoration:underline;}"));
            opsBtn->setCursor(Qt::PointingHandCursor);
            QObject::connect(opsBtn, &QPushButton::clicked, page, [page, openUserDialog, toggleUserStatus, resetPassword, loadUsers, userId, userSts, username]() {
                QMenu menu(page);
                auto *editAct = menu.addAction(QStringLiteral("编辑用户"));
                auto *toggleAct = menu.addAction(userSts == 0 ? QStringLiteral("禁用用户") : QStringLiteral("启用用户"));
                auto *resetAct = menu.addAction(QStringLiteral("重置密码"));
                QAction *chosen = menu.exec(QCursor::pos());
                if (chosen == editAct) openUserDialog(userId, loadUsers);
                else if (chosen == toggleAct) toggleUserStatus(userId, userSts, username, loadUsers);
                else if (chosen == resetAct) resetPassword(userId, username, loadUsers);
            });
            table->setCellWidget(row, 6, UiKit::createActionCell({opsBtn}, table));
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

    layout->addWidget(table);
    layout->addWidget(emptyHint);
}

} // namespace PageFactory
