
#include "circular_buffer.h"


/*************** Send Buffer ***************/
CircularBuffer::CircularBuffer(int size, char * filename, unsigned long long int bytesToSend)
{
    sourcefile.open(filename, std::ios::in);
    if (!(sourcefile.is_open())) {
        std::cerr << "Unable to open source file\n";
    }

    sourcefile.seekg (0, sourcefile.end);
    int fileLength = sourcefile.tellg();
    sourcefile.seekg (0, sourcefile.beg);

    state.resize(size, available);
    data.resize(size);
    length.resize(size);

    seqNum = 0;
    sIdx = 0;
    bytesToTransfer = min((unsigned long long)fileLength, bytesToSend);
}

bool CircularBuffer::fill()
{
    size_t j = sIdx;
    for(size_t i = 0; i < data.size(); i++) {
        if(bytesToTransfer <= 0)
            return false;

        if(state[j] == available){
            int packetLength = min((unsigned long long)PAYLOAD, bytesToTransfer + sizeof(msg_header_t));

#ifdef DEBUG
            printf("Packet Length: %lu, SeqNum: %d\n" , packetLength - sizeof(msg_header_t), seqNum);
#endif
            // initialize header
            data[j].header.type = DATA;
            data[j].header.seqNum = htonl(seqNum++);
            length[j] = packetLength;

            // read data into buffer
            sourcefile.read(data[j].msg, packetLength - sizeof(msg_header_t));

            // book keeping
            state[j] = filled;
            bytesToTransfer -= (packetLength - sizeof(msg_header_t)) ;
        }
        j = (j+1)%data.size();
    }

    // launch thread to fill large buffer that isnt the cirular buffer

    return true;
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
    length.resize(size);

    seqNum = 0;
    sIdx = 0;
}

void CircularBuffer::flush()
{
    for(size_t i = 0; i < data.size(); i++) {
        if(state[sIdx] == received){

#ifdef DEBUG
            printf("sIdx: %d\n" , sIdx);
            printf("Packet Length: %u, SeqNum: %u\n" , length[sIdx], seqNum);
#endif
            // write to file
            destfile.write(data[sIdx].msg, length[sIdx]);
            destfile.flush();

            // book keeping
            state[sIdx] = waiting;
            seqNum++;
            sIdx = (sIdx+1)%data.size();
        } else{
            return;
        }
    }
}

void CircularBuffer::storeReceivedPacket(msg_packet_t & packet, uint32_t packetLength)
{
    packet.header.seqNum = ntohl(packet.header.seqNum);

    // drop packet if the seqNum is smaller than expected.
    if(packet.header.seqNum < seqNum) return;

    if(state[(packet.header.seqNum % data.size())] == waiting){
        state[(packet.header.seqNum % data.size())] = received;
        data[(packet.header.seqNum % data.size())] = packet;
        length[(packet.header.seqNum % data.size())] = packetLength - sizeof(msg_header_t);
#ifdef DEBUG
        printf("Index Store: %lu, Received length: %lu\n", (packet.header.seqNum % data.size()), packetLength - sizeof(msg_header_t));
#endif
    }

    // TODO: launch thread to flush file.
}
