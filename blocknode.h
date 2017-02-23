#ifndef BLOCKNODE_H
#define BLOCKNODE_H

#include <time.h>
#include "block.h"

class BlockNode {
public:
	BlockNode(Block block, BlockNode *parentNode, time_t arrivalTime) : 
		_block(block), _parentNode(parentNode), _arrivalTime(arrivalTime)
	{
		_height = parentNode ? _parentNode->height() + 1 : 1;
	}

	unsigned long height() const { return _height; }

	BlockNode* parentNode() const { return _parentNode; }

	time_t arrivalTime() const { return _arrivalTime; }

	Block block() const { return _block; }
private:
	Block _block;
	unsigned long _height;
	BlockNode *_parentNode;
	time_t _arrivalTime;
};

#endif // BLOCKNODE_H