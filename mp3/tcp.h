#ifndef TCP_H
#define TCP_H

#include "parameters.h"
#include "types.h"
#include "circular_buffer.h"

class TCP
{
    public:
        // Sender Constructor
        TCP(char * hostname, char * hostUDPport);
        // Receiver Constructor
        TCP(char * hostUDPport);
        ~TCP();

        // Public Sender Member Functions
        void reliableSend(char * filename, unsigned long long int bytesToTransfer);
        void sendWindow();
        void processAcks();

        // Public Receiver Member Functions
        void reliableReceive(char * filename);
    private:
        // Private Sender Member Functions
        void senderSetupConnection();
        void senderTearDownConnection();
        void manager();

        // Private Receiver Memeber Functions
        bool receivePacket();
        void receiverSetupConnection();
        void receiverTearDownConnection();

        // Private Startup Handshake functions
        int receiveStartSyn();
        int receiveStartSynAck();
        void receiveStartAck(msg_header_t syn_ack);

        // Private Teardown Handshake functions
        int receiveEndFinAck();
        void receiveEndAck(msg_header_t fin_ack);

        // RTT function
        unsigned long long calcRTO();
        unsigned long long calcSRRT();
        unsigned long long stdDev();

        // socket communication
        int sockfd;
        struct sockaddr receiverAddr, senderAddr;          // needed for sendto
        socklen_t receiverAddrLen, senderAddrLen;          // needed for sendto

        // Circular buffer that contains packets
        CircularBuffer * buffer;

        // Round trip time and Retransmit time out
        struct timeval rtt, srtt, rto;
        list<unsigned long long> history;

        // Ack processing
        mutex ackQLock;
        condition_variable ackCV;
        queue<ack_process_t> ackQ;

        // Book keeping
        tcp_state_t state;
};


#endif
