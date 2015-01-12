#ifndef SENDBITCOINSDIALOG_H
#define SENDBITCOINSDIALOG_H

#include <QDialog>
#include <QString>
#include <QtNetwork/QtNetwork>

namespace Ui {
    class SendBitCoinsDialog;
}
class WalletModel;
class SendBitCoinsEntry;
class SendCoinsRecipient;
class ClientModel;

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

/** Dialog for sending bitcoins */
class SendBitCoinsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendBitCoinsDialog(QWidget *parent = 0);
    ~SendBitCoinsDialog();

    void setModel(WalletModel *model);

    /** Set up the tab chain manually, as Qt messes up the tab chain by default in some cases (issue https://bugreports.qt-project.org/browse/QTBUG-10907).
     */
    QWidget *setupTabChain(QWidget *prev);

    void pasteEntry(const SendCoinsRecipient &rv);
    bool handleURI(const QString &uri);

public slots:
    void clear();
    void reject();
    void accept();
    SendBitCoinsEntry *addEntry();
    void updateRemoveEnabled();
    void setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance);
    void passResponse(QNetworkReply *finished);

signals:
    void dataready( QByteArray &dataR );

private:
    Ui::SendBitCoinsDialog *ui;
    WalletModel *model;
    ClientModel *currentModel;
    bool fNewRecipientAllowed;

private slots:
    void removeEntry(SendBitCoinsEntry* entry);
    void updateDisplayUnit();
    void coinControlFeatureChanged(bool);
    void coinControlButtonClicked();
    void coinControlChangeChecked(int);
    void coinControlChangeEdited(const QString &);
    void coinControlUpdateLabels();
    void coinControlClipboardQuantity();
    void coinControlClipboardAmount();
    void coinControlClipboardFee();
    void coinControlClipboardAfterFee();
    void coinControlClipboardBytes();
    void coinControlClipboardPriority();
    void coinControlClipboardLowOutput();
    void coinControlClipboardChange();
    void on_veriBitSendButton_clicked();
};

#endif // SENDCOINSDIALOG_H
