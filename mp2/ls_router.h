
#ifndef LS_ROUTER_H
#define LS_ROUTER_H

#include "router.h"
#include "graph.h"

#include <ctime>
#include <set>

using std::time;
using std::srand;
using std::rand;
using std::set;

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

#pragma pack(1)
typedef struct {
    int weight;
    unsigned char status;
    unsigned char neighbor;
} linkChange_t;

#pragma pack(1)
typedef struct {
    char header[4] = "lsc";
    int sequenceNum;
    unsigned char producerNode;
    unsigned char numLinks;
    linkChange_t links[NUM_NODES];
} lsc_t;


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
        void sendLSC();
        void createLSC(lsc_t & lsc);
        void forwardLSC(char * LSC_Buf, int heardFromNode);
        bool processLSC(lsc_t * lscNetwork);

        void sendLSP();
        void createLSP(lsp_t & lsp, vector<int> & neighbors);
        void forwardLSP(char * LSP_Buf, int bytesRecvd, int heardFromNode);
        bool processLSP(lsp_t * lspNetwork);

        // debug functions
        void lspLogger(int seqNum, int from, int to, int weight, int mode);

        // Graph stores the current network topology
        Graph * network;

        // sequence numbers for LSP packets
        vector<int> seqNums;
        vector<int> changeSeqNums;

        // Neighbors that I need to broadcast about
        set<int> changeSet;

        // debug vars
        FILE * lspFileptr;

        bool changed;
        mutex changedLock;
};


#endif
