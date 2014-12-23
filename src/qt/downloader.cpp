#include "downloader.h"
#include "ui_downloader.h"

Downloader::Downloader(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Downloader)
{
    ui->setupUi(this);
    ui->urlEdit->setText("");
    ui->statusLabel->setWordWrap(true);
    ui->downloadButton->setAutoDefault(false);
    ui->continueButton->setAutoDefault(false);
    ui->quitButton->setAutoDefault(false);

    progressDialog = new QProgressDialog(this);
    progressDialog->setCancelButton(NULL);
    // Create a timer to handle hung download requests
    downloadTimer = new QTimer(this);
    connect(downloadTimer, SIGNAL(timeout()), this, SLOT(timerCheckDownloadProgress()));

    // These will be set true when Cancel/Continue/Quit pressed
    downloaderQuit = false;
    httpRequestAborted = false;
    autoDownload = false;
    downloadFinished = false;

    connect(ui->urlEdit, SIGNAL(textChanged(QString)),
                this, SLOT(enableDownloadButton()));
    connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
}

Downloader::~Downloader()
{
    delete ui;
}

void Downloader::startDownload()
{
    if (autoDownload)
    {
        ui->quitButton->setEnabled(false);
    }
    on_downloadButton_clicked();
}

void Downloader::on_continueButton_clicked()
{
    downloadFinished = true;

    this->close();
}

void Downloader::on_quitButton_clicked()
{
    downloaderQuit = true;

    if (!downloadFinished && progressDialog->value() > 0)
    {
        // Clean up
        httpRequestAborted = true;
        reply->abort();
        downloaderFinished();
    }

    downloadFinished = false;

    this->close();
}

void Downloader::on_urlEdit_returnPressed()
{
    on_downloadButton_clicked();
}

void Downloader::enableDownloadButton()
{
    ui->downloadButton->setEnabled(!(ui->urlEdit->text()).isEmpty());
}

// During the download progress, it can be canceled
void Downloader::cancelDownload()
{
    // Finished with timer
    if (downloadTimer->isActive())
    {
        downloadTimer->stop();
    }

    httpRequestAborted = true;
    reply->abort();

    ui->statusLabel->setText(tr("Download canceled."));
    ui->downloadButton->setEnabled(true);
    ui->downloadButton->setDefault(true);
    ui->continueButton->setEnabled(false);
}

void Downloader::on_downloadButton_clicked()
{
    downloadFinished = false;

    // get url
    url = (ui->urlEdit->text());

    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();

    if (fileName.isEmpty())
    {
        if (!autoDownload)
        {
            QMessageBox::information(this, tr("Downloader"),
                      tr("Filename cannot be empty.")
                      );
        }
        return;
    }

    if (!fileDest.fileName().isEmpty())
    {
        fileName = fileDest.filePath();
    }
    fileDest = QFileInfo(fileName);

    if (fileDest.exists())
    {
        if (!autoDownload)
        {
            if (QMessageBox::question(this, tr("Downloader"),
                tr("There already exists a file called %1. Overwrite?").arg(fileName),
                QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
                == QMessageBox::No)
            {
                ui->continueButton->setEnabled(true);
                return;
            }
        }
        QFile::remove(fileName);
    }

    manager = new QNetworkAccessManager(this);

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly))
    {
        if (!autoDownload)
        {
            QMessageBox::information(this, tr("Downloader"),
                      tr("Unable to save the file %1: %2.")
                      .arg(fileName).arg(file->errorString()));
        }
        delete file;
        file = 0;
        ui->continueButton->setEnabled(false);
        return;
    }

    // These will be set true when Cancel/Continue/Quit pressed
    downloaderQuit = false;
    httpRequestAborted = false;

    progressDialog->setWindowTitle(tr("Downloader"));
    progressDialog->setLabelText(tr("Downloading %1.").arg(fileName));
    progressDialog->setValue(0);

    // download button disabled after requesting download.
    ui->downloadButton->setEnabled(false);
    ui->continueButton->setEnabled(false);

    startRequest(url);
}

// This will be called when download button is clicked (or from Autodownload feature)
void Downloader::startRequest(QUrl url)
{
    downloadProgress = 0;
    downloadFinished = false;

    // Start the timer
    downloadTimer->start(30000);

    // get() method posts a request
    // to obtain the contents of the target request
    // and returns a new QNetworkReply object
    // opened for reading which emits
    // the readyRead() signal whenever new data arrives.
    reply = manager->get(QNetworkRequest(url));
    reply->ignoreSslErrors();

    // Whenever more data is received from the network,
    // this readyRead() signal is emitted
    connect(reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()));

    // Also, downloadProgress() signal is emitted when data is received
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(updateDownloadProgress(qint64,qint64)));

    // This signal is emitted when the reply has finished processing.
    // After this signal is emitted,
    // there will be no more updates to the reply's data or metadata.
    connect(reply, SIGNAL(finished()),
            this, SLOT(downloaderFinished()));

    ui->statusLabel->setText(tr("Downloading %1.").arg(url.url()));
    if (this->isVisible())
    {
        progressDialog->show();
    }
}

// When download finished or canceled, this will be called
void Downloader::downloaderFinished()
{
    // Finished with timer
    if (downloadTimer->isActive())
    {
        downloadTimer->stop();
    }

    // when canceled
    if (httpRequestAborted)
    {
        if (file)
        {
            file->close();
            file->remove();
            delete file;
            file = 0;
        }
        reply->deleteLater();
        progressDialog->hide();
        ui->downloadButton->setEnabled(true);
        ui->downloadButton->setDefault(true);
        ui->continueButton->setEnabled(false);
        return;
    }

    // download finished normally
    progressDialog->hide();
    file->flush();
    file->close();

    // get redirection url
    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error())
    {
        file->remove();
        if (!autoDownload)
        {
            QMessageBox::information(this, tr("Downloader"),
                                 tr("Download failed: %1.").arg(reply->errorString()));
        }
        ui->downloadButton->setEnabled(true);
        ui->downloadButton->setDefault(true);
        ui->continueButton->setEnabled(false);
    }
    else
    {
        if (!redirectionTarget.isNull())
        {
            QUrl newUrl = url.resolved(redirectionTarget.toUrl());
            if (autoDownload || QMessageBox::question(this, tr("Downloader"),
                                  tr("Redirect to %1 ?").arg(newUrl.toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            {
                url = newUrl;
                reply->deleteLater();
                file->open(QIODevice::WriteOnly);
                file->resize(0);
                startRequest(url);
                return;
            }
        }
        else
        {
            ui->statusLabel->setText(tr("Downloaded %1.").arg(fileDest.filePath()));
            ui->downloadButton->setEnabled(false);
            ui->continueButton->setEnabled(true);
            ui->continueButton->setDefault(true);
            ui->quitButton->setDefault(false);
        }
    }

    reply->deleteLater();
    reply = 0;
    delete file;
    file = 0;
    manager = 0;
    downloadFinished = true;

    if (autoDownload)
    {
        if (ui->continueButton->isEnabled())
        {
            on_continueButton_clicked();
        }
        else
        {
            on_quitButton_clicked();
        }
    }
}

void Downloader::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (file)
        file->write(reply->readAll());
}

void Downloader::updateDownloadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (httpRequestAborted)
        return;

    progressDialog->setMaximum(totalBytes);
    progressDialog->setValue(bytesRead);

    progressDialog->raise();
}

// This is called during the download to check for a hung state
void Downloader::timerCheckDownloadProgress()
{
    if (progressDialog->value() > downloadProgress)
    {
        downloadProgress = progressDialog->value();
        return;
    }
    else
    {
        if (!downloadFinished)
        {
            // We appear to be hung.
            cancelDownload();
        }
    }
}

// This is called when the URL is already pre-defined and you want to bypass the dialog window (overloaded)
void Downloader::setUrl(std::string source)
{
    QUrl u;
    u.setUrl(QString::fromStdString(source));
    setUrl(u);
}

// This is called when the URL is already pre-defined and you want to bypass the dialog window
void Downloader::setUrl(QUrl source)
{
    url = source;

    ui->urlEdit->setText(url.url());
    ui->urlEdit->setEnabled(false);
}

// This is called when the Destination is already pre-defined and you want to bypass the dialog window (overloaded)
void Downloader::setDest(std::string dest)
{
    QString d = QString::fromStdString(dest);
    setDest(d);
}

// This is called when the Destination is already pre-defined and you want to bypass the dialog window
void Downloader::setDest(QString dest)
{
    fileDest = QFileInfo(dest);

    if (fileDest.exists())
    {
        ui->statusLabel->setText(tr("File: %1 exists.").arg(fileDest.filePath()));
        ui->continueButton->setEnabled(true);
    }
    else
    {
        ui->continueButton->setEnabled(false);
    }
}

void Downloader::setAutoDownload(bool nogui)
{
    autoDownload = nogui;
}
