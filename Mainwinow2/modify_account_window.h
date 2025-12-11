#pragma once
#include <QWidget>
#include <QString>

namespace Ui { class ModifyAccountWindow; }

class ModifyAccountWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ModifyAccountWindow(QWidget* parent = nullptr);
    ~ModifyAccountWindow();

    // Public API
    void loadUser(const QString& username);

signals:
    void backToDashboard();

private slots:
    void on_saveBtn_clicked();

private:
    Ui::ModifyAccountWindow* ui;
    QString currentUser;

    // Helper to persist user data; returns true on success
    bool saveUser(const QString& name, const QString& pass, const QString& accNum, const QString& address);
};