// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include <uint256.h>
#include <limits>
#include <map>
#include <string>

namespace Consensus {

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;

    /** Proof of stake parameters */

    /** Proof of work parameters */
    bool fPowNoRetargeting;
    int64_t nPowTargetTimespan;
    int64_t nPowTargetSpacing;
    uint256 nMinimumChainWork;
    uint256 defaultAssumeValid;

    /** VIP */
    int PoSTProtocolHeight;
};

} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
