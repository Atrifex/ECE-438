#ifndef GRAPH_H
#define GRAPH_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#define NUM_NODES 256
#define INIT_COST 1
#define INVALID -1

using namespace std;

class Graph
{
    public:
        // Constructor
        Graph(int id);
        Graph(int id, char * filename);

        // get and set functions
        int get_link_cost(int from, int to);
        void update_link(bool status, int linkCost, int from, int to);

        // testing function
        void display();

    private:
        int globalNodeID;
        
        // NOTE: vectors are indexed [From][To]
        vector< vector<bool> > valid;
        vector< vector<int> > cost;
};


#endif 

