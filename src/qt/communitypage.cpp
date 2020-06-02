// Copyright (c) 2011-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/communitypage.h>
#include <qt/forms/ui_communitypage.h>

#include <qt/bitcoinunits.h>
#include <qt/clientmodel.h>
#include <qt/guiconstants.h>
#include <qt/guiutil.h>
#include <qt/platformstyle.h>

#include <QGraphicsDropShadowEffect>

CommunityPage::CommunityPage(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommunityPage),
    clientModel(nullptr)
{
    ui->setupUi(this);
    QGraphicsDropShadowEffect* shadow1 = new QGraphicsDropShadowEffect();
    shadow1->setOffset(QPointF(5, 5));
    shadow1->setBlurRadius(20.0);
    QGraphicsDropShadowEffect* shadow2 = new QGraphicsDropShadowEffect();
    shadow2->setOffset(QPointF(5, 5));
    shadow2->setBlurRadius(20.0);
    QGraphicsDropShadowEffect* shadow3 = new QGraphicsDropShadowEffect();
    shadow3->setOffset(QPointF(5, 5));
    shadow3->setBlurRadius(20.0);
    QGraphicsDropShadowEffect* shadow4 = new QGraphicsDropShadowEffect();
    shadow4->setOffset(QPointF(5, 5));
    shadow4->setBlurRadius(20.0);
    ui->explorerBox->setGraphicsEffect(shadow1);
    ui->twitterBox->setGraphicsEffect(shadow2);
    ui->chatBox->setGraphicsEffect(shadow3);
    ui->websiteBox->setGraphicsEffect(shadow4);
}

CommunityPage::~CommunityPage()
{
    delete ui;
}

void CommunityPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
}