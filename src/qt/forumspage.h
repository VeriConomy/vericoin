#ifndef FORUMSPAGE_H
#define FORUMSPAGE_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QTimer>

namespace Ui {
    class ForumsPage;
}
class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Trade page widget */
class ForumsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ForumsPage(QWidget *parent = 0);
    ~ForumsPage();

    void setModel(ClientModel *clientModel);
    void setModel(WalletModel *walletModel);

public slots:

// signals:

private:
    Ui::ForumsPage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;

private slots:

    void on_explorerButton_clicked();
    void on_chatButton_clicked();
    void on_forumButton_clicked();
    void on_siteButton_clicked();
};

#endif // FORUMSPAGE_H
