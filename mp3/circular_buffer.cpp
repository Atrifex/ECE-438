
#include "circular_buffer.h"

// Socket information for sending ACKS
int ackfd;
struct sockaddr ackAddr;
socklen_t ackAddrLen;

CircularBuffer::~CircularBuffer() {
    if (sourcefd > 0) {
        close(sourcefd);
    }
    if (destfd > 0) {
        close(destfd);
    }
}

void CircularBuffer::setSocketAddrInfo(int sockfd, struct sockaddr senderAddr, socklen_t senderAddrLen)
{
    ackfd = sockfd;
    ackAddr = senderAddr;
    ackAddrLen = senderAddrLen;
}

unsigned long long CircularBuffer::timeSinceStart()
{
    struct timeval curTime;
    gettimeofday(&curTime, 0);

    return US_PER_SEC*(curTime.tv_sec - start.tv_sec) + curTime.tv_usec - start.tv_usec;
}

/*************** Send Buffer ***************/
CircularBuffer::CircularBuffer(int size, char * filename, unsigned long long int bytesToSend)
{
    sourcefd = open(filename, O_RDONLY);
    if (sourcefd < 0) {
        std::cerr << "Unable to open source file\n";
        exit(1);
    }

    state.resize(size, AVAILABLE);
    timestamp.resize(size);
    length.resize(size);
    data.resize(size);

    sIdx = 0;
    eIdx = INIT_SWS - 1;
    windowSize = INIT_SWS;

    payload = PAYLOAD;
    seqNum = 0;
    fileLoadCompleted = false;
    bytesToTransfer = bytesToSend;

    gettimeofday(&start, 0);

    #ifdef DEBUG
        ffile.open("fillLog", std::ios::out);
    #endif

}

void CircularBuffer::initialFill()
{
    for(uint32_t i = 0; i < data.size(); i++) {
        if(bytesToTransfer <= 0){
            fileLoadCompleted = true;
            return;
        }

        int packetLength = min((unsigned long long)payload, bytesToTransfer);

        #ifdef DEBUG
            ffile << "Data length: " <<  packetLength  << ", seqNum: " << seqNum << ", TIME: " << timeSinceStart() << "us" << endl;
        #endif

        // initialize header
        data[i].header.type = DATA_HEADER;
        data[i].header.seqNum = htonl(seqNum++);
        length[i] = packetLength + sizeof(msg_header_t);

        // read data into buffer
        read(sourcefd, data[i].msg, packetLength);

        // book keeping
        state[i] = FILLED;
        bytesToTransfer -= packetLength;
    }
}

bool CircularBuffer::outsideWindow(uint32_t index)
{
    #ifdef DEBUG
        // ffile << "Top of outside window function.\n\n"; ffile.flush();
        ffile << "Index: " <<  index <<  ", start: " << sIdx << ", end: " << eIdx << "\n"; ffile.flush();
    #endif

    if((sIdx <= eIdx) && (index < sIdx || eIdx < index)){
        return true;
    }else if(eIdx <= sIdx && (index < sIdx && eIdx < index)){
        return true;
    }

    #ifdef DEBUG
        ffile << "Somehow inside window.\n\n"; ffile.flush();
    #endif

    return false;
}


void CircularBuffer::fillBuffer()
{
    static uint32_t i = 0;
    for( ; i < data.size(); i = (i + 1)%BUFFER_SIZE) {
        if(bytesToTransfer <= 0){
            fileLoadCompleted = true;
            #ifdef DEBUG
                ffile << "Load of file completed.\n\n"; ffile.flush();
            #endif
            return;
        }

        if(state[i] == AVAILABLE){
            int packetLength = min((unsigned long long)payload, bytesToTransfer);

            #ifdef DEBUG
                ffile << "Data length: " <<  packetLength  << ", seqNum: " << seqNum << ", TIME: " << timeSinceStart() << "us" << endl;
            #endif

            // initialize header
            data[i].header.type = DATA_HEADER;
            data[i].header.seqNum = htonl(seqNum++);
            length[i] = packetLength + sizeof(msg_header_t);

            // read data into buffer
            read(sourcefd, data[i].msg, packetLength);

            // book keeping
            state[i] = FILLED;
            bytesToTransfer -= packetLength;
        }else{
            break;
        }
    }
}

/*************** Receive Buffer ***************/
CircularBuffer::CircularBuffer(int size, char * filename)
{
    destfd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (destfd < 0) {
        std::cerr << "Unable to open dest file\n";
        exit(1);
    }

    state.resize(size, WAITING);
    data.resize(size);
    length.resize(size);

    seqNum = 0;
    sIdx = 0;

    #ifdef DEBUG
        recvfile.open("recvLog", std::ios::out);
    #endif
}

void CircularBuffer::flushBuffer()
{
    for(size_t i = 0; i < data.size(); i++) {
        if(state[sIdx] == RECEIVED){
            // write to file
            write(destfd, data[sIdx].msg, length[sIdx]);
            // recvfile << "Writing to file: " <<  data[sIdx].header.seqNum << "\n";

            // book keeping
            state[sIdx] = WAITING;
            sIdx = (sIdx+1)%data.size();
        } else{
            break;
        }
    }
}

void CircularBuffer::sendAck()
{
    ack_packet_t ack;
    ack.type = ACK_HEADER;

    uint32_t j = seqNum%data.size();                // index for expected message
    for(size_t i = 0; i < data.size(); i++) {
        if(state[j] == RECEIVED){
            seqNum++;
            j = (j+1)%data.size();
        } else{
            break;
        }
    }

    #ifdef DEBUG
        recvfile << "Sending Ack for " <<  seqNum - 1 << " as received\n";
        recvfile.flush();
    #endif

    ack.seqNum = htonl(seqNum - 1);
    sendto(ackfd, (char *)&ack, sizeof(ack_packet_t), 0, &ackAddr, ackAddrLen);
}

void CircularBuffer::storeReceivedPacket(msg_packet_t & packet, uint32_t packetLength)
{
    // ack_packet_t ack;
    // ack.type = ACK_HEADER;
    packet.header.seqNum = ntohl(packet.header.seqNum);
    size_t bufIdx = packet.header.seqNum % data.size();

    #ifdef DEBUG
        recvfile << "Packet Seen: " << packet.header.seqNum << endl;
    #endif

    // static counter = 0;

    if(packet.header.seqNum == seqNum - 1){
        // ack.seqNum = htonl(seqNum - 1);
        // sendto(ackfd, (char *)&ack, sizeof(ack_packet_t), 0, &ackAddr, ackAddrLen);
        return;
    }else if(packet.header.seqNum < seqNum){
        return;
    }

    if(state[bufIdx] == WAITING){
        state[bufIdx] = RECEIVED;
        sendAck();
        data[bufIdx] = packet;
        length[bufIdx] = packetLength - sizeof(msg_header_t);
    }

    flushBuffer();
}
