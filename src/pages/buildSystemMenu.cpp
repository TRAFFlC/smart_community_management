#include "pages/PageFactory.h"
#include "pages/SystemPageHelpers.h"
#include "PagesCommon.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "services/AuthService.h"

namespace PageFactory {

void buildSystemMenu(BasePage *page, QVBoxLayout *layout, DatabaseManager &db, QLabel *emptyHint)
{
    (void)emptyHint; // 菜单树不使用空状态提示

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
    toolbar->setStyleSheet("background:#ffffff; border-radius:2px; padding:4px 8px; border:1px solid #D4D0C8;");
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 4, 4, 4);
    tbLayout->addStretch();
    auto *addMenuBtn = new QPushButton(QStringLiteral("+ 新增菜单"), toolbar);
    addMenuBtn->setStyleSheet(SystemPageHelpers::primaryBtnStyle());
    addMenuBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(addMenuBtn);
    layout->addWidget(toolbar);

    // 菜单新增/编辑对话框
    auto openMenuDialog = [page, &db](qint64 menuId, std::function<void()> reload)
    {
        bool isEdit = (menuId > 0);
        QFormLayout *form = nullptr;
        QDialog *dlg = nullptr;
        auto *dlgLayout = SystemPageHelpers::createFormDialog(page, isEdit ? QStringLiteral("编辑菜单") : QStringLiteral("新增菜单"), form, dlg);

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

        SystemPageHelpers::styleInput(parentCombo);
        SystemPageHelpers::styleInput(nameEdit);
        SystemPageHelpers::styleInput(pathEdit);
        SystemPageHelpers::styleInput(iconEdit);
        SystemPageHelpers::styleInput(sortSpin);
        SystemPageHelpers::styleInput(typeCombo);
        SystemPageHelpers::styleInput(statusCombo2);
        SystemPageHelpers::styleInput(permEdit);

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

        SystemPageHelpers::addDialogButtons(dlgLayout, dlg, QStringLiteral("确定"), [=, &db]() -> bool
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
            return true;
        });
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
}

} // namespace PageFactory
