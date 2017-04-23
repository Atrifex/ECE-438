#ifndef SEND_BUFFER_H
#define SEND_BUFFER_H

#include "parameters.h"
#include "types.h"

class SendBuffer
{
    public:
        // Constructor
        SendBuffer(){}
        SendBuffer(int size, char * filename, unsigned long long int bytesToSend);

        // Public member function
        void fill();

        int startIdx;
        int seqNum;
        vector<sendState_t> state;
        vector<msg_packet_t> data;
        unsigned long long int bytesToTransfer;

    private:
        ifstream sourcefile;
};


#endif
