#include "ls_router.h"

// Instances of static members
int LS_Router::globalNodeID;
int LS_Router::socket_fd;
FILE * LS_Router::logFilePtr;
struct timeval LS_Router::globalLastHeartbeat[NUM_NODES];
struct sockaddr_in LS_Router::globalNodeAddrs[NUM_NODES];
Graph LS_Router::network;
vector<int> LS_Router::forwardingTable;

LS_Router::LS_Router(int id, char * graphFileName, char * logFileName)
{
    char myAddr[100];
    struct sockaddr_in bindAddr;

    // initialize graph
    Graph temp(id, graphFileName);
    network = temp;

    globalNodeID = id;

    //initialization: get this process's node ID, record what time it is,
    //and set up our sockaddr_in's for sending to the other nodes.
    for(int i=0;i<256;i++)
    {
        gettimeofday(&globalLastHeartbeat[i], 0);

        char tempaddr[100];
        sprintf(tempaddr, "10.1.1.%d", i);
        memset(&globalNodeAddrs[i], 0, sizeof(globalNodeAddrs[i]));
        globalNodeAddrs[i].sin_family = AF_INET;
        globalNodeAddrs[i].sin_port = htons(GLOBAL_COM_PORT);
        inet_pton(AF_INET, tempaddr, &globalNodeAddrs[i].sin_addr);
    }

    //socket() and bind() our socket. We will do all sendto()ing and recvfrom()ing on this one.
    if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    sprintf(myAddr, "10.1.1.%d", globalNodeID);     // prints ip address to my addr
    memset(&bindAddr, 0, sizeof(bindAddr));         // clears bindaddr
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(GLOBAL_COM_PORT);                // port for communication
    inet_pton(AF_INET, myAddr, &bindAddr.sin_addr); // writing addr to sin_addr

    // need to bind so we can listen through this socket
    if(bind(socket_fd, (struct sockaddr*)&bindAddr, sizeof(struct sockaddr_in)) < 0) {
        perror("bind");
        close(socket_fd);
        exit(1);
    }

    // initialize the forwarding table
    forwardingTable.resize(NUM_NODES, INVALID);

    // initialize file pointer
    if((logFilePtr = fopen(logFileName, "w")) == NULL){
        perror("fopen");
        close(socket_fd);
        exit(1);
    }
}

LS_Router::~LS_Router()
{
    close(socket_fd);
    fclose(logFilePtr);
}


void LS_Router::hackyBroadcast(const char* buf, int length)
{
    int i;
    for(i=0;i<NUM_NODES;i++){
        if(i != globalNodeID){
            sendto(socket_fd, buf, length, 0, (struct sockaddr*)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
        }
    }
}

void LS_Router::announceToNeighbors()
{
    pthread_t announcerThread;
    pthread_create(&announcerThread, 0, announcer, (void*)0);
}

void * LS_Router::announcer(void * arg)
{
    struct timespec sleepFor;
    sleepFor.tv_sec = 0;
    sleepFor.tv_nsec = 300 * 1000 * 1000;   //300 ms
    while(1)
    {
        hackyBroadcast("HEREIAM", 7);
        nanosleep(&sleepFor, 0);
    }

    return arg;
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
                network.updateLink(false, globalNodeID, i);
                network.updateLink(false, i, globalNodeID);
                updateForwardingTable();
                
                // TODO: LSP Packet
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
    while(1)
    {
        memset(recvBuf, 0, 1000);
        senderAddrLen = sizeof(senderAddr);
        if ((bytesRecvd = recvfrom(socket_fd, recvBuf, 1000 , 0,
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
            if(network.getLinkCost(globalNodeID, heardFromNode) == INVALID){
                network.updateLink(true, globalNodeID, heardFromNode);
                network.updateLink(true, heardFromNode, globalNodeID);
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

                sendto(socket_fd, recvBuf, SEND_SIZE, 0, 
                        (struct sockaddr*)&globalNodeAddrs[nextNode], 
                        sizeof(globalNodeAddrs[nextNode]));
                recvBuf[106] = '\0'; 
                logToFile(SEND_MES, destID, nextNode, (char *)recvBuf + 6);
            } else{
                logToFile(UNREACHABLE_MES, destID, nextNode, (char *)recvBuf + 6);
            }
                
        } else if(strncmp((const char*)recvBuf, (const char*)"forw", 4) == 0) {
            destID = ntohs(((short int*)recvBuf)[2]);
            
            if(destID == globalNodeID){
                recvBuf[106] = '\0'; 
                logToFile(RECV_MES, destID, nextNode, (char *)recvBuf + 6);
                continue;
            }

            if((nextNode = forwardingTable[destID]) != INVALID) {
                sendto(socket_fd, recvBuf, SEND_SIZE, 0, 
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
            network.updateLink(ntohs(*((int*)&(((char*)recvBuf)[6]))), globalNodeID, destID);

            if(network.getLinkCost(globalNodeID, heardFromNode) != INVALID){
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
        while(predecessor[nextNode] != globalNodeID)
            nextNode = predecessor[nextNode];

        forwardingTable[i] = nextNode;
    }

    return;
}

/*
 * int logToFile()
 * Logs a message to an open file specified by file pointer fp.
 * The message can be one of four types, as defined in ls_router.h
 * Return value: 0 on success, -1 on failure
 */
int LS_Router::logToFile(int log_type, short int dest, short int nexthop, char* message)
{
    char logLine[256]; // Message is <= 100 bytes, so this is always enough

    // Determine the logLine to write based on the message type
    switch(log_type)
    {
        case FORWARD_MES:
            sprintf(logLine, "forward packet dest %d nexthop %d message %s\n",
                                                        dest, nexthop, message);
            break;
        case SEND_MES:
            sprintf(logLine, "send packet dest %d nexthop %d message %s\n", 
                                                        dest, nexthop, message);
            break;
        case RECV_MES:
            sprintf(logLine, "receive packet message %s\n", message);
            break;
        case UNREACHABLE_MES:
            sprintf(logLine, "unreachable dest %d\n", dest);
            break;
        default: // Should never happen
            perror("logToFile: invalid log_type\n");
            return -1;
    }

    // Write to logFile
    if(fwrite(logLine, 1, strlen(logLine), logFilePtr) != strlen(logLine))
    {
            fprintf(stderr, "logToFile: Error in fwrite(): Not all data written\n");
            return -1;
    }

    fflush(logFilePtr);

    return 0;
}


