
#ifndef LS_ROUTER_H
#define LS_ROUTER_H

#include "router.h"
#include "graph.h"

class LS_Router : public Router
{
    public:
        // Constructor and Destructor
        LS_Router(int id, char * graphFileNamem, char * logFileName);

        // Member Functions
        void listenForNeighbors();
        void updateForwardingTable();
        void checkHeartBeat();

        // TODO: Need thread to check heartbeat threshold, perform Dijkstra if changing from valid to invalid
        // TODO: Need thread to forward heartbeat and cost
        // TODO: Deal with manager's messages

    private:
        Graph network;
};


#endif
