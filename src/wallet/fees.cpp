// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <wallet/fees.h>

#include <util/system.h>
#include <wallet/coincontrol.h>
#include <wallet/wallet.h>


CAmount GetRequiredFee(const CWallet& wallet, unsigned int nTxBytes)
{
    CAmount nBaseFee = WALLET_MIN_TX_FEE;
    CAmount nMinFee = (1 + nTxBytes / 1000) * nBaseFee;

    if (!MoneyRange(nMinFee))
        nMinFee = MAX_MONEY;
    return nMinFee;
}

CAmount GetMinimumFee(const CWallet& wallet, unsigned int nTxBytes, const CCoinControl& coin_control, FeeCalculation* feeCalc)
{
    //TODO implement
    return GetRequiredFee(wallet, nTxBytes);
}

CFeeRate GetRequiredFeeRate(const CWallet& wallet)
{
    return wallet.chain().relayMinFee();
}

CFeeRate GetMinimumFeeRate(const CWallet& wallet, const CCoinControl& coin_control, FeeCalculation* feeCalc)
{
    /* User control of how to calculate fee uses the following parameter precedence:
       1. coin_control.m_feerate
       2. m_pay_tx_fee (user-set member variable of wallet)
       The first parameter that is set is used.
    */
    CFeeRate feerate_needed;
    if (coin_control.m_feerate) { // 1.
        feerate_needed = *(coin_control.m_feerate);
        if (feeCalc) feeCalc->reason = FeeReason::PAYTXFEE;
        // Allow to override automatic min/max check over coin control instance
        if (coin_control.fOverrideFeeRate) return feerate_needed;
    } else {
        feerate_needed = wallet.m_pay_tx_fee;
        if (feeCalc) feeCalc->reason = FeeReason::PAYTXFEE;
    }

    // prevent user from paying a fee below the required fee rate
    CFeeRate required_feerate = GetRequiredFeeRate(wallet);
    if (required_feerate > feerate_needed) {
        feerate_needed = required_feerate;
        if (feeCalc) feeCalc->reason = FeeReason::REQUIRED;
    }
    return feerate_needed;
}

CFeeRate GetDiscardRate(const CWallet& wallet)
{
    return wallet.m_discard_rate;
}
