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

        // Public Member Functions
        void reliableSend(char * filename, unsigned long long int bytesToTransfer);
        void reliableReceive(char * filename);
    private:
        // Sender Member Functions
        void sendWindow();
        void senderSetupConnection();
        void senderTearDownConnection();
        int receiveSynAck();

        // Receiver Memeber Functions
        bool receivePacket();
        void receiverSetupConnection();
        void receiverTearDownConnection();

        // socket communication
        int sockfd;
        struct sockaddr receiverAddr, senderAddr;          // needed for sendto
        socklen_t receiverAddrLen, senderAddrLen;          // needed for sendto
        struct timeval rtt;

        // Circular buffer that contains packets
        CircularBuffer * buffer;

        // Book keeping
};


#endif
