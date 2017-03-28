
#include "ls_router.h"

int main(int argc, char** argv)
{
    if(argc != 4) {
        fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
        exit(1);
    }

    /*
     * TODO:
     *      1) handle send (Rishi)
     *      2) Logging output for grading and debugging (Nathan)
     *      3) Test with neighbors (together)
     *      4) look at heartbeats (Whoever gets here first)
     *      5) send lsp (together)
     *          - avoid flooding
     *          - might require using variation of lsp
     */

    // Parse input and initialize router
    LS_Router router(atoi(argv[1]), argv[2]);

    // start announcing
    router.announceToNeighbors();

    // start listening
    router.listenForNeighbors();
}
