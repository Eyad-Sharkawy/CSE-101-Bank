#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

namespace Ui { class LoginWindow; }

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);
    ~LoginWindow();

signals:
    void loginSuccess(QString username); // Signal to tell Main Window we logged in
    void goToRegister();                 // Signal to switch to Register screen

private slots:
    void on_loginBtn_clicked();
    void on_registerLabel_linkActivated(const QString& link); // For the link
    void on_showPassBtn_toggled(bool checked); // toggle password visibility

private:
    Ui::LoginWindow* ui;
};