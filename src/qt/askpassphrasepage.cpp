#include "askpassphrasepage.h"
#include "ui_askpassphrasepage.h"
#include "init.h"
#include "util.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "walletmodel.h"
#include "bitcoingui.h"

#include <QPushButton>
#include <QKeyEvent>

using namespace GUIUtil;

AskPassphrasePage::AskPassphrasePage(Mode mode, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AskPassphrasePage),
    mode(mode),
    model(0),
    fCapsLock(false)
{
    // Setup header and styles
    if (fNoHeaders)
        GUIUtil::header(this, QString(""));
    else if (fSmallHeaders)
        GUIUtil::header(this, QString(":images/headerOverviewSmall"));
    else
        GUIUtil::header(this, QString(":images/headerOverview"));

    ui->setupUi(this);
    this->layout()->setContentsMargins(0, 0 + HEADER_HEIGHT, 0, 0);
    this->setStyleSheet(GUIUtil::veriStyleSheet);
    this->setFont(veriFont);

    ui->passEdit1->setMaxLength(MAX_PASSPHRASE_SIZE);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    // Setup Caps Lock detection.
    ui->passEdit1->installEventFilter(this);

    switch(mode)
    {
        case Unlock: // Ask passphrase
            ui->warningLabel->setText(tr("Plese enter your passphrase to unlock the wallet."));
            break;
    }

    connect(ui->passEdit1, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    connect(ui->passEdit1, SIGNAL(returnPressed()), this, SLOT(ok_clicked()));
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(ok_clicked()));
}

AskPassphrasePage::~AskPassphrasePage()
{
    // Attempt to overwrite text so that they do not linger around in memory
    ui->passEdit1->setText(QString(" ").repeated(ui->passEdit1->text().size()));
    delete ui;
}

void AskPassphrasePage::setModel(WalletModel *model)
{
    this->model = model;
}

void AskPassphrasePage::ok_clicked()
{
    SecureString oldpass;
    if(!model)
        return;

    oldpass.reserve(MAX_PASSPHRASE_SIZE);
    // TODO: get rid of this .c_str() by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make this input mlock()'d to begin with.
    oldpass.assign(ui->passEdit1->text().toStdString().c_str());

    switch(mode)
    {
    case Unlock:
        if(!model->setWalletLocked(false, oldpass))
        {
            ui->warningLabel->setText(tr("The passphrase entered for the wallet is incorrect."));
        }
        else
        {
            ui->warningLabel->clear();
            // Attempt to overwrite text so that they do not linger around in memory
            ui->passEdit1->setText(QString(" ").repeated(ui->passEdit1->text().size()));

            emit unlockWalletFeatures();
        }
        break;
    }
}

void AskPassphrasePage::textChanged()
{
    // Validate input, set Ok button to enabled when acceptable
    bool acceptable = true;
    switch(mode)
    {
    case Unlock: // Old passphrase x1
        break;
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(acceptable);
}

bool AskPassphrasePage::event(QEvent *event)
{
    // Detect Caps Lock key press.
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_CapsLock) {
            fCapsLock = !fCapsLock;
        }
        if (fCapsLock) {
            ui->capsLabel->setText(tr("Warning: The Caps Lock key is on!"));
        } else {
            ui->capsLabel->clear();
        }
    }
    return QWidget::event(event);
}

bool AskPassphrasePage::eventFilter(QObject *object, QEvent *event)
{
    /* Detect Caps Lock.
     * There is no good OS-independent way to check a key state in Qt, but we
     * can detect Caps Lock by checking for the following condition:
     * Shift key is down and the result is a lower case character, or
     * Shift key is not down and the result is an upper case character.
     */
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        QString str = ke->text();
        if (str.length() != 0) {
            const QChar *psz = str.unicode();
            bool fShift = (ke->modifiers() & Qt::ShiftModifier) != 0;
            if ((fShift && psz->isLower()) || (!fShift && psz->isUpper())) {
                fCapsLock = true;
                ui->capsLabel->setText(tr("Warning: The Caps Lock key is on!"));
            } else if (psz->isLetter()) {
                fCapsLock = false;
                ui->capsLabel->clear();
            }
        }
    }
    return QWidget::eventFilter(object, event);
}
