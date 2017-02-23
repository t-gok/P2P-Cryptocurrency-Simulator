#include <iostream>
#include "network.h"

using namespace std;

int main(int argc, char **argv) {
    int n = 10;
    double z = 0.3;
    Network network(n,z);
    network.print();
    network.simulate();
    return 0;
}