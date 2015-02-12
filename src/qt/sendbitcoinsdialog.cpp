#include "sendbitcoinsdialog.h"
#include "ui_sendbitcoinsdialog.h"

#include "init.h"
#include "walletmodel.h"
#include "addresstablemodel.h"
#include "addressbookpage.h"

#include "bitcoinunits.h"
#include "veribitcoinunits.h"
#include "addressbookpage.h"
#include "optionsmodel.h"
#include "sendbitcoinsentry.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "askpassphrasedialog.h"
#include "coincontrol.h"
#include "coincontroldialog.h"
#include "clientmodel.h"

#include <QMessageBox>
#include <QLocale>
#include <QTextDocument>
#include <QScrollBar>
#include <QClipboard>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QByteArray>
#include <QSignalMapper>
#include <QGraphicsView>

using namespace GUIUtil;

SendBitCoinsDialog::SendBitCoinsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendBitCoinsDialog),
    model(0)
{
    // Setup header and styles
    if (fNoHeaders)
        GUIUtil::header(this, QString(""));
    else if (fSmallHeaders)
        GUIUtil::header(this, QString(":images/headerVeriBitSmall"));
    else
        GUIUtil::header(this, QString(":images/headerVeriBit"));

    ui->setupUi(this);
    this->layout()->setContentsMargins(10, 10 + HEADER_HEIGHT, 10, 10);
    ui->scrollAreaWidgetContents->setStyleSheet("QWidget { background: white; }"); // SDW - hack until I can do this globally.

#if QT_VERSION >= 0x040700
    /* Do not move this to the XML file, Qt before 4.7 will choke on it */
    ui->lineEditCoinControlChange->setPlaceholderText(tr("Enter a BitCoin address (e.g. 1LRWAyE3WKwTzXszEmtqKXzikQvoq7NJBa)"));
#endif

    addEntry();

    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addEntry()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));

    // Coin Control
    ui->lineEditCoinControlChange->setFont(GUIUtil::bitcoinAddressFont());
    connect(ui->pushButtonCoinControl, SIGNAL(clicked()), this, SLOT(coinControlButtonClicked()));
    connect(ui->checkBoxCoinControlChange, SIGNAL(stateChanged(int)), this, SLOT(coinControlChangeChecked(int)));
    connect(ui->lineEditCoinControlChange, SIGNAL(textEdited(const QString &)), this, SLOT(coinControlChangeEdited(const QString &)));

    // Coin Control: clipboard actions
    QAction *clipboardQuantityAction = new QAction(tr("Copy quantity"), this);
    QAction *clipboardAmountAction = new QAction(tr("Copy amount"), this);
    QAction *clipboardFeeAction = new QAction(tr("Copy fee"), this);
    QAction *clipboardAfterFeeAction = new QAction(tr("Copy after fee"), this);
    QAction *clipboardBytesAction = new QAction(tr("Copy bytes"), this);
    QAction *clipboardPriorityAction = new QAction(tr("Copy priority"), this);
    QAction *clipboardLowOutputAction = new QAction(tr("Copy low output"), this);
    QAction *clipboardChangeAction = new QAction(tr("Copy change"), this);
    connect(clipboardQuantityAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardQuantity()));
    connect(clipboardAmountAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardAmount()));
    connect(clipboardFeeAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardFee()));
    connect(clipboardAfterFeeAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardAfterFee()));
    connect(clipboardBytesAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardBytes()));
    connect(clipboardPriorityAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardPriority()));
    connect(clipboardLowOutputAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardLowOutput()));
    connect(clipboardChangeAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardChange()));
    ui->labelCoinControlQuantity->addAction(clipboardQuantityAction);
    ui->labelCoinControlAmount->addAction(clipboardAmountAction);
    ui->labelCoinControlFee->addAction(clipboardFeeAction);
    ui->labelCoinControlAfterFee->addAction(clipboardAfterFeeAction);
    ui->labelCoinControlBytes->addAction(clipboardBytesAction);
    ui->labelCoinControlPriority->addAction(clipboardPriorityAction);
    ui->labelCoinControlLowOutput->addAction(clipboardLowOutputAction);
    ui->labelCoinControlChange->addAction(clipboardChangeAction);

    fNewRecipientAllowed = true;
}

void SendBitCoinsDialog::setModel(WalletModel *model)
{
    this->model = model;

    for(int i = 0; i < ui->entries->count(); ++i)
    {
        SendBitCoinsEntry *entry = qobject_cast<SendBitCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            entry->setModel(model);
        }
    }
    if(model && model->getOptionsModel())
    {
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
        connect(model->getOptionsModel(), SIGNAL(hideAmountsChanged(bool)), this, SLOT(updateHideAmounts()));

        // Coin Control
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(coinControlUpdateLabels()));
        connect(model->getOptionsModel(), SIGNAL(decimalPointsChanged(int)), this, SLOT(coinControlUpdateLabels()));
        connect(model->getOptionsModel(), SIGNAL(coinControlFeaturesChanged(bool)), this, SLOT(coinControlFeatureChanged(bool)));
        connect(model->getOptionsModel(), SIGNAL(transactionFeeChanged(qint64)), this, SLOT(coinControlUpdateLabels()));
        ui->frameCoinControl->setVisible(model->getOptionsModel()->getCoinControlFeatures());
        coinControlUpdateLabels();
    }
}

SendBitCoinsDialog::~SendBitCoinsDialog()
{
    delete ui;
}

void SendBitCoinsDialog::clear()
{
    // Remove entries until only one left
    while(ui->entries->count())
    {
        delete ui->entries->takeAt(0)->widget();
    }
    addEntry();

    updateRemoveEnabled();

    ui->veriBitSendButton->setDefault(true);
}

void SendBitCoinsDialog::reject()
{
    clear();
}

void SendBitCoinsDialog::accept()
{
    clear();
}

SendBitCoinsEntry *SendBitCoinsDialog::addEntry()
{
    SendBitCoinsEntry *entry = new SendBitCoinsEntry(this);
    entry->setModel(model);
    ui->entries->addWidget(entry);
    connect(entry, SIGNAL(removeEntry(SendBitCoinsEntry*)), this, SLOT(removeEntry(SendBitCoinsEntry*)));
    connect(entry, SIGNAL(payAmountChanged()), this, SLOT(coinControlUpdateLabels()));

    updateRemoveEnabled();

    // Focus the field, so that entry can start immediately
    entry->clear();
    entry->setFocus();
    ui->scrollAreaWidgetContents->resize(ui->scrollAreaWidgetContents->sizeHint());
    QCoreApplication::instance()->processEvents();
    QScrollBar* bar = ui->scrollArea->verticalScrollBar();
    if(bar)
        bar->setSliderPosition(bar->maximum());
    return entry;
}

void SendBitCoinsDialog::updateRemoveEnabled()
{
    // Remove buttons are enabled as soon as there is more than one send-entry
    bool enabled = (ui->entries->count() > 1);
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        SendBitCoinsEntry *entry = qobject_cast<SendBitCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            entry->setRemoveEnabled(enabled);
        }
    }
    setupTabChain(0);
    coinControlUpdateLabels();
}

void SendBitCoinsDialog::removeEntry(SendBitCoinsEntry* entry)
{
    delete entry;
    updateRemoveEnabled();
}

QWidget *SendBitCoinsDialog::setupTabChain(QWidget *prev)
{
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        SendBitCoinsEntry *entry = qobject_cast<SendBitCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            prev = entry->setupTabChain(prev);
        }
    }
    QWidget::setTabOrder(prev, ui->addButton);
    QWidget::setTabOrder(ui->addButton, ui->veriBitSendButton);
    return ui->veriBitSendButton;
}

void SendBitCoinsDialog::pasteEntry(const SendCoinsRecipient &rv)
{
    if(!fNewRecipientAllowed)
        return;

    SendBitCoinsEntry *entry = 0;
    // Replace the first entry if it is still unused
    if(ui->entries->count() == 1)
    {
        SendBitCoinsEntry *first = qobject_cast<SendBitCoinsEntry*>(ui->entries->itemAt(0)->widget());
        if(first->isClear())
        {
            entry = first;
        }
    }
    if(!entry)
    {
        entry = addEntry();
    }

    entry->setValue(rv);
}

bool SendBitCoinsDialog::handleURI(const QString &uri)
{
    SendCoinsRecipient rv;
    // URI has to be valid
    if (GUIUtil::parseBitcoinURI(uri, &rv))
    {
        CBitcoinAddress address(rv.address.toStdString());
        if (!address.IsValid())
            return false;
        pasteEntry(rv);
        return true;
    }

    return false;
}

void SendBitCoinsDialog::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    Q_UNUSED(stake);
    Q_UNUSED(unconfirmedBalance);
    Q_UNUSED(immatureBalance);
    if(!model || !model->getOptionsModel())
        return;

    int unit = model->getOptionsModel()->getDisplayUnit();
    ui->labelBalance->setText(BitcoinUnits::formatWithUnitWithMaxDecimals(unit, balance, BitcoinUnits::maxdecimals(unit), false, model->getOptionsModel()->getHideAmounts()));
}

void SendBitCoinsDialog::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        // Update labelBalance with the current balance and the current unit
        ui->labelBalance->setText(BitcoinUnits::formatWithUnitWithMaxDecimals(model->getOptionsModel()->getDisplayUnit(), model->getBalance(), BitcoinUnits::maxdecimals(model->getOptionsModel()->getDisplayUnit()), false, model->getOptionsModel()->getHideAmounts()));
    }
}

void SendBitCoinsDialog::updateHideAmounts()
{
    updateDisplayUnit();
}

// Coin Control: copy label "Quantity" to clipboard
void SendBitCoinsDialog::coinControlClipboardQuantity()
{
    QApplication::clipboard()->setText(ui->labelCoinControlQuantity->text());
}

// Coin Control: copy label "Amount" to clipboard
void SendBitCoinsDialog::coinControlClipboardAmount()
{
    QApplication::clipboard()->setText(ui->labelCoinControlAmount->text().left(ui->labelCoinControlAmount->text().indexOf(" ")));
}

// Coin Control: copy label "Fee" to clipboard
void SendBitCoinsDialog::coinControlClipboardFee()
{
    QApplication::clipboard()->setText(ui->labelCoinControlFee->text().left(ui->labelCoinControlFee->text().indexOf(" ")));
}

// Coin Control: copy label "After fee" to clipboard
void SendBitCoinsDialog::coinControlClipboardAfterFee()
{
    QApplication::clipboard()->setText(ui->labelCoinControlAfterFee->text().left(ui->labelCoinControlAfterFee->text().indexOf(" ")));
}

// Coin Control: copy label "Bytes" to clipboard
void SendBitCoinsDialog::coinControlClipboardBytes()
{
    QApplication::clipboard()->setText(ui->labelCoinControlBytes->text());
}

// Coin Control: copy label "Priority" to clipboard
void SendBitCoinsDialog::coinControlClipboardPriority()
{
    QApplication::clipboard()->setText(ui->labelCoinControlPriority->text());
}

// Coin Control: copy label "Low output" to clipboard
void SendBitCoinsDialog::coinControlClipboardLowOutput()
{
    QApplication::clipboard()->setText(ui->labelCoinControlLowOutput->text());
}

// Coin Control: copy label "Change" to clipboard
void SendBitCoinsDialog::coinControlClipboardChange()
{
    QApplication::clipboard()->setText(ui->labelCoinControlChange->text().left(ui->labelCoinControlChange->text().indexOf(" ")));
}

// Coin Control: settings menu - coin control enabled/disabled by user
void SendBitCoinsDialog::coinControlFeatureChanged(bool checked)
{
    ui->frameCoinControl->setVisible(checked);

    if (!checked && model) // coin control features disabled
        CoinControlDialog::coinControl->SetNull();
}

// Coin Control: button inputs -> show actual coin control dialog
void SendBitCoinsDialog::coinControlButtonClicked()
{
    CoinControlDialog dlg;
    dlg.setModel(model);
    dlg.exec();
    coinControlUpdateLabels();
}

// Coin Control: checkbox custom change address
void SendBitCoinsDialog::coinControlChangeChecked(int state)
{
    if (model)
    {
        if (state == Qt::Checked)
            CoinControlDialog::coinControl->destChange = CBitcoinAddress(ui->lineEditCoinControlChange->text().toStdString()).Get();
        else
            CoinControlDialog::coinControl->destChange = CNoDestination();
    }

    ui->lineEditCoinControlChange->setEnabled((state == Qt::Checked));
    ui->labelCoinControlChangeLabel->setEnabled((state == Qt::Checked));
}

// Coin Control: custom change address changed
void SendBitCoinsDialog::coinControlChangeEdited(const QString & text)
{
    if (model)
    {
        CoinControlDialog::coinControl->destChange = CBitcoinAddress(text.toStdString()).Get();

        // label for the change address
        ui->labelCoinControlChangeLabel->setStyleSheet("QLabel { color: " + STRING_VERIFONT + "; }");
        if (text.isEmpty())
            ui->labelCoinControlChangeLabel->setText("");
        else if (!CBitcoinAddress(text.toStdString()).IsValid())
        {
            ui->labelCoinControlChangeLabel->setStyleSheet("QLabel { color: red; }");
            ui->labelCoinControlChangeLabel->setText(tr("WARNING: Invalid VeriCoin address"));
        }
        else
        {
            QString associatedLabel = model->getAddressTableModel()->labelForAddress(text);
            if (!associatedLabel.isEmpty())
                ui->labelCoinControlChangeLabel->setText(associatedLabel);
            else
            {
                CPubKey pubkey;
                CKeyID keyid;
                CBitcoinAddress(text.toStdString()).GetKeyID(keyid);   
                if (model->getPubKey(keyid, pubkey))
                    ui->labelCoinControlChangeLabel->setText(tr("(no label)"));
                else
                {
                    ui->labelCoinControlChangeLabel->setStyleSheet("QLabel { color: red; }");
                    ui->labelCoinControlChangeLabel->setText(tr("WARNING: unknown change address"));
                }
            }
        }
    }
}

// Coin Control: update labels
void SendBitCoinsDialog::coinControlUpdateLabels()
{
    if (!model || !model->getOptionsModel() || !model->getOptionsModel()->getCoinControlFeatures())
        return;

    // set pay amounts
    CoinControlDialog::payAmounts.clear();
    for(int i = 0; i < ui->entries->count(); ++i)
    {
        SendBitCoinsEntry *entry = qobject_cast<SendBitCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
            CoinControlDialog::payAmounts.append(entry->getValue().amount);
    }

    if (CoinControlDialog::coinControl->HasSelected())
    {
        // actual coin control calculation
        CoinControlDialog::updateLabels(model, this);

        // show coin control stats
        ui->labelCoinControlAutomaticallySelected->hide();
        ui->widgetCoinControl->show();
    }
    else
    {
        // hide coin control stats
        ui->labelCoinControlAutomaticallySelected->show();
        ui->widgetCoinControl->hide();
        ui->labelCoinControlInsuffFunds->hide();
    }
}

// VeriBitSend Button command
void SendBitCoinsDialog::on_veriBitSendButton_clicked()
{
    QDateTime lastBlockDate = currentModel->getLastBlockDate();
    int secs = lastBlockDate.secsTo(QDateTime::currentDateTime());
    int currentBlock = currentModel->getNumBlocks();
    int peerBlock = currentModel->getNumBlocksOfPeers();
    if(secs >= 90*60 && currentBlock < peerBlock)
    {
        QMessageBox::warning(this, tr("VeriBit"),
            tr("Error: %1").
            arg("Please wait until the wallet is in sync to use VeriBit."),
            QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QList<SendCoinsRecipient> recipients;
    bool valid = true;

    if(!model)
        return;

    for(int i = 0; i < ui->entries->count(); ++i)
    {
        SendBitCoinsEntry *entry = qobject_cast<SendBitCoinsEntry*>(ui->entries->itemAt(i)->widget());
        if(entry)
        {
            if(entry->validate())
            {
                recipients.append(entry->getValue());
            }
            else
            {
                valid = false;
            }
        }
    }

    if(!valid || recipients.isEmpty())
    {
        return;
    }

    // Format confirmation message
    QStringList formatted;
    QString sendto, amount, label;
    foreach(const SendCoinsRecipient &rcp, recipients)
    {
        formatted.append(tr("<b>%1</b> to %2 (%3)").arg(veriBitcoinUnits::formatWithUnitWithMaxDecimals(veriBitcoinUnits::BTC, rcp.amount, veriBitcoinUnits::maxdecimals(veriBitcoinUnits::BTC)), Qt::escape(rcp.label), rcp.address));
        amount.append(tr("%1").arg(BitcoinUnits::formatMaxDecimals(BitcoinUnits::VRC, rcp.amount, BitcoinUnits::maxdecimals(BitcoinUnits::VRC))));
        sendto.append(tr("%1").arg(rcp.address));
        label.append(tr("%1").arg(rcp.label));
    }

    fNewRecipientAllowed = false;

    //send address and amount to VeriBit Server
    QUrl serviceUrl = QUrl("http://verisend.vericoin.info/apisendbtc");
    QByteArray postData;
    postData.append("sendto=").append(sendto).append("&amount=").append(amount);
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(passResponse(QNetworkReply*)));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    networkManager->post(request,postData);
}


// Parse response from VeriBit Server and send coin to unique address
void SendBitCoinsDialog::passResponse( QNetworkReply *finished )
{
    WalletModel::UnlockContext ctx(model->requestUnlock());
    if(!ctx.isValid())
    {
        // Unlock wallet was cancelled
        fNewRecipientAllowed = true;
        return;
    }

    //Parse response
    QByteArray dataR = finished->readAll();
    QList<SendCoinsRecipient> recipients;
    SendCoinsRecipient rv;
    dataR.replace(QByteArray("}"), QByteArray(""));
    dataR.replace(QByteArray("{"), QByteArray(""));
    dataR.replace(QByteArray(":"), QByteArray(""));
    QList <QByteArray> dataSplit = dataR.split('"');

    //Error check
    char *errorTest = dataSplit[1].data();
    char errorTested[] = "error";
    if (strcmp (errorTest,errorTested) == 0)
    {
        QString emessage(dataSplit[3].constData());
        QMessageBox::warning(this, tr("VeriBit Send"),
            tr("Error: %1").
            arg(emessage),
            QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    //Prepare data
    QString address(dataSplit[3].constData());
    rv.address = address;
    QString label("");
    rv.label = label;
    rv.amount = (dataSplit[6].toDouble()*100000000); // convert to verisatoshis
    recipients.append(rv);

    if(recipients.isEmpty())
    {
        return;
    }
    fNewRecipientAllowed = false;

    QStringList formatted;
    foreach(const SendCoinsRecipient &rcp, recipients)
    {
        formatted.append(tr("<b>%1</b>").arg(BitcoinUnits::formatWithUnitWithMaxDecimals(BitcoinUnits::VRC, rcp.amount, BitcoinUnits::maxdecimals(BitcoinUnits::VRC)), Qt::escape(rcp.label), rcp.address));
    }

    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm VeriBit send of your VeriCoins"),
                          tr("Are you sure you want to use VeriBit to send BitCoin? VeriBit will require %1.").arg(formatted.join(tr(" and "))),
          QMessageBox::Yes|QMessageBox::Cancel,
          QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        fNewRecipientAllowed = true;
        return;
    }

    WalletModel::SendCoinsReturn sendstatus;

    if (!model->getOptionsModel() || !model->getOptionsModel()->getCoinControlFeatures())
        sendstatus = model->sendCoins(recipients);
    else
        sendstatus = model->sendCoins(recipients, CoinControlDialog::coinControl);

    switch(sendstatus.status)
    {
    case WalletModel::InvalidAddress:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("The recipient address is not valid, please recheck."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::InvalidAmount:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("The amount to pay must be larger than 0."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::AmountExceedsBalance:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("The amount exceeds your balance."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::AmountWithFeeExceedsBalance:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("The total exceeds your balance when the %1 transaction fee is included.").
            arg(BitcoinUnits::formatWithUnitFee(BitcoinUnits::VRC, sendstatus.fee)),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::DuplicateAddress:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("Duplicate address found, can only send to each address once per send operation."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::TransactionCreationFailed:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("Error: Transaction creation failed."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::TransactionCommitFailed:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("Error: The transaction was rejected. This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::Aborted: // User aborted, nothing to do
        break;
    case WalletModel::OK:
        accept();
        CoinControlDialog::coinControl->UnSelectAll();
        coinControlUpdateLabels();
        break;
    }
    fNewRecipientAllowed = true;
}
