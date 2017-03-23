
#include "ls_router.h"

int main(int argc, char** argv)
{
    if(argc != 4) {
        fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
        exit(1);
    }

    Graph example;

    example.updateLink(true, 60, 0, 87);
    example.updateLink(true, 60, 87, 0);

    example.updateLink(true, 5, 0, 5);
    example.updateLink(true, 5, 5, 0);

    example.updateLink(true, 101, 5, 87);
    example.updateLink(true, 101, 87, 5);

    example.updateLink(true, 100, 5, 200);
    example.updateLink(true, 100, 200, 5);

    example.updateLink(true, 32, 87, 200);
    example.updateLink(true, 32, 200, 87);

    example.updateLink(true, 340, 87, 69);
    example.updateLink(true, 340, 69, 87);

    example.updateLink(true, 90, 69, 200);
    example.updateLink(true, 90, 200, 69);

    example.updateLink(true, 1, 69, 70);
    example.updateLink(true, 1, 70, 69);

    example.updateLink(true, 2, 70, 71);
    example.updateLink(true, 2, 71, 70);

  //  example.display();

    stack<int> path;
    path = example.dijkstraTest(71);
    path = example.dijkstraTest(87);
    path = example.dijkstraTest(200);
    path = example.dijkstraTest(255);

    // Parse input and initialize router
    //LS_Router router(atoi(argv[1]), argv[2]);

    // start announcing
    //router.announceToNeighbors();

    // start listening
    //router.listenForNeighbors();
}
