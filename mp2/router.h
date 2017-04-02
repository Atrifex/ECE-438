
#ifndef ROUTER_H
#define ROUTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "graph.h"

// Log message types
#define FORWARD_MES 0
#define SEND_MES 1
#define RECV_MES 2
#define UNREACHABLE_MES 3

#define GLOBAL_COM_PORT 7777
#define HEARTBEAT_THRESHOLD 700000

class Router
{
    public:
        // Constructor and Destructor
        Router();
        Router(int id, char * logFileName);
        ~Router();

        // Member Functions
        void announceToNeighbors();

        // all three of these will perform different tasks based on LS vs DV routing
        virtual void listenForNeighbors() = 0;
        virtual void updateForwardingTable() = 0;
        virtual void checkHeartBeat() = 0;

        // logging function
        int logToFile(int log_type, short int dest, short int nexthop, char * message);

    protected:
        // broadcasts to all direct neighbors
        void hackyBroadcast(const char* buf, int length);

        // Contains time of last message from a given node
        struct timeval globalLastHeartbeat[NUM_NODES];

        // sockaddr for sending on port 7777 to 10.1.1.0 - 255
        struct sockaddr_in globalNodeAddrs[NUM_NODES];

        // Global Node ID
        int myNodeID;

        // our all-purpose UDP socket, to be bound to 10.1.1.globalNodeID, port 7777
        int sockfd;

        // pointer to file for log, opened in the constructor
        FILE * logFilePtr;

        // forwarding table
        vector<int> forwardingTable;
};


#endif
