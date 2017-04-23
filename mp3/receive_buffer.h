#ifndef RECEIVE_BUFFER_H
#define RECEIVE_BUFFER_H

#include "parameters.h"
#include "types.h"

class ReceiveBuffer
{
    public:
        // Constructor
        ReceiveBuffer(){}
        ReceiveBuffer(int size, char * filename);

        // Public member function
        void flush();

        int startIdx;
        int seqNum;
        vector<receiveState_t> state;
        vector<msg_packet_t> data;
    private:
        ofstream destfile;
};


#endif
