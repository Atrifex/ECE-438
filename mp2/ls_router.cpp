
#include "ls_router.h"

LS_Router::LS_Router(int id, char * graphFileName, char * logFileName) : Router(id, logFileName) {
    network = new Graph(id, graphFileName);
    seqNums.resize(NUM_NODES, INVALID);

#ifdef DEBUG
    char lspFileName[100];
    sprintf(lspFileName, "log/packetsLSP%d", id);
    // initialize file pointer
    if((lspFileptr = fopen(lspFileName, "w")) == NULL){
        perror("fopen");
        close(sockfd);
        exit(1);
    }
    changed = false;
#endif
}

LS_Router::~LS_Router() {
    delete network;
}

void LS_Router::createLSP(lsp_t & lsp, vector<int> & neighbors)
{
    lsp.producerNode = htonl(myNodeID);
    lsp.sequenceNum = htonl(++seqNums[myNodeID]);

    for(size_t i = 0; i < neighbors.size(); i++){
        lsp.links[i].neighbor = htonl(neighbors[i]);
        lsp.links[i].weight = htonl((int)network->getLinkCost(myNodeID, neighbors[i]));
        lsp.links[i].status = htonl((int)network->getLinkStatus(myNodeID, neighbors[i]));
    }

    lsp.numLinks = htonl(neighbors.size());
}

void LS_Router::sendLSP()
{
    lsp_t lsp;
    vector<int> neighbors;

    changedLock.lock();
    if(changed == false){
        changedLock.unlock();
        return;
    }

    network->getNeighbors(myNodeID, neighbors);
    createLSP(lsp, neighbors);

    changed = false;
    changedLock.unlock();

    int sizeOfLSP = neighbors.size()*sizeof(link_t) + 3*sizeof(int) + 4*sizeof(char);
    for(size_t i = 0; i < neighbors.size(); i++){
        sendto(sockfd, (char *)&lsp, sizeOfLSP, 0,
            (struct sockaddr *)&globalNodeAddrs[neighbors[i]], sizeof(globalNodeAddrs[neighbors[i]]));
    }
}

void LS_Router::announceToNeighbors()
{
    struct timespec sleepFor;
    sleepFor.tv_sec = 0;
    sleepFor.tv_nsec = 300 * 1000 * 1000;   //300 ms
    while(1)
    {
        hackyBroadcast("HEREIAM", 7);
        sendLSP();
        nanosleep(&sleepFor, 0);
    }
}

bool LS_Router::processLSP(lsp_t * lspNetwork)
{
    vector<bool> lspStatus(NUM_NODES, false);
    vector<int> lspCost(NUM_NODES, INVALID);

    int producerNode = ntohl(lspNetwork->producerNode);
    int numLinks = ntohl(lspNetwork->numLinks);

    // network->resetNodeInfo(producerNode);
    for(int i = 0; i < numLinks; i++){
        int neighbor = ntohl(lspNetwork->links[i].neighbor);
        lspCost[neighbor] = ntohl(lspNetwork->links[i].weight);
        lspStatus[neighbor] = (bool)ntohl(lspNetwork->links[i].status);
#ifdef DEBUG
        int seqNum = ntohl(lspNetwork->sequenceNum);
        lspLogger(seqNum, producerNode, neighbor, lspStatus[neighbor], lspCost[neighbor]);
#endif
    }

    return network->updateAndCheckChanges(producerNode, lspStatus, lspCost);
}

void LS_Router::forwardLSP(char * LSP_Buf, int bytesRecvd, int heardFromNode)
{
    vector<int> neighbors;
    network->getNeighbors(myNodeID, neighbors);

    for(size_t i = 0; i < neighbors.size(); i++)
    {
        int nextNode = neighbors[i];

        if(nextNode != heardFromNode){
            sendto(sockfd, LSP_Buf, bytesRecvd, 0, (struct sockaddr *)&globalNodeAddrs[nextNode], sizeof(globalNodeAddrs[nextNode]));
        }
    }
}

void LS_Router::updateForwardingTable()
{
    vector<int> predecessor = network->dijkstra();

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
}

void LS_Router::checkHeartBeat()
{
    vector<int> neighbors;
    struct timeval tv, tv_heart;
    gettimeofday(&tv, 0);

    long int cur_time = tv.tv_sec*1000000 + tv.tv_usec;
    long int lastHeartbeat_usec;

    network->getNeighbors(myNodeID, neighbors);

    changedLock.lock();
    for(size_t i = 0; i < neighbors.size(); i++)
    {
        int nextNode = neighbors[i];
        tv_heart = globalLastHeartbeat[nextNode];
        lastHeartbeat_usec = tv_heart.tv_sec*1000000 + tv_heart.tv_usec;

        if(cur_time - lastHeartbeat_usec > HEARTBEAT_THRESHOLD)
        {
            changed = true;
            network->updateStatus(false, myNodeID, nextNode);
            network->updateStatus(false, nextNode, myNodeID);
            seqNums[nextNode] = INVALID;
        }
    }
    changedLock.unlock();


}

void LS_Router::listenForNeighbors()
{
    char fromAddr[100];
    struct sockaddr_in senderAddr;
    socklen_t senderAddrLen;
    unsigned char recvBuf[1000];

    int bytesRecvd;

    struct timeval lastGraphUpdate, graphUpdateCheck;
    gettimeofday(&graphUpdateCheck, 0);
    lastGraphUpdate = graphUpdateCheck;

    while(1)
    {
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
            gettimeofday(&globalLastHeartbeat[heardFromNode], 0);

            changedLock.lock();
            if(network->getLinkStatus(myNodeID, heardFromNode) == false){
                changed = true;
            }
            network->updateStatus(true, myNodeID, heardFromNode);
            changedLock.unlock();
        }

        short int destID = 0;
        short int nextNode = 0;
        if(strncmp((const char*)recvBuf, (const char*)"send", 4) == 0) {
            // send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>

            destID = ntohs(((short int*)recvBuf)[2]);
            updateForwardingTable();

            // sending to next hop
            if((nextNode = forwardingTable[destID]) != INVALID) {

                recvBuf[0] = 'f';
                recvBuf[1] = 'o';
                recvBuf[2] = 'r';
                recvBuf[3] = 'w';

                sendto(sockfd, recvBuf, bytesRecvd, 0,
                        (struct sockaddr*)&globalNodeAddrs[nextNode],
                        sizeof(globalNodeAddrs[nextNode]));
                recvBuf[bytesRecvd] = '\0';
                logToFile(SEND_MES, destID, nextNode, (char *)recvBuf + 6);
            } else{
                logToFile(UNREACHABLE_MES, destID, 0, NULL);
            }

        } else if(strncmp((const char*)recvBuf, (const char*)"forw", 4) == 0) {
            // forward format: 'forw'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>

            destID = ntohs(((short int*)recvBuf)[2]);

            // if we are the dest
            if(destID == myNodeID){
                recvBuf[bytesRecvd] = '\0';
                logToFile(RECV_MES, destID, nextNode, (char *)recvBuf + 6);
                continue;
            }

            updateForwardingTable();

            // forward if we are not the dest
            if((nextNode = forwardingTable[destID]) != INVALID) {
                sendto(sockfd, recvBuf, bytesRecvd, 0,
                        (struct sockaddr*)&globalNodeAddrs[nextNode],
                        sizeof(globalNodeAddrs[nextNode]));
                recvBuf[bytesRecvd] = '\0';
                logToFile(FORWARD_MES, destID, nextNode, (char *)recvBuf + 6);
            } else{
                logToFile(UNREACHABLE_MES, destID, 0, NULL);
            }

        } else if(strncmp((const char*)recvBuf, (const char*)"cost", 4) == 0){
            //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
            destID = ntohs(((short int*)recvBuf)[2]);
            int costNew = ntohs(*((int*)&(((char*)recvBuf)[6])));

            changedLock.lock();
            network->updateCost(costNew, myNodeID, destID);
            if(network->getLinkStatus(myNodeID, destID) == true){
                changed = true;
            }
            changedLock.unlock();

        } else if(strcmp((const char*)recvBuf, (const char*)"lsp") == 0){
            //'lsp\0'<4 ASCII bytes>, rest of lsp_t struct
            int producerNode = ntohl(((int *)recvBuf)[1]);
            int sequenceNum = ntohl(((int *)recvBuf)[2]);
            if(sequenceNum > seqNums[producerNode]){
                seqNums[producerNode] = sequenceNum;
                forwardLSP((char *)recvBuf, bytesRecvd, heardFromNode);
                bool changeInter = processLSP((lsp_t *)recvBuf);

                changedLock.lock();
                if(changeInter == true)
                    changed = true;
                changedLock.unlock();
            }
        }

        checkHeartBeat();

#ifdef GRADE
        gettimeofday(&graphUpdateCheck, 0);
        if(graphUpdateCheck.tv_sec >= lastGraphUpdate.tv_sec + 5) {
            network->writeToFile();
            lastGraphUpdate = graphUpdateCheck;
        }
#endif
    }
}

void LS_Router::lspLogger(int seqNum, int from, int to, bool status, int weight)
{
    char logLine[256]; // Message is <= 100 bytes, so this is always enough

    sprintf(logLine, "seqNum %d, from %d, to %d, status %d, cost %d\n", seqNum, from, to, status, weight);
    if(myNodeID == 0)
        cout << logLine;

    // Write to logFile
    if(fwrite(logLine, 1, strlen(logLine), lspFileptr) != strlen(logLine)) {
            fprintf(stderr, "logToFile: Error in fwrite(): Not all data written\n");
    }

    fflush(lspFileptr);
}
