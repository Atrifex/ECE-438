

#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include "ls_router.h"

using std::thread;
using std::ref;

void announceToNeighbors(LS_Router & router) {
    router.announceToNeighbors();
}

int main(int argc, char** argv)
{
    if(argc != 4) {
        fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
        exit(1);
    }

    /*
     * TODO:
     *      1) Fix dijkstra's
     *      2) Make graph generator and test cases
     *      3) Test large and topologies and complex ones
     */

    // Parse input and initialize router
    LS_Router router(atoi(argv[1]), argv[2], argv[3]);

    // start announcing
    thread announcer(announceToNeighbors, ref(router));

    // start listening
    router.listenForNeighbors();
}
