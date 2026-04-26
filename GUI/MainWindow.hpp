#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QSlider>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "Client.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(Client *client, QWidget *parent = nullptr);

private slots:
    void onPlaylistReceived(const QStringList &songs);
    void onRefresh();
    void onPlay();
    void onDownload();
    void onSongReady(const QString &path);
    void onDownloadDone(const QString &path);
    void onError(const QString &msg);
    void onProgress(qint64 received, qint64 total);
    void onPlayerStateChanged();
    void onDurationChanged(qint64 duration);
    void onPositionChanged(qint64 position);
    void onSeek(int value);
    void onVolumeChanged(int value);

private:
    Client        *m_client;
    QListWidget   *m_list;
    QPushButton   *m_playBtn;
    QPushButton   *m_downloadBtn;
    QPushButton   *m_refreshBtn;
    QPushButton   *m_prevBtn;
    QPushButton   *m_nextBtn;
    QLabel        *m_nowPlaying;
    QLabel        *m_timeCurrent;
    QLabel        *m_timeTotal;
    QLabel        *m_statusLabel;
    QProgressBar  *m_progressBar;
    QSlider       *m_seekSlider;
    QSlider       *m_volSlider;
    QMediaPlayer  *m_player;
    QAudioOutput  *m_audio;

    QStringList    m_songs;
    bool           m_isSeeking = false;

    QString currentSongName();
    void playIndex(int index);
    void setStatus(const QString &msg, bool isError = false);
    QString getDownloadDir();
};
