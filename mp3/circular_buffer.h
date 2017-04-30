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
        bool initialFill();
        void fillBuffer();
        bool outsideWindow(uint32_t index);

        // receiver member function
        void storeReceivedPacket(msg_packet_t & packet, uint32_t packetLength);
        void flushBuffer();
        void sendAck(msg_packet_t & packet);

        void setSocketAddrInfo(int sockfd, struct sockaddr senderAddr, socklen_t senderAddrLen);

        // member variables
        mutex idxLock;
        uint32_t sIdx, eIdx, windowSize;
        unsigned int payload;

        // seqNum
        int seqNum;

        // data
        vector<packet_state_t> state;
        vector<struct timeval> timestamp;
        vector<msg_packet_t> data;
        vector<uint32_t> length;
        mutex pktLocks[MAX_WINDOW_SIZE];
        condition_variable senderCV;
        condition_variable fillerCV;
        condition_variable openWinCV;

        // Meta data
        unsigned long long int bytesToTransfer;
        bool fileLoadCompleted;
        int sourcefd;
        int destfd;

        // debuging
        unsigned long long timeSinceStart();
        struct timeval start;

        ofstream ffile;
        ofstream recvfile;
};


#endif
