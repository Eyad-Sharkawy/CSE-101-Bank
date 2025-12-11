#include "Mainwinow2.h"

Mainwinow2::Mainwinow2(QWidget* parent)
    : QMainWindow(parent)
{
    // Setup Stack
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    this->resize(1024, 768);

    // Create Pages
    loginPage = new LoginWindow();
    registerPage = new RegistrationWindow();
    dashboardPage = new DashboardWindow();
    modifyPage = new ModifyAccountWindow();

    // Add to Stack
    stackedWidget->addWidget(loginPage);       // Index 0
    stackedWidget->addWidget(registerPage);    // Index 1
    stackedWidget->addWidget(dashboardPage);   // Index 2
    stackedWidget->addWidget(modifyPage);      // Index 3

    // --- CONNECTIONS ---

    // 1. Login -> Register
    connect(loginPage, &LoginWindow::goToRegister, [=]() {
        stackedWidget->setCurrentWidget(registerPage);
    });

    // 2. Login -> Dashboard
    connect(loginPage, &LoginWindow::loginSuccess, this, &Mainwinow2::showDashboard);

    // 3. Register -> Login
    connect(registerPage, &RegistrationWindow::goToLogin, [=]() {
        stackedWidget->setCurrentWidget(loginPage);
    });

    // 4. Dashboard -> Modify
    connect(dashboardPage, &DashboardWindow::goToModify, this, &Mainwinow2::showModify);

    // 5. Dashboard -> Logout
    connect(dashboardPage, &DashboardWindow::logout, [=]() {
        stackedWidget->setCurrentWidget(loginPage);
    });

    // 6. Modify -> Dashboard
    connect(modifyPage, &ModifyAccountWindow::backToDashboard, [=]() {
        // Refresh balance/details on dashboard
        dashboardPage->setUsername(dashboardPage->getCurrentUser());
        stackedWidget->setCurrentWidget(dashboardPage);
    });

    stackedWidget->setCurrentWidget(loginPage);
}

Mainwinow2::~Mainwinow2() {}

void Mainwinow2::showRegister()
{
    stackedWidget->setCurrentWidget(registerPage);
}

void Mainwinow2::showLogin()
{
    stackedWidget->setCurrentWidget(loginPage);
}

void Mainwinow2::showDashboard(QString username)
{
    dashboardPage->setUsername(username);
    stackedWidget->setCurrentWidget(dashboardPage);
}

void Mainwinow2::showModify()
{
    modifyPage->loadUser(dashboardPage->getCurrentUser());
    stackedWidget->setCurrentWidget(modifyPage);
}

void Mainwinow2::handleLogout()
{
    stackedWidget->setCurrentWidget(loginPage);
}