#include "login_window.h"
#include "ui_LoginWindow.h" 
#include <QMessageBox>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>

LoginWindow::LoginWindow(QWidget* parent) : QWidget(parent), ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    // Connect the register button to the navigation signal
    connect(ui->registerBtn, &QPushButton::clicked, this, &LoginWindow::goToRegister);

    // Connect show/hide password toggle (checkable button)
    connect(ui->showPassBtn, &QPushButton::toggled, this, &LoginWindow::on_showPassBtn_toggled);
}

LoginWindow::~LoginWindow() { delete ui; }

void LoginWindow::on_loginBtn_clicked()
{
    QString userInput = ui->userEdit->text().trimmed();
    QString pass = ui->passEdit->text();

    if (userInput.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Login Failed", "Please enter username/account number and password.");
        return;
    }

    QFile file("users.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // If file doesn't exist or can't be opened, fall back to admin check
        if (userInput == "admin" && pass == "admin") {
            emit loginSuccess(userInput);
            return;
        }
        QMessageBox::warning(this, "Login Failed", "Unable to open users file and credentials are not admin/admin.");
        return;
    }

    QTextStream in(&file);
    bool found = false;
    QString foundName;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList parts = line.split('|');
        if (parts.size() < 3) continue; // malformed line

        QString name = parts[0];
        QString storedHash = parts[1];
        QString accNum = parts[2];

        // Allow login by username or account number
        if (userInput == name || userInput == accNum) {
            // Hash entered password with salt = accNum
            QByteArray salted = pass.toUtf8() + accNum.toUtf8();
            QByteArray hashed = QCryptographicHash::hash(salted, QCryptographicHash::Sha256).toHex();
            if (hashed == storedHash.toUtf8()) {
                found = true;
                foundName = name;
                break;
            }
        }
    }
    file.close();

    if (found) {
        emit loginSuccess(foundName);
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username/account number or password.");
    }
}

void LoginWindow::on_registerLabel_linkActivated(const QString& link)
{
    emit goToRegister();
}

void LoginWindow::on_showPassBtn_toggled(bool checked)
{
    if (checked) {
        ui->passEdit->setEchoMode(QLineEdit::Normal);
    } else {
        ui->passEdit->setEchoMode(QLineEdit::Password);
    }
}