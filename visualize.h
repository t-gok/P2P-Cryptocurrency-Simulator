#ifndef VISUALIZE_H
#define VISUALIZE_H

#include <fstream>
#include "blockchain.h"

using namespace std;

void output_blockchain(BlockChain &blockChain, string filename) {
	map<Id,BlockNode*> blockMap = blockChain.blockMap();
	ofstream file(filename, ios::out);
	if (!file.is_open()) {
		cout << "can not open " << filename << endl;
	}
	file << "digraph G {" << endl;
	for (auto b : blockMap) {
		if (b.second->parentNode() != NULL) {
			file << b.first << " -> " << b.second->parentNode()->block().id() << endl;
		}
	}
	file << "}";
	file.close();
}

void visualize_blockchain(Node *node) {
	string filename = "graphs/" + std::to_string(node->id()) + ".dot";
	output_blockchain(node->blockChain(), filename);
}

#endif // VISUALIZE_H

// dot -Tps output.dot -o graph.ps
// if you want to have a interaction with  "graph.ps" then run "gv graph.ps" on command line