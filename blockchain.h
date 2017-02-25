#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <map>
#include <queue>
#include "blocknode.h"
#include "transaction.h"

using namespace std;

class BlockChain {
public:
	BlockChain() {
		vector<Transaction> txns;
		Block genesisBlock(0, txns);
		_top = new BlockNode(genesisBlock, NULL, 0);
		_blockMap[genesisBlock.id()] = _top;
	}

	unsigned long height() const { return _top->height(); }

	BlockNode *top() const { return _top; }

	// adds a block to the blockchain
	// returns true if succesful
	// returns false if parent block not in blockchain
	bool add_block(Block &block, Time arrivalTime) {
		if (_blockMap.find(block.parentId()) == _blockMap.end()) {
			_orphanBlocks.push_back(block);
			_orphanArrivalTimes.push_back(arrivalTime);
			return false;
		} else {
			add_blockNode(block, block.parentId(), arrivalTime);
			// check if new added block is parent of any orphan block
			queue<Id> q;
			q.push(block.id());
			while (!q.empty()) {
				Id parentId = q.front();
				q.pop();
				int index;
				while ((index = is_parent_of_orphan(parentId)) >= 0) {
					add_blockNode(_orphanBlocks[index], parentId, _orphanArrivalTimes[index]);
					q.push(_orphanBlocks[index].id());
					remove_orphan_block(index);
				}
			}

			return true;
		}

	}

	map<Id,BlockNode*>& blockMap() { return _blockMap; }

private:
	BlockNode *_top;
	map<Id,BlockNode*> _blockMap; // use unordered_map instead
	vector<Block> _orphanBlocks; // block whose parent block is missing in blockchain
	vector<Time> _orphanArrivalTimes;

	int is_parent_of_orphan(Id blockId) {
		for (int index = 0; index < _orphanBlocks.size(); index++) {
			if (_orphanBlocks[index].parentId() == blockId) {
				return index;
			}
		}
		return -1;
	}

	void remove_orphan_block(int index) {
		_orphanBlocks[index] = _orphanBlocks.back();
		_orphanBlocks.pop_back();
		_orphanArrivalTimes[index] = _orphanArrivalTimes.back();
		_orphanArrivalTimes.pop_back();
	}

	void add_blockNode(Block &block, Id parentId, Time arrivalTime) {
		BlockNode *parentNode = _blockMap[parentId];
		BlockNode *bnode = new BlockNode(block, parentNode, arrivalTime);
		_blockMap[block.id()] = bnode;

		// update top if this becomes the longest chain
		if (bnode->height() > _top->height()) {
			_top = bnode;
		}
	}
};

#endif // BLOCKCHAIN_H