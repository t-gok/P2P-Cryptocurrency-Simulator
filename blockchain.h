#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <map>
#include "blocknode.h"
#include "transaction.h"

using namespace std;

class BlockChain {
public:
	BlockChain() {
		vector<Transaction> txns;
		Block genesisBlock(0, txns);
		_top = new BlockNode(genesisBlock, NULL, 0);
	}

	unsigned long height() const { return _top->height(); }

	BlockNode *top() const { return _top; }

	// adds a block to the blockchain
	// returns true if succesful
	// returns false if parent block not in blockchain
	bool add_block(Block &block, Time arrivalTime) {
		if (_blockMap.find(block.parentId()) == _blockMap.end()) {
			return false;
		}

		Id blockId = block.id();
		Id parentId = block.parentId();
		BlockNode *parentNode = _blockMap[parentId];
		BlockNode *bnode = new BlockNode(block, parentNode, arrivalTime);
		_blockMap[blockId] = bnode;

		// update top if this becomes the longest chain, use time to break ties
		if (bnode->height() > _top->height()) {
			_top = bnode;
		}

		return true;
	}
private:
	BlockNode *_top;
	map<Id,BlockNode*> _blockMap; // use unordered_map instead
};

#endif // BLOCKCHAIN_H