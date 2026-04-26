#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QStringList>

class Client : public QObject {
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);

    bool connectToServer(const QString &host, quint16 port);
    void login(const QString &user, const QString &pass);
    void signup(const QString &user, const QString &pass);
    void requestList();
    void playSong(const QString &name);
    void downloadSong(const QString &name, const QString &savePath);

signals:
    void loginResult(bool ok, const QString &msg);
    void signupResult(bool ok, const QString &msg);
    void playlistReceived(const QStringList &songs);
    void songReady(const QString &path);
    void downloadDone(const QString &path);
    void error(const QString &msg);
    void downloadProgress(qint64 received, qint64 total);

private:
    QTcpSocket *m_socket;

    bool sendLine(const QString &line);
    QString recvString();
    bool recvAll(char *buf, qint64 size);
    bool recvFile(const QString &savePath, qint64 &outSize);
};
