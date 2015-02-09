#ifndef SUPERNETPAGE_H
#define SUPERNETPAGE_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QTimer>

namespace Ui {
    class SuperNETPage;
}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Trade page widget */
class SuperNETPage : public QWidget
{
    Q_OBJECT

public:
    explicit SuperNETPage(QWidget *parent = 0);
    ~SuperNETPage();

    void setModel(ClientModel *clientModel);
    void setModel(WalletModel *walletModel);

public slots:

// signals:

private:
    Ui::SuperNETPage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;

private slots:

};

#endif // SUPERNETPAGE_H
