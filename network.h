#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <cmath>
#include <assert.h>
#include <random>
#include "node.h"
#include "event.h"

using namespace std;

class Network {
public:
    Network(int n, double z) : _propDelays(n, vector<double>(n)),
                               _queuingDelays(n, vector<double>(n)),
                               _bottleneckSpeeds(n, vector<double>(n)),
                               _generator(time(NULL))
    {
        // create n nodes of which z% are slow and rest are fast
        int t = floor(n*z);
        NodeType type;
        double txnCreationMean, blockCreationMean;
        srand(time(NULL));
        for (int id = 0; id < n; id++) {
            type = (id < t) ? SLOW : FAST;
            txnCreationMean = 0.01 + (rand() % 1000) / 100000.0;
            blockCreationMean = 0.001 + (rand() % 1000) / 1e7;
            _nodes.push_back(new Node(id,type,txnCreationMean,blockCreationMean));
        }

        // add random number of peers to each node
        for (int i = 0; i < n; i++) {
            for (int j = i+1; j < n; j++) {
                if (rand() % 2) {
                    _nodes[i]->add_neighbor(j);
                    _nodes[j]->add_neighbor(i);
                }
            }
            // if node has no peer
            if(_nodes[i]->nbrs().size() == 0) {
                int j;
                while ((j = rand() % n) == i);
                _nodes[i]->add_neighbor(j);
                _nodes[j]->add_neighbor(i);
            }
        }

        initialize_parameters();
    }

    void simulate() {
        initialize_events();
        int eventCounter = 0;
        int maxEvents = 20;
        while (!_eventsQueue.empty() && eventCounter < maxEvents) {
            Event *event = _eventsQueue.top();
            _eventsQueue.pop();
            eventCounter++;
            cout << "------------------ Event " << eventCounter << " at " << event->occurenceTime() << " ----------------------" << endl;
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
            cout << endl;
        }
    }

    void print() {
        cout << "----------------------------- P2P Network ---------------------------" << endl;
        for (Node *node : _nodes) {
            cout << (node->type() == SLOW ? "Slow " : "Fast ");
            cout << node->id() << ": ";
            for (Id nbr : node->nbrs()) {
                cout << nbr << " ";
            }
            cout << "| " << node->txnCreationTime() << " " << node->blockCreationTime();
            cout << endl;
        }
        cout << endl;
    }

private:
    vector<Node*> _nodes;
    vector<vector<double>> _propDelays;
    vector<vector<double>> _queuingDelays;
    vector<vector<double>> _bottleneckSpeeds;
    priority_queue<Event*, vector<Event*>, EventComparison> _eventsQueue;
    std::default_random_engine _generator;

    void initialize_parameters() {
        int n = _nodes.size();
        std::uniform_int_distribution<int> uniformDistribution(10,500);
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
                
                // initialize propagation delay from a uniform distribution between 10ms and 500ms
                _propDelays[i][j] = _propDelays[j][i] = uniformDistribution(_generator) / 1000.0;
            }
        }
    }

    time_t get_latency(Id i, Id j, int size_m) {
        std::exponential_distribution<double> expDistribution(0.12 / _bottleneckSpeeds[i][j]);
        double latency = _propDelays[i][j] + (size_m / _bottleneckSpeeds[i][j]) + expDistribution(_generator);
        return floor(latency);
    }

    void initialize_events() {
        for (Node *node : _nodes) {
            _eventsQueue.push(new Event(node->txnCreationTime(), CREATE_TRANSACTION, NULL, NULL, -1, -1, node->id()));
            _eventsQueue.push(new Event(node->blockCreationTime(), CREATE_BLOCK, NULL, NULL, -1, -1, node->id()));
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

        cout << "Create Transaction " << txn->id() << ": " << txn->payer() << "->" << txn->payee() << ", " << txn->amount() << endl;
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

            cout << "Create Block " << block->id() << ": " << creatorId << endl;
        }
    }

    void receive_transaction(Event *event) {
        Id senderId = event->senderId();
        Id receiverId = event->receiverId();
        Transaction *txn = event->txn();
        Node *receiver = _nodes[receiverId];
        
        cout << "Receive Transaction " << txn->id() << ": " << receiverId << "<-" << senderId << endl;
        
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
            cout << "Successful" << endl;
        } else {
            cout << "Rejected" << endl;
        }
    }

    void receive_block(Event *event) {
        Id senderId = event->senderId();
        Id receiverId = event->receiverId();
        Block *block = event->block();
        Node *receiver = _nodes[receiverId];
        
        cout << "Receive Block " << block->id() << ": " << receiverId << "<-" << senderId << endl;
        
        // if the block is not already heard from any other connected peer
        if (!receiver->has_heard_block(block->id())) {
            receiver->receive_block(block, event->occurenceTime());
            int size_m = 100;
            // broadcast the block to all the connected peers except the peer who sent the block
            for (Id nbr : receiver->nbrs()) {
                if (nbr == senderId) {
                    continue;
                }
                time_t otime = event->occurenceTime() + get_latency(receiverId, nbr, size_m);
                _eventsQueue.push(new Event(otime, RECEIVE_BLOCK, NULL, block, receiverId, nbr, -1));
            }
            cout << "Successful" << endl;
        } else {
            cout << "Rejected" << endl;
        }
    }
};

#endif // NETWORK_H