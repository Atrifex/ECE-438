
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

unsigned long long CircularBuffer::timeSinceStart()
{
    struct timeval curTime;
    gettimeofday(&curTime, 0);

    return US_PER_SEC*(curTime.tv_sec - start.tv_sec) + curTime.tv_usec - start.tv_usec;
}

/*************** Send Buffer ***************/
CircularBuffer::CircularBuffer(int size, char * filename, unsigned long long int bytesToSend)
{
    sourcefile.open(filename, std::ios::in);
    if (!(sourcefile.is_open())) {
        std::cerr << "Unable to open source file\n";
        exit(1);
    }

    state.resize(size, AVAILABLE);
    timestamp.resize(size);
    length.resize(size);
    data.resize(size);

    payload = PAYLOAD;
    seqNum = 0;
    sIdx = 0;
    fileLoadCompleted = false;
    bytesToTransfer = bytesToSend;

#ifdef FILE_CHECK
    sourcefile.seekg (0, sourcefile.end);
    int fileLength = sourcefile.tellg();
    sourcefile.seekg (0, sourcefile.beg);
    bytesToTransfer = min((unsigned long long)fileLength, bytesToSend);
#endif

    gettimeofday(&start, 0);

    #ifdef DEBUG
        ffile.open("fillLog", std::ios::out);
    #endif

}

void CircularBuffer::fill()
{
    while(1){
        int bufferSize = data.size();
        for(int i = 0; i < bufferSize; i++) {
            if(bytesToTransfer <= 0){
                fileLoadCompleted = true;
                return;
            }

            unique_lock<mutex> lkFill(pktLocks[i]);
            fillerCV.wait(lkFill, [=]{return state[i] == AVAILABLE;});

            int packetLength = min((unsigned long long)payload, bytesToTransfer);

            #ifdef DEBUG
                ffile << "Data length: " <<  packetLength  << ", seqNum: " << seqNum << ", TIME: " << timeSinceStart() << "us" << endl;
            #endif
            // initialize header
            data[i].header.type = DATA_HEADER;
            data[i].header.seqNum = htonl(seqNum++);
            length[i] = packetLength + sizeof(msg_header_t);

            // read data into buffer
            sourcefile.read(data[i].msg, packetLength);

            // book keeping
            state[i] = FILLED;
            lkFill.unlock();
            senderCV.notify_one();

            bytesToTransfer -= packetLength;
        }
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

    state.resize(size, WAITING);
    data.resize(size);
    length.resize(size);

    seqNum = 0;
    sIdx = 0;
}

void CircularBuffer::flush()
{
    for(size_t i = 0; i < data.size(); i++) {
        pktLocks[sIdx].lock();
        if(state[sIdx] == RECEIVED){

            #ifdef DEBUG
                cout << "Sent ACK for: " << data[sIdx].header.seqNum << endl;
                // printf("sIdx: %d\n" , sIdx);
                // printf("Packet Length: %u, SeqNum: %u\n" , length[sIdx], seqNum);
            #endif
            // write to file
            destfile.write(data[sIdx].msg, length[sIdx]);
            destfile.flush();

            // book keeping
            state[sIdx] = WAITING;
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
    // Set up ack packet
    ack_packet_t ack;
    ack.type = ACK_HEADER;

    packet.header.seqNum = ntohl(packet.header.seqNum);

    cout << "Message seen: " <<  packet.header.seqNum << endl;
    // drop packet if the seqNum is smaller than expected.
    if(packet.header.seqNum < seqNum){
        cout << "DUPLICATE ACK for: " <<  seqNum - 1 << endl;
        // send ACK the previous message
        ack.seqNum = htonl(seqNum - 1);
        sendto(ackfd, (char *)&ack, sizeof(ack_packet_t), 0, &ackAddr, ackAddrLen);
        return;
    }else{
        // send ACK for expected message
        ack.seqNum = htonl(seqNum);
        sendto(ackfd, (char *)&ack, sizeof(ack_packet_t), 0, &ackAddr, ackAddrLen);
        seqNum++;
    }

    size_t bufIdx = packet.header.seqNum % data.size();
    pktLocks[bufIdx].lock();
    if(state[bufIdx] == WAITING){
        state[bufIdx] = RECEIVED;
        data[bufIdx] = packet;
        length[bufIdx] = packetLength - sizeof(msg_header_t);

#ifdef DEBUG
        // printf("\nReceived seqNum: %d\n", packet.header.seqNum);
        // printf("Index Store: %lu, RECEIVED length: %lu\n", bufIdx, packetLength - sizeof(msg_header_t));
#endif

    }
    pktLocks[bufIdx].unlock();

    thread flusher(bufferFlusher, ref(*this)); flusher.detach();
}
