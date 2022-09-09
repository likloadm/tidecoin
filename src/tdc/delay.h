#include "pow.h"
#include "arith_uint256.h"
#include "primitives/block.h"
#include "streams.h"
#include "uint256.h"
#include "chain.h"
#include "chainparams.h"
#include <logging.h>
#include "tinyformat.h"
#include <boost/foreach.hpp>
#include <consensus/validation.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/thread.hpp>

int GetBlockDelay (const CBlockIndex& newBlock,const CBlockIndex& prevBlock, const int activeChainHeight, bool isStartupSyncing);
bool IsChainPenalised (const CChain& chain);
bool RelayAlternativeChain(CValidationState &state, const std::shared_ptr<const CBlock> pblock, BlockSet* sForkTips)
