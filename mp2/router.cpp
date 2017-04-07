
#include "router.h"

Router::Router(int id, char * logFileName)
{
    char myAddr[100];
    struct sockaddr_in bindAddr;

    myNodeID = id;

    // initialize the forwarding table
    forwardingTable.resize(NUM_NODES, INVALID);

    // initialize file pointer
    if((logFilePtr = fopen(logFileName, "w")) == NULL){
        perror("fopen");
        close(sockfd);
        exit(1);
    }

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
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    // setup address for binding purposes
    sprintf(myAddr, "10.1.1.%d", myNodeID);         // prints ip address to my addr
    memset(&bindAddr, 0, sizeof(bindAddr));         // clears bindaddr
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(GLOBAL_COM_PORT);     // port for communication
    inet_pton(AF_INET, myAddr, &bindAddr.sin_addr); // writing addr to sin_addr

    // bind for listining
    if(bind(sockfd, (struct sockaddr*)&bindAddr, sizeof(struct sockaddr_in)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

}

Router::~Router()
{
    fclose(logFilePtr);
    close(sockfd);
}


void Router::hackyBroadcast(const char* buf, int length)
{
    int i;
    for(i=0;i<NUM_NODES;i++){
        if(i != myNodeID){
            sendto(sockfd, buf, length, 0, (struct sockaddr*)&globalNodeAddrs[i],
                sizeof(globalNodeAddrs[i]));
        }
    }
}

void Router::announceToNeighbors()
{
    struct timespec sleepFor;
    sleepFor.tv_sec = 0;
    sleepFor.tv_nsec = 300 * 1000 * 1000;   //300 ms
    while(1)
    {
        hackyBroadcast("HEREIAM", 7);
        nanosleep(&sleepFor, 0);
    }
}


/*
 * int logToFile()
 * Logs a message to an open file specified by file pointer fp.
 * The message can be one of four types, as defined in ls_router.h
 * Return value: 0 on success, -1 on failure
 */
int Router::logToFile(int log_type, short int dest, short int nexthop, char* message)
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
            sprintf(logLine, "sending packet dest %d nexthop %d message %s\n",
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

#ifdef LOG
    cout << logLine;
#endif

    // Write to logFile
    if(fwrite(logLine, 1, strlen(logLine), logFilePtr) != strlen(logLine))
    {
            fprintf(stderr, "logToFile: Error in fwrite(): Not all data written\n");
            return -1;
    }

    fflush(logFilePtr);

    return 0;
}
