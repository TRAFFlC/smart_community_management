#ifndef SYSTEMPAGEHELPERS_H
#define SYSTEMPAGEHELPERS_H

#include <QString>
#include <functional>

class QWidget;
class QVBoxLayout;
class QFormLayout;
class QDialog;

namespace SystemPageHelpers {

QVBoxLayout *createFormDialog(QWidget *parent, const QString &title,
                              QFormLayout *&outForm, QDialog *&outDlg);

void styleInput(QWidget *w);

void addDialogButtons(QVBoxLayout *dlgLayout, QDialog *dlg, const QString &okText,
                      std::function<bool()> onOk);

QString primaryBtnStyle();

} // namespace SystemPageHelpers

#endif // SYSTEMPAGEHELPERS_H
