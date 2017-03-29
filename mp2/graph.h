#ifndef GRAPH_H
#define GRAPH_H

#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <queue>
#include <stack>
#include <utility>

#define NUM_NODES 256
#define INIT_COST 1
#define INVALID -1

using std::cout;
using std::endl;
using std::vector;
using std::numeric_limits;
using std::priority_queue;
using std::pair;
using std::make_pair;
using std::greater;
using std::queue;
using std::stack;

// pairs are stored as <distance, nodeID>
typedef pair<int,int> int_pair;

class Graph
{
    public:
        // Constructor
        Graph();
        Graph(int id, char * filename);
        Graph & operator=(Graph & other);

        // get and set functions
        int getLinkCost(int from, int to);
        int getLinkStatus(int from, int to);
        void updateStatus(bool status, int from, int to);
        void updateCost(int linkCost, int from, int to);
        void updateLink(bool status, int linkCost, int from, int to);

        // Member Functions
        void getNeighbors(int nodeID, vector<int> & neighbors);
        vector<int> dijkstra();

        // testing function
        void display();
        stack<int> dijkstraTest(int to);

    private:
        int globalNodeID;

        // NOTE: vectors are indexed [From][To]
        vector< vector<bool> > valid;
        vector< vector<int> > cost;
};


#endif
