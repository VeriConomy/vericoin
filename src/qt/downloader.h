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
    void setUrl(QUrl url);
    void setDest(QString dest);
    void startRequest(QUrl url);
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

    void enableDownloadButton();
    void cancelDownload();

private:
    Ui::Downloader *ui;
    QUrl url;
    QString fileDest;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QProgressDialog *progressDialog;
    QFile *file;
    qint64 fileSize;

};

#endif // DOWNLOADER_H
