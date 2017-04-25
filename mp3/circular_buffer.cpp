
#include "circular_buffer.h"

// Socket information for sending ACKS
int ackfd;
struct sockaddr ackAddr;
socklen_t ackAddrLen;

CircularBuffer::~CircularBuffer() {
    if (sourcefile.is_open()) {
        sourcefile.close();
    }
    if (destfile.is_open()) {
        destfile.close();
    }
}

void CircularBuffer::setSocketAddrInfo(int sockfd, struct sockaddr senderAddr, socklen_t senderAddrLen)
{
    ackfd = sockfd;
    ackAddr = senderAddr;
    ackAddrLen = senderAddrLen;
}


/*************** Send Buffer ***************/
CircularBuffer::CircularBuffer(int size, char * filename, unsigned long long int bytesToSend)
{
    sourcefile.open(filename, std::ios::in);
    if (!(sourcefile.is_open())) {
        std::cerr << "Unable to open source file\n";
        exit(1);
    }

    sourcefile.seekg (0, sourcefile.end);
    int fileLength = sourcefile.tellg();
    sourcefile.seekg (0, sourcefile.beg);

    state.resize(size, AVAILABLE);
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

        if(state[j] == AVAILABLE){
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
            state[j] = FILLED;
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

    state.resize(size, WAITING);
    data.resize(size);
    length.resize(size);

    seqNum = 0;
    sIdx = 0;
}

void sendAck(int seqNum){
    // Set up ack packet
    ack_packet_t ack;
    ack.type = ACK_HEADER;
    ack.seqNum = htonl(seqNum);

    // send ack
    sendto(ackfd, (char *)&ack, sizeof(ack_packet_t), 0, &ackAddr, ackAddrLen);
}

void CircularBuffer::flush()
{
    for(size_t i = 0; i < data.size(); i++) {
        pktLocks[sIdx].lock();
        if(state[sIdx] == RECEIVED){

            thread acker(sendAck, data[sIdx].header.seqNum);
            acker.detach();

#ifdef DEBUG
            printf("sIdx: %d\n" , sIdx);
            printf("Packet Length: %u, SeqNum: %u\n" , length[sIdx], seqNum);
#endif
            // write to file
            destfile.write(data[sIdx].msg, length[sIdx]);
            destfile.flush();

            // book keeping
            state[sIdx] = WAITING;
            seqNumLock.lock();
            seqNum++;
            seqNumLock.unlock();
            sIdx = (sIdx+1)%data.size();
        } else{
            pktLocks[sIdx].unlock();
            return;
        }
        pktLocks[sIdx].unlock();
    }
}

void bufferFlusher(CircularBuffer & buffer) {
    buffer.flush();
}

void CircularBuffer::storeReceivedPacket(msg_packet_t & packet, uint32_t packetLength)
{
    packet.header.seqNum = ntohl(packet.header.seqNum);

    // drop packet if the seqNum is smaller than expected.
    seqNumLock.lock();
    if(packet.header.seqNum < seqNum){
        seqNumLock.unlock();
        return;
    }
    seqNumLock.unlock();

    size_t bufIdx = packet.header.seqNum % data.size();
    pktLocks[bufIdx].lock();
    if(state[bufIdx] == WAITING){
        state[bufIdx] = RECEIVED;
        data[bufIdx] = packet;
        length[bufIdx] = packetLength - sizeof(msg_header_t);

#ifdef DEBUG
        printf("\nReceived seqNum: %lu\n", packet.header.seqNum);
        printf("Index Store: %lu, RECEIVED length: %lu\n", bufIdx, packetLength - sizeof(msg_header_t));
#endif

    }
    pktLocks[bufIdx].unlock();

    thread flusher(bufferFlusher, ref(*this));
    flusher.detach();
}
