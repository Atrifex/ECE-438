#ifndef TCP_H
#define TCP_H

#include "parameters.h"
#include "types.h"

class TCP
{
    public:
        // Constructors
        TCP(char* hostname, unsigned short int hostUDPport);

        // Public Member Functions
        void send(char* filename, unsigned long long int bytesToTransfer);
        void receive();


    private:
        int sockfd;


};


#endif
