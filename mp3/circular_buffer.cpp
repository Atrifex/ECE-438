
#include "circular_buffer.h"


/*************** Send Buffer ***************/
CircularBuffer::CircularBuffer(int size, char * filename, unsigned long long int bytesToSend)
{
    sourcefile.open(filename, std::ios::in);
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

void CircularBuffer::fill()
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

/*************** Receive Buffer ***************/
CircularBuffer::CircularBuffer(int size, char * filename)
{
    destfile.open(filename, std::ios::out);
    if (!destfile.is_open()) {
        perror("file open error");
        exit(1);
    }

    state.resize(size, waiting);
    data.resize(size);

    seqNum = 0;
    startIdx = 0;
}

void CircularBuffer::flush()
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
        } else{
            return;
        }
        j = (j+1)%data.size();
    }
}
