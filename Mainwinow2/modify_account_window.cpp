#include "modify_account_window.h"
#include "ui_modify_account_window.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QCryptographicHash>

ModifyAccountWindow::ModifyAccountWindow(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ModifyAccountWindow)
{
    ui->setupUi(this);
}

ModifyAccountWindow::~ModifyAccountWindow()
{
    delete ui;
}

void ModifyAccountWindow::loadUser(const QString& username)
{
    currentUser = username;

    QFile file("users.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split("|");

            // Format: Name|HashedPass|AccNum|Address|Balance
            if (parts.length() >= 5 && parts[0] == currentUser) {
                ui->nameEdit->setText(parts[0]);
                // Do not display hashed password; leave passEdit empty for user to enter new password if desired
                ui->passEdit->setText("");
                ui->accNumEdit->setText(parts[2]);
                // addressEdit is a QLineEdit in the UI, use setText
                ui->addressEdit->setText(parts[3]);
                break;
            }
        }
        file.close();
    }
}

// Helper to save user data to file
bool ModifyAccountWindow::saveUser(const QString& name, const QString& pass, const QString& accNum, const QString& address)
{
    QStringList lines;
    bool found = false;

    QFile fileRead("users.txt");
    if (!fileRead.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&fileRead);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split("|");

        if (parts.length() >= 5 && parts[0] == currentUser) {
            // If password field is empty, keep existing hashed password
            QString hashedPass = parts[1];
            if (!pass.isEmpty()) {
                // Hash new password with accNum
                QByteArray salted = pass.toUtf8() + accNum.toUtf8();
                hashedPass = QCryptographicHash::hash(salted, QCryptographicHash::Sha256).toHex();
            }

            parts[0] = name;
            parts[1] = hashedPass;
            parts[2] = accNum;
            parts[3] = address;
            line = parts.join("|");
            found = true;
            currentUser = name; // update cache
        }
        lines.append(line);
    }
    fileRead.close();

    if (!found)
        return false;

    QFile fileWrite("users.txt");
    if (!fileWrite.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&fileWrite);
    for (const QString& line : lines) {
        out << line << "\n";
    }
    fileWrite.close();
    return true;
}

// 2. SAVE CHANGES BUTTON
void ModifyAccountWindow::on_saveBtn_clicked()
{
    QString name = ui->nameEdit->text().trimmed();
    QString pass = ui->passEdit->text();
    QString accNum = ui->accNumEdit->text();
    QString address = ui->addressEdit->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Name cannot be empty.");
        return;
    }

    if (saveUser(name, pass, accNum, address)) {
        QMessageBox::information(this, "Success", "Account Updated Successfully!");
        emit backToDashboard();
    }
    else {
        QMessageBox::warning(this, "Error", "Failed to save user. User not found or file error.");
    }
}