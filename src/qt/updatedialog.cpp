#include "updatedialog.h"
#include "ui_updatedialog.h"
#include "clientmodel.h"
#include "util.h"
#include "version.h"

bool updateAccepted = false;

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    std::string title = GetArg("-vTitle", "Update Available").c_str();
    std::string description = GetArg("-vDescription", "A new software update is available.").c_str();
    std::string version = GetArg("-vVersion", "1.0.0.0").c_str();
    std::string postreq = std::string("\n\nPost-install: ").append((GetBoolArg("-vBootstrap") ? "Auto Bootstrap required." : "Auto Bootstrap not required."));
    ui->setupUi(this);
    ui->title->setText(title.c_str());
    ui->description->setText(version.append(": ").append(description).append(postreq).c_str());

    this->setStyleSheet("background-color: #FFFFFF;");
}

void UpdateDialog::setModel(ClientModel *model)
{
    if(model)
    {
        ui->versionLabel->setText(model->formatFullVersion());
    }
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

void UpdateDialog::on_buttonBox_accepted()
{
    updateAccepted = true;
    close();
}

void UpdateDialog::on_buttonBox_rejected()
{
    updateAccepted = false;
    close();
}
