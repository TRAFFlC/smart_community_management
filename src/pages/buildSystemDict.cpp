#include "pages/PageFactory.h"
#include "pages/SystemPageHelpers.h"
#include "PagesCommon.h"
#include "services/AuthService.h"

namespace PageFactory {

void buildSystemDict(BasePage *page, QVBoxLayout *layout, DatabaseManager &db, QLabel *emptyHint)
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
    addTypeBtn->setStyleSheet(SystemPageHelpers::primaryBtnStyle());
    addTypeBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(addTypeBtn);
    auto *addItemBtn = new QPushButton(QStringLiteral("+ 新增字典项"), toolbar);
    addItemBtn->setStyleSheet(SystemPageHelpers::primaryBtnStyle());
    addItemBtn->setCursor(Qt::PointingHandCursor);
    tbLayout->addWidget(addItemBtn);
    layout->addWidget(toolbar);

    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({QStringLiteral("字典类型"), QStringLiteral("类型名称"),
                                      QStringLiteral("字典标签"), QStringLiteral("字典值"), QStringLiteral("排序"), QStringLiteral("操作")});
    table->setColumnWidth(5, 160);

    // 字典类型新增/编辑对话框
    auto openDictTypeDialog = [page, &db](qint64 typeId, std::function<void()> reload)
    {
        bool isEdit = (typeId > 0);
        QFormLayout *form = nullptr;
        QDialog *dlg = nullptr;
        auto *dlgLayout = SystemPageHelpers::createFormDialog(page, isEdit ? QStringLiteral("编辑字典类型") : QStringLiteral("新增字典类型"), form, dlg);
        auto *nameEdit = new QLineEdit(dlg);
        auto *typeEdit = new QLineEdit(dlg);
        auto *statusCombo2 = new QComboBox(dlg);
        statusCombo2->addItem(QStringLiteral("正常"), 0);
        statusCombo2->addItem(QStringLiteral("禁用"), 1);
        auto *remarkEdit = new QTextEdit(dlg);
        remarkEdit->setFixedHeight(80);
        SystemPageHelpers::styleInput(nameEdit);
        SystemPageHelpers::styleInput(typeEdit);
        SystemPageHelpers::styleInput(statusCombo2);
        SystemPageHelpers::styleInput(remarkEdit);

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

        SystemPageHelpers::addDialogButtons(dlgLayout, dlg, QStringLiteral("确定"), [=, &db]() -> bool
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
            return true;
        });
        dlg->exec();
        delete dlg;
    };

    // 字典项新增/编辑对话框
    auto openDictItemDialog = [page, &db, dictTypeCombo](qint64 itemId, std::function<void()> reload)
    {
        bool isEdit = (itemId > 0);
        QFormLayout *form = nullptr;
        QDialog *dlg = nullptr;
        auto *dlgLayout = SystemPageHelpers::createFormDialog(page, isEdit ? QStringLiteral("编辑字典项") : QStringLiteral("新增字典项"), form, dlg);
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
        SystemPageHelpers::styleInput(typeCombo);
        SystemPageHelpers::styleInput(labelEdit);
        SystemPageHelpers::styleInput(valueEdit);
        SystemPageHelpers::styleInput(sortSpin);
        SystemPageHelpers::styleInput(statusCombo2);

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

        SystemPageHelpers::addDialogButtons(dlgLayout, dlg, QStringLiteral("确定"), [=, &db]() -> bool
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
            return true;
        });
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
            auto *editItem = UiKit::createActionItem(QStringLiteral("编辑"), QStringLiteral("#b45309"), itemId, itemId);
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
}

} // namespace PageFactory
