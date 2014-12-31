/*
 * Qt5 bitcoin GUI.
 *
 * W.J. van der Laan 2011-2012
 * The Bitcoin Developers 2011-2012
 */
#include "bitcoingui.h"
#include "transactiontablemodel.h"
#include "addressbookpage.h"
#include "sendcoinsdialog.h"
#include "sendbitcoinsdialog.h"
#include "signverifymessagedialog.h"
#include "accessnxtinsidedialog.h"
#include "optionsdialog.h"
#include "aboutdialog.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "editaddressdialog.h"
#include "optionsmodel.h"
#include "transactiondescdialog.h"
#include "addresstablemodel.h"
#include "transactionview.h"
#include "overviewpage.h"
#include "bitcoinunits.h"
#include "guiconstants.h"
#include "askpassphrasedialog.h"
#include "notificator.h"
#include "guiutil.h"
#include "rpcconsole.h"
#include "ui_fiatpage.h"
#include "ui_newspage.h"
#include "ui_chatpage.h"
#include "ui_supernetpage.h"
#include "tooltip.h"
#include "downloader.h"
#include "updatedialog.h"
#include "rescandialog.h"

#include "JlCompress.h"
#include "walletdb.h"
#include "wallet.h"
#include "txdb.h"
#include <boost/version.hpp>
#include <boost/filesystem.hpp>

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QIcon>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QLocale>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QStackedWidget>
#include <QDateTime>
#include <QMovie>
#include <QFileDialog>
#include <QDesktopServices>
#include <QTimer>
#include <QDragEnterEvent>
#include <QUrl>
#include <QStyle>
#include <QFontDatabase>
#include <QInputDialog>
#include <QGraphicsView>

#include <iostream>

using namespace GUIUtil;

extern CWallet* pwalletMain;
extern int64_t nLastCoinStakeSearchInterval;
extern unsigned int nTargetSpacing;
double GetPoSKernelPS();
bool blocksIcon = true;
bool fFiatPageLoaded = false;
bool fNewsPageLoaded = false;
bool fChatPageLoaded = false;
bool fSuperNETPageLoaded = false;

// ProgressBar2 is just to the right of ProgressBar (available using extern pointers).
QLabel *progressBar2Label;
QProgressBar *progressBar2;


BitcoinGUI::BitcoinGUI(QWidget *parent):
    QMainWindow(parent),
    clientModel(0),
    walletModel(0),
    encryptWalletAction(0),
    changePassphraseAction(0),
    unlockWalletAction(0),
    aboutQtAction(0),
    trayIcon(0),
    notificator(0),
    rpcConsole(0)
{
    _TOOLTIP_INIT_QAPP

    setMinimumSize(820, 246);
    setWindowTitle(tr("VeriCoin Wallet - ") + tr(FormatVersion(CLIENT_VERSION).c_str()));
    qApp->setWindowIcon(QIcon(":icons/bitcoin"));
    qApp->setFont(veriFont);
    setWindowIcon(QIcon(":icons/bitcoin"));
#ifdef Q_OS_MAC
    setUnifiedTitleAndToolBarOnMac(false);
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    // Accept D&D of URIs
    setAcceptDrops(true);

    // Create actions for the toolbar, menu bar and tray/dock icon
    createActions();

    // Create application menu bar
    createMenuBar();

    // Create the toolbars
    createToolBars();

    // Create the tray icon (or setup the dock icon)
    createTrayIcon();

    // Create tab items
    overviewPage = new OverviewPage();

    transactionsPage = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout();
    transactionView = new TransactionView(this);
    vbox->addWidget(transactionView);
    transactionsPage->setLayout(vbox);

    addressBookPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::SendingTab);

    receiveCoinsPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::ReceivingTab);

    sendCoinsPage = new SendCoinsDialog(this);

    sendBitCoinsPage = new SendBitCoinsDialog(this);

    signVerifyMessageDialog = new SignVerifyMessageDialog(this);

    accessNxtInsideDialog = new AccessNxtInsideDialog(this);

    fiatPage = new QWebView(this);
    Ui::fiatPage fiat;
    fiat.setupUi(fiatPage);
    fiat.webView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
    QPushButton * back = fiatPage->findChild<QPushButton *>("back");
    QPushButton * reload = fiatPage->findChild<QPushButton *>("reload");
    connect(back, SIGNAL(clicked()), fiatPage->findChild<QWebView *>("webView"), SLOT(back()));
    connect(reload, SIGNAL(clicked()), fiatPage->findChild<QWebView *>("webView"), SLOT(reload()));
    connect(fiat.webView->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )), this, SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError> & )));
    connect(fiat.webView->page(), SIGNAL(linkClicked(const QUrl&)), this, SLOT(openUrl(const QUrl&)), Qt::DirectConnection);

    newsPage = new QWebView(this);
    Ui::newsPage news;
    news.setupUi(newsPage);
    news.webView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
    news.webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(news.webView->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )), this, SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError> & )));
    connect(news.webView->page(), SIGNAL(linkClicked(const QUrl&)), this, SLOT(openUrl(const QUrl&)), Qt::DirectConnection);

    chatPage = new QWebView(this);
    Ui::chatPage chat;
    chat.setupUi(chatPage);
    chat.webView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
    connect(chat.webView->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )), this, SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError> & )));
    connect(chat.webView->page(), SIGNAL(linkClicked(const QUrl&)), this, SLOT(openUrl(const QUrl&)), Qt::DirectConnection);

    superNETPage = new QWebView(this);
    Ui::superNETPage superNET;
    superNET.setupUi(superNETPage);
    superNET.webView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
    connect(superNET.webView->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )), this, SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError> & )));
    connect(superNET.webView->page(), SIGNAL(linkClicked(const QUrl&)), this, SLOT(openUrl(const QUrl&)), Qt::DirectConnection);

    centralWidget = new QStackedWidget(this);
    centralWidget->setFrameShape(QFrame::NoFrame);
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: white; }");
    centralWidget->addWidget(overviewPage);
    centralWidget->addWidget(transactionsPage);
    centralWidget->addWidget(addressBookPage);
    centralWidget->addWidget(receiveCoinsPage);
    centralWidget->addWidget(sendCoinsPage);
    centralWidget->addWidget(sendBitCoinsPage);
    centralWidget->addWidget(fiatPage);
    centralWidget->addWidget(newsPage);
    centralWidget->addWidget(chatPage);
    centralWidget->addWidget(superNETPage);
    setCentralWidget(centralWidget);

    // Create status bar
    statusBar();
    statusBar()->setContentsMargins(6,0,0,0);
    statusBar()->setStyleSheet("QStatusBar { background-color: " + STRING_VERIBLUE + "; color: white; } QStatusBar::item { border: 0px solid black; }");
    statusBar()->setFont(veriFontSmall);
    stakingLabel = new QLabel();
    stakingLabel->setFont(veriFontSmall);
    stakingLabel->setText(QString("Syncing..."));
    connectionsLabel= new QLabel();
    connectionsLabel->setFont(veriFontSmall);
    connectionsLabel->setText(QString("Connecting..."));

    QFrame *frameBlocks = new QFrame();
    frameBlocks->setContentsMargins(0,0,0,0);
    frameBlocks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    frameBlocks->setStyleSheet("color: white;");
    QHBoxLayout *frameBlocksLayout = new QHBoxLayout(frameBlocks);

    frameBlocksLayout->setContentsMargins(3,3,3,3);
    frameBlocksLayout->setSpacing(10);
    labelStakingIcon = new QLabel();
    labelConnectionsIcon = new QLabel();
    labelConnectionsIcon->setFont(veriFontSmall);
    labelBlocksIcon = new QLabel();
    labelBlocksIcon->setVisible(true);
    labelBlocksIcon->setPixmap(QIcon(":/icons/notsynced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
    QPushButton *minimize = new QPushButton();
    minimize->setFocusPolicy(Qt::NoFocus);
    QIcon *ico = new QIcon();
    ico->addPixmap(QPixmap(":/icons/maximize"),QIcon::Normal,QIcon::On);
    ico->addPixmap(QPixmap(":/icons/minimize"),QIcon::Normal,QIcon::Off);
    minimize->setIcon(*ico);
    minimize->setCheckable(true);
    minimize->setToolTip("Simple Wallet");
    connect(minimize, SIGNAL(clicked()), this, SLOT(resizeGUI()));

    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelStakingIcon);
    frameBlocksLayout->addWidget(labelBlocksIcon);
    frameBlocksLayout->addWidget(stakingLabel);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelConnectionsIcon);
    frameBlocksLayout->addWidget(connectionsLabel);


    frameBlocksLayout->addStretch();

    connect(minimize, SIGNAL(clicked()), this, SLOT(resizeGUI()));

    if (GetBoolArg("-staking", true))
    {
        QTimer *timerStakingIcon = new QTimer(labelStakingIcon);
        connect(timerStakingIcon, SIGNAL(timeout()), this, SLOT(updateStakingIcon()));
        timerStakingIcon->start(30 * 1000);
        updateStakingIcon();
    }

    // Set a timer to check for updates daily
    QTimer *tCheckForUpdate = new QTimer(this);
    connect(tCheckForUpdate, SIGNAL(timeout()), this, SLOT(timerCheckForUpdate()));
    tCheckForUpdate->start(86400 * 1000);

    // Progress bar and label for blocks download
    progressBarLabel = new QLabel();
    progressBarLabel->setFont(veriFontSmall);
    progressBarLabel->setVisible(false);
    progressBarLabel->setFrameShape(QFrame::NoFrame);
    progressBarLabel->setStyleSheet("border-color: " + STRING_VERIBLUE + "; color: white;");
    progressBar = new QProgressBar();
    progressBar->setFixedWidth(350);
    progressBar->setFont(veriFontSmall);
    progressBar->setStyleSheet("QProgressBar::chunk { background-color: " + STRING_VERIBLUE_LT + "; } QProgressBar {color: black; border-color: " + STRING_VERIBLUE + "; border-width: 2px; border-style: solid;}");
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setVisible(false);
    // Override style sheet for progress bar for styles that have a segmented progress bar,
    // as they make the text unreadable (workaround for issue #1071)
    // See https://qt-project.org/doc/qt-4.8/gallery.html
    QString curStyle = qApp->style()->metaObject()->className();
    if(curStyle == "QWindowsStyle" || curStyle == "QWindowsXPStyle")
    {
        progressBar->setStyleSheet("QProgressBar { background-color: white; color: black; border: 0px; padding: 1px; text-align: center; } QProgressBar::chunk { background: QLinearGradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #FF8000, stop: 1 " + STRING_VERIBLUE_LT + "); border-radius: 7px; margin: 0px; }");
    }

    // Progress bar and label for blockchain download/extract, and auto update
    progressBar2Label = new QLabel();
    progressBar2Label->setFont(veriFontSmall);
    progressBar2Label->setVisible(false);
    progressBar2Label->setFrameShape(QFrame::NoFrame);
    progressBar2Label->setStyleSheet("border-color: " + STRING_VERIBLUE + "; color: white;");
    progressBar2 = new QProgressBar();
    progressBar2->setFixedWidth(100);
    progressBar2->setFont(veriFontSmall);
    progressBar2->setStyleSheet("QProgressBar::chunk { background-color: " + STRING_VERIBLUE_LT + "; } QProgressBar {color: black; border-color: " + STRING_VERIBLUE + "; border-width: 2px; border-style: solid;}");
    progressBar2->setAlignment(Qt::AlignCenter);
    progressBar2->setVisible(false);
    if(curStyle == "QWindowsStyle" || curStyle == "QWindowsXPStyle")
    {
        progressBar2->setStyleSheet("QProgressBar { background-color: white; color: black; border: 0px; padding: 1px; text-align: center; } QProgressBar::chunk { background: QLinearGradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #FF8000, stop: 1 " + STRING_VERIBLUE_LT + "); border-radius: 7px; margin: 0px; }");
    }

    statusBar()->addWidget(progressBarLabel);
    statusBar()->addWidget(progressBar);
    statusBar()->addPermanentWidget(progressBar2Label);
    statusBar()->addPermanentWidget(progressBar2);
    statusBar()->addPermanentWidget(frameBlocks);

    syncIconMovie = new QMovie(":/movies/update_spinner", "mng", this);

    // Clicking on a transaction on the overview page simply sends you to transaction history page
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), this, SLOT(gotoHistoryPage()));
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), transactionView, SLOT(focusTransaction(QModelIndex)));

    // Double-clicking on a transaction on the transaction history page shows details
    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), transactionView, SLOT(showDetails()));

    rpcConsole = new RPCConsole(this);
    connect(openRPCConsoleAction, SIGNAL(triggered()), rpcConsole, SLOT(show()));

    // Clicking on "Verify Message" in the address book sends you to the verify message tab
    connect(addressBookPage, SIGNAL(verifyMessage(QString)), this, SLOT(gotoVerifyMessageTab(QString)));
    // Clicking on "Sign Message" in the receive coins page sends you to the sign message tab
    connect(receiveCoinsPage, SIGNAL(signMessage(QString)), this, SLOT(gotoSignMessageTab(QString)));

    // Clicking on "Access Nxt Inside" in the receive coins page sends you to access Nxt inside tab
	connect(receiveCoinsPage, SIGNAL(accessNxt(QString)), this, SLOT(gotoAccessNxtInsideTab(QString)));

    gotoOverviewPage();
}

BitcoinGUI::~BitcoinGUI()
{
    if(trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
#ifdef Q_OS_MAC
    delete appMenuBar;
#endif
}

void BitcoinGUI::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);

    overviewAction = new QAction(QIcon(":/icons/overview"), tr("Overview"), this);
    overviewAction->setFont(veriFontSmall);
    overviewAction->setToolTip(tr("Wallet Overview"));
    overviewAction->setCheckable(true);
    overviewAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    tabGroup->addAction(overviewAction);

    sendCoinsAction = new QAction(QIcon(":/icons/send"), tr("Send"), this);
    sendCoinsAction->setFont(veriFontSmall);
    sendCoinsAction->setToolTip(tr("Send VeriCoin"));
    sendCoinsAction->setCheckable(true);
    sendCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
    tabGroup->addAction(sendCoinsAction);

    sendBitCoinsAction = new QAction(QIcon(":/icons/veriBit"), tr("VeriBit"), this);
    sendBitCoinsAction->setFont(veriFontSmall);
    sendBitCoinsAction->setToolTip(tr("Send Bitcoin"));
    sendBitCoinsAction->setCheckable(true);
    sendBitCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
    tabGroup->addAction(sendBitCoinsAction);

    receiveCoinsAction = new QAction(QIcon(":/icons/receiving_addresses"), tr("Receive"), this);
    receiveCoinsAction->setFont(veriFontSmall);
    receiveCoinsAction->setToolTip(tr("Receive Addresses"));
    receiveCoinsAction->setCheckable(true);
    receiveCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
    tabGroup->addAction(receiveCoinsAction);

    historyAction = new QAction(QIcon(":/icons/history"), tr("History"), this);
    historyAction->setFont(veriFontSmall);
    historyAction->setToolTip(tr("Transaction History"));
    historyAction->setCheckable(true);
    historyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
    tabGroup->addAction(historyAction);

    addressBookAction = new QAction(QIcon(":/icons/address-book"), tr("Address"), this);
    addressBookAction->setFont(veriFontSmall);
    addressBookAction->setToolTip(tr("Saved Addresses"));
    addressBookAction->setCheckable(true);
    addressBookAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
    tabGroup->addAction(addressBookAction);

    fiatAction = new QAction(QIcon(":/icons/fiat"), tr("Get VRC"), this);
    fiatAction->setFont(veriFontSmall);
    fiatAction->setToolTip(tr("Buy VeriCoin with Fiat"));
    fiatAction->setCheckable(true);
    fiatAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_6));
    tabGroup->addAction(fiatAction);

    newsAction = new QAction(QIcon(":/icons/news"), tr("News"), this);
    newsAction->setFont(veriFontSmall);
    newsAction->setToolTip(tr("Get the latest VeriCoin news"));
    newsAction->setCheckable(true);
    newsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_7));
    tabGroup->addAction(newsAction);

    chatAction = new QAction(QIcon(":/icons/chat"), tr("Chat"), this);
    chatAction->setFont(veriFontSmall);
    chatAction->setToolTip(tr("Join the VeriCoin chat room"));
    chatAction->setCheckable(true);
    chatAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_8));
    tabGroup->addAction(chatAction);

    superNETAction = new QAction(QIcon(":/icons/supernet_white"), tr("SuperNET"), this);
    superNETAction->setFont(veriFontSmall);
    superNETAction->setToolTip(tr("Enter the SuperNET"));
    superNETAction->setCheckable(true);
    superNETAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_9));
    tabGroup->addAction(superNETAction);

    connect(overviewAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(sendBitCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendBitCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendBitCoinsPage()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(gotoHistoryPage()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(gotoAddressBookPage()));
    connect(fiatAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(fiatAction, SIGNAL(triggered()), this, SLOT(gotoFiatPage()));
    connect(newsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(newsAction, SIGNAL(triggered()), this, SLOT(gotoNewsPage()));
    connect(chatAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(chatAction, SIGNAL(triggered()), this, SLOT(gotoChatPage()));
    connect(superNETAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(superNETAction, SIGNAL(triggered()), this, SLOT(gotoSuperNETPage()));

    quitAction = new QAction(QIcon(":/icons/quit"), tr("E&xit"), this);
    quitAction->setToolTip(tr("Quit application"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setMenuRole(QAction::QuitRole);
    aboutAction = new QAction(QIcon(":/icons/bitcoin"), tr("&About VeriCoin"), this);
    aboutAction->setToolTip(tr("Show information about VeriCoin"));
    aboutAction->setMenuRole(QAction::AboutRole);
    aboutQtAction = new QAction(QIcon(":icons/about-qt"), tr("About &Qt"), this);
    aboutQtAction->setToolTip(tr("Show information about Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    optionsAction = new QAction(QIcon(":/icons/options"), tr("&Options..."), this);
    optionsAction->setToolTip(tr("Modify configuration options for VeriCoin"));
    optionsAction->setMenuRole(QAction::PreferencesRole);
    toggleHideAction = new QAction(QIcon(":/icons/bitcoin"), tr("&Show / Hide"), this);
    encryptWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Set Password..."), this);
    encryptWalletAction->setToolTip(tr("Encrypt or decrypt wallet"));
    backupWalletAction = new QAction(QIcon(":/icons/filesave"), tr("&Backup Wallet..."), this);
    backupWalletAction->setToolTip(tr("Backup wallet to another location"));
    changePassphraseAction = new QAction(QIcon(":/icons/key"), tr("&Change Password..."), this);
    changePassphraseAction->setToolTip(tr("Change the passphrase used for wallet encryption"));
    unlockWalletAction = new QAction(QIcon(":/icons/lock_open"), tr("&Unlock Wallet for Staking..."), this);
    unlockWalletAction->setToolTip(tr("Unlock wallet for staking"));
    signMessageAction = new QAction(QIcon(":/icons/edit"), tr("Sign &Message..."), this);
    verifyMessageAction = new QAction(QIcon(":/icons/transaction_0"), tr("&Verify Message..."), this);
    accessNxtInsideAction = new QAction(QIcon(":/icons/supernet"), tr("Enter &SuperNET..."), this);
    reloadBlockchainAction = new QAction(QIcon(":/icons/blockchain"), tr("&Reload Blockchain..."), this);
    reloadBlockchainAction->setToolTip(tr("Reload the blockchain from bootstrap."));
    rescanBlockchainAction = new QAction(QIcon(":/icons/rescan"), tr("Re&scan Blockchain..."), this);
    rescanBlockchainAction->setToolTip(tr("Restart and rescan the blockchain."));
    checkForUpdateAction = new QAction(QIcon(":/icons/tx_inout"), tr("Check For &Update..."), this);
    checkForUpdateAction->setToolTip(tr("Check for a new version of the wallet and update."));
    forumsAction = new QAction(QIcon(":/icons/bitcoin"), tr("VeriCoin &Forums..."), this);
    forumsAction->setToolTip(tr("Go to VeriCoin forums."));
    webAction = new QAction(QIcon(":/icons/bitcoin"), tr("VeriCoin on the &Web..."), this);
    webAction->setToolTip(tr("Go to VeriCoin website."));

    exportAction = new QAction(QIcon(":/icons/export"), tr("&Export..."), this);
    exportAction->setToolTip(tr("Export the data in the current tab to a file"));
    openRPCConsoleAction = new QAction(QIcon(":/icons/debugwindow"), tr("&Console"), this);
    openRPCConsoleAction->setToolTip(tr("Open debugging and diagnostic console"));

    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(encryptWalletAction, SIGNAL(triggered(bool)), this, SLOT(encryptWallet(bool)));
    connect(backupWalletAction, SIGNAL(triggered()), this, SLOT(backupWallet()));
    connect(changePassphraseAction, SIGNAL(triggered()), this, SLOT(changePassphrase()));
    connect(unlockWalletAction, SIGNAL(triggered()), this, SLOT(unlockWallet()));
    connect(signMessageAction, SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
    connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
    connect(accessNxtInsideAction, SIGNAL(triggered()), this, SLOT(gotoAccessNxtInsideTab()));
    connect(reloadBlockchainAction, SIGNAL(triggered()), this, SLOT(reloadBlockchain()));
    connect(rescanBlockchainAction, SIGNAL(triggered()), this, SLOT(rescanBlockchain()));
    connect(checkForUpdateAction, SIGNAL(triggered()), this, SLOT(menuCheckForUpdate()));
    connect(forumsAction, SIGNAL(triggered()), this, SLOT(forumsClicked()));
    connect(webAction, SIGNAL(triggered()), this, SLOT(webClicked()));
}

void BitcoinGUI::createMenuBar()
{
#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();
#endif
    appMenuBar->setFont(veriFont);

    // Configure the menus
    QMenu *file = appMenuBar->addMenu(tr("&File"));
    file->setFont(veriFont);
    file->addAction(backupWalletAction);
    file->addAction(exportAction);
    file->addSeparator();
    file->addAction(signMessageAction);
    file->addAction(verifyMessageAction);
    file->addSeparator();
    file->addAction(accessNxtInsideAction);
    file->addSeparator();
    file->addAction(reloadBlockchainAction);
    file->addAction(rescanBlockchainAction);
    file->addSeparator();
    file->addAction(quitAction);

    QMenu *settings = appMenuBar->addMenu(tr("&Settings"));
    settings->setFont(veriFont);
    settings->addAction(encryptWalletAction);
    settings->addAction(changePassphraseAction);
    settings->addSeparator();
    settings->addAction(unlockWalletAction);
    settings->addSeparator();
    settings->addAction(optionsAction);

    QMenu *help = appMenuBar->addMenu(tr("&Help"));
    help->setFont(veriFont);
    help->addAction(openRPCConsoleAction);
    help->addSeparator();
    help->addAction(forumsAction);
    help->addAction(webAction);
    help->addSeparator();
    help->addAction(checkForUpdateAction);
    help->addSeparator();
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);
}

void BitcoinGUI::createToolBars()
{
    QToolBar *toolbar = addToolBar(tr("Tabs toolbar"));
    addToolBar(Qt::LeftToolBarArea, toolbar);
    toolbar->setMovable(false);
    toolbar->setAutoFillBackground(true);
    toolbar->setFont(veriFontSmall);
    toolbar->setOrientation(Qt::Vertical);
    toolbar->setIconSize(QSize(60,32));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setStyleSheet("QToolBar { background: " + STRING_VERIBLUE + "; color: white; border: none; } \
                           QToolButton { background-color: " + STRING_VERIBLUE + "; color: white; border: none; width: 60px; padding-bottom: 8px; } \
                           QToolButton:hover { background-color: " + STRING_VERIBLUE_LT + "; color: white; border: none; width: 60px; padding-bottom: 8px; } \
                           QToolButton:pressed { background-color: " + STRING_VERIBLUE_LT + "; color: white; border: none; width: 60px; padding-bottom: 8px; } \
                           QToolButton:checked { background-color: " + STRING_VERIBLUE_LT + "; color: white; border: none; width: 60px; padding-bottom: 8px; }");
    toolbar->addAction(overviewAction);
    toolbar->addAction(sendCoinsAction);
    toolbar->addAction(receiveCoinsAction);
    toolbar->addAction(historyAction);
    toolbar->addAction(addressBookAction);
    toolbar->addAction(sendBitCoinsAction);
    toolbar->addAction(fiatAction);
    toolbar->addAction(newsAction);
    toolbar->addAction(chatAction);
    toolbar->addAction(superNETAction);
}

void BitcoinGUI::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
    if(clientModel)
    {
        // Replace some strings and icons, when using the testnet
        if(clientModel->isTestNet())
        {
            setWindowTitle(windowTitle() + QString(" ") + tr("[testnet]"));
#ifndef Q_OS_MAC
            qApp->setWindowIcon(QIcon(":icons/bitcoin_testnet"));
            setWindowIcon(QIcon(":icons/bitcoin_testnet"));
#else
            MacDockIconHandler::instance()->setIcon(QIcon(":icons/bitcoin_testnet"));
#endif
            if(trayIcon)
            {
                trayIcon->setToolTip(tr("VeriCoin Wallet") + QString(" ") + tr("[testnet]"));
                trayIcon->setIcon(QIcon(":/icons/toolbar_testnet"));
                toggleHideAction->setIcon(QIcon(":/icons/toolbar_testnet"));
            }

            aboutAction->setIcon(QIcon(":/icons/toolbar_testnet"));
        }

        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(clientModel->getNumBlocks(), clientModel->getNumBlocksOfPeers());
        connect(clientModel, SIGNAL(numBlocksChanged(int,int)), this, SLOT(setNumBlocks(int,int)));

        // Report errors from network/worker thread
        connect(clientModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        rpcConsole->setClientModel(clientModel);
        addressBookPage->setOptionsModel(clientModel->getOptionsModel());
        receiveCoinsPage->setOptionsModel(clientModel->getOptionsModel());
    }
}

void BitcoinGUI::setWalletModel(WalletModel *walletModel)
{
    this->walletModel = walletModel;
    if(walletModel)
    {
        // Report errors from wallet thread
        connect(walletModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        // Put transaction list in tabs
        transactionView->setModel(walletModel);

        overviewPage->setModel(walletModel);
        addressBookPage->setModel(walletModel->getAddressTableModel());
        receiveCoinsPage->setModel(walletModel->getAddressTableModel());
        sendCoinsPage->setModel(walletModel);
        sendBitCoinsPage->setModel(walletModel);
        signVerifyMessageDialog->setModel(walletModel);
        accessNxtInsideDialog->setModel(walletModel);

        setEncryptionStatus(walletModel->getEncryptionStatus());
        connect(walletModel, SIGNAL(encryptionStatusChanged(int)), this, SLOT(setEncryptionStatus(int)));

        // Balloon pop-up for new transaction
        connect(walletModel->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(incomingTransaction(QModelIndex,int,int)));

        // Ask for passphrase if needed
        connect(walletModel, SIGNAL(requireUnlock()), this, SLOT(unlockWallet()));
    }
}

void BitcoinGUI::createTrayIcon()
{
    QMenu *trayIconMenu;
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("VeriCoin Wallet"));
    trayIcon->setIcon(QIcon(":/icons/toolbar"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();
#else
    // Note: On Mac, the dock icon is used to provide the tray's functionality.
    MacDockIconHandler *dockIconHandler = MacDockIconHandler::instance();
    dockIconHandler->setMainWindow((QMainWindow *)this);
    trayIconMenu = dockIconHandler->dockMenu();
#endif

    // Configuration of the tray icon (or dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(sendCoinsAction);
    trayIconMenu->addAction(receiveCoinsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(signMessageAction);
    trayIconMenu->addAction(verifyMessageAction);
    trayIconMenu->addSeparator();
	trayIconMenu->addAction(accessNxtInsideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(optionsAction);
    trayIconMenu->addAction(openRPCConsoleAction);
#ifndef Q_OS_MAC // This is built-in on Mac
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif

    notificator = new Notificator(qApp->applicationName(), trayIcon);
}

#ifndef Q_OS_MAC
void BitcoinGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        // Click on system tray icon triggers show/hide of the main window
        toggleHideAction->trigger();
    }
}
#endif

void BitcoinGUI::optionsClicked()
{
    if(!clientModel || !clientModel->getOptionsModel())
        return;
    OptionsDialog dlg;
    dlg.setModel(clientModel->getOptionsModel());
    dlg.exec();
}

void BitcoinGUI::forumsClicked()
{
    QString link("http://www.vericoinforums.com");
    QDesktopServices::openUrl(QUrl(link));
}

void BitcoinGUI::webClicked()
{
    QString link(walletUrl);
    QDesktopServices::openUrl(QUrl(link));
}

void BitcoinGUI::aboutClicked()
{
    AboutDialog dlg;
    dlg.setModel(clientModel);
    dlg.exec();
}

void BitcoinGUI::setNumConnections(int count)
{
    QString icon;
    switch(count)
    {
    case 0: icon = ":/icons/connect_0"; break;
    case 1: case 2: case 3: icon = ":/icons/connect_1"; break;
    case 4: case 5: case 6: icon = ":/icons/connect_2"; break;
    case 7: case 8: case 9: icon = ":/icons/connect_3"; break;
    default: icon = ":/icons/connect_4"; break;
    }
    QString connections = QString::number(count);
    QString label = " connections";
    QString connectionlabel = connections + label;
    connectionsLabel->setText(QString(connectionlabel));
    labelConnectionsIcon->setPixmap(QIcon(icon).pixmap(72,STATUSBAR_ICONSIZE));
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to VeriCoin network", "", count));
}

void BitcoinGUI::setNumBlocks(int count, int nTotalBlocks)
{
    // don't show / hide progress bar and its label if we have no connection to the network
    if (!clientModel || clientModel->getNumConnections() == 0)
    {
        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);

        return;
    }

    QString strStatusBarWarnings = clientModel->getStatusBarWarnings();
    QString tooltip;

    if(count < nTotalBlocks)
    {
        int nRemainingBlocks = nTotalBlocks - count;
        float nPercentageDone = count / (nTotalBlocks * 0.01f);

        if (strStatusBarWarnings.isEmpty())
        {
            progressBarLabel->setText(tr("Synchronizing..."));
            progressBarLabel->setVisible(true);
            progressBar->setFormat(tr("~%n block(s) remaining", "", nRemainingBlocks));
            progressBar->setMaximum(nTotalBlocks);
            progressBar->setValue(count);
            progressBar->setVisible(true);
        }

        tooltip = tr("Downloaded %1 of %2 blocks of transaction history (%3% done).").arg(count).arg(nTotalBlocks).arg(nPercentageDone, 0, 'f', 2);
    }
    else
    {
        if (strStatusBarWarnings.isEmpty())
            progressBarLabel->setVisible(false);

        progressBar->setVisible(false);
        tooltip = tr("Downloaded %1 blocks of transaction history.").arg(count);
    }

    // Override progressBarLabel text and hide progress bar, when we have warnings to display
    if (!strStatusBarWarnings.isEmpty())
    {
        progressBarLabel->setText(strStatusBarWarnings);
        progressBarLabel->setVisible(true);
        progressBar->setVisible(false);
    }

    QDateTime lastBlockDate = clientModel->getLastBlockDate();
    int secs = lastBlockDate.secsTo(QDateTime::currentDateTime());
    QString text;

    // Represent time from last generated block in human readable text
    if(secs <= 0)
    {
        // Fully up to date. Leave text empty.
    }
    else if(secs < 60)
    {
        text = tr("%n second(s) ago","",secs);
    }
    else if(secs < 60*60)
    {
        text = tr("%n minute(s) ago","",secs/60);
    }
    else if(secs < 24*60*60)
    {
        text = tr("%n hour(s) ago","",secs/(60*60));
    }
    else
    {
        text = tr("%n day(s) ago","",secs/(60*60*24));
    }

    // Set icon state: spinning if catching up, tick otherwise
    if(secs < 90*60 && count >= nTotalBlocks)
    {
        //tooltip = tr("Up to date") + QString(".<br>") + tooltip;
        //labelBlocksIcon->setPixmap(QIcon(":/icons/synced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
        //labelBlocksIcon->hide();
        overviewPage->showOutOfSyncWarning(false);
    }
    else
    {
        stakingLabel->setText(QString("Syncing..."));
        labelStakingIcon->hide();
        labelBlocksIcon->show();
        tooltip = tr("Syncing") + QString(".<br>") + tooltip;
        labelBlocksIcon->setPixmap(QIcon(":/icons/notsynced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));

        overviewPage->showOutOfSyncWarning(true);
    }

    if(!text.isEmpty())
    {
        tooltip += QString("<br>");
        tooltip += tr("Last received block was generated %1.").arg(text);
    }

    // Don't word-wrap this (fixed-width) tooltip
    tooltip = QString("<nobr>") + tooltip + QString("</nobr>");

    labelBlocksIcon->setToolTip(tooltip);
    progressBarLabel->setToolTip(tooltip);
    progressBar->setToolTip(tooltip);
}

void BitcoinGUI::error(const QString &title, const QString &message, bool modal)
{
    // Report errors from network/worker thread
    if(modal)
    {
        QMessageBox::critical(this, title, message, QMessageBox::Ok, QMessageBox::Ok);
    } else {
        notificator->notify(Notificator::Critical, title, message);
    }
}

void BitcoinGUI::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC // Ignored on Mac
    if(e->type() == QEvent::WindowStateChange)
    {
        if(clientModel && clientModel->getOptionsModel()->getMinimizeToTray())
        {
            QWindowStateChangeEvent *wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if(!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized())
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}

void BitcoinGUI::closeEvent(QCloseEvent *event)
{
    if(clientModel)
    {
#ifndef Q_OS_MAC // Ignored on Mac
        if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
           !clientModel->getOptionsModel()->getMinimizeOnClose())
        {
            qApp->quit();
        }
#endif
    }
    QMainWindow::closeEvent(event);
}

void BitcoinGUI::askFee(qint64 nFeeRequired, bool *payFee)
{
    QString strMessage =
        tr("This transaction is over the size limit.  You can still send it for a fee of %1, "
          "which goes to the nodes that process your transaction and helps to support the network.  "
          "Do you want to pay the fee?").arg(
                BitcoinUnits::formatWithUnit(BitcoinUnits::VRC, nFeeRequired));
    QMessageBox::StandardButton retval = QMessageBox::question(
          this, tr("Confirm transaction fee"), strMessage,
          QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Yes);
    *payFee = (retval == QMessageBox::Yes);
}

void BitcoinGUI::incomingTransaction(const QModelIndex & parent, int start, int end)
{
    if(!walletModel || !clientModel)
        return;
    TransactionTableModel *ttm = walletModel->getTransactionTableModel();
    qint64 amount = ttm->index(start, TransactionTableModel::Amount, parent)
                    .data(Qt::EditRole).toULongLong();
    if(!clientModel->inInitialBlockDownload())
    {
        BitcoinUnits *bcu = new BitcoinUnits(this, walletModel);
        // On new transaction, make an info balloon
        // Unless the initial block download is in progress, to prevent balloon-spam
        QString date = ttm->index(start, TransactionTableModel::Date, parent)
                        .data().toString();
        QString type = ttm->index(start, TransactionTableModel::Type, parent)
                        .data().toString();
        QString address = ttm->index(start, TransactionTableModel::ToAddress, parent)
                        .data().toString();
        QIcon icon = qvariant_cast<QIcon>(ttm->index(start,
                            TransactionTableModel::ToAddress, parent)
                        .data(Qt::DecorationRole));

        notificator->notify(Notificator::Information,
                            (amount)<0 ? tr("Sent transaction") :
                                         tr("Incoming transaction"),
                              tr("Date: %1\n"
                                 "Amount: %2\n"
                                 "Type: %3\n"
                                 "Address: %4\n")
                              .arg(date)
                              .arg(bcu->formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), amount, true))
                              .arg(type)
                              .arg(address), icon);
        delete bcu;
    }
}

void BitcoinGUI::gotoOverviewPage()
{
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: white; }");
    overviewPage->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }");
    overviewAction->setChecked(true);
    centralWidget->setCurrentWidget(overviewPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoHistoryPage()
{
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: white; }");
    transactionsPage->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }");
    historyAction->setChecked(true);
    centralWidget->setCurrentWidget(transactionsPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), transactionView, SLOT(exportClicked()));
}

void BitcoinGUI::gotoAddressBookPage()
{
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: white; }");
    addressBookPage->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }");
    addressBookAction->setChecked(true);
    centralWidget->setCurrentWidget(addressBookPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), addressBookPage, SLOT(exportClicked()));
}

void BitcoinGUI::gotoReceiveCoinsPage()
{
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: white; }");
    receiveCoinsPage->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }");
    receiveCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(receiveCoinsPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), receiveCoinsPage, SLOT(exportClicked()));
}

void BitcoinGUI::gotoSendCoinsPage()
{
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: white; }");
    sendCoinsPage->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }");
    sendCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(sendCoinsPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoSendBitCoinsPage()
{
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: white; }");
    sendBitCoinsPage->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }");
    sendBitCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(sendBitCoinsPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoFiatPage()
{
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: white; }");
    fiatPage->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }");
    QUrl url(QString(walletUrl).append("wallet/fiat.html"));

    if (!fFiatPageLoaded)
    {
        fiatPage->findChild<QWebView *>("webView")->load(url);
        fFiatPageLoaded = true;
    }

    fiatAction->setChecked(true);
    centralWidget->setCurrentWidget(fiatPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoNewsPage()
{
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: #EBEBEB; }");
    newsPage->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }");
    QUrl url(QString(walletUrl).append("wallet/news.html"));

    if (!fNewsPageLoaded)
    {
        newsPage->findChild<QWebView *>("webView")->load(url);
        fNewsPageLoaded = true;
    }

    newsAction->setChecked(true);
    centralWidget->setCurrentWidget(newsPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoChatPage()
{
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: white; }");
    chatPage->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }");
    QUrl url(QString(walletUrl).append("wallet/chat.html"));

    if (!fChatPageLoaded)
    {
        chatPage->findChild<QWebView *>("webView")->load(url);
        fChatPageLoaded = true;
    }

    chatAction->setChecked(true);
    centralWidget->setCurrentWidget(chatPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoSuperNETPage()
{
    centralWidget->setStyleSheet("QStackedWidget { background: url(:images/headerOverview) no-repeat 0px 0px; padding-top: 160px; background-color: white; }");
    superNETPage->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }");
    QUrl url(QString(walletUrl).append("wallet/supernet.html"));

    if (!fSuperNETPageLoaded)
    {
        superNETPage->findChild<QWebView *>("webView")->load(url);
        fSuperNETPageLoaded = true;
    }

    superNETAction->setChecked(true);
    centralWidget->setCurrentWidget(superNETPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::resizeGUI()
{
    if (clientModel->getOptionsModel()->getCoinControlFeatures())
    {
        QMessageBox::warning(this, tr("Simple Wallet"),
            tr("%1").
            arg("Please turn coin control off in options to use the simple wallet."),
            QMessageBox::Ok, QMessageBox::Ok);
        bool checked = dynamic_cast<QPushButton*>(sender())->isChecked();
        dynamic_cast<QPushButton*>(sender())->setChecked(!checked);
        return;
    }
    else
    {
        if (QMainWindow::height() > 270)
        {
           resize(880, 256);
           #ifdef Q_OS_WIN
                resize(880, 246);
           #endif
           #ifdef Q_OS_MAC
                resize(880, 246);
           #endif
           dynamic_cast<QPushButton*>(sender())->setToolTip("Full Wallet");
        }
        else
        {
           resize(880, 720);
           #ifdef Q_OS_WIN
                resize(880, 710);
           #endif
           #ifdef Q_OS_MAC
                resize(880, 702);
           #endif
           dynamic_cast<QPushButton*>(sender())->setToolTip("Simple Wallet");
        }
    }
}

void BitcoinGUI::gotoSignMessageTab(QString addr)
{
    // call show() in showTab_SM()
    signVerifyMessageDialog->showTab_SM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_SM(addr);
}

void BitcoinGUI::gotoVerifyMessageTab(QString addr)
{
    // call show() in showTab_VM()
    signVerifyMessageDialog->showTab_VM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_VM(addr);

}

void BitcoinGUI::gotoAccessNxtInsideTab(QString addr)
{
    // call show() in showTab_AN()
    accessNxtInsideDialog->showTab_AN(true);

    if(!addr.isEmpty())
        accessNxtInsideDialog->setAddress_AN(addr);
}

void BitcoinGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only URIs
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BitcoinGUI::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        int nValidUrisFound = 0;
        int nValidUrisFoundBit = 0;
        QList<QUrl> uris = event->mimeData()->urls();
        foreach(const QUrl &uri, uris)
        {
            if (sendCoinsPage->handleURI(uri.toString()))
                nValidUrisFound++;
            else if (sendBitCoinsPage->handleURI(uri.toString()))
                nValidUrisFoundBit++;
        }

        // if valid URIs were found
        if (nValidUrisFound)
            gotoSendCoinsPage();
        else if (nValidUrisFoundBit)
            gotoSendBitCoinsPage();
        else
            notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid VeriCoin address or malformed URI parameters."));
    }

    event->acceptProposedAction();
}

void BitcoinGUI::handleURI(QString strURI)
{
    // URI has to be valid
    if (sendCoinsPage->handleURI(strURI))
    {
        showNormalIfMinimized();
        gotoSendCoinsPage();
    }
    else if (sendBitCoinsPage->handleURI(strURI))
    {
        showNormalIfMinimized();
        gotoSendBitCoinsPage();
    }
    else
        notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid VeriCoin address or malformed URI parameters."));
}

void BitcoinGUI::setEncryptionStatus(int status)
{
    switch(status)
    {
    case WalletModel::Unencrypted:
        encryptWalletAction->setChecked(false);
        changePassphraseAction->setEnabled(false);
        unlockWalletAction->setEnabled(false);
        encryptWalletAction->setEnabled(true);
        break;
    case WalletModel::Unlocked:
        //labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        //labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setEnabled(false);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    case WalletModel::Locked:
        //labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        //labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>locked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setEnabled(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    }
}

void BitcoinGUI::encryptWallet(bool status)
{
    if(!walletModel)
        return;
    AskPassphraseDialog dlg(status ? AskPassphraseDialog::Encrypt:
                                     AskPassphraseDialog::Decrypt, this);
    dlg.setModel(walletModel);
    dlg.exec();

    setEncryptionStatus(walletModel->getEncryptionStatus());
}

void BitcoinGUI::backupWallet()
{
    //QString saveDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    QString saveDir = GetDataDir().string().c_str();
    QString filename = QFileDialog::getSaveFileName(this, tr("Backup Wallet"), saveDir, tr("Wallet Data (*.dat)"));
    if(!filename.isEmpty()) {
        if(!walletModel->backupWallet(filename)) {
            QMessageBox::warning(this, tr("Backup Failed"), tr("There was an error trying to save the wallet data to the new location."));
        }
    }
}

void BitcoinGUI::changePassphrase()
{
    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this);
    dlg.setModel(walletModel);
    dlg.exec();
}

void BitcoinGUI::unlockWallet()
{
    if(!walletModel)
        return;
    // Unlock wallet when requested by wallet model
    if(walletModel->getEncryptionStatus() == WalletModel::Locked)
    {
        AskPassphraseDialog dlg(AskPassphraseDialog::Unlock, this);
        dlg.setModel(walletModel);
        dlg.exec();
    }
}

void BitcoinGUI::showNormalIfMinimized(bool fToggleHidden)
{
    // activateWindow() (sometimes) helps with keyboard focus on Windows
    if (isHidden())
    {
        show();
        activateWindow();
    }
    else if (isMinimized())
    {
        showNormal();
        activateWindow();
    }
    else if (GUIUtil::isObscured(this))
    {
        raise();
        activateWindow();
    }
    else if(fToggleHidden)
        hide();
}

void BitcoinGUI::toggleHidden()
{
    showNormalIfMinimized(true);
}

void BitcoinGUI::updateStakingIcon()
{
    QDateTime lastBlockDate = clientModel->getLastBlockDate();
    int secs = lastBlockDate.secsTo(QDateTime::currentDateTime());
    int64_t currentBlock = clientModel->getNumBlocks();
    int peerBlock = clientModel->getNumBlocksOfPeers();
    if((secs >= 90*60 && currentBlock < peerBlock) || !pwalletMain)
    {
        return;
    }
    uint64_t nMinWeight = 0, nMaxWeight = 0, nWeight = 0;
    pwalletMain->GetStakeWeight(*pwalletMain, nMinWeight, nMaxWeight, nWeight);
    progressBarLabel->setVisible(false);
    progressBar->setVisible(false);
    overviewPage->showOutOfSyncWarning(false);
    if (nLastCoinStakeSearchInterval && nWeight)
    {
        uint64_t nNetworkWeight = GetPoSKernelPS();
        unsigned nEstimateTime = nTargetSpacing * nNetworkWeight / nWeight;

        QString text;
        if (nEstimateTime < 60)
        {
            text = tr("%n second(s)", "", nEstimateTime);
        }
        else if (nEstimateTime < 60*60)
        {
            text = tr("%n minute(s)", "", nEstimateTime/60);
        }
        else if (nEstimateTime < 24*60*60)
        {
            text = tr("%n hour(s)", "", nEstimateTime/(60*60));
        }
        else
        {
            text = tr("%n day(s)", "", nEstimateTime/(60*60*24));
        }
        stakingLabel->setText(QString("Staking"));
        labelBlocksIcon->hide();
        labelStakingIcon->show();
        labelStakingIcon->setPixmap(QIcon(":/icons/staking_on").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelStakingIcon->setToolTip(tr("In sync and staking.<br>Block number %1<br>Expected time to earn interest is: %2").arg(currentBlock).arg(text));
    }
    else
    {
        stakingLabel->setText(QString("In sync"));
        labelBlocksIcon->hide();
        labelStakingIcon->show();
        labelStakingIcon->setPixmap(QIcon(":/icons/staking_off").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        if (pwalletMain && pwalletMain->IsLocked())
            labelStakingIcon->setToolTip(tr("In sync at block %1<br>Not staking, wallet is locked<br>Unlock wallet in settings").arg(currentBlock));
        else if (vNodes.empty())
            labelStakingIcon->setToolTip(tr("Out of sync and not staking because wallet is offline"));
        else
            labelStakingIcon->setToolTip(tr("In sync at block %1 <br> Not staking because you don't have mature coins").arg(currentBlock));
    }
}

void BitcoinGUI::openUrl(const QUrl &url)
{
    if (!isTrustedUrl(url))
    {
        printf("User requested URL: %s\n", url.toString().toStdString().c_str());
        QDesktopServices::openUrl(url);
    }
    else
    {
        qobject_cast<QWebView *>(sender())->load(url);
    }
}

void BitcoinGUI::sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist)
{

  // Show list of all ssl errors
  foreach (QSslError err, errlist)
      printf((QString("sslErrorHandler Url: %1 , Error: %2\n").arg(qnr->url().toString()).arg(err.errorString())).toAscii());

   qnr->ignoreSslErrors();
}

void BitcoinGUI::reloadBlockchainActionEnabled(bool enabled)
{
    reloadBlockchainAction->setEnabled(enabled);
}

void BitcoinGUI::reloadBlockchain()
{
    reloadBlockchainActionEnabled(false); // Sets back to true when dialog closes.

    boost::filesystem::path pathBootstrap(GetDataDir() / "bootstrap.zip");
    QUrl url(QString(walletDownloadsUrl).append("bootstrap.zip"));

    if (boost::filesystem::exists(pathBootstrap))
    {
        std::time_t bs_time = boost::filesystem::last_write_time(pathBootstrap);
        if (std::difftime(std::time(0), bs_time) > (48 * 60 * 60)) // 48 hour time difference
        {
            // Old boostrap, remove it.
            boost::filesystem::remove(pathBootstrap);
        }
    }

    printf("Downloading blockchain data...\n");
    Downloader *bs = new Downloader(this, walletModel);
    bs->setWindowTitle("Blockchain Reload");
    bs->setUrl(url);
    bs->setDest(boostPathToQString(pathBootstrap));
    bs->processBlockchain = true;
    if (GetBoolArg("-bootstrapturbo")) // Get bootsrap in auto mode
    {
        bs->autoDownload = true;
        bs->startDownload();
    }
    bs->show();
}

void BitcoinGUI::rescanBlockchain()
{
    // No turning back. Ask permission.
    RescanDialog rs;
    rs.setModel(clientModel);
    rs.exec();
    if (!rs.rescanAccepted)
    {
        return;
    }

    if (!walletModel->rescanBlockchain())
    {
        QMessageBox::warning(this, tr("Rescan Failed"), tr("There was an error trying to rescan the blockchain."));
    }
}

// Called by user
void BitcoinGUI::menuCheckForUpdate()
{
    fMenuCheckForUpdate = true;

    if (!fTimerCheckForUpdate)
        checkForUpdate();

    fMenuCheckForUpdate = false;
}

// Called by timer
void BitcoinGUI::timerCheckForUpdate()
{
    fTimerCheckForUpdate = true;

    if (!fMenuCheckForUpdate)
        checkForUpdate();

    fTimerCheckForUpdate = false;
}

// Called by external (bitcoin.cpp)
void BitcoinGUI::CheckForUpdate()
{
    if (!fMenuCheckForUpdate && !fTimerCheckForUpdate)
        checkForUpdate();
}

void BitcoinGUI::checkForUpdate()
{
    boost::filesystem::path fileName(GetProgramDir());
    QUrl url;

    printf("Downloading and parsing version data...\n");
    ReadVersionFile();

    if (fNewVersion)
    {
        // No turning back. Ask permission.
        UpdateDialog ud;
        ud.setModel(clientModel);
        ud.exec();
        if (!ud.updateAccepted)
        {
            return;
        }

        std::string basename = GetArg("-vFileName","vericoin-qt");
        fileName = fileName / basename.c_str();
        url.setUrl(QString(walletDownloadsUrl).append(basename.c_str()));

        printf("Downloading new wallet...\n");
        Downloader * w = new Downloader(this);
        w->setWindowTitle("Wallet Download");
        w->setUrl(url);
        w->setDest(boostPathToQString(fileName));
        w->autoDownload = true;
        w->startDownload();
        w->exec();
        if (!w->downloadFinished)
        {
            printf("Update download failed!\n");
            QMessageBox::warning(this, tr("Update Failed"), tr("There was an error trying to download the wallet.  Please try again later."));
            fNewVersion = false;
            delete w;
            return;
        }
        delete w;

#if !defined(WIN32) && !defined(MAC_OSX)
        // If Linux, extract zip contents and make vericoin-qt executable then restart.
        // Test the archive by looking for the version number that was in VERSION.json.
        QStringList zlist = JlCompress::getFileList(boostPathToQString(fileName), -1);
        if (!zlist.isEmpty() && zlist[0].contains(GetArg("-vVersion","1.0.0.0").c_str()))
        {
            printf("Update structure is valid.\n");
        }
        else
        {
            printf("Update structure is invalid!\n");
            return;
        }
        // Extract the Update
        QStringList zextracted = JlCompress::extractDir(this, boostPathToQString(fileName), boostPathToQString(fileName.parent_path()));
        if (!zextracted.isEmpty())
        {
            printf("Update extract successful.\n");
        }
        else
        {
            printf("Update extract failed!\n");
            return;
        }

        if (!boost::filesystem::exists(GetProgramDir() / zlist[0].toStdString().append("vericoin-qt")))
        {
            printf("Update extract is invalid!\n");
            return;
        }
        try
        {
            // Rename the old and move in the new binary then make sure it's executable
            boost::filesystem::rename(GetArg("-programpath","vericoin-qt"), GetArg("-programpath","vericoin-qt").append(".old"));
            boost::filesystem::rename(zlist[0].toStdString().append("vericoin-qt"), GetArg("-programpath","vericoin-qt"));
            boost::filesystem::permissions(boost::filesystem::path(GetArg("-programpath","vericoin-qt")), boost::filesystem::others_exe | boost::filesystem::owner_exe);
            // Rename the old and move in the new config
            boost::filesystem::rename(GetConfigFile(), GetConfigFile().operator +=(".old"));
            boost::filesystem::rename(zlist[0].toStdString().append("vericoin.conf"), GetConfigFile());
            // Get the README
            boost::filesystem::rename(zlist[0].toStdString().append("README"), "README");
            boost::filesystem::remove_all(zlist[0].toStdString());
        }
        catch (std::exception &e) {
            printf("Update filesystem error!\n");
            return;
        }
#endif // Linux

        // Restart with the executable.
        if (!walletModel->checkForUpdate())
        {
            QMessageBox::warning(this, tr("Update Failed"), tr("There was an error trying to update the wallet."));
        }
    }
    else
    {
        if (fMenuCheckForUpdate)
        {
            QMessageBox::about(this, tr("Update Not Required"), tr("You have the most current wallet version. No update required."));
        }
    }
}
