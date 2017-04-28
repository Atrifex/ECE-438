#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include "parameters.h"
#include "types.h"

class CircularBuffer
{
    public:
        // Constructor
        CircularBuffer(){}
        CircularBuffer(int size, char * filename, unsigned long long int bytesToSend);
        CircularBuffer(int size, char * filename);
        ~CircularBuffer();

        // sender member function
        void fill();

        // receiver member function
        void storeReceivedPacket(msg_packet_t & packet, uint32_t packetLength);
        void flush();

        void setSocketAddrInfo(int sockfd, struct sockaddr senderAddr, socklen_t senderAddrLen);

        // member variables
        int sIdx;
        unsigned int payload;

        mutex seqNumLock;
        uint32_t seqNum;

        vector<packet_state_t> state;
        vector<struct timeval> timestamp;
        vector<msg_packet_t> data;
        vector<uint32_t> length;


        mutex pktLocks[MAX_WINDOW_SIZE];
        condition_variable senderCV;
        condition_variable fillerCV;

        unsigned long long int bytesToTransfer;
        ifstream sourcefile;
        ofstream destfile;

        // debuging
        unsigned long long timeSinceStart();
        struct timeval start;

        ofstream ffile;
};


#endif
