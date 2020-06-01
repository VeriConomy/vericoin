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

CommunityPage::CommunityPage(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommunityPage),
    clientModel(nullptr)
{
    ui->setupUi(this);
}

CommunityPage::~CommunityPage()
{
    delete ui;
}

void CommunityPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
}