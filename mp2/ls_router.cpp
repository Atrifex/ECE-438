
#include "ls_router.h"

// Instances of static members
int LS_Router::globalNodeID;
int LS_Router::socket_fd;
struct timeval LS_Router::globalLastHeartbeat[NUM_NODES];
struct sockaddr_in LS_Router::globalNodeAddrs[NUM_NODES];
Graph LS_Router::network;
vector<int> LS_Router::forwardingTable;

LS_Router::LS_Router(int id, char * filename)
{
    char myAddr[100];
    struct sockaddr_in bindAddr;

    // initialize graph
    Graph temp(id, filename);
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
    if(bind(socket_fd, (struct sockaddr*)&bindAddr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        close(socket_fd);
        exit(1);
    }

    forwardingTable.resize(NUM_NODES, INVALID);
}

LS_Router::~LS_Router()
{
    close(socket_fd);
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

void LS_Router::listenForNeighbors()
{
    char fromAddr[100];
    struct sockaddr_in senderAddr;
    socklen_t senderAddrLen;
    unsigned char recvBuf[1000];

    int bytesRecvd;
    while(1)
    {
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
                forwardingTable[heardFromNode] = network.dijkstraGetNextNode(heardFromNode);
            }
            
            //record that we heard from heardFromNode just now.
            gettimeofday(&globalLastHeartbeat[heardFromNode], 0);
        }

        short int destID, nextNode;
        // send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
        if(strncmp((const char*)recvBuf, (const char*)"send", 4) == 0) {
            destID = ntohs(((short int*)recvBuf)[2]);

            if((nextNode = forwardingTable[destID]) != INVALID) {
                sendto(socket_fd, recvBuf, SEND_SIZE, 0, 
                        (struct sockaddr*)&globalNodeAddrs[nextNode], 
                        sizeof(globalNodeAddrs[nextNode]));
                // TODO: Log send
            } else{
                // TODO: Log unreachable
            }
                

        } else if(strncmp((const char*)recvBuf, (const char*)"cost", 4) == 0){
            //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
            //TODO record the cost change (remember, the link might currently be down! in that case,
            //this is the new cost you should treat it as having once it comes back up.)
            // ...
            destID = ntohs(((short int*)recvBuf)[2]);
            network.updateLink(ntohs(*((int*)&(((char*)recvBuf)[6]))), globalNodeID, destID);

            // TODO: perform Dijkstra


        }

        //TODO now check for the various types of packets you use in your own protocol
        //else if(!strncmp(recvBuf, "your other message types", ))
        // ...
    }
}

/*
 * int logToFile()
 * Logs a message to an open file specified by file pointer fp.
 * The message can be one of four types, as defined in ls_router.h
 * Return value: 0 on success, -1 on failure
 */
int LS_Router::logToFile(FILE* fp, int log_type, int dest, int nexthop, char* message)
{
	// Argument check
	if(fp == NULL)
	{
		perror("logToFile: invalid file pointer\n");
		return -1;
	}

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
	if(fwrite(logLine, 1, strlen(logLine), fp) != strlen(logLine))
	{
		fprintf(stderr, "logToFile: Error in fwrite(): Not all data written\n");
		return -1;
	}

	return 0;
}


