
#ifndef LS_ROUTER_H
#define LS_ROUTER_H

#include "router.h"
#include "graph.h"

#define TRUE 1
#define FALSE 0

typedef struct
{
    int sourceNode;
    int destNode;
    int cost;
    int valid;
} Link_t;

typedef struct
{
    char header[4] = "lsp";
    int producerNode;
    int sequence_num;
    // struct timespec ttl;
    Link_t updatedLink;
} LSPL_t;

class LS_Router : public Router
{
    public:
        // Constructor and Destructor
        LS_Router(int id, char * graphFileNamem, char * logFileName);

        // Member Functions
        void listenForNeighbors();
        void updateForwardingTable();
        void checkHeartBeat();
        void generateLSPL(int sourceNode, int destNode);
        void forwardLSPL(char * LSPL_Buf, int recvNode);
        void sendLSPU(vector<LSPL_t> & networkState, int destNode);

        // LSP:
        //      Ideas:
        //          - Have a queue
        //          - When network change is detected, enqueue the LSP
        //          - at a fixed time interval dequeue and send
        //          - when sending, see if change in queue matches graph state
        //      TODO:
        //          -

    private:
        // Functions to convert to an from network order
        LSPL_t hostToNetworkLSPL(LSPL_t * hostval);
        LSPL_t networkToHostLSPL(LSPL_t * networkval);

        // Graph stores the current network topology
        Graph network;

        // sequence numbers for LSP packets
        vector<int> seqNums;
};


#endif
