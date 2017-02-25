#ifndef NODE_H
#define NODE_H

#include <vector>
#include <time.h>
#include <cstdlib>
#include <cmath>
#include <unordered_set>
#include <random>
#include "types.h"
#include "blockchain.h"

using namespace std;

typedef unsigned int NodeType;
const NodeType SLOW = 0;
const NodeType FAST = 1;

class Node {
public:
    Node(Id id, NodeType nodeType, double txnCreationRate, double blockCreationRate) :
        _txnCreationDistribution(txnCreationRate), _blockCreationDistribution(blockCreationRate), _generator(time(NULL))
    {
        _id = id;
        _nodeType = nodeType;
        _money = 100;
        _txnCreationTime = _txnCreationDistribution(_generator);
        _blockCreationTime = _blockCreationDistribution(_generator);
    }

    Id id() const { return _id; }

    NodeType type() const { return _nodeType; }

    vector<Id>& nbrs() { return _nbrs; }

    BlockChain& blockChain() { return _blockChain; }

    Time txnCreationTime() const { return _txnCreationTime; }

    Time blockCreationTime() const { return _blockCreationTime; } 

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

    void receive_block(Block *block, Time arrivalTime) {
        if (!_blockChain.add_block(*block, arrivalTime)) {
            cout << "blockchain can not receive block " << block->id() << endl;
        }
        _heardBlocks.insert(block->id());
        _blockCreationTime = arrivalTime + _blockCreationDistribution(_generator); // update block creation time
        
        // remove transactions in the received block from unspent transactions list
        vector<Transaction> blockTxns = block->transactions();
        for (Transaction txn : blockTxns) {
            remove_txn(txn.id());   // remove txn from unspent txn list
            Coin amount = txn.amount(); // txn amount
            // update money of this node if it is involved in any txn
            if (_id == txn.payee()) {
                _money += amount;
            } else if (_id == txn.payer()) {
                _money -= amount;
            } 
        }
    }

    Transaction* create_new_transaction(Id payee) {
        double percentage = (rand() % 50) / 100.0;
        Coin amount = _money * percentage;
        Transaction *txn = new Transaction(_id, payee, amount);
        receive_transaction(txn);
        _txnCreationTime += _txnCreationDistribution(_generator);
        return txn;
    }

    Block* create_new_block() {
        // if there are no unspent transactions then do not create a block
        if (_unspentTxns.size() == 0) {
            _blockCreationTime += _blockCreationDistribution(_generator); // update block creation time
            cout << "new block creation time = " << _blockCreationTime << endl;
            return NULL;
        }
        BlockNode *topNode = _blockChain.top();
        Id parentId = topNode->block().id();
        Block *block = new Block(parentId, _unspentTxns);
        _unspentTxns.clear();
        receive_block(block, _blockCreationTime);
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
    Time _txnCreationTime; // time when a new transaction should be created
    Time _blockCreationTime; // time when a new block should be created
    std::default_random_engine _generator;
    std::exponential_distribution<double> _txnCreationDistribution; // distribution for txn interarrival
    std::exponential_distribution<double> _blockCreationDistribution; // distribution for waiting time for block creation

    void remove_txn(Id txnId) {
        for (int i = 0; i < _unspentTxns.size(); i++) {
            if (_unspentTxns[i].id() == txnId) {
                _unspentTxns[i] = _unspentTxns.back();
                _unspentTxns.pop_back();
                break;
            }
        }
    }
};

#endif // NODE_H