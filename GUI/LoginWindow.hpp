#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "Client.h"

class LoginWindow : public QWidget {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);

private slots:
    void onLogin();
    void onSignup();
    void onLoginResult(bool ok, const QString &msg);
    void onSignupResult(bool ok, const QString &msg);

private:
    QLineEdit *m_userEdit;
    QLineEdit *m_passEdit;
    QLabel    *m_statusLabel;
    QPushButton *m_loginBtn;
    QPushButton *m_signupBtn;
    Client    *m_client;

    void openMainWindow();
    void setButtonsEnabled(bool enabled);
};
