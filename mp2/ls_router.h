
#ifndef LS_ROUTER_H
#define LS_ROUTER_H

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

#define GLOBAL_COM_PORT 7777

class LS_Router
{
    public:
        // Constructor and Destructor
        LS_Router(int id, char * filename);
        ~LS_Router();

        // Member Functions
        void announceToNeighbors();
        void listenForNeighbors();
        // TODO: Need thread to check heartbeat threshold, perform Dijkstra if changing from valid to invalid
        // TODO: Need thread to forward heartbeat and cost
        // TODO: Deal with manager's messages

        // testing function


    private:
        static int globalNodeID;

        // our all-purpose UDP socket, to be bound to 10.1.1.globalNodeID, port 7777
        static int socket_fd;

        // last time you heard from each node.
        // TODO: you will want to monitor this in order to realize when a neighbor has gotten cut off from you.
        static struct timeval globalLastHeartbeat[NUM_NODES];

        // pre-filled for sending to 10.1.1.0 - 255, port 7777
        static struct sockaddr_in globalNodeAddrs[NUM_NODES];

        static Graph network;

        //Yes, this is terrible. It's also terrible that, in Linux, a socket
        //can't receive broadcast packets unless it's bound to INADDR_ANY,
        //which we can't do in this assignment.
        static void hackyBroadcast(const char* buf, int length);

        static void * announcer(void * arg);

        // Forwarding table
        static vector<int> forwardingTable;
};


#endif
