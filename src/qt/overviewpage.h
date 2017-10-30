#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include <QWidget>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
    class OverviewPage;
}
class WalletModel;
class ClientModel;
class TxViewDelegate;
class TransactionFilterProxy;

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget *parent = 0);
    ~OverviewPage();

    void setModel(WalletModel *model);
    void showOutOfSyncWarning(bool fShow);
    bool mining;
    int processors;
    QString miningColor;
    QString notminingColor;

public slots:
    void setBalance(qint64 balance, qint64 unconfirmedBalance, qint64 immatureBalance);
    void setStatistics();

signals:
    void transactionClicked(const QModelIndex &index);
    void displayUnitChanged(int unit);
    void decimalPointsChanged(int decimals);
    void hideAmountsChanged(bool hideamounts);

private:
    Ui::OverviewPage *ui;
    WalletModel *model;
    ClientModel *clientmodel;
    qint64 currentBalance;
    qint64 currentUnconfirmedBalance;
    qint64 currentImmatureBalance;

    TxViewDelegate *txdelegate;
    TransactionFilterProxy *filter;

private slots:
    void updateDisplayUnit();
    void updateDecimalPoints();
    void updateHideAmounts();
    void handleTransactionClicked(const QModelIndex &index);
    void myOpenUrl(QUrl url);
    void sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist);
    void on_mineButton_clicked();
    void on_spinBox_valueChanged(int procs);
};

#endif // OVERVIEWPAGE_H
