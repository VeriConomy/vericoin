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

    // This will be set true when Quit/Continue pressed
    downloaderQuit = false;
    downloaderContinue = false;

    connect(ui->urlEdit, SIGNAL(textChanged(QString)),
                this, SLOT(enableDownloadButton()));
    connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
}

Downloader::~Downloader()
{
    delete ui;
}

void Downloader::on_downloadButton_clicked()
{
    // get url
    url = (ui->urlEdit->text());

    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();

    if (fileName.isEmpty())
    {
        QMessageBox::information(this, tr("Downloader"),
                      tr("Filename cannot be empty.")
                      );
        return;
    }

    if (!fileDest.isEmpty())
    {
        fileName = fileDest;
    }

    if (QFile::exists(fileName))
    {
        if (QMessageBox::question(this, tr("Downloader"),
                tr("There already exists a file called %1 in "
                "the current directory. Overwrite?").arg(fileName),
                QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
                == QMessageBox::No)
        {
            ui->continueButton->setEnabled(true);
            return;
        }
        QFile::remove(fileName);
    }

    manager = new QNetworkAccessManager(this);

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Downloader"),
                      tr("Unable to save the file %1: %2.")
                      .arg(fileName).arg(file->errorString()));
        delete file;
        file = 0;
        ui->continueButton->setEnabled(false);
        return;
    }

    // used for progressDialog
    // This will be set true when canceled from progress dialog
    httpRequestAborted = false;

    progressDialog->setWindowTitle(tr("Downloader"));
    progressDialog->setLabelText(tr("Downloading %1.").arg(fileName));


    // download button disabled after requesting download.
    ui->downloadButton->setEnabled(false);
    ui->continueButton->setEnabled(false);

    startRequest(url);
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
}

void Downloader::on_continueButton_clicked()
{
    downloaderContinue = true;

    this->close();
}

void Downloader::on_quitButton_clicked()
{
    downloaderQuit = true;

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
    ui->statusLabel->setText(tr("Download canceled."));
    httpRequestAborted = true;
    reply->abort();
    ui->downloadButton->setEnabled(true);
    ui->downloadButton->setDefault(true);
    ui->continueButton->setEnabled(false);
}

// When download finished or canceled, this will be called
void Downloader::downloaderFinished()
{
    downloadFinished = true;

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
        QMessageBox::information(this, tr("Downloader"),
                                 tr("Download failed: %1.")
                                 .arg(reply->errorString()));
        ui->downloadButton->setEnabled(true);
        ui->downloadButton->setDefault(true);
        ui->continueButton->setEnabled(false);
    }
    else
    {
        if (!redirectionTarget.isNull())
        {
            QUrl newUrl = url.resolved(redirectionTarget.toUrl());
            if (QMessageBox::question(this, tr("Downloader"),
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
            QString fileName = QFileInfo(QUrl(ui->urlEdit->text()).path()).fileName();
            ui->statusLabel->setText(tr("Downloaded %1 to %2.").arg(fileName).arg(fileDest));
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
}

// This will be called when download button is clicked
void Downloader::startRequest(QUrl url)
{
    downloadProgress = 0;
    downloadFinished = false;

    // Create a timer to handle hung download requests
    downloadTimer = new QTimer(this);
    connect(downloadTimer, SIGNAL(timeout()), this, SLOT(timerCheckDownloadProgress()));
    downloadTimer->start(30000);

    // get() method posts a request
    // to obtain the contents of the target request
    // and returns a new QNetworkReply object
    // opened for reading which emits
    // the readyRead() signal whenever new data arrives.
    reply = manager->get(QNetworkRequest(url));

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

    progressDialog->show();
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
        // Finished with timer
        if (downloadTimer->isActive())
            downloadTimer->stop();
    }
}

// This is called when the URL is already pre-defined and you want to bypass the dialog window
void Downloader::setUrl(QUrl url)
{
    ui->urlEdit->setText(url.url());
    ui->urlEdit->setEnabled(false);
}

// This is called when the Destination is already pre-defined and you want to bypass the dialog window
void Downloader::setDest(QString dest)
{
    fileDest = dest;

    if (QFile::exists(fileDest))
    {
        ui->statusLabel->setText(tr("File: %1 exists.").arg(fileDest));
        ui->continueButton->setEnabled(true);
    }
    else
    {
        ui->continueButton->setEnabled(false);
    }
}
