#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include "types.h"
#include "transaction.h"

using namespace std;

int blockIdCounter = 1;

class Block {
public:
    Block(Id parentId, const vector<Transaction> &transactions) : _transactions(transactions) {
        _id = parentId ? ++blockIdCounter : 1;
        _parentId = parentId;
    }

    Id id() const { return _id; }

    Id parentId() const { return _parentId; }

    bool has_transaction(Id txnId) {
        for (Transaction txn : _transactions) {
            if (txn.id() == txnId) {
                return true;
            }
        }
        return false;
    }

    vector<Transaction>& transactions() {
    	return _transactions;
    }

private:
    Id _id;
    Id _parentId;
    vector<Transaction> _transactions;
};

#endif // BLOCK_H