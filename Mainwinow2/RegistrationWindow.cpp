#include "RegistrationWindow.h"
#include "ui_RegistrationWindow.h" // Matches your solution explorer filename
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QCryptographicHash>

RegistrationWindow::RegistrationWindow(QWidget* parent) : QWidget(parent), ui(new Ui::RegistrationWindow)
{
    ui->setupUi(this);
    connect(ui->loginLink, &QLabel::linkActivated, this, &RegistrationWindow::goToLogin);
}

RegistrationWindow::~RegistrationWindow() { delete ui; }

void RegistrationWindow::on_createAccountBtn_clicked()
{
    QString accNum = ui->accNumEdit->text().trimmed();
    QString name = ui->nameEdit->text().trimmed();
    QString pass = ui->passEdit->text();
    QString address = ui->addressEdit->toPlainText().trimmed();

    if (name.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Name and password are required.");
        return;
    }

    // If acc number not provided, generate one based on timestamp
    if (accNum.isEmpty()) {
        accNum = QString::number(QDateTime::currentMSecsSinceEpoch());
    }

    // Check for existing user or account
    QFile fileRead("users.txt");
    if (fileRead.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&fileRead);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.isEmpty()) continue;
            QStringList parts = line.split('|');
            if (parts.size() >= 3) {
                if (parts[0] == name) {
                    QMessageBox::warning(this, "Error", "A user with that name already exists.");
                    fileRead.close();
                    return;
                }
                if (parts[2] == accNum) {
                    QMessageBox::warning(this, "Error", "Generated account number already exists. Try again.");
                    fileRead.close();
                    return;
                }
            }
        }
        fileRead.close();
    }

    // Hash the password with SHA-256 and salt using account number
    QByteArray salted = pass.toUtf8() + accNum.toUtf8();
    QByteArray hashed = QCryptographicHash::hash(salted, QCryptographicHash::Sha256).toHex();

    // Append new user: Name|HashedPass|AccNum|Address|Balance
    QFile fileWrite("users.txt");
    if (!fileWrite.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Unable to open users file for writing.");
        return;
    }

    QTextStream out(&fileWrite);
    out << name << "|" << hashed << "|" << accNum << "|" << address.replace('\n', " ") << "|" << QString::number(0.0, 'f', 2) << "\n";
    fileWrite.close();

    QMessageBox::information(this, "Success", "Account created successfully!");
    emit goToLogin(); // Switch back to login page
}

void RegistrationWindow::on_loginLink_linkActivated(const QString& link)
{
    emit goToLogin();
}
