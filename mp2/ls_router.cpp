
#include "ls_router.h"

LS_Router::LS_Router(int id, char * graphFileName, char * logFileName) : Router(id, logFileName) {
    network = new Graph(id, graphFileName);
    seqNums.resize(NUM_NODES, INVALID);
    changeSeqNums.resize(NUM_NODES, INVALID);
    srand(id);

#ifdef DEBUG
    char lspFileName[100];
    sprintf(lspFileName, "log/packetsLSP%d", id);
    // initialize file pointer
    if((lspFileptr = fopen(lspFileName, "w")) == NULL){
        perror("fopen");
        close(sockfd);
        exit(1);
    }
#endif
}

LS_Router::~LS_Router() {
    delete network;
}

bool LS_Router::processLSC(lsc_t * lscNetwork)
{
    bool linkFailed = false;

    unsigned char producerNode = lscNetwork->producerNode;
    unsigned char numLinks = lscNetwork->numLinks;

    for(int i = 0; i < numLinks; i++){
        unsigned char neighbor = lscNetwork->links[i].neighbor;
        int weight = ntohl(lscNetwork->links[i].weight);
        bool status = (bool)lscNetwork->links[i].status;

        network->updateLink(status, weight, producerNode, neighbor);

        if(status == false){
            linkFailed = true;
        }

#ifdef DEBUG
        int seqNum = ntohl(lscNetwork->sequenceNum);
        lspLogger(seqNum, producerNode, neighbor, weight, 1);
#endif

    }

    return linkFailed;

}

void LS_Router::forwardLSC(char * LSC_Buf, int bytesRecvd, int heardFromNode)
{
    vector<int> neighbors;
    network->getNeighbors(myNodeID, neighbors);

    for(size_t i = 0; i < neighbors.size(); i++)
    {
        int nextNode = neighbors[i];

        if(nextNode != heardFromNode){
            sendto(sockfd, LSC_Buf, bytesRecvd, 0, (struct sockaddr *)&globalNodeAddrs[nextNode], sizeof(globalNodeAddrs[nextNode]));
        }
    }
}

void LS_Router::createLSC(lsc_t & lsc)
{
    lsc.sequenceNum = htonl(++changeSeqNums[myNodeID]);
    lsc.producerNode = (unsigned char) myNodeID;
    lsc.numLinks = (unsigned char) changeSet.size();

    int i = 0;
    for(int node : changeSet){
        lsc.links[i].weight = htonl((int)network->getLinkCost(myNodeID, node));
        lsc.links[i].status = (unsigned char) network->getLinkStatus(myNodeID, node);
        lsc.links[i].neighbor = (unsigned char) node;
        i++;
    }
}

void LS_Router::sendLSC()
{
    lsc_t lsc;
    vector<int> neighbors;

    changedLock.lock();
    if(changeSet.empty()){
        changedLock.unlock();
        return;
    }

    createLSC(lsc);
    int setSize = changeSet.size();
    changeSet.clear();

    network->getNeighbors(myNodeID, neighbors);
    changedLock.unlock();

    int sizeOfLSC = sizeof(linkChange_t)*setSize + sizeof(int) + 6*sizeof(char);
    for(size_t i = 0; i < neighbors.size(); i++){
        sendto(sockfd, (char *)&lsc, sizeOfLSC, 0,
            (struct sockaddr *)&globalNodeAddrs[neighbors[i]], sizeof(globalNodeAddrs[neighbors[i]]));
    }

}

void LS_Router::announceToNeighbors()
{
    struct timespec sleepFor;
    sleepFor.tv_sec = 0;
    sleepFor.tv_nsec = 300 * 1000 * 1000;   //300 ms

    struct timespec staggerSleepSetup;
    staggerSleepSetup.tv_sec = 0;
    staggerSleepSetup.tv_nsec = (myNodeID*STAGGER_TIME_LSC) % (300*NS_PER_MS);

    struct timespec staggerSleepHold;
    staggerSleepHold.tv_sec = 0;
    staggerSleepHold.tv_nsec = (300*NS_PER_MS) - ((myNodeID*STAGGER_TIME_LSC) % (300*NS_PER_MS));


    long long staggerSleepTotal = (myNodeID*STAGGER_TIME_LSC) % NS_PER_SEC;

    while(1){
        hackyBroadcast("HEREIAM", 7);
        nanosleep(&sleepFor, 0);

        if(staggerSleepTotal >= (300*NS_PER_MS)){
            hackyBroadcast("HEREIAM", 7);
            nanosleep(&sleepFor, 0);
        }

        hackyBroadcast("HEREIAM", 7);
        nanosleep(&staggerSleepSetup, 0);
        sendLSC();
        nanosleep(&staggerSleepHold, 0);
    }
}

bool LS_Router::processLSP(lsp_t * lspNetwork)
{
    vector<bool> lspStatus(NUM_NODES, false);
    vector<int> lspCost(NUM_NODES, INVALID);

    unsigned char producerNode = lspNetwork->producerNode;
    unsigned char numLinks = lspNetwork->numLinks;

    for(int i = 0; i < numLinks; i++){
        unsigned char neighbor = lspNetwork->links[i].neighbor;
        lspCost[neighbor] = ntohl(lspNetwork->links[i].weight);
        lspStatus[neighbor] = true;

#ifdef DEBUG
        int seqNum = ntohl(lspNetwork->sequenceNum);
        lspLogger(seqNum, producerNode, neighbor, lspCost[neighbor], 0);
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

void LS_Router::createLSP(lsp_t & lsp, vector<int> & neighbors)
{
    lsp.producerNode = (unsigned char) myNodeID;
    lsp.sequenceNum = htonl(++seqNums[myNodeID]);

    for(size_t i = 0; i < neighbors.size(); i++){
        lsp.links[i].neighbor = (unsigned char) neighbors[i];
        lsp.links[i].weight = htonl((int)network->getLinkCost(myNodeID, neighbors[i]));
    }

    lsp.numLinks = (unsigned char) neighbors.size();
}

void LS_Router::sendLSP()
{
    lsp_t lsp;
    vector<int> neighbors;

    changedLock.lock();
    network->getNeighbors(myNodeID, neighbors);
    createLSP(lsp, neighbors);
    changedLock.unlock();

    int sizeOfLSP = neighbors.size()*sizeof(link_t) + sizeof(int) + 6*sizeof(char);
    for(size_t i = 0; i < neighbors.size(); i++){
        sendto(sockfd, (char *)&lsp, sizeOfLSP, 0,
            (struct sockaddr *)&globalNodeAddrs[neighbors[i]], sizeof(globalNodeAddrs[neighbors[i]]));
    }

}

void LS_Router::generateLSP()
{
    struct timespec startOffsetSleep;
    startOffsetSleep.tv_sec = (myNodeID*STAGGER_TIME_LSP) / NS_PER_SEC;
    startOffsetSleep.tv_nsec = (myNodeID*STAGGER_TIME_LSP) % NS_PER_SEC;

    nanosleep(&startOffsetSleep, 0);

    struct timespec sleepFor;
    sleepFor.tv_sec = 5;
    sleepFor.tv_nsec = 0;       // 500 * 1000 * 1000;   // 500 ms

    while(1){
        sendLSP();
        nanosleep(&sleepFor, 0);
    }
}

void LS_Router::updateForwardingTable()
{
    vector<int> predecessor = network->dijkstra();

    for(int i = 0; i < NUM_NODES; i++){
        if(predecessor[i] == INVALID){
            forwardingTable[i] = INVALID;
            if(i != myNodeID){
                seqNums[i] = INVALID;
                changeSeqNums[i] = INVALID;
            }
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
            changeSet.insert(nextNode);
            network->updateStatus(false, myNodeID, nextNode);
            network->updateStatus(false, nextNode, myNodeID);
            seqNums[nextNode] = INVALID;
            changeSeqNums[nextNode] = INVALID;
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
                changeSet.insert(heardFromNode);
            }
            network->updateStatus(true, myNodeID, heardFromNode);
            changedLock.unlock();
        }

        checkHeartBeat();

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
            int costNew = ntohl(*((int*)&(((char*)recvBuf)[6])));

            changedLock.lock();
            network->updateCost(costNew, myNodeID, destID);
            if(network->getLinkStatus(myNodeID, destID) == true){
                changeSet.insert(destID);
            }
            changedLock.unlock();

        } else if(strcmp((const char*)recvBuf, (const char*)"lsp") == 0){
            //'lsp\0'<4 ASCII bytes>, rest of lsp_t struct

            unsigned char producerNode = ((lsp_t *)recvBuf)->producerNode;
            int sequenceNum = ntohl(((lsp_t *)recvBuf)->sequenceNum);
            if(sequenceNum > seqNums[producerNode]){
                seqNums[producerNode] = sequenceNum;
                forwardLSP((char *)recvBuf, bytesRecvd, heardFromNode);
                processLSP((lsp_t *)recvBuf);
            }
        } else if(strcmp((const char*)recvBuf, (const char*)"lsc") == 0){
            //'lsc\0'<4 ASCII bytes>, rest of linkChange_t struct

            lsc_t * lscNetwork = (lsc_t*)recvBuf;
            unsigned char producerNode = lscNetwork->producerNode;
            int sequenceNum = ntohl(lscNetwork->sequenceNum);
            if(sequenceNum > changeSeqNums[producerNode]){
                changeSeqNums[producerNode] = sequenceNum;

                forwardLSC((char *)recvBuf, bytesRecvd, heardFromNode);
                if(processLSC(lscNetwork) == true){
                    updateForwardingTable();
                }
            }
        }


#ifdef GRADE
        gettimeofday(&graphUpdateCheck, 0);
        if(graphUpdateCheck.tv_sec >= lastGraphUpdate.tv_sec + 5) {
            network->writeToFile();
            lastGraphUpdate = graphUpdateCheck;
        }
#endif
    }
}

void LS_Router::lspLogger(int seqNum, int from, int to, int weight, int mode)
{
    char logLine[256]; // Message is <= 100 bytes, so this is always enough

    if(mode == 0)
        sprintf(logLine, "LSP:: seqNum %d, from %d, to %d, cost %d\n", seqNum, from, to, weight);
    else
        sprintf(logLine, "LSC:: __changeSeqNum__ %d, from %d, to %d, cost %d\n", seqNum, from, to, weight);

    if(myNodeID == 1)
        cout << "NODE: " << myNodeID << " " << logLine;

    // Write to logFile
    if(fwrite(logLine, 1, strlen(logLine), lspFileptr) != strlen(logLine)) {
            fprintf(stderr, "logToFile: Error in fwrite(): Not all data written\n");
    }

    fflush(lspFileptr);
}
