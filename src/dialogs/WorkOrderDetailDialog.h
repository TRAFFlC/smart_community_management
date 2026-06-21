#ifndef WORK_ORDER_DETAIL_DIALOG_H
#define WORK_ORDER_DETAIL_DIALOG_H

#include <QDialog>
#include <QString>
#include <QtGlobal>

class QWidget;
class QHBoxLayout;

namespace WorkOrderDetailActions {
    void setupActionButtons(QDialog& dlg, QHBoxLayout* btnRow,
                            qint64 workOrderId, int status,
                            qint64 reporterId, qint64 assignTo,
                            const QString& title);
}

// 工单详情对话框
// 用法：WorkOrderDetailDialog::show(parent, workOrderId);
class WorkOrderDetailDialog {
public:
    static void show(QWidget* parent, qint64 workOrderId);
};

#endif // WORK_ORDER_DETAIL_DIALOG_H
