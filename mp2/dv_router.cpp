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
                // TODO: updateForwardingTable();
                // TODO: send DV update packet to other neighbors
            }
        }
    }
}

void DV_Router::listenForNeighbors()
{
    char fromAddr[100];
    struct sockaddr_in senderAddr;
    socklen_t senderAddrLen;
    unsigned char recvBuf[1000];

    int bytesRecvd;

    while(1)
    {
        memset(recvBuf, 0, 1000);
        senderAddrLen = sizeof(senderAddr);
        if ((bytesRecvd = recvfrom(sockfd, recvBuf, 1000 , 0,
                (struct sockaddr*)&senderAddr, &senderAddrLen)) == -1) {
            perror("connectivity listener: recvfrom failed");
            exit(1);
        }

        inet_ntop(AF_INET, &senderAddr.sin_addr, fromAddr, 100);

        short int heardFromNode = -1;
        if(strstr(fromAddr, "10.1.1."))
        {
            heardFromNode = atoi(strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);

            //record that we heard from heardFromNode just now.
            gettimeofday(&globalLastHeartbeat[heardFromNode], 0);

            // this node can consider heardFromNode to be directly connected to it; do any such logic now.
            if(valid[heardFromNode] == false){
                valid[heardFromNode] = true;
                // TODO: updateForwardingTable();
                // TODO: send DV update packet to neighbors, including the new node.
                // Maybe this should be done periodically?
            }
        }

        short int destID = 0;
        short int nextNode = 0;
        if(strncmp((const char*)recvBuf, (const char*)"send", 4) == 0) {
            // send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>

            destID = ntohs(((short int*)recvBuf)[2]);

            // sending to next hop
            if((nextNode = forwardingTable[destID]) != INVALID) {

                recvBuf[0] = 'f';
                recvBuf[1] = 'o';
                recvBuf[2] = 'r';
                recvBuf[3] = 'w';

                sendto(sockfd, recvBuf, SEND_SIZE, 0,
                        (struct sockaddr*)&globalNodeAddrs[nextNode],
                        sizeof(globalNodeAddrs[nextNode]));
                recvBuf[106] = '\0';
                logToFile(SEND_MES, destID, nextNode, (char *)recvBuf + 6);
            } else{
                logToFile(UNREACHABLE_MES, destID, nextNode, (char *)recvBuf + 6);
            }

        } else if(strncmp((const char*)recvBuf, (const char*)"forw", 4) == 0) {
            // forward format: 'forw'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>

            destID = ntohs(((short int*)recvBuf)[2]);

            // if we are the dest
            if(destID == myNodeID){
                recvBuf[106] = '\0';
                logToFile(RECV_MES, destID, nextNode, (char *)recvBuf + 6);
                continue;
            }

            // forward if we are not the dest
            if((nextNode = forwardingTable[destID]) != INVALID) {
                sendto(sockfd, recvBuf, SEND_SIZE, 0,
                        (struct sockaddr*)&globalNodeAddrs[nextNode],
                        sizeof(globalNodeAddrs[nextNode]));
                recvBuf[106] = '\0';
                logToFile(FORWARD_MES, destID, nextNode, (char *)recvBuf + 6);
            } else{
                logToFile(UNREACHABLE_MES, destID, nextNode, (char *)recvBuf + 6);
            }

        } else if(strncmp((const char*)recvBuf, (const char*)"cost", 4) == 0){
            //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
            destID = ntohs(((short int*)recvBuf)[2]);
            network.updateCost(ntohs(*((int*)&(((char*)recvBuf)[6]))), myNodeID, destID);

            if(network.getLinkStatus(myNodeID, heardFromNode) == true){
                updateForwardingTable();
                generateLSPL(myNodeID, heardFromNode);
            }

        } /*else if(strcmp((const char*)recvBuf, (const char*)"lsp") == 0){
            //'lsp\0'<4 ASCII bytes>, rest of LSPL_t struct

            if(bytesRecvd != sizeof(LSPL_t)){
                perror("incorrect bytes received for LSPL_t");
             } else{
                LSPL_t lsplForward = networkToHostLSPL((LSPL_t *)recvBuf);
                if(lsplForward.sequence_num > seqNums[lsplForward.producerNode]){
                    seqNums[lsplForward.producerNode] = lsplForward.sequence_num;

                    network.updateStatus((bool)lsplForward.updatedLink.valid,
                        lsplForward.updatedLink.sourceNode,lsplForward.updatedLink.destNode);

                    network.updateCost(lsplForward.updatedLink.cost,
                        lsplForward.updatedLink.sourceNode, lsplForward.updatedLink.destNode);
                    updateForwardingTable();
                    forwardLSPL((char *)recvBuf, heardFromNode);
                }
             }
        }*/ // TODO: Modify this for DV packets

        checkHeartBeat();
    }
}



