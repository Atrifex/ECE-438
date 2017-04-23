#ifndef TCP_RECEIVER_H
#define TCP_RECEIVER_H

#include "parameters.h"
#include "types.h"
#include "receive_buffer.h"

class TCPReceiver
{
    public:
        // Constructors
        TCPReceiver(char * hostname, char * hostUDPport);

        // Public Member Functions
        void reliableReceive(char * filename);
        void receiveWindow();
    private:
        // socket communication
        int sockfd;
        struct sockaddr saddr;

        // Circular buffer that contains packets
        ReceiveBuffer buffer;

        // Book keeping
};


#endif
