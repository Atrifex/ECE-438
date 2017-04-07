
#ifndef LS_ROUTER_H
#define LS_ROUTER_H

#include "router.h"
#include "graph.h"

#include <ctime>

using std::time;
using std::srand;
using std::rand;

#define NS_PER_SEC 1000000000
#define STAGGER_TIME 19531250

#pragma pack(1)
typedef struct {
    int weight;
    unsigned char neighbor;
} link_t;

#pragma pack(1)
typedef struct {
    char header[4] = "lsp";
    // producerNode == source node for links
    int sequenceNum;
    unsigned char producerNode;
    unsigned char numLinks;
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
        void generateLSP();

    private:
        // Private Member functions
        void updateForwardingTable();
        void checkHeartBeat();

        // Functions to handle LSP
        void sendLSChanges();
        void sendFullLSP();
        void createLSP(lsp_t & lsp, vector<int> & neighbors);
        void forwardLSP(char * LSP_Buf, int bytesRecvd, int heardFromNode);
        bool processLSP(lsp_t * lspNetwork);

        // debug functions
        void lspLogger(int seqNum, int from, int to, int weight);

        // Graph stores the current network topology
        Graph * network;

        // sequence numbers for LSP packets
        vector<int> seqNums;
        queue<link_t> LSPQueue;

        // debug vars
        FILE * lspFileptr;

        bool changed;
        mutex changedLock;
};


#endif
