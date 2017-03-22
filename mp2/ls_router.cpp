
#include "ls_router.h"

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
    if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
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
            sendto(socket_fd, buf, length, 0, 
                (struct sockaddr*)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
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
}

void LS_Router::listenForNeighbors()
{
    char fromAddr[100];
    struct sockaddr_in theirAddr;
    socklen_t theirAddrLen;
    unsigned char recvBuf[1000];

    int bytesRecvd;
    while(1)
    {
        theirAddrLen = sizeof(theirAddr);
        if ((bytesRecvd = recvfrom(socket_fd, recvBuf, 1000 , 0,
                (struct sockaddr*)&theirAddr, &theirAddrLen)) == -1) {
            perror("connectivity listener: recvfrom failed");
            exit(1);
        }

        inet_ntop(AF_INET, &theirAddr.sin_addr, fromAddr, 100);

        short int heardFrom = -1;
        if(strstr(fromAddr, "10.1.1."))
        {
                heardFrom = atoi(strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);

                //TODO: this node can consider heardFrom to be directly connected to it; do any such logic now.

                //record that we heard from heardFrom just now.
                gettimeofday(&globalLastHeartbeat[heardFrom], 0);
        }

        //Is it a packet from the manager? (see mp2 specification for more details)
        //send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
        if(!strncmp((const char*)recvBuf, (const char*)"send", 4))
        {
                //TODO send the requested message to the requested destination node
                // ...
        }
        //'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
        else if(!strncmp((const char*)recvBuf, (const char*)"cost", 4))
        {
                //TODO record the cost change (remember, the link might currently be down! in that case,
                //this is the new cost you should treat it as having once it comes back up.)
                // ...
        }

        //TODO now check for the various types of packets you use in your own protocol
        //else if(!strncmp(recvBuf, "your other message types", ))
        // ...
    }
}
