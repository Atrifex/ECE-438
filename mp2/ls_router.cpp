
#include "ls_router.h"

LS_Router::LS_Router(int id, char * graphFileName, char * logFileName) : Router(id, logFileName)
{
    network = new Graph(id, graphFileName);

    seqNums.resize(NUM_NODES, INVALID);

    gettimeofday(&updateQueueTime, 0);
    lastUpdateQueueTime = updateQueueTime;
}

LS_Router::~LS_Router() {
    delete network;
}

void LS_Router::announceToNeighbors()
{
    struct timespec sleepFor;
    sleepFor.tv_sec = 0;
    sleepFor.tv_nsec = 300 * 1000 * 1000;   //300 ms
    while(1)
    {
        hackyBroadcast("HEREIAM", 7);
        lspManager();
        nanosleep(&sleepFor, 0);
    }
}

void LS_Router::checkHeartBeat()
{
    vector<int> neighbors;
    struct timeval tv, tv_heart;
    gettimeofday(&tv, 0);

    long int cur_time = tv.tv_sec*1000000 + tv.tv_usec;
    long int lastHeartbeat_usec;

    network.getNeighbors(myNodeID, neighbors);
    for(size_t i = 0; i < neighbors.size(); i++)
    {
        int nextNode = neighbors[i];
        tv_heart = globalLastHeartbeat[nextNode];
        lastHeartbeat_usec = tv_heart.tv_sec*1000000 + tv_heart.tv_usec;

        if(cur_time - lastHeartbeat_usec > HEARTBEAT_THRESHOLD) // Link has died
        {
            network.updateStatus(false, myNodeID, nextNode);
            network.updateStatus(false, nextNode, myNodeID);
            // generateLSPL(myNodeID, nextNode);
            // TODO: Need to mark change
            seqNums[nextNode] = INVALID;
        }
    }
}

void LS_Router::generateLSPL(int sourceNode, int destNode)
{
    vector<int> neighbors;
    network.getNeighbors(myNodeID, neighbors);

    lsp_t lspl_inst;
    lspl_inst.producerNode = myNodeID;
    lspl_inst.sequence_num = ++seqNums[myNodeID];
    lspl_inst.updatedLink.sourceNode = sourceNode;
    lspl_inst.updatedLink.destNode = destNode;
    lspl_inst.updatedLink.cost = network.getLinkCost(sourceNode, destNode);
    lspl_inst.updatedLink.valid = (int)network.getLinkStatus(sourceNode, destNode);

    lsp_t netLSP = hostToNetworkLSPL(&lspl_inst);

    for(size_t i = 0; i < neighbors.size(); i++)
    {
        int nextNode = neighbors[i];
        sendto(sockfd, (char*)&netLSP, sizeof(lsp_t), 0,
            (struct sockaddr *)&globalNodeAddrs[nextNode], sizeof(globalNodeAddrs[nextNode]));
    }
}

void LS_Router::periodicLSPL()
{
    vector<int> neighbors;
    network.getNeighbors(myNodeID, neighbors);

    for(size_t i = 0; i < neighbors.size(); i++){
        lsp_t lspl_inst;
        lspl_inst.producerNode = myNodeID;
        lspl_inst.sequence_num = ++seqNums[myNodeID];
        lspl_inst.updatedLink.sourceNode = myNodeID;
        lspl_inst.updatedLink.destNode = neighbors[i];
        lspl_inst.updatedLink.cost = network.getLinkCost(myNodeID, neighbors[i]);
        lspl_inst.updatedLink.valid = (int)network.getLinkStatus(myNodeID, neighbors[i]);
        lsp_t netLSP = hostToNetworkLSPL(&lspl_inst);

        for(size_t i = 0; i < neighbors.size(); i++)
        {
            int nextNode = neighbors[i];
            sendto(sockfd, (char*)&netLSP, sizeof(lsp_t), 0,
                (struct sockaddr *)&globalNodeAddrs[nextNode], sizeof(globalNodeAddrs[nextNode]));
        }
    }
}

void LS_Router::forwardLSPL(char * LSPL_Buf, int heardFromNode)
{
    vector<int> neighbors;
    network.getNeighbors(myNodeID, neighbors);

    for(size_t i = 0; i < neighbors.size(); i++)
    {
        int nextNode = neighbors[i];

        if(nextNode != heardFromNode){
            sendto(sockfd, LSPL_Buf, sizeof(lsp_t), 0,
                (struct sockaddr *)&globalNodeAddrs[nextNode], sizeof(globalNodeAddrs[nextNode]));
        }
    }
}

void LS_Router::generateLSPU(int linkSource, int linkDest, int destNode)
{
    lsp_t lspl_inst;
    lspl_inst.producerNode = myNodeID;
    lspl_inst.sequence_num = ++seqNums[myNodeID];
    lspl_inst.updatedLink.sourceNode = linkSource;
    lspl_inst.updatedLink.destNode = linkDest;
    lspl_inst.updatedLink.cost = network.getLinkCost(linkSource, linkDest);
    lspl_inst.updatedLink.valid = (int)network.getLinkStatus(linkSource, linkDest);
    lsp_t netLSP = hostToNetworkLSPL(&lspl_inst);

    sendto(sockfd, (char*)&netLSP, sizeof(lsp_t), 0,
        (struct sockaddr *)&globalNodeAddrs[destNode], sizeof(globalNodeAddrs[destNode]));
}

void LS_Router::lspManager()
{
    if(LSPQueue.empty()) return;

    gettimeofday(&LSPQueueTime, 0);
    int LSPQueueTime_us = LSPQueueTime.tv_sec*1000000 + LSPQueueTime.tv_usec;
    int lastLSPQueueTime_us = lastLSPQueueTime.tv_sec*1000000 + lastLSPQueueTime.tv_usec;
    if(LSPQueueTime_us <= QUEUE_THRESHOLD + lastLSPQueueTime_us)
        return;

    lastLSPQueueTime = LSPQueueTime;

    int_pair curElem = LSPQueue.front();

    for(int i = 0; i <  LSPL_PER_EPOCH; i++)
    {
        generateLSPL(curElem.first, curElem.second);
        LSPQueue.pop();

        if(LSPQueue.empty()) return;
        curElem = LSPQueue.front();
    }
}

void LS_Router::updateManager()
{
    if(updateQueue.empty()) return;

    gettimeofday(&updateQueueTime, 0);
    int updateQueueTime_us = updateQueueTime.tv_sec*1000000 + updateQueueTime.tv_usec;
    int lastUpdateQueueTime_us = lastUpdateQueueTime.tv_sec*1000000 + lastUpdateQueueTime.tv_usec;
    if(updateQueueTime_us <= QUEUE_THRESHOLD + lastUpdateQueueTime_us)
        return;

    lastUpdateQueueTime = updateQueueTime;

    int_pair curElem = updateQueue.front();
    while(network.getLinkStatus(myNodeID, curElem.first) == false) {
        updateQueue.pop();
        if(updateQueue.empty()) return;
        curElem = updateQueue.front();
    }

    int counter = 0;
    int i = 0;
    for(i = curElem.second; i < NUM_NODES*NUM_NODES; i++){
        if(counter == LSPU_PER_EPOCH)
            break;
        if(network.getLinkStatus(i%NUM_NODES, i/NUM_NODES) == true && (i%NUM_NODES != curElem.first))
        {
            generateLSPU(i % NUM_NODES, i/NUM_NODES, curElem.first);
            counter++;
        }
    }

    updateQueue.pop();

    if(i < NUM_NODES*NUM_NODES)
    {
        curElem.second = i;
        updateQueue.push(curElem);
    }
}

void LS_Router::sendLSPU(int destNode)
{
    lsp_t lspl_inst, netLSP;

    lspl_inst.producerNode = myNodeID;
    lspl_inst.updatedLink.valid = TRUE;

    for(int from = 0; from < NUM_NODES; from++){
        for(int to = 0; to < NUM_NODES; to++){
            if(network.getLinkStatus(from, to) == true && from != destNode){
                lspl_inst.sequence_num = ++seqNums[myNodeID];
                lspl_inst.updatedLink.sourceNode = from;
                lspl_inst.updatedLink.destNode = to;
                lspl_inst.updatedLink.cost = network.getLinkCost(from, to);
                netLSP = hostToNetworkLSPL(&lspl_inst);
                sendto(sockfd, (char*)&netLSP, sizeof(lsp_t), 0,
                    (struct sockaddr *)&globalNodeAddrs[destNode], sizeof(globalNodeAddrs[destNode]));
            }
        }
    }
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
            if(network.getLinkStatus(myNodeID, heardFromNode) == false){
                network.updateStatus(true, myNodeID, heardFromNode);

                generateLSPL(myNodeID, heardFromNode);
                //sendLSPU(heardFromNode);
                updateQueue.push(make_pair(heardFromNode,0));
            }
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

                sendto(sockfd, recvBuf, SEND_SIZE, 0,
                        (struct sockaddr*)&globalNodeAddrs[nextNode],
                        sizeof(globalNodeAddrs[nextNode]));
                recvBuf[bytesRecvd] = '\0';
                logToFile(SEND_MES, destID, nextNode, (char *)recvBuf + 6);
            } else{
                logToFile(UNREACHABLE_MES, destID, nextNode, (char *)recvBuf + 6);
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
                sendto(sockfd, recvBuf, SEND_SIZE, 0,
                        (struct sockaddr*)&globalNodeAddrs[nextNode],
                        sizeof(globalNodeAddrs[nextNode]));
                recvBuf[bytesRecvd] = '\0';
                logToFile(FORWARD_MES, destID, nextNode, (char *)recvBuf + 6);
            } else{
                logToFile(UNREACHABLE_MES, destID, nextNode, (char *)recvBuf + 6);
            }

        } else if(strncmp((const char*)recvBuf, (const char*)"cost", 4) == 0){
            //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
            destID = ntohs(((short int*)recvBuf)[2]);
            network.updateCost(ntohs(*((int*)&(((char*)recvBuf)[6]))), myNodeID, destID);

            if(network.getLinkStatus(myNodeID, heardFromNode) == true){
                generateLSPL(myNodeID, heardFromNode);
            }

        } else if(strcmp((const char*)recvBuf, (const char*)"lsp") == 0){
            //'lsp\0'<4 ASCII bytes>, rest of lsp_t struct

            if(bytesRecvd != sizeof(lsp_t)){
                perror("incorrect bytes received for lsp_t");
             } else{
                lsp_t lsplForward = networkToHostLSPL((lsp_t *)recvBuf);
                if(lsplForward.sequence_num > seqNums[lsplForward.producerNode]){
                    seqNums[lsplForward.producerNode] = lsplForward.sequence_num;

                    network.updateStatus((bool)lsplForward.updatedLink.valid,
                        lsplForward.updatedLink.sourceNode,lsplForward.updatedLink.destNode);

                    network.updateCost(lsplForward.updatedLink.cost,
                        lsplForward.updatedLink.sourceNode, lsplForward.updatedLink.destNode);
                    forwardLSPL((char *)recvBuf, heardFromNode);
                }
             }
        }

        checkHeartBeat();

#ifdef GRADE
        gettimeofday(&graphUpdateCheck, 0);
        if(graphUpdateCheck.tv_sec >= lastGraphUpdate.tv_sec + 5) {
            network.writeToFile();
            lastGraphUpdate = graphUpdateCheck;
        }
#endif

        updateManager();
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

lsp_t LS_Router::hostToNetworkLSPL(lsp_t * hostval)
{
    lsp_t ret;

    // lsp_t contains only ints, so it is 4-byte aligned.
    // Therefore, we can individually convert each member of the struct into network byte order

    ret.producerNode = htonl(hostval->producerNode);
    ret.sequence_num = htonl(hostval->sequence_num);
    ret.updatedLink.sourceNode = htonl(hostval->updatedLink.sourceNode);
    ret.updatedLink.destNode = htonl(hostval->updatedLink.destNode);
    ret.updatedLink.cost = htonl(hostval->updatedLink.cost);
    ret.updatedLink.valid = htonl(hostval->updatedLink.valid);

    return ret;
}

lsp_t LS_Router::networkToHostLSPL(lsp_t * networkval)
{
    lsp_t ret;

    // Since lsp_t is 4-byte aligned, everything works out nicely here
    ret.producerNode = ntohl(networkval->producerNode);
    ret.sequence_num = ntohl(networkval->sequence_num);
    ret.updatedLink.sourceNode = ntohl(networkval->updatedLink.sourceNode);
    ret.updatedLink.destNode = ntohl(networkval->updatedLink.destNode);
    ret.updatedLink.cost = ntohl(networkval->updatedLink.cost);
    ret.updatedLink.valid = ntohl(networkval->updatedLink.valid);

    return ret;
}
