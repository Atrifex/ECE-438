
#ifndef LS_ROUTER_H
#define LS_ROUTER_H

#include "router.h"
#include "graph.h"

#define TRUE 1
#define FALSE 0

#define QUEUE_THRESHOLD 300000
#define LSPU_PER_EPOCH 64
#define LSP_PER_EPOCH 10

typedef struct {
    int neighbor;
    int weight;
    int status;
} link_t;

typedef struct {
    char header[4] = "lsp";
    // producerNode == source node for links
    int producerNode;
    int sequenceNum;
    int numLinks;
    link_t links[NUM_NODES];
} lsp_t;

class LS_Router : public Router
{
    public:
        // Constructor and Destructor
        LS_Router(int id, char * graphFileName, char * logFileName);
        ~LS_Router();

        // Member Functions
        void listenForNeighbors();
        void announceToNeighbors();

    private:
        // Private Member functions
        void updateForwardingTable();
        void checkHeartBeat();

        // Functions to handle LSP
        void sendLSP();
        void createLSP(lsp_t & lsp, vector<int> & neighbors);
        void forwardLSP(char * LSP_Buf, int bytesRecvd, int heardFromNode);
        void processLSP(lsp_t * lspNetwork);

        // debug functions
        void lspLogger(int from, int to, bool status, int weight);

        // Graph stores the current network topology
        Graph * network;

        // sequence numbers for LSP packets
        vector<int> seqNums;
        queue<link_t> LSPQueue;

        // debug vars
        FILE * lspFileptr;
};


#endif
