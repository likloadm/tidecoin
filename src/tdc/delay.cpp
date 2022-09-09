#include "tdc/delay.h"
#include "net.h"

int GetBlockDelay(const CBlockIndex& newBlock, const CBlockIndex& prevBlock, const int activeChainHeight, const bool isStartupSyncing)
{
    const int PENALTY_THRESHOLD = 5;

    if(isStartupSyncing) {
    	return 0;
    }

    if(newBlock.nHeight < activeChainHeight ) {
      	LogPrintf("Received a delayed block (activeChainHeight: %d, newBlockHeight: %d)!\n", activeChainHeight, newBlock.nHeight);
    }

    // if the current chain is penalised.
    if (prevBlock.nChainDelay > 0) {
        // Positive values to increase the penalty until
        // we reach the current active height.
        if (activeChainHeight >= newBlock.nHeight ) {
        	return (activeChainHeight - newBlock.nHeight);
        } else {
        	LogPrintf("Decreasing penalty to chain (activeChainHeight: %d, newBlockHeight: %d, prevBlockChainDelay: %d)!\n", activeChainHeight, newBlock.nHeight, prevBlock.nChainDelay);
        	// -1 to decrease the penalty afterwards.
            return -1;
        }
    // no penalty yet, or penalty already resolved.
    } else {
        // Introduce penalty in case we receive a historic block.
        // (uses a threshold value)
        if (activeChainHeight - newBlock.nHeight > PENALTY_THRESHOLD ){
            return (activeChainHeight - newBlock.nHeight);
        // no delay detected.
        } else {
            return 0;
        }
    }
}
bool IsChainPenalised(const CChain& chain)
{
    return (chain.Tip()->nChainDelay < 0);
}


bool RelayAlternativeChain(CValidationState &state, const std::shared_ptr<const CBlock> pblock, BlockSet* sForkTips)
{
    if (!pblock)
    {
//        LogPrint("forks", "%s():%d - Null pblock!\n", __func__, __LINE__);
        return false;
    }

    const CChainParams& chainParams = Params();
    uint256 hashAlternativeTip = pblock->GetHash();
    //LogPrint("forks", "%s():%d - Entering with hash[%s]\n", __func__, __LINE__, hashAlternativeTip.ToString() );

    // 1. check this is the best chain tip, in this case exit
    if (chainActive.Tip()->GetBlockHash() == hashAlternativeTip)
    {
        //LogPrint("forks", "%s():%d - Exiting: already best tip\n", __func__, __LINE__);
        return true;
    }

    CBlockIndex* pindex = NULL;
    BlockMap::iterator mi = mapBlockIndex.find(hashAlternativeTip);
    if (mi != mapBlockIndex.end())
    {
        pindex = (*mi).second;
    }

    if (!pindex)
    {
//        LogPrint("forks", "%s():%d - Null pblock index!\n", __func__, __LINE__);
        return false;
    }

    // 2. check this block is a fork from best chain, otherwise exit
    if (chainActive.Contains(pindex))
    {
        //LogPrint("forks", "%s():%d - Exiting: it belongs to main chain\n", __func__, __LINE__);
        return true;
    }

    // 3. check we have complete list of ancestors
    // --
    // This is due to the fact that blocks can easily be received in sparse order
    // By skipping this block we choose to delay its propagation in the loop
    // below where we look for the best height possible.
    // --
    // Consider that it can be a fork but also be a future best tip as soon as missing blocks are received
    // on the main chain
    if ( pindex->nChainTx <= 0 )
    {
//        LogPrint("forks", "%s():%d - Exiting: nChainTx=0\n", __func__, __LINE__);
        return true;
    }

    // 4. Starting from this block, look for the best height that has a complete chain of ancestors
    // --
    // This is done for all of possible forks stem after starting block, potentially more than one height could be found.

    //dump_global_tips();

//    LogPrint("forks", "%s():%d - sForkTips(%d) - h[%d] %s\n",
//        __func__, __LINE__, sForkTips->size(), pindex->nHeight, pindex->GetBlockHash().ToString() );

    std::vector<CInv> vInv;

    BOOST_FOREACH(const CBlockIndex* block, *sForkTips)
    {
        vInv.push_back(CInv(MSG_BLOCK, block->GetBlockHash()) );
    }

    // 5. push inv list up to the alternative tips
    int nBlockEstimate = 0;
    if (fCheckpointsEnabled)
        nBlockEstimate = Checkpoints::GetTotalBlocksEstimate(chainParams.Checkpoints());

    int nodeHeight = -1;
    if (g_connman->GetLocalServices() & NODE_NETWORK) {
        g_connman->ForEachNode([&vInv, &nodeHeight, &nBlockEstimate](CNode* pnode) {
//            if (pnode->nStartingHeight != -1)
//            {
//                nodeHeight = (pnode->nStartingHeight - 2000);
//            }
//            else
//            {
//                nodeHeight = nBlockEstimate;
//            }
//            if (chainActive.Height() > nodeHeight)
//            {
//                {
//                    BOOST_FOREACH(CInv& inv, vInv)
//                    {
////                        LogPrint("forks", "%s():%d - Pushing inv to Node (id=%d) hash[%s]\n",
////                            __func__, __LINE__, pnode->GetId(), inv.hash.ToString() );
//                        pnode->PushInventory(inv);
//                    }
//                }
//            }
        });
    }
    return true;
}
