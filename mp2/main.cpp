
#include "ls_router.h"

int main(int argc, char** argv)
{
    if(argc != 4) {
        fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
        exit(1);
    }

    Graph example;

    example.updateLink(true, 50, 0, 33);
    example.updateLink(true, 50, 33, 0);

    example.updateLink(true, 200, 0, 10);
    example.updateLink(true, 200, 10, 0);

    example.updateLink(true, 130, 0, 20);
    example.updateLink(true, 130, 20, 0);

    example.updateLink(true, 30, 0, 30);
    example.updateLink(true, 30, 30, 0);

    example.updateLink(true, 10, 0, 66);
    example.updateLink(true, 10, 66, 0);

    example.updateLink(true, 20, 33, 66);
    example.updateLink(true, 20, 66, 33);

    example.updateLink(true, 60, 33, 10);
    example.updateLink(true, 60, 10, 33);

    example.updateLink(true, 3, 10, 20);
    example.updateLink(true, 3, 20, 10);

    example.updateLink(true, 90, 20, 30);
    example.updateLink(true, 90, 30, 20);

    example.updateLink(true, 21, 30, 66);
    example.updateLink(true, 21, 66, 30);

    cout << example.dijkstraGetNextNode(66) << endl;
    cout << example.dijkstraGetNextNode(30) << endl;
    cout << example.dijkstraGetNextNode(20) << endl;
    cout << example.dijkstraGetNextNode(10) << endl;
    cout << example.dijkstraGetNextNode(33) << endl;
    cout << example.dijkstraGetNextNode(250) << endl;

    // Parse input and initialize router
    //LS_Router router(atoi(argv[1]), argv[2]);

    // start announcing
    //router.announceToNeighbors();

    // start listening
    //router.listenForNeighbors();
}
