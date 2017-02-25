#include <iostream>
#include "network.h"

using namespace std;

int main(int argc, char **argv) {
	if (argc != 4) {
		cout << "Usage: " << argv[0] << " [no. of nodes] [z] [Max no. of events]" << endl;
		exit(0);
	}

    int n = stoi(argv[1]);
    double z = stod(argv[2]);
    int maxEvents = stoi(argv[3]);
    Network network(n,z);
    network.print();
    network.simulate(maxEvents);
    network.visualize_blockchains();
    return 0;
}