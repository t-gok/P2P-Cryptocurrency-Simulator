#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <queue>
#include <cstdlib>
#include <cmath>
#include <assert.h>
#include "node.h"
#include "event.h"

using namespace std;

class Network {
public:
    Network(int n, double z) : _propDelays(n, vector<double>(n)),
                               _queuingDelays(n, vector<double>(n)),
                               _bottleneckSpeeds(n, vector<double>(n)) 
    {
        // create n nodes of which z% are slow and rest are fast
        int t = floor(n*z);
        NodeType type;
        for (int id = 0; id < n; id++) {
            type = (id < t) ? SLOW : FAST;
            _nodes.push_back(new Node(id,type));
        }

        // add random number of peers to each node
        for (int i = 0; i < n; i++) {
            for (int j = i+1; j < n; j++) {
                if (rand() % 2) {
                    _nodes[i]->add_neighbor(j);
                    _nodes[j]->add_neighbor(i);
                }
            }
        }

        initialize_parameters();
    }

    void simulate() {
        initialize_events();
        while (!_eventsQueue.empty()) {
            Event *event = _eventsQueue.top();
            _eventsQueue.pop();
            switch(event->type()) {
                case CREATE_TRANSACTION:
                    create_transaction(event);
                    break;
                case CREATE_BLOCK:
                    create_block(event);
                    break;
                case RECEIVE_TRANSACTION:
                    receive_transaction(event);
                    break;
                case RECEIVE_BLOCK:
                    receive_block(event);
                    break;
                default:
                    assert(false); // should not come here
            }
        }
    }

private:
    vector<Node*> _nodes;
    vector<vector<double>> _propDelays;
    vector<vector<double>> _queuingDelays;
    vector<vector<double>> _bottleneckSpeeds;
    priority_queue<Event*, vector<Event*>, EventComparison> _eventsQueue;

    void initialize_parameters() {
        int n = _nodes.size();
        for (int i = 0; i < n; i++) {
            Node *ni = _nodes[i];
            for (int j = i+1; j < n; j++) {
                Node *nj = _nodes[j];
                // if both nodes are fast, link speed is 100 Mbps else it is 5 Mbps
                if (ni->type() == FAST && nj->type() == FAST) {
                    _bottleneckSpeeds[i][j] = _bottleneckSpeeds[j][i] = 100;
                } else {
                    _bottleneckSpeeds[i][j] = _bottleneckSpeeds[j][i] = 5;
                }
                
                // fill prop delay and queuing delay values using appropriate distribution
            }
        }
    }

    time_t get_latency(Id i, Id j, int size_m) {
        return _propDelays[i][j] + (size_m / _bottleneckSpeeds[i][j]) + _queuingDelays[i][j];
    }

    void initialize_events() {
        for (int i = 0; i < _nodes.size(); i++) {

        }
    }

    void create_transaction(Event *event) {
        Id creatorId = event->creatorId();
        Id payee = rand() % _nodes.size(); // random payee
        Node *creator = _nodes[creatorId];
        Transaction *txn = creator->create_new_transaction(payee);
        int size_m = 0;
        for (Id nbr : creator->nbrs()) {
            time_t otime = event->occurenceTime() + get_latency(creatorId, nbr, size_m);
            _eventsQueue.push(new Event(otime, RECEIVE_TRANSACTION, txn, NULL, creatorId, nbr, -1));
        }

        // add a new event which creates a new transaction by this node at updated txn creation time
        _eventsQueue.push(new Event(creator->txnCreationTime(), CREATE_TRANSACTION, NULL, NULL, -1, -1, creatorId));
    }

    void create_block(Event *event) {
        Id creatorId = event->creatorId();
        Node *creator = _nodes[creatorId];
        if (creator->blockCreationTime() == event->occurenceTime()) {
            Block *block = creator->create_new_block();
            int size_m = 100;
            for (Id nbr : creator->nbrs()) {
                time_t otime = event->occurenceTime() + get_latency(creatorId, nbr, size_m);
                _eventsQueue.push(new Event(otime, RECEIVE_BLOCK, NULL, block, creatorId, nbr, -1));
            }

            // add a new event for creation of new block by this node at update block creation time
            _eventsQueue.push(new Event(creator->blockCreationTime(), CREATE_BLOCK, NULL, NULL, -1, -1, creatorId));
        }
    }

    void receive_transaction(Event *event) {
        Id senderId = event->senderId();
        Id receiverId = event->receiverId();
        Transaction *txn = event->txn();
        Node *receiver = _nodes[receiverId];
        // if the transaction is not already heard from any other connected peer
        if (!receiver->has_heard_txn(txn->id())) {
            receiver->receive_transaction(txn);
            int size_m = 0;
            // broadcast the transaction to all the connected peers except the peer who sent the transaction
            for (Id nbr : receiver->nbrs()) {
                if (nbr == senderId) {
                    continue;
                }
                time_t otime = event->occurenceTime() + get_latency(receiverId, nbr, size_m);
                _eventsQueue.push(new Event(otime, RECEIVE_TRANSACTION, txn, NULL, receiverId, nbr, -1));
            }
        }
    }

    void receive_block(Event *event) {
        Id senderId = event->senderId();
        Id receiverId = event->receiverId();
        Block *block = event->block();
        Node *receiver = _nodes[receiverId];
        // if the block is not already heard from any other connected peer
        if (!receiver->has_heard_block(block->id())) {
            receiver->receive_block(block);
            int size_m = 100;
            // broadcast the block to all the connected peers except the peer who sent the block
            for (Id nbr : receiver->nbrs()) {
                if (nbr == senderId) {
                    continue;
                }
                time_t otime = event->occurenceTime() + get_latency(receiverId, nbr, size_m);
                _eventsQueue.push(new Event(otime, RECEIVE_BLOCK, NULL, block, receiverId, nbr, -1));
            }
        }
    }
};

#endif // NETWORK_H