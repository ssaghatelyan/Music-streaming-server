#include "LoginWindow.h"
#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QThread>
#include <QMessageBox>

static const QString BTN_STYLE = R"(
QPushButton {
    background: #7c6af5;
    color: white;
    border: none;
    border-radius: 6px;
    padding: 8px 20px;
    font-size: 13px;
}
QPushButton:hover { background: #9080f7; }
QPushButton:disabled { background: #3a3a3a; color: #666; }
)";

static const QString BTN_OUTLINE = R"(
QPushButton {
    background: transparent;
    color: #888;
    border: 1px solid #333;
    border-radius: 6px;
    padding: 8px 20px;
    font-size: 13px;
}
QPushButton:hover { background: #2a2a2a; color: #ccc; }
QPushButton:disabled { color: #444; }
)";

static const QString INPUT_STYLE = R"(
QLineEdit {
    background: #2a2a2a;
    color: #ddd;
    border: 1px solid #3a3a3a;
    border-radius: 6px;
    padding: 6px 10px;
    font-size: 13px;
}
QLineEdit:focus { border-color: #7c6af5; }
)";

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("MusicStream — Login");
    setFixedSize(340, 280);
    setStyleSheet("background-color: #1a1a1a;");

    m_client = new Client(this);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(40, 40, 40, 40);
    root->setSpacing(16);

    auto *title = new QLabel("♪ MusicStream", this);
    title->setStyleSheet("color: #a89df7; font-size: 20px; font-weight: 500;");
    title->setAlignment(Qt::AlignCenter);
    root->addWidget(title);

    auto *form = new QFormLayout();
    form->setSpacing(10);

    m_userEdit = new QLineEdit(this);
    m_userEdit->setPlaceholderText("Username");
    m_userEdit->setStyleSheet(INPUT_STYLE);
    form->addRow(m_userEdit);

    m_passEdit = new QLineEdit(this);
    m_passEdit->setPlaceholderText("Password");
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setStyleSheet(INPUT_STYLE);
    form->addRow(m_passEdit);

    root->addLayout(form);

    auto *btnRow = new QHBoxLayout();
    m_loginBtn  = new QPushButton("Login",  this);
    m_signupBtn = new QPushButton("Signup", this);
    m_loginBtn->setStyleSheet(BTN_STYLE);
    m_signupBtn->setStyleSheet(BTN_OUTLINE);
    btnRow->addWidget(m_loginBtn);
    btnRow->addWidget(m_signupBtn);
    root->addLayout(btnRow);

    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet("color: #e24b4b; font-size: 12px;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(m_statusLabel);

    connect(m_loginBtn,  &QPushButton::clicked, this, &LoginWindow::onLogin);
    connect(m_signupBtn, &QPushButton::clicked, this, &LoginWindow::onSignup);
    connect(m_passEdit,  &QLineEdit::returnPressed, this, &LoginWindow::onLogin);

    connect(m_client, &Client::loginResult,  this, &LoginWindow::onLoginResult);
    connect(m_client, &Client::signupResult, this, &LoginWindow::onSignupResult);

    if (!m_client->connectToServer("127.0.0.1", 8080)) {
        m_statusLabel->setText("Cannot connect to server");
        m_loginBtn->setEnabled(false);
        m_signupBtn->setEnabled(false);
    }
}

void LoginWindow::setButtonsEnabled(bool enabled) {
    m_loginBtn->setEnabled(enabled);
    m_signupBtn->setEnabled(enabled);
}

void LoginWindow::onLogin() {
    QString u = m_userEdit->text().trimmed();
    QString p = m_passEdit->text();
    if (u.isEmpty() || p.isEmpty()) {
        m_statusLabel->setText("Fill in all fields");
        return;
    }
    setButtonsEnabled(false);
    m_statusLabel->setText("Connecting...");

    QThread *t = QThread::create([this, u, p]{ m_client->login(u, p); });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
}

void LoginWindow::onSignup() {
    QString u = m_userEdit->text().trimmed();
    QString p = m_passEdit->text();
    if (u.isEmpty() || p.isEmpty()) {
        m_statusLabel->setText("Fill in all fields");
        return;
    }
    setButtonsEnabled(false);
    m_statusLabel->setText("Signing up...");

    QThread *t = QThread::create([this, u, p]{ m_client->signup(u, p); });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
}

void LoginWindow::onLoginResult(bool ok, const QString &msg) {
    setButtonsEnabled(true);
    if (ok) {
        openMainWindow();
    } else {
        m_statusLabel->setText(msg == "LOGIN_FAILED" ? "Wrong username or password" : msg);
    }
}

void LoginWindow::onSignupResult(bool ok, const QString &msg) {
    setButtonsEnabled(true);
    if (ok) {
        m_statusLabel->setStyleSheet("color: #3b6d11; font-size: 12px;");
        m_statusLabel->setText("Account created! You can login now.");
    } else {
        m_statusLabel->setStyleSheet("color: #e24b4b; font-size: 12px;");
        m_statusLabel->setText(msg == "USER_EXISTS" ? "User already exists" : msg);
    }
}

void LoginWindow::openMainWindow() {
    auto *w = new MainWindow(m_client);
    w->show();
    close();
}
