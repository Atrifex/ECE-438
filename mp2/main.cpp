
#include "ls_router.h"

int main(int argc, char** argv)
{
    if(argc != 4) {
        fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
        exit(1);
    }

    /*
     * TODO:
     *      1) Test heartbeats and fixed Dijkstra
     *      2) send lsp (together)
     *          - avoid flooding
     *          - might require using variation of lsp
     *          - Need to figure out where to send --> what function
     *          - Need to send on ANY change
     */

    // Parse input and initialize router
    LS_Router router(atoi(argv[1]), argv[2], argv[3]);

    // start announcing
    router.announceToNeighbors();

    // start listening
    router.listenForNeighbors();
}
