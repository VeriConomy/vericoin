#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QProgressDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QTimer>

namespace Ui {
class Downloader;
}

class Downloader : public QDialog
{
    Q_OBJECT

public:
    explicit Downloader(QWidget *parent = 0);
    ~Downloader();

public:
    void setAutoDownload(bool nogui);
    void setUrl(QUrl url);
    void setDest(QString dest);
    void startDownload();
    bool httpRequestAborted;
    bool downloaderContinue;
    bool downloaderQuit;

private slots:
    void on_downloadButton_clicked();

    void on_quitButton_clicked();

    void on_continueButton_clicked();

    void on_urlEdit_returnPressed();

    // slot for readyRead() signal
    void httpReadyRead();

    // slot for finished() signal from reply
    void downloaderFinished();

    // slot for downloadProgress()
    void updateDownloadProgress(qint64, qint64);
    void timerCheckDownloadProgress();

    void enableDownloadButton();
    void cancelDownload();

private:
    void startRequest(QUrl url);
    Ui::Downloader *ui;
    QUrl url;
    QFileInfo fileDest;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QProgressDialog *progressDialog;
    QFile *file;
    bool downloadFinished;
    qint64 downloadProgress;
    qint64 fileSize;
    QTimer *downloadTimer;
    bool autoDownload;
};

#endif // DOWNLOADER_H
