
#include "ls_router.h"

LS_Router::LS_Router(int id, char * graphFileName, char * logFileName) : Router(id,logFileName)
{
    Graph temp(id, graphFileName);
    network = temp;
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

            // this node can consider heardFromNode to be directly connected to it; do any such logic now.
            if(network.getLinkCost(myNodeID, heardFromNode) == INVALID){
                network.updateLink(true, myNodeID, heardFromNode);
                network.updateLink(true, heardFromNode, myNodeID);
                updateForwardingTable();
            }

            //record that we heard from heardFromNode just now.
            gettimeofday(&globalLastHeartbeat[heardFromNode], 0);
        }

        short int destID, nextNode;
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

            if(destID == myNodeID){
                recvBuf[106] = '\0';
                logToFile(RECV_MES, destID, 0, (char *)recvBuf + 6);
                continue;
            }

            if((nextNode = forwardingTable[destID]) != INVALID) {
                sendto(sockfd, recvBuf, SEND_SIZE, 0,
                        (struct sockaddr*)&globalNodeAddrs[nextNode],
                        sizeof(globalNodeAddrs[nextNode]));
                recvBuf[106] = '\0';
                logToFile(FORWARD_MES, destID, nextNode, (char *)recvBuf + 6);
            } else{
                // TODO: Think about convergence, ask TA/Prof
                logToFile(UNREACHABLE_MES, destID, nextNode, (char *)recvBuf + 6);
            }

        } else if(strncmp((const char*)recvBuf, (const char*)"cost", 4) == 0){
            //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
            destID = ntohs(((short int*)recvBuf)[2]);
            network.updateLink(ntohs(*((int*)&(((char*)recvBuf)[6]))), myNodeID, destID);

            if(network.getLinkCost(myNodeID, heardFromNode) != INVALID){
                updateForwardingTable();
            }

        }

        checkHeartBeat();

        //TODO: LSP packets
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

void LS_Router::checkHeartBeat()
{
    struct timeval tv, tv_heart;
    gettimeofday(&tv, 0);

    long int cur_time = (tv.tv_sec - tv.tv_sec)*1000000 + tv.tv_usec - tv.tv_usec;
    long int lastHeartbeat_usec;
    for(int i = 0; i < NUM_NODES; i++)
    {
        if(forwardingTable[i] != INVALID)
        {
            tv_heart = globalLastHeartbeat[i];
            lastHeartbeat_usec = (tv_heart.tv_sec - tv_heart.tv_sec)*1000000 + tv_heart.tv_usec - tv_heart.tv_usec;

            if(cur_time - lastHeartbeat_usec > HEARTBEAT_THRESHOLD) // Link has died
            {
                network.updateLink(false, myNodeID, i);
                network.updateLink(false, i, myNodeID);
                updateForwardingTable();
                // TODO: LSP Packet
            }
        }
    }
}