#pragma once
#include <QWidget>
#include <QString>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFileSystemWatcher>

namespace Ui { class DashboardWindow; }

class DashboardWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWindow(QWidget* parent = nullptr);
    ~DashboardWindow();

    QString getCurrentUser() const; // Getter for current user

public slots:
    void setUsername(QString name); // To update the "Welcome, User" label

signals:
    void logout();
    void goToModify();

private slots:
    void on_logoutBtn_clicked();
    void on_btnModify_clicked(); // The dark blue gear button
    void on_btnWithdraw_clicked();
    void on_btnDeposit_clicked();
    void on_btnTransfer_clicked();

    // Update balance and record transaction type
    void updateBalance(double change, const QString& type);

    // Helper functions for transactions/history
    void saveTransaction(const QString& type, double amount);
    void loadTransactionHistory();

    // Filters and export
    void applyFilters();
    void exportCsv();

    // Live updates
    void onTransactionsFileChanged(const QString& path);
    void onTransactionsDirChanged(const QString& path);

    // New: transfer to another user
    void transferTo(const QString& recipientIdentifier, double amount);

private:
    Ui::DashboardWindow* ui;
    QString currentUser;

    // Filter UI
    QDateEdit* fromDateEdit;
    QDateEdit* toDateEdit;
    QComboBox* typeCombo;
    QPushButton* applyFilterBtn;
    QPushButton* exportCsvBtn;

    // File watcher for live updates
    QFileSystemWatcher* watcher;
};