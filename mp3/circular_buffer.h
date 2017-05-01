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
        void initialFill();
        void fillBuffer();
        bool outsideWindow(uint32_t index);

        // receiver member function
        void storeReceivedPacket(msg_packet_t & packet, uint32_t packetLength);
        void flushBuffer();
        void sendAck();
        uint64_t createFlags(uint32_t & counter);

        void setSocketAddrInfo(int sockfd, struct sockaddr senderAddr, socklen_t senderAddrLen);

        // member variables
        condition_variable openWinCV;
        mutex windowLock;
        uint32_t sIdx, eIdx, windowSize;
        unsigned int payload;

        // seqNum
        int seqNum;

        // data
        vector<packet_state_t> state;
        vector<struct timeval> timestamp;
        vector<msg_packet_t> data;
        vector<uint32_t> length;

        mutex pktLocks[BUFFER_SIZE];
        condition_variable senderCV;
        condition_variable fillerCV;

        // Meta data
        unsigned long long int bytesToTransfer;
        bool fileLoadCompleted;
        int sourcefd;
        int destfd;

        // debuging
        unsigned long long timeSinceStart();
        struct timeval start;
};


#endif
