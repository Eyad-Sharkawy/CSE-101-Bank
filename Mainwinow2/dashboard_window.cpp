#include "dashboard_window.h"
#include "ui_dashboard_window.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include <QPushButton>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QDateEdit>
#include <QComboBox>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QBrush>
#include <QColor>

DashboardWindow::DashboardWindow(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::DashboardWindow),
    watcher(new QFileSystemWatcher(this))
{
    ui->setupUi(this);

    // Connect Buttons
    connect(ui->btnWithdraw, &QPushButton::clicked, this, &DashboardWindow::on_btnWithdraw_clicked);
    connect(ui->btnDeposit, &QPushButton::clicked, this, &DashboardWindow::on_btnDeposit_clicked);
    connect(ui->btnTransfer, &QPushButton::clicked, this, &DashboardWindow::on_btnTransfer_clicked);
    connect(ui->btnModify, &QPushButton::clicked, this, &DashboardWindow::on_btnModify_clicked);
    connect(ui->logoutBtn, &QPushButton::clicked, this, &DashboardWindow::on_logoutBtn_clicked);

    // Style the table
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make read-only
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Initialize UI filter widgets
    ui->fromDateEdit->setCalendarPopup(true);
    ui->fromDateEdit->setDisplayFormat("yyyy-MM-dd");
    ui->fromDateEdit->setDate(QDate::currentDate().addMonths(-1));

    ui->toDateEdit->setCalendarPopup(true);
    ui->toDateEdit->setDisplayFormat("yyyy-MM-dd");
    ui->toDateEdit->setDate(QDate::currentDate());

    ui->typeCombo->addItem("All");
    ui->typeCombo->addItem("Deposit");
    ui->typeCombo->addItem("Withdraw");
    ui->typeCombo->addItem("Transfer");

    connect(ui->applyFilterBtn, &QPushButton::clicked, this, &DashboardWindow::applyFilters);
    connect(ui->exportCsvBtn, &QPushButton::clicked, this, &DashboardWindow::exportCsv);

    // Ensure the table has columns
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Date" << "Type" << "Amount");

    // Set up file system watcher to watch transactions.txt
    QString txPath = QDir::current().filePath("transactions.txt");
    if (!QFile::exists(txPath)) {
        // create empty file so watcher can monitor it
        QFile temp(txPath);
        temp.open(QIODevice::WriteOnly);
        temp.close();
    }
    watcher->addPath(txPath);
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &DashboardWindow::onTransactionsFileChanged);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &DashboardWindow::onTransactionsDirChanged);
}

DashboardWindow::~DashboardWindow() { delete ui; }

QString DashboardWindow::getCurrentUser() const { return currentUser; }

// Helper to add a formatted row to the table
static void addTransactionRow(QTableWidget* table, const QString& dateStr, const QString& type, double amount)
{
    int row = table->rowCount();
    table->insertRow(row);

    // Date
    QTableWidgetItem* dateItem = new QTableWidgetItem(dateStr);
    dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable);
    // Make date text slightly darker for contrast
    dateItem->setForeground(QBrush(QColor(51, 51, 51))); // #333333
    table->setItem(row, 0, dateItem);

    // Type
    QTableWidgetItem* typeItem = new QTableWidgetItem(type);
    typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
    typeItem->setForeground(QBrush(QColor(51, 51, 51))); // #333333
    table->setItem(row, 1, typeItem);

    // Amount with sign and color
    QString amtText = (amount < 0 ? "- $" : "+ $") + QString::number(qAbs(amount), 'f', 2);
    QTableWidgetItem* amtItem = new QTableWidgetItem(amtText);
    amtItem->setFlags(amtItem->flags() & ~Qt::ItemIsEditable);
    amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (amount < 0) {
        amtItem->setForeground(QBrush(QColor(220, 53, 69))); // red
    } else {
        amtItem->setForeground(QBrush(QColor(40, 167, 69))); // green
    }
    table->setItem(row, 2, amtItem);
}

// --- 1. LOAD USER & HISTORY ---
void DashboardWindow::setUsername(QString username)
{
    currentUser = username;
    ui->welcomeLabel->setText("Welcome, " + username);
    ui->transTitle->setText("Recent Transactions");

    // Read Balance from users.txt
    QFile file("users.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split("|");

            if (parts.length() >= 5 && parts[0] == currentUser) {
                QString balance = parts[4];
                ui->balanceValue_1->setText("$" + balance);
                break;
            }
        }
        file.close();
    }

    // Load the Table from transactions.txt
    loadTransactionHistory();
}

// --- 2. BUTTONS ---
void DashboardWindow::on_btnWithdraw_clicked() {
    bool ok;
    double amount = QInputDialog::getDouble(this, "Withdraw", "Amount:", 0, 0, 100000, 2, &ok);
    if (ok && amount > 0) updateBalance(-amount, "Withdraw");
}

void DashboardWindow::on_btnDeposit_clicked() {
    bool ok;
    double amount = QInputDialog::getDouble(this, "Deposit", "Amount:", 0, 0, 100000, 2, &ok);
    if (ok && amount > 0) updateBalance(amount, "Deposit");
}

void DashboardWindow::on_btnTransfer_clicked() {
    bool ok;
    QString recipient = QInputDialog::getText(this, "Transfer", "Recipient (username or account number):", QLineEdit::Normal, QString(), &ok);
    if (!ok || recipient.trimmed().isEmpty()) return;

    double amount = QInputDialog::getDouble(this, "Transfer", "Amount:", 0, 0, 100000, 2, &ok);
    if (ok && amount > 0) {
        transferTo(recipient.trimmed(), amount);
    }
}

// --- 3. UPDATE BALANCE (LOGIC) ---
void DashboardWindow::updateBalance(double change, const QString &type)
{
    QStringList allLines;
    bool found = false;
    double newBal = 0.0;

    // A. Update users.txt
    QFile fileRead("users.txt");
    if (fileRead.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&fileRead);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split("|");

            if (parts.length() >= 5 && parts[0] == currentUser) {
                double currentBal = parts[4].toDouble();
                if (currentBal + change < 0) {
                    QMessageBox::warning(this, "Error", "Insufficient Funds!");
                    fileRead.close();
                    return;
                }
                newBal = currentBal + change;
                parts[4] = QString::number(newBal, 'f', 2);
                line = parts.join("|");
                found = true;
            }
            allLines.append(line);
        }
        fileRead.close();
    }

    if (found) {
        QFile fileWrite("users.txt");
        if (fileWrite.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&fileWrite);
            for (const QString& line : allLines) {
                out << line << "\n";
            }
            fileWrite.close();
        }

        // Update Label
        ui->balanceValue_1->setText("$" + QString::number(newBal, 'f', 2));

        // Save to transactions.txt
        saveTransaction(type, change);
    }
}

// --- 4. SAVE TO TRANSACTION.TXT ---
void DashboardWindow::saveTransaction(const QString &type, double amount)
{
    QFile file("transactions.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);

        // FORMAT: User | Date | Type | Amount
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
        out << currentUser << "|" << date << "|" << type << "|" << QString::number(amount, 'f', 2) << "\n";

        file.close();

        // Reload table immediately to show new row
        loadTransactionHistory();
    }
}

// --- 5. LOAD FROM TRANSACTION.TXT TO TABLE ---
void DashboardWindow::loadTransactionHistory()
{
    ui->tableWidget->setRowCount(0); // Clear old rows

    QFile file("transactions.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split("|");

            // Format check: User | Date | Type | Amount
            if (parts.length() >= 4 && parts[0] == currentUser) {
                // Format date to human readable form
                QDateTime dt = QDateTime::fromString(parts[1], "yyyy-MM-dd HH:mm");
                QString dateStr = dt.isValid() ? dt.toString("MMMM d, yyyy") : parts[1];
                double amt = parts[3].toDouble();
                addTransactionRow(ui->tableWidget, dateStr, parts[2], amt);
            }
        }
        file.close();
    }
}

void DashboardWindow::on_logoutBtn_clicked() { emit logout(); }
void DashboardWindow::on_btnModify_clicked() { emit goToModify(); }

void DashboardWindow::applyFilters()
{
    // Reload transactions and apply filters in-memory
    QDate from = ui->fromDateEdit->date();
    QDate to = ui->toDateEdit->date();
    QString typeFilter = ui->typeCombo->currentText();

    ui->tableWidget->setRowCount(0);

    QFile file("transactions.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList parts = line.split("|");
        if (parts.size() < 4) continue;
        if (parts[0] != currentUser) continue;

        QDateTime dt = QDateTime::fromString(parts[1], "yyyy-MM-dd HH:mm");
        if (!dt.isValid()) continue;
        QDate d = dt.date();
        if (d < from || d > to) continue;

        QString ttype = parts[2];
        if (typeFilter != "All" && ttype != typeFilter) continue;

        QString dateStr = dt.toString("MMMM d, yyyy");
        double amt = parts[3].toDouble();
        addTransactionRow(ui->tableWidget, dateStr, ttype, amt);
    }
    file.close();
}

void DashboardWindow::exportCsv()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Transactions", QString("transactions_%1.csv").arg(currentUser), "CSV Files (*.csv);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile inFile("transactions.txt");
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Failed", "Cannot open transactions.txt for reading.");
        return;
    }

    QFile outFile(fileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Failed", "Cannot create export file.");
        inFile.close();
        return;
    }

    QTextStream in(&inFile);
    QTextStream out(&outFile);

    // Write CSV header
    out << "Date,Type,Amount\n";

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList parts = line.split('|');
        if (parts.size() < 4) continue;
        if (parts[0] != currentUser) continue;

        // CSV: date,type,amount
        QString date = parts[1];
        QString type = parts[2];
        QString amount = parts[3];
        out << "\"" << date << "\",\"" << type << "\",\"" << amount << "\"\n";
    }

    inFile.close();
    outFile.close();

    QMessageBox::information(this, "Export Complete", "Transactions exported to " + fileName);
}

void DashboardWindow::onTransactionsFileChanged(const QString& path)
{
    Q_UNUSED(path);
    // Re-add path to watcher because some editors replace file rather than modify
    QString txPath = QDir::current().filePath("transactions.txt");
    if (!watcher->files().contains(txPath)) watcher->addPath(txPath);

    // Reload and reapply filters
    applyFilters();
}

void DashboardWindow::onTransactionsDirChanged(const QString& path)
{
    Q_UNUSED(path);
    QString txPath = QDir::current().filePath("transactions.txt");
    if (!watcher->files().contains(txPath) && QFile::exists(txPath)) watcher->addPath(txPath);

    // Reload and reapply filters
    applyFilters();
}

void DashboardWindow::transferTo(const QString &recipientIdentifier, double amount)
{
    QString sender = currentUser;
    if (recipientIdentifier == sender) {
        QMessageBox::warning(this, "Transfer Error", "Cannot transfer to yourself.");
        return;
    }

    // Read all users into memory
    QVector<QStringList> users;
    QFile fileRead("users.txt");
    if (!fileRead.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Transfer Error", "Unable to open users file.");
        return;
    }

    QTextStream in(&fileRead);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;
        QStringList parts = line.split('|');
        users.append(parts);
    }
    fileRead.close();

    int senderIdx = -1;
    int recIdx = -1;
    for (int i = 0; i < users.size(); ++i) {
        const QStringList &parts = users[i];
        if (parts.size() < 5) continue;
        if (parts[0] == sender) senderIdx = i;
        if (parts[0] == recipientIdentifier || parts[2] == recipientIdentifier) recIdx = i;
    }

    if (senderIdx == -1) {
        QMessageBox::warning(this, "Transfer Error", "Sender not found.");
        return;
    }
    if (recIdx == -1) {
        QMessageBox::warning(this, "Transfer Error", "Recipient not found.");
        return;
    }

    double senderBal = users[senderIdx][4].toDouble();
    double recBal = users[recIdx][4].toDouble();
    if (senderBal < amount) {
        QMessageBox::warning(this, "Transfer Error", "Insufficient funds.");
        return;
    }

    // Update balances
    senderBal -= amount;
    recBal += amount;
    users[senderIdx][4] = QString::number(senderBal, 'f', 2);
    users[recIdx][4] = QString::number(recBal, 'f', 2);

    // Write back all users
    QFile fileWrite("users.txt");
    if (!fileWrite.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Transfer Error", "Unable to write users file.");
        return;
    }
    QTextStream out(&fileWrite);
    for (const QStringList &parts : users) {
        out << parts.join('|') << "\n";
    }
    fileWrite.close();

    // Record transactions for both users
    QFile txFile("transactions.txt");
    if (txFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream tout(&txFile);
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
        QString recName = users[recIdx][0];
        QString senderType = QString("Transfer to %1").arg(recName);
        QString recType = QString("Transfer from %1").arg(sender);

        tout << sender << "|" << date << "|" << senderType << "|" << QString::number(-amount, 'f', 2) << "\n";
        tout << recName << "|" << date << "|" << recType << "|" << QString::number(amount, 'f', 2) << "\n";
        txFile.close();
    }

    // Update UI balance and transaction list
    ui->balanceValue_1->setText("$" + QString::number(senderBal, 'f', 2));
    loadTransactionHistory();
    QMessageBox::information(this, "Transfer Complete", QString("Transferred $%1 to %2").arg(QString::number(amount, 'f', 2), users[recIdx][0]));
}