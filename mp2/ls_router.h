
#ifndef LS_ROUTER_H
#define LS_ROUTER_H

#include "router.h"
#include "graph.h"

#define TRUE 1
#define FALSE 0

#define QUEUE_THRESHOLD 50000
#define PACKETS_PER_LSPU 32

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
        LS_Router(int id, char * graphFileName, char * logFileName);

        // Member Functions
        void listenForNeighbors();

    private:
        // Private Member functions
        void updateForwardingTable();
        void checkHeartBeat();
        void periodicLSPL();
        void generateLSPL(int sourceNode, int destNode);
        void forwardLSPL(char * LSPL_Buf, int heardFromNode);

        void sendLSPU(int destNode);
        void updateManager();
        void generateLSPU(int linkSource, int linkDest, int destNode);


        // Functions to convert to an from network order
        LSPL_t hostToNetworkLSPL(LSPL_t * hostval);
        LSPL_t networkToHostLSPL(LSPL_t * networkval);

        // Graph stores the current network topology
        Graph network;

        // sequence numbers for LSP packets
        vector<int> seqNums;

        // Member variables for update manager
        queue<int_pair> updateQueue;
        struct timeval updateQueueTime, lastUpdateQueueTime;
};


#endif
