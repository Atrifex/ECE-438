#ifndef TCP_H
#define TCP_H

#include "parameters.h"
#include "types.h"
#include "send_buffer.h"

class TCPReceiver
{
    public:
        // Constructors
        TCPSender(char * hostname, char * hostUDPport);

        // Public Member Functions
        void reliableSend(char * filename, unsigned long long int bytesToTransfer);
        void sendWindow();
    private:
        // socket communication
        int sockfd;
        struct sockaddr saddr;

        // Circular buffer that contains packets
        SendBuffer buffer;

        // Book keeping
};


#endif
