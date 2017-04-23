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

        // Public Member Functions
        void reliableSend(char * filename, unsigned long long int bytesToTransfer);
        void reliableReceive(char * filename);
    private:
        // Sender Member Functions
        void sendWindow();
        void setupConnection();
        void tearDownConnection();

        // Receiver Memeber Functions
        void receiveWindow();

        // socket communication
        int sockfd;
        struct sockaddr saddr;

        // Circular buffer that contains packets
        CircularBuffer buffer;

        // Book keeping
};


#endif
