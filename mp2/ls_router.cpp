
#include "ls_router.h"

LS_Router::LS_Router(int id, char * graphFileName, char * logFileName) : Router(id, logFileName)
{
    // initialize graph
    Graph temp(id, graphFileName);
    network = temp;
}

void LS_Router::checkHeartBeat()
{
    vector<int> neighbors;
    struct timeval tv, tv_heart;
    gettimeofday(&tv, 0);

    long int cur_time = (tv.tv_sec - tv.tv_sec)*1000000 + tv.tv_usec - tv.tv_usec;
    long int lastHeartbeat_usec;

    network.getNeighbors(myNodeID, neighbors);
    for(size_t i = 0; i < neighbors.size(); i++)
    {
        int nextNode = neighbors[i];
        if(forwardingTable[nextNode] != INVALID)
        {
            tv_heart = globalLastHeartbeat[nextNode];
            lastHeartbeat_usec = (tv_heart.tv_sec - tv_heart.tv_sec)*1000000 + tv_heart.tv_usec - tv_heart.tv_usec;

            if(cur_time - lastHeartbeat_usec > HEARTBEAT_THRESHOLD) // Link has died
            {
                network.updateLink(false, myNodeID, nextNode);
                network.updateLink(false, nextNode, myNodeID);
                updateForwardingTable();

                // TODO: LSP Packet that we lost connection
            }
        }
    }
}

void LS_Router::sendLSPL(LSPL_t * linkState, int heardFromNode)
{
    vector<int> neighbors;
    network.getNeighbors(myNodeID, neighbors);

    LSPL_t netLSP = hostToNetworkLSPL(linkState);

    for(size_t i = 0; i < neighbors.size(); i++)
    {
        int nextNode = neighbors[i];

        // Don't send back to the node that forwarded us the LSP
        if(nextNode != heardFromNode && forwardingTable[nextNode] != INVALID)
        {
            sendto(sockfd, (char*)&netLSP, sizeof(LSPL_t), 0,
                (struct sockaddr *)&globalNodeAddrs[nextNode], sizeof(globalNodeAddrs[nextNode]));
        }
    }
}

void LS_Router::sendLSPU(vector<LSPL_t> & networkState, int destNode)
{
    for(size_t i = 0; i < networkState.size(); i++){
        sendto(sockfd, (char*)&networkState[i], sizeof(LSPL_t), 0,
            (struct sockaddr *)&globalNodeAddrs[destNode], sizeof(globalNodeAddrs[destNode]));
    }
}

void LS_Router::listenForNeighbors()
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
            if(network.getLinkCost(myNodeID, heardFromNode) == INVALID){
                network.updateLink(true, myNodeID, heardFromNode);
                network.updateLink(true, heardFromNode, myNodeID);
                updateForwardingTable();

                // TODO: Send LSPU to the heardFromNode
                // TODO: Send LSP to all other neighbors
            }
        }

        short int destID = 0;
        short int nextNode = 0;
        // send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
        if(strncmp((const char*)recvBuf, (const char*)"send", 4) == 0) {
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
                // TODO: Send LSP about the node that you heard from
            }

        } else if(strncmp((const char*)recvBuf, (const char*)"cost", 4) == 0){
            //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
            destID = ntohs(((short int*)recvBuf)[2]);
            network.updateLink(ntohs(*((int*)&(((char*)recvBuf)[6]))), myNodeID, destID);

            if(network.getLinkCost(myNodeID, heardFromNode) != INVALID){
                updateForwardingTable();
                // TODO: Send LSP about the node that you heard from
            }

        }

        checkHeartBeat();
    }
}

void LS_Router::updateForwardingTable()
{

    vector<int> predecessor = network.dijkstra();

    for(int i = 0; i < NUM_NODES; i++){
        if(predecessor[i] == INVALID){
            forwardingTable[i] = INVALID;
            continue;
        }

        int nextNode = i;
        while(predecessor[nextNode] != myNodeID)
            nextNode = predecessor[nextNode];

        forwardingTable[i] = nextNode;
    }

    return;
}

LSPL_t LS_Router::hostToNetworkLSPL(LSPL_t * hostval)
{
    LSPL_t ret;

    // LSPL_t contains only ints, so it is 4-byte aligned.
    // Therefore, we can individually convert each member of the struct into network byte order

    ret.producerNode = htonl(hostval->producerNode);
    ret.sequence_num = htonl(hostval->sequence_num);
    ret.updatedLink.sourceNode = htonl(hostval->updatedLink.sourceNode);
    ret.updatedLink.destNode = htonl(hostval->updatedLink.destNode);
    ret.updatedLink.cost = htonl(hostval->updatedLink.cost);
    ret.updatedLink.valid = htonl(hostval->updatedLink.valid);

    return ret;
}

LSPL_t LS_Router::networkToHostLSPL(LSPL_t * networkval)
{
    LSPL_t ret;

    // Since LSPL_t is 4-byte aligned, everything works out nicely here
    ret.producerNode = ntohl(networkval->producerNode);
    ret.sequence_num = ntohl(networkval->sequence_num);
    ret.updatedLink.sourceNode = ntohl(networkval->updatedLink.sourceNode);
    ret.updatedLink.destNode = ntohl(networkval->updatedLink.destNode);
    ret.updatedLink.cost = ntohl(networkval->updatedLink.cost);
    ret.updatedLink.valid = ntohl(networkval->updatedLink.valid);

    return ret;
}

