#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "types.h"

int txnIdCounter = 0;

class Transaction {
public:
    Transaction(Id payer, Id payee, Coin amount) {
        _id = txnIdCounter++;
        _payer = payer;
        _payee = payee;
        _amount = amount;
    }

    Id id() const { return _id; }

    Id payer() const { return _payer; }

    Id payee() const { return _payee; }

    Coin amount() const { return _amount; }
private:
    Id _id;
    Id _payer;
    Id _payee;
    Coin _amount;
};

#endif // TRANSACTION_H