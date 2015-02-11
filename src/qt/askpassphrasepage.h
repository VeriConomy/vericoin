#ifndef ASKPASSPHRASEPAGE_H
#define ASKPASSPHRASEPAGE_H

#include <QWidget>

namespace Ui {
    class AskPassphrasePage;
}

class WalletModel;

/** Ask for passphrase. Used for unlocking at startup.
 */
class AskPassphrasePage : public QWidget
{
    Q_OBJECT

public:
    enum Mode {
        Lock,       /**< Ask passphrase and lock */
        Unlock      /**< Ask passphrase and unlock */
    };

    explicit AskPassphrasePage(Mode mode, QWidget *parent = 0);
    ~AskPassphrasePage();

    void setModel(WalletModel *model);

private:
    Ui::AskPassphrasePage *ui;
    Mode mode;
    WalletModel *model;
    bool fCapsLock;

private slots:
    void ok_clicked();
    void textChanged();
    bool event(QEvent *event);
    bool eventFilter(QObject *, QEvent *event);

signals:
    void unlockWalletFeatures();
};

#endif // ASKPASSPHRASEPAGE_H
