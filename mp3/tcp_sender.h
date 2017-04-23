#ifndef TCP_SENDER_H
#define TCP_SENDER_H

#include "parameters.h"
#include "types.h"
#include "send_buffer.h"

class TCPSender
{
    public:
        // Constructors
        TCPSender(char * hostname, char * hostUDPport);

        // Public Member Functions
        void reliableSend(char * filename, unsigned long long int bytesToTransfer);
    private:
        // Private Member Functions
        void sendWindow();
        void setupConnection();
        void tearDownConnection();

        // socket communication
        int sockfd;
        struct sockaddr saddr;

        // Circular buffer that contains packets
        SendBuffer buffer;

        // Book keeping
};


#endif
