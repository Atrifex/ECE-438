#ifndef TCP_H
#define TCP_H

#include "parameters.h"
#include "types.h"

class TCP
{
    public:
        // Constructors
        TCP(char * hostname, char * hostUDPport);

        // Public Member Functions
        void send(char* srcfile, unsigned long long int bytesToTransfer);
        void receive(char* destfile);

    private:
        int sockfd;
        struct sockaddr saddr;

};


#endif
