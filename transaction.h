#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "types.h"

class Transaction {
public:
    Transaction(Id payer, Id payee, Coin amount) {
        // generate random transaction id
        _payer = payer;
        _payee = payee;
        _amount = amount;
    }

    Id id() const { return _txnId; }

    Id payer() const { return _payer; }

    Id payee() const { return _payee; }

    Coin amount() const { return _amount; }
private:
    Id _txnId;
    Id _payer;
    Id _payee;
    Coin _amount;
};

#endif // TRANSACTION_H