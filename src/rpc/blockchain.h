// Copyright (c) 2017-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RPC_BLOCKCHAIN_H
#define BITCOIN_RPC_BLOCKCHAIN_H

#include <amount.h>
#include <sync.h>

#include <stdint.h>
#include <vector>

extern RecursiveMutex cs_main;

class CBlock;
class CBlockIndex;
class CTxMemPool;
class UniValue;

static constexpr double dminDifficulty = 0.00000048; //standard scrypt^2 difficulty minimum

/**
 * Get the difficulty of the net wrt to the given block index.
 *
 * @return A floating point number that is a multiple of the main net minimum
 * difficulty (4295032833 hashes).
 */
double GetDifficulty(const CBlockIndex* blockindex);
double GetPoWKHashPM();

/** Callback for when block tip changed. */
void RPCNotifyBlockChange(bool ibd, const CBlockIndex *);

/** Block description to JSON */
UniValue blockToJSON(const CBlock& block, const CBlockIndex* tip, const CBlockIndex* blockindex, bool txDetails = false) LOCKS_EXCLUDED(cs_main);

/** Mempool information to JSON */
UniValue MempoolInfoToJSON(const CTxMemPool& pool);

/** Mempool to JSON */
UniValue MempoolToJSON(const CTxMemPool& pool, bool verbose = false);

/** Block header to JSON */
UniValue blockheaderToJSON(const CBlockIndex* tip, const CBlockIndex* blockindex) LOCKS_EXCLUDED(cs_main);

#endif
