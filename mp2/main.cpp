
#include "ls_router.h"

int main(int argc, char** argv)
{
    if(argc != 4) {
        fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
        exit(1);
    }

    // Parse input and initialize router
    LS_Router router(atoi(argv[1]), argv[2]);

    // start announcing
    router.announceToNeighbors();

    // start listening
    router.listenForNeighbors();
}
