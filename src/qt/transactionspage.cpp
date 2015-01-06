#include "transactionspage.h"
#include "ui_transactionspage.h"

#include "guiconstants.h"
#include "bitcoingui.h"
#include "transactionview.h"
#include <QGraphicsView>
#include <QVBoxLayout>

//#include "transactionspage.moc"

TransactionsPage::TransactionsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransactionsPage)
{
    ui->setupUi(this);
    this->setStyleSheet("QToolTip { background-color: white; color: #444748; padding: 5px; }" + veriPushButtonStyleSheet + " " + veriDialogButtonBoxStyleSheet);

    this->setContentsMargins(0,0,0,0);
    ui->header->setMinimumSize(2048,HEADER_HEIGHT);
    ui->header->setGeometry(0,0,2048,HEADER_HEIGHT);
    ui->header->setAutoFillBackground(true);
    ui->header->setStyleSheet("QGraphicsView {background-color: " + STRING_VERIBLUE + ";}");

}

TransactionsPage::~TransactionsPage()
{
    delete ui;
}
/*
void TransactionsPage::setLayout(QVBoxLayout *vbox)
{
    this->setLayout(vbox);
}
*/
