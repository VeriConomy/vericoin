#ifndef GETVERICOINPAGE_H
#define GETVERICOINPAGE_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QTimer>

namespace Ui {
    class GetVeriCoinPage;
}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Trade page widget */
class GetVeriCoinPage : public QWidget
{
    Q_OBJECT

public:
    explicit GetVeriCoinPage(QWidget *parent = 0);
    ~GetVeriCoinPage();

    void setModel(ClientModel *clientModel);
    void setModel(WalletModel *walletModel);

public slots:

// signals:

private:
    Ui::GetVeriCoinPage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;

private slots:

};

#endif // GETVERICOINPAGE_H
