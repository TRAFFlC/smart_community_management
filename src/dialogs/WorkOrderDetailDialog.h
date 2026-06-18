#ifndef WORK_ORDER_DETAIL_DIALOG_H
#define WORK_ORDER_DETAIL_DIALOG_H

#include <QDialog>
#include <QtGlobal>

class QWidget;

// 工单详情对话框
// 用法：WorkOrderDetailDialog::show(parent, workOrderId);
class WorkOrderDetailDialog {
public:
    static void show(QWidget* parent, qint64 workOrderId);
};

#endif // WORK_ORDER_DETAIL_DIALOG_H
