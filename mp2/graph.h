#ifndef GRAPH_H
#define GRAPH_H

#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

#define NUM_NODES 256
#define INIT_COST 1
#define INVALID -1

using std::cout;
using std::endl;
using std::vector;

class Graph
{
    public:
        // Constructor
        Graph(){};
        Graph(int id, char * filename);
        Graph & operator=(Graph & other);

        // get and set functions
        int getLinkCost(int from, int to);
        void updateLink(bool status, int from, int to);
        void updateLink(int linkCost, int from, int to);
        void updateLink(bool status, int linkCost, int from, int to);

        // TODO: Dijkstra's Algorithm

        // testing function
        void display();

    private:
        int globalNodeID;

        // NOTE: vectors are indexed [From][To]
        vector< vector<bool> > valid;
        vector< vector<int> > cost;
};


#endif
