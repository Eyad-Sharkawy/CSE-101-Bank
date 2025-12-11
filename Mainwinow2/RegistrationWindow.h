#pragma once
#include <QWidget>

namespace Ui { class RegistrationWindow; }

class RegistrationWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegistrationWindow(QWidget* parent = nullptr);
    ~RegistrationWindow();

signals:
    void goToLogin();

private slots:
    void on_createAccountBtn_clicked();
    void on_loginLink_linkActivated(const QString& link);

private:
    Ui::RegistrationWindow* ui;
};