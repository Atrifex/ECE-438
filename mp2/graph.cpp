
#include "graph.h"

Graph::Graph(int id, char * filename)
{
    int j, k;
    size_t num_bytes;
    int bytes_read;
    char * initialcostsfile = filename;
    FILE * fp = fopen(initialcostsfile, "r");

    globalNodeID = id;

    valid.resize(NUM_NODES, vector<bool>(NUM_NODES, false));
    cost.resize(NUM_NODES, vector<int>(NUM_NODES, INIT_COST));

    if(fp == NULL)
    {
        fprintf(stderr, "Erroneous initial costs file provided\n");
        exit(1);
    }

    // Check for an empty file
    fseek(fp, 0, SEEK_END);
    int filesize = ftell(fp); // size is 0 if empty
    rewind(fp);

    if(filesize != 0)
    {
        char cur_node[4];
        char cur_cost[8]; // Cost must be less than 2^23
        char * cur_link = (char*) malloc((num_bytes + 1)*sizeof(char));
        while((bytes_read = getline(&cur_link, &num_bytes, fp)) != -1) // Read a line from the initial costs file
        {
                // Parse each line individually, filling out the cur_node and cur_cost strings
                j = 0;
                while(cur_link[j] != ' ')
                {
                        cur_node[j] = cur_link[j];
                        j++;
                }
                cur_node[j] = '\0'; // Assume they aren't trying to break stuff

                j++;
                k = 0;
                while(cur_link[j] != '\n')
                {
                        cur_cost[k] = cur_link[j];
                        k++; j++;
                }
                cur_cost[k] = '\0'; // Again, assume they aren't trying to break things

                int node_to = atoi(cur_node);
                int cost_to = atoi(cur_cost);

                cost[globalNodeID][node_to] = cost_to;
        }

        free(cur_link);
    }
}

Graph & Graph::operator=(Graph & other)
{
    if(this != &other)
    {
        globalNodeID = other.globalNodeID;
        valid = other.valid;
        cost = other.cost;
    }

    return *this;
}

int Graph::getLinkCost(int from, int to)
{
    return (valid[from][to] == false)? INVALID : cost[from][to];
}

void Graph::updateLink(bool status, int from, int to)
{
    valid[from][to] = status;
}

void Graph::updateLink(int linkCost, int from, int to)
{
    cost[from][to] = linkCost;
}

void Graph::updateLink(bool status, int linkCost, int from, int to)
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
