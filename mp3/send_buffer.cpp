
#include "send_buffer.h"

SendBuffer::SendBuffer(int size, char * filename, unsigned long long int bytesToSend)
{
    sourcefile = std::ifstream(filename, std::ios::in);
    if (!(sourcefile.is_open())) {
        std::cerr << "Unable to open source file\n";
    }

    sourcefile.seekg (0, sourcefile.end);
    int length = sourcefile.tellg();
    sourcefile.seekg (0, sourcefile.beg);

    state.resize(size, available);
    data.resize(size);

    seqNum = 0;
    startIdx = 0;
    bytesToTransfer = min((unsigned long long)length, bytesToSend);
}

void SendBuffer::fill()
{
    size_t j = startIdx;
    for(size_t i = 0; i < data.size(); i++) {
        if(state[j] == available && bytesToTransfer > 0){
            int length = min((unsigned long long)PAYLOAD, bytesToTransfer);

            // initialize header
            data[j].header.seqNum = htonl(seqNum++);
            data[j].header.length = htons(length);

            sourcefile.read(data[j].msg, length);

            // book keeping
            state[j] = filled;
            bytesToTransfer -= length;
        }
        j = (j+1)%data.size();
    }
}
