#ifndef NODE_H
#define NODE_H

#include <vector>
#include <time.h>
#include <cstdlib>
#include <cmath>
#include <unordered_set>
#include "types.h"
#include "blockchain.h"

using namespace std;

typedef unsigned int NodeType;
const NodeType SLOW = 0;
const NodeType FAST = 1;

class Node {
public:
    Node(Id id, NodeType nodeType) : _id(id), _nodeType(nodeType) {
        _money = 0;
    }

    Id id() const { return _id; }

    NodeType type() const { return _nodeType; }

    vector<Id>& nbrs() { return _nbrs; }

    time_t txnCreationTime() const { return _txnCreationTime; }

    time_t blockCreationTime() const { return _blockCreationTime; } 

    void add_neighbor(Id nbr) {
        _nbrs.push_back(nbr);
    }

    bool has_heard_txn(Id txnId) {
        return _heardTxns.count(txnId);
    }

    bool has_heard_block(Id blockId) {
        return _heardBlocks.count(blockId);
    }

    void receive_transaction(Transaction *txn) {
        _unspentTxns.push_back(*txn);
        _heardTxns.insert(txn->id());
    }

    void receive_block(Block *block) {
        _blockChain.add_block(*block);
        _heardBlocks.insert(block->id());
        // update block creation time
        // remove transactions in the received block from unspent transactions list
    }

    Transaction* create_new_transaction(Id payee) {
        double percentage = (rand() % 50) / 100.0;
        Coin amount = floor(_money * percentage);
        Transaction *txn = new Transaction(_id, payee, amount);
        receive_transaction(txn);
        return txn;
    }

    Block* create_new_block() {
        BlockNode *topNode = _blockChain.top();
        Id parentId = topNode->block().parentId();  // get parent id directly from blockNode?
        Block *block = new Block(parentId, _unspentTxns);
        _unspentTxns.clear();
        receive_block(block);
        return block;
    }


private:
    Id _id; // unique id
    NodeType _nodeType; // slow/fast
    Coin _money;
    vector<Id> _nbrs; // connected peers
    BlockChain _blockChain;
    vector<Transaction> _unspentTxns; // unspent transactions
    unordered_set<Id> _heardTxns; // transaction received so far (including those not in blockchain)
    unordered_set<Id> _heardBlocks; // blocks received so far (all blocks in blockchain)
    time_t _txnCreationTime; // time when a new transaction should be created
    time_t _blockCreationTime; // time when a new block should be created
};

#endif // NODE_H