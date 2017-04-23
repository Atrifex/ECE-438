#ifndef SEND_BUFFER_H
#define SEND_BUFFER_H

#include "parameters.h"
#include "types.h"

class CircularBuffer
{
    public:
        // Constructor
        CircularBuffer(){}
        CircularBuffer(int size, char * filename, unsigned long long int bytesToSend);
        CircularBuffer(int size, char * filename);

        // send and receive buffer member function
        void fill();
        void flush();

        int startIdx;
        int seqNum;
        vector<state_t> state;
        vector<msg_packet_t> data;
        unsigned long long int bytesToTransfer;

    private:
        ifstream sourcefile;
        ofstream destfile;
};


#endif
