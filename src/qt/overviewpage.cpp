// Copyright (c) 2011-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/overviewpage.h>
#include <qt/forms/ui_overviewpage.h>

#include <qt/bitcoinunits.h>
#include <qt/clientmodel.h>
#include <qt/guiconstants.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/transactionfilterproxy.h>
#include <qt/transactiontablemodel.h>
#include <qt/walletmodel.h>

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

#define DECORATION_SIZE 24
#define NUM_ITEMS 5

Q_DECLARE_METATYPE(interfaces::WalletBalances)

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit TxViewDelegate(const PlatformStyle *_platformStyle, QObject *parent=nullptr):
        QAbstractItemDelegate(parent), unit(BitcoinUnits::BTC),
        platformStyle(_platformStyle)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(TransactionTableModel::RawDecorationRole));
        QRect mainRect = option.rect;

        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(value.canConvert<QBrush>())
        {
            QBrush brush = qvariant_cast<QBrush>(value);
            foreground = brush.color();
        }

        painter->setPen(foreground);
        QRect boundingRect;
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address, &boundingRect);

        if (index.data(TransactionTableModel::WatchonlyRole).toBool())
        {
            QIcon iconWatchonly = qvariant_cast<QIcon>(index.data(TransactionTableModel::WatchonlyDecorationRole));
            QRect watchonlyRect(boundingRect.right() + 5, mainRect.top()+ypad+halfheight, 16, halfheight);
            iconWatchonly.paint(painter, watchonlyRect);
        }

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true, BitcoinUnits::separatorAlways);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE+20);
    }

    int unit;
    const PlatformStyle *platformStyle;

};


#include <qt/overviewpage.moc>

OverviewPage::OverviewPage(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    clientModel(nullptr),
    walletModel(nullptr),
    txdelegate(new TxViewDelegate(platformStyle, this))
{
    ui->setupUi(this);

    for (int i = 0; i < ui->topBoxes->count(); ++i)
    {
        QWidget *box = ui->topBoxes->itemAt(i)->widget();
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
        shadow->setOffset(QPointF(5, 5));
        shadow->setBlurRadius(20.0);
        box->setGraphicsEffect(shadow);
    }


    m_balances.balance = -1;

    // Recent transactions
    ui->lastTransactionsContent->setItemDelegate(txdelegate);
    ui->lastTransactionsContent->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->lastTransactionsContent, &QListView::clicked, this, &OverviewPage::handleTransactionClicked);

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
    connect(ui->availableWarn, &QPushButton::clicked, this, &OverviewPage::handleOutOfSyncWarningClicks);
    connect(ui->unconfirmedWarn, &QPushButton::clicked, this, &OverviewPage::handleOutOfSyncWarningClicks);
    connect(ui->immatureWarn, &QPushButton::clicked, this, &OverviewPage::handleOutOfSyncWarningClicks);
    connect(ui->totalWarn, &QPushButton::clicked, this, &OverviewPage::handleOutOfSyncWarningClicks);

    // manage receive/send button
    ui->receiveBox->installEventFilter(this);
    ui->sendBox->installEventFilter(this);
}

bool OverviewPage::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if(object->objectName() == ui->receiveBox->objectName())
            Q_EMIT receiveClicked();
        else if(object->objectName() == ui->sendBox->objectName())
            Q_EMIT sendClicked();
    }
    return QWidget::eventFilter(object, event);
}

void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        Q_EMIT transactionClicked(filter->mapToSource(index));
}

void OverviewPage::handleOutOfSyncWarningClicks()
{
    Q_EMIT outOfSyncWarningClicked();
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(const interfaces::WalletBalances& balances)
{
    int unit = walletModel->getOptionsModel()->getDisplayUnit();
    m_balances = balances;
    if (walletModel->privateKeysDisabled()) {
        ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balances.watch_only_balance, false, BitcoinUnits::separatorAlways));
        ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, balances.unconfirmed_watch_only_balance, false, BitcoinUnits::separatorAlways));
        ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, balances.immature_watch_only_balance, false, BitcoinUnits::separatorAlways));
        ui->labelTotal->setText(BitcoinUnits::formatWithUnit(unit, balances.watch_only_balance + balances.unconfirmed_watch_only_balance + balances.immature_watch_only_balance, false, BitcoinUnits::separatorAlways));
    } else {
        ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balances.balance, false, BitcoinUnits::separatorAlways));
        ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, balances.unconfirmed_balance, false, BitcoinUnits::separatorAlways));
        ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, balances.immature_balance, false, BitcoinUnits::separatorAlways));
        ui->labelTotal->setText(BitcoinUnits::formatWithUnit(unit, balances.balance + balances.unconfirmed_balance + balances.immature_balance, false, BitcoinUnits::separatorAlways));
        ui->labelWatchAvailable->setText(BitcoinUnits::formatWithUnit(unit, balances.watch_only_balance, false, BitcoinUnits::separatorAlways));
        ui->labelWatchUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, balances.unconfirmed_watch_only_balance, false, BitcoinUnits::separatorAlways));
        ui->labelWatchImmature->setText(BitcoinUnits::formatWithUnit(unit, balances.immature_watch_only_balance, false, BitcoinUnits::separatorAlways));
        ui->labelWatchTotal->setText(BitcoinUnits::formatWithUnit(unit, balances.watch_only_balance + balances.unconfirmed_watch_only_balance + balances.immature_watch_only_balance, false, BitcoinUnits::separatorAlways));
    }
}

// show/hide watch-only labels
void OverviewPage::updateWatchOnlyLabels(bool showWatchOnly)
{
    ui->watchAvailableBox->setVisible(showWatchOnly);
    ui->watchUnconfirmedBox->setVisible(showWatchOnly);
    ui->watchImmatureBox->setVisible(showWatchOnly);
    ui->watchTotalBox->setVisible(showWatchOnly);
}

void OverviewPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if (model) {
        // Show warning, for example if this is a prerelease version
        connect(model, &ClientModel::alertsChanged, this, &OverviewPage::updateAlerts);
        updateAlerts(model->getStatusBarWarnings());
    }
}

void OverviewPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter.reset(new TransactionFilterProxy());
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->setShowInactive(false);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);

        ui->lastTransactionsContent->setModel(filter.get());
        ui->lastTransactionsContent->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        interfaces::Wallet& wallet = model->wallet();
        interfaces::WalletBalances balances = wallet.getBalances();
        setBalance(balances);
        connect(model, &WalletModel::balanceChanged, this, &OverviewPage::setBalance);

        connect(model->getOptionsModel(), &OptionsModel::displayUnitChanged, this, &OverviewPage::updateDisplayUnit);

        updateWatchOnlyLabels(wallet.haveWatchOnly() && !model->privateKeysDisabled());
        connect(model, &WalletModel::notifyWatchonlyChanged, [this](bool showWatchOnly) {
            updateWatchOnlyLabels(showWatchOnly && !walletModel->privateKeysDisabled());
        });
    }

    // update the display unit, to not use the default ("VRM")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(walletModel && walletModel->getOptionsModel())
    {
        if (m_balances.balance != -1) {
            setBalance(m_balances);
        }

        // Update txdelegate->unit with the current unit
        txdelegate->unit = walletModel->getOptionsModel()->getDisplayUnit();

        ui->lastTransactionsContent->update();
    }
}

void OverviewPage::updateAlerts(const QString &warnings)
{
    this->ui->labelAlerts->setVisible(!warnings.isEmpty());
    this->ui->labelAlerts->setText(warnings);
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->availableWarn->setVisible(fShow);
    ui->unconfirmedWarn->setVisible(fShow);
    ui->immatureWarn->setVisible(fShow);
    ui->totalWarn->setVisible(fShow);
}
