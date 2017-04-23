
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
    for(size_t i = 0; i < data.size(); i++) {
        if(state[i] == available && bytesToTransfer > 0){
            // initialize header
            data[i].header.seqNum = seqNum++;
            data[i].header.length = min((unsigned long long)PAYLOAD, bytesToTransfer);

            sourcefile.read(data[i].msg, data[i].header.length);

            // book keeping
            state[i] = filled;
            bytesToTransfer -= data[i].header.length;
        }
    }
}
