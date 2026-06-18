#ifndef CHANGE_PASSWORD_DIALOG_H
#define CHANGE_PASSWORD_DIALOG_H

#include <QDialog>

class QWidget;

// 修改密码对话框
// 用法：ChangePasswordDialog::show(parent);
class ChangePasswordDialog {
public:
    // 模态显示对话框，内部完成校验与数据库更新
    static void show(QWidget* parent);
};

#endif // CHANGE_PASSWORD_DIALOG_H
