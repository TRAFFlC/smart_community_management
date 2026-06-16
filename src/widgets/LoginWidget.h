#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(QWidget* parent = nullptr);

signals:
    void loginSuccess();

private slots:
    void onLogin();

private:
    void setupUI();
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QPushButton* m_loginBtn = nullptr;
};

#endif // LOGINWIDGET_H
