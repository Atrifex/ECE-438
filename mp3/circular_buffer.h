#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include "parameters.h"
#include "types.h"

class CircularBuffer
{
    public:
        // Constructor
        CircularBuffer(int size);

        void fill();
        void flush();
    private:
        // Current start index in buffer
        int startIdx;

        vector<bool> sent;
        vector<bool> received;
        vector<packet_t> buffer;
};


#endif
