#ifndef PROFILE_DIALOG_H
#define PROFILE_DIALOG_H

#include <QDialog>

class QWidget;

// 个人中心对话框
// 用法：ProfileDialog::show(parent);
class ProfileDialog {
public:
    static void show(QWidget* parent);
};

#endif // PROFILE_DIALOG_H
