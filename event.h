#ifndef EVENT_H
#define EVENT_H

#include <time.h>
#include "transaction.h"
#include "block.h"
#include "types.h"

const int CREATE_TRANSACTION = 0;
const int CREATE_BLOCK = 1;
const int RECEIVE_TRANSACTION = 2;
const int RECEIVE_BLOCK = 3;

class Event {
public:
	Event(time_t occurenceTime, EventType eventType, Transaction *txn, Block *block,
			Id senderId, Id receiverId, Id creatorId) 
	{
		_occurenceTime = occurenceTime;
		_eventType = eventType;
		_txn = txn;
		_block = block;
		_senderId = senderId;
		_receiverId = receiverId;
		_creatorId = creatorId;
	}

	time_t occurenceTime() const { return _occurenceTime; }

	EventType type() const { return _eventType; }

	Transaction* txn() const { return _txn; }

	Block* block() const { return _block; }

	Id senderId() const { return _senderId; }

	Id receiverId() const { return _receiverId; }

	Id creatorId() const { return _creatorId; }
	
private:
	time_t _occurenceTime; // global time at which event occurs
	EventType _eventType; // type of the event
	Transaction *_txn; // not NULL for RECEIVE_TRANSACTION event
	Block *_block; // not NULL for RECEIVE_BLOCK event
	Id _senderId; // id of the node creating RECEIVE_* event
	Id _receiverId; // id of the node which is receiving RECEIVE_* event
	Id _creatorId;	// id of the node which has created the CREATE_* event
};

class EventComparison {
public:
	bool operator() (const Event *lhs, const Event *rhs) const {
		return lhs->occurenceTime() > rhs->occurenceTime();
	}
};

#endif //EVENT_H