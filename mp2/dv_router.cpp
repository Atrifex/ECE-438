#include "dv_router.h"

DV_Router::DV_Router(int id, char * initialcostsfile, char * logFileName) : Router(id, logFileName)
{
    int i, j, num_bytes, bytes_read;

    // Direct-link costs are assumed to be 1 unless the file says otherwise
    costs.resize(NUM_NODES, 1);

    // Parse initial costs
    FILE * fp = fopen(initialcostsfile, "r");

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

                costs[node_to] = cost_to; // Update initial cost
        }

        free(cur_link);
    }

    fclose(fp);
}

    valid.resize(NUM_NODES, false); // All links are initially considered invalid

    // We don't know our neighbors yet, so all distances are initially infinity
    distances.resize(NUM_NODES, INFINITY);
    distances[id] = 0; // Distance to ourself is 0

    // We don't know our neighbors, so we can't yet store neighbor distance vectors.
    //neighborDistances.resize(NUM_NODES, vector<int>());
}

void DV_Router::checkHeartBeat()
{
    vector<int> neighbors;
    struct timeval tv, tv_heart;
    gettimeofday(&tv, 0);

    long int cur_time = tv.tv_sec*1000000 + tv.tv_usec;
    long int lastHeartbeat_usec;

    // Get our neighbors
    for(int i = 0; i < NUM_NODES; i++)
    {
        if(valid[i] && (i != myNodeID))
            neighbors.push_back(i);
    }

    for(size_t i = 0; i < neighbors.size(); i++)
    {
        int nextNode = neighbors[i];
        if(forwardingTable[nextNode] != INVALID)
        {
            tv_heart = globalLastHeartbeat[nextNode];
            lastHeartbeat_usec = tv_heart.tv_sec*1000000 + tv_heart.tv_usec;

            if(cur_time - lastHeartbeat_usec > HEARTBEAT_THRESHOLD) // Link has died
            {
                valid[nextNode] = false;
                distances[nextNode] = INFINITY;  
                // updateForwardingTable(); // Can't do this without neighbor's info
                // TODO: send DV update packet to other neighbors
            }
        }
    }
}





