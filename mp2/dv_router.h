#ifndef DV_ROUTER_H
#define DV_ROUTER_H

#include "router.h"

#define NUM_NODES 256
#define INFINITY 8388608 // Given: costs won't be more than 2^23

class DV_Router : public Router
{
    public:
        // Constructor
        DV_Router(int id, char * initialcostsfile, char * logFileName);

        // Member functions
        void checkHeartBeat();
        void listenForNeighbors();
        void updateForwardingTable();

    private:
       vector<int> costs; // Costs of direct links from node to its neighbors
       vector<bool> valid; // Validity of direct links
       vector<int> distances; // Estimates of shortest distance to other nodes
       // vector<vector<int>> neighborDistances; // Neighbors' distance vectors- might not need
};


#endif
