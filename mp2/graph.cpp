
#include "graph.h"

Graph::Graph()
{
    globalNodeID = 0;
    valid.resize(NUM_NODES, vector<bool>(NUM_NODES, false));
    cost.resize(NUM_NODES, vector<int>(NUM_NODES, INIT_COST));
}

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
        char * cur_link = (char*) malloc((filesize + 1)*sizeof(char));
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
                while(cur_link[j] != '\n' && cur_link[j] != '\0')
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

    fclose(fp);
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
    return cost[from][to];
}

int Graph::getLinkStatus(int from, int to)
{
    return valid[from][to];
}


void Graph::updateStatus(bool status, int from, int to)
{
    valid[from][to] = status;
}

void Graph::updateCost(int linkCost, int from, int to)
{
    cost[from][to] = linkCost;
}

void Graph::updateLink(bool status, int linkCost, int from, int to)
{
    valid[from][to] = status;
    cost[from][to] = linkCost;
}

void Graph::getNeighbors(int nodeID, vector<int> & neighbors)
{
    for(int i = 0; i < NUM_NODES; i++) {
        if(valid[nodeID][i] == true)
            neighbors.push_back(i);
    }
}

vector<int> Graph::dijkstra()
{
    priority_queue< int_pair, vector<int_pair>, greater<int_pair> > pq;
    vector<int> distance(NUM_NODES, numeric_limits<int>::max());
    vector<int> predecessor(NUM_NODES, INVALID);

    // initialize vectors
    distance[globalNodeID] = 0;
    pq.push(make_pair(0, globalNodeID));

    while(!pq.empty()){

        int_pair cur = pq.top();
        int nodeID = cur.second;
        int distAbs = cur.first;
        pq.pop();

        if(distAbs > distance[nodeID])
            continue;

        vector<int> neighbors(0);
        getNeighbors(nodeID, neighbors);

        for(size_t i = 0; i < neighbors.size(); i++) {
            if(distAbs + cost[nodeID][neighbors[i]] < distance[neighbors[i]]){
                distance[neighbors[i]] = distAbs + cost[nodeID][neighbors[i]];
                predecessor[neighbors[i]] = nodeID;
                pq.push(make_pair(distance[neighbors[i]],neighbors[i]));
            }
        }
    }

    return predecessor;
}

stack<int> Graph::dijkstraTest(int to)
{
    priority_queue< int_pair, vector<int_pair>, greater<int_pair> > pq;
    vector<int> distance(NUM_NODES, numeric_limits<int>::max());
    vector<int> predecessor(NUM_NODES, INVALID);
    stack<int> path;

    // initialize vectors
    distance[globalNodeID] = 0;
    pq.push(make_pair(0, globalNodeID));

    while(!pq.empty()){

        int_pair cur = pq.top();
        int nodeID = cur.second;
        int distAbs = cur.first;
        pq.pop();

        if(distAbs > distance[nodeID])
            continue;

        vector<int> neighbors(0);
        getNeighbors(nodeID, neighbors);

        for(size_t i = 0; i < neighbors.size(); i++) {
            if(distAbs + cost[nodeID][neighbors[i]] < distance[neighbors[i]]){
                distance[neighbors[i]] = distAbs + cost[nodeID][neighbors[i]];
                predecessor[neighbors[i]] = nodeID;
                pq.push(make_pair(distance[neighbors[i]],neighbors[i]));
            }
        }
    }

    if(predecessor[to] == INVALID) return path;

    int nextNode = to;
    while(nextNode != globalNodeID)
    {
        path.push(nextNode);
        nextNode = predecessor[nextNode];
    }
    path.push(nextNode);

    return path;
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
