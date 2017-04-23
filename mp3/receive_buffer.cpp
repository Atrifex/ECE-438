
#include "receive_buffer.h"

ReceiveBuffer::ReceiveBuffer(int size, char * filename)
{
    destfile = std::ofstream(filename, std::ios::out);
    if (!destfile.is_open()) {
        perror("file open error");
        exit(1);
    }

    state.resize(size, waiting);
    data.resize(size);

    seqNum = 0;
    startIdx = 0;
}

void ReceiveBuffer::flush()
{
    size_t j = startIdx;
    for(size_t i = 0; i < data.size(); i++) {
        if(state[j] == received){
            // write to file
            int length = ntohs(data[j].header.length);
            destfile.write(data[j].msg, length);

            // book keeping
            state[j] = waiting;
            startIdx++;
        }
        j = (j+1)%data.size();
    }
}
