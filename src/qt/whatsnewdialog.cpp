#include "whatsnewdialog.h"
#include "ui_whatsnewdialog.h"
#include "clientmodel.h"
#include "util.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "version.h"

using namespace GUIUtil;

bool whatsNewAccepted = false;

WhatsNewDialog::WhatsNewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WhatsNewDialog)
{
    std::string title = "You Have the Most Current VeriCoin Wallet\n";
    std::string description = GetArg("-vDescription", "Lot's of new features.").c_str();
    std::string version = "NEW IN " + GetArg("-vVersion", "1.0.0.0");
    ui->setupUi(this);

    ui->title->setFont(veriFontLarge);
    ui->title->setText(title.c_str());
    ui->description->setFont(veriFont);
    ui->description->setText(version.append(": ").append(description).c_str());
}

void WhatsNewDialog::setModel(ClientModel *model)
{
    if(model)
    {
        ui->versionLabel->setText(model->formatFullVersion().append(" ").append(GetArg("-vArch", "").c_str()));
    }
}

WhatsNewDialog::~WhatsNewDialog()
{
    delete ui;
}

void WhatsNewDialog::on_buttonBox_accepted()
{
    whatsNewAccepted = true;
    close();
}
