#include "transactionspage.h"
#include "ui_transactionspage.h"

#include "guiconstants.h"
#include "guiutil.h"
#include "bitcoingui.h"
#include "transactionview.h"
#include <QGraphicsView>
#include <QVBoxLayout>

using namespace GUIUtil;

TransactionsPage::TransactionsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransactionsPage)
{
    ui->setupUi(this);
    // Setup header and styles
    ui->header = GUIUtil::header(this, QString(":images/headerHistory"));
}

TransactionsPage::~TransactionsPage()
{
    delete ui;
}
