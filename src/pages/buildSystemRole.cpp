#include "pages/PageFactory.h"
#include "pages/SystemPageHelpers.h"
#include "PagesCommon.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "services/AuthService.h"

namespace PageFactory {

void buildSystemRole(BasePage *page, QVBoxLayout *layout, DatabaseManager &db, QLabel *emptyHint)
{
    auto *table = new QTableWidget(page);
    table->setAlternatingRowColors(true);
    UiKit::configureTable(table);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet(UiKit::TABLE_STYLE);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);

    auto *toolbar = new QWidget(page);
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
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
    addRoleBtn->setStyleSheet(SystemPageHelpers::primaryBtnStyle());
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
    auto openRoleDialog = [page, &db, buildMenuTree, collectCheckedMenus](
                              qint64 roleId, std::function<void()> reload)
    {
        bool isEdit = (roleId > 0);
        QFormLayout *form = nullptr;
        QDialog *dlg = nullptr;
        auto *dlgLayout = SystemPageHelpers::createFormDialog(page, isEdit ? QStringLiteral("编辑角色") : QStringLiteral("新增角色"), form, dlg);

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

        SystemPageHelpers::styleInput(nameEdit);
        SystemPageHelpers::styleInput(keyEdit);
        SystemPageHelpers::styleInput(domainCombo);
        SystemPageHelpers::styleInput(scopeCombo);
        SystemPageHelpers::styleInput(statusCombo2);

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

        SystemPageHelpers::addDialogButtons(dlgLayout, dlg, QStringLiteral("确定"), [=, &db]() -> bool
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
            return true;
        });
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
            auto *editItem = UiKit::createActionItem(QStringLiteral("编辑"), QStringLiteral("#b45309"), rid, rid);
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
}

} // namespace PageFactory
