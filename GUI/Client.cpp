#include "Client.h"
#include <QDataStream>
#include <QFile>

Client::Client(QObject *parent) : QObject(parent) {
    m_socket = new QTcpSocket(this);
}

bool Client::connectToServer(const QString &host, quint16 port) {
    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected(3000);
}

bool Client::sendLine(const QString &line) {
    QByteArray data = (line + "\n").toUtf8();
    qint64 written = 0;
    while (written < data.size()) {
        qint64 n = m_socket->write(data.constData() + written, data.size() - written);
        if (n <= 0) return false;
        written += n;
    }
    return m_socket->waitForBytesWritten(3000);
}

bool Client::recvAll(char *buf, qint64 size) {
    qint64 total = 0;
    while (total < size) {
        if (m_socket->bytesAvailable() == 0 && !m_socket->waitForReadyRead(5000))
            return false;
        qint64 n = m_socket->read(buf + total, size - total);
        if (n <= 0) return false;
        total += n;
    }
    return true;
}

QString Client::recvString() {
    quint64 len = 0;
    if (!recvAll(reinterpret_cast<char*>(&len), sizeof(len)))
        return {};
    if (len == 0 || len > 10 * 1024 * 1024)
        return {};
    QByteArray data(len, '\0');
    if (!recvAll(data.data(), len))
        return {};
    return QString::fromUtf8(data);
}

bool Client::recvFile(const QString &savePath, qint64 &outSize) {
    quint64 fileSize = 0;
    if (!recvAll(reinterpret_cast<char*>(&fileSize), sizeof(fileSize)))
        return false;

    outSize = fileSize;
    if (fileSize == 0)
        return false;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    char buf[4096];
    qint64 received = 0;
    while (received < (qint64)fileSize) {
        if (m_socket->bytesAvailable() == 0 && !m_socket->waitForReadyRead(10000))
            break;
        qint64 toRead = qMin((qint64)sizeof(buf), (qint64)fileSize - received);
        qint64 n = m_socket->read(buf, toRead);
        if (n <= 0) break;
        file.write(buf, n);
        received += n;
        emit downloadProgress(received, fileSize);
    }
    file.close();
    return received == (qint64)fileSize;
}

void Client::login(const QString &user, const QString &pass) {
    sendLine("login " + user + " " + pass);
    QString res = recvString();
    emit loginResult(res == "LOGIN_OK", res);
}

void Client::signup(const QString &user, const QString &pass) {
    sendLine("signup " + user + " " + pass);
    QString res = recvString();
    emit signupResult(res == "SIGNUP_OK", res);
}

void Client::requestList() {
    sendLine("list");
    QString raw = recvString();
    if (raw.isEmpty()) {
        emit error("Failed to get playlist");
        return;
    }
    QStringList songs = raw.split("\n", Qt::SkipEmptyParts);
    songs.sort();
    emit playlistReceived(songs);
}

void Client::playSong(const QString &name) {
    sendLine("play " + name);
    QString tmpPath = "/tmp/stream_song.mp3";
    qint64 size = 0;
    if (recvFile(tmpPath, size)) {
        emit songReady(tmpPath);
    } else if (size == 0) {
        emit error("File not found on server");
    } else {
        emit error("Download error");
    }
}

void Client::downloadSong(const QString &name, const QString &savePath) {
    sendLine("play " + name);
    qint64 size = 0;
    if (recvFile(savePath, size)) {
        emit downloadDone(savePath);
    } else if (size == 0) {
        emit error("File not found on server");
    } else {
        emit error("Download failed");
    }
}
