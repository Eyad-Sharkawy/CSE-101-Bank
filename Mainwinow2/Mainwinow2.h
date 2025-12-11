#pragma once

#include <QtWidgets/QMainWindow>
#include <QStackedWidget>

// Include all our pages
#include "login_window.h"
#include "RegistrationWindow.h"
#include "dashboard_window.h"
#include "modify_account_window.h"

class Mainwinow2 : public QMainWindow
{
    Q_OBJECT // Correct usage: macro, not a member variable

public:
    Mainwinow2(QWidget* parent = nullptr);
    ~Mainwinow2();

private slots:
    // Slots to handle page switching
    void showRegister();
    void showLogin();
    void showDashboard(QString username);
    void showModify();
    void handleLogout();

private:
    QStackedWidget* stackedWidget;

    // Pointers to our pages
    LoginWindow* loginPage;
    RegistrationWindow* registerPage;
    DashboardWindow* dashboardPage;
    ModifyAccountWindow* modifyPage;
};