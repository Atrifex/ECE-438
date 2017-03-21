
#include "graph.h"

Graph::Graph(int id)
{

    valid.resize(NUM_NODES, vector<bool>(NUM_NODES, false));
    cost.resize(NUM_NODES, vector<int>(NUM_NODES, INIT_COST));

}

Graph::Graph(int id, char * filename)
{

    valid.resize(NUM_NODES, vector<bool>(NUM_NODES, false));
    cost.resize(NUM_NODES, vector<int>(NUM_NODES, INIT_COST));

}

int Graph::get_link_cost(int from, int to)
{
    return (valid[from][to] == false)? INVALID : cost[from][to];
}

void Graph::update_link(bool status, int linkCost, int from, int to)
{
    valid[from][to] = status;
    cost[from][to] = linkCost;
}


void Graph::display()
{
    for (int i = 0; i < NUM_NODES; i++)
    {
        cout << "Node: " << i << endl;
        for (int j = 0; j < NUM_NODES; j++)
        {
            if(valid[i][j] == true)
            {
                cout << "   ->" << j << ", cost = " << cost[i][j] << endl;
            }
        }
    }
}

