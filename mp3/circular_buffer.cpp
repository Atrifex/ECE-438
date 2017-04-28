
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

#ifdef FILE_CHECK
    sourcefile.seekg (0, sourcefile.end);
    int fileLength = sourcefile.tellg();
    sourcefile.seekg (0, sourcefile.beg);
    bytesToTransfer = min((unsigned long long)fileLength, bytesToSend);
#else
    bytesToTransfer = bytesToSend;
#endif

    gettimeofday(&start, 0);

    ffile.open("fillLog", std::ios::out);

}

void CircularBuffer::fill()
{
    while(1){
        int bufferSize = data.size();
        for(int i = 0; i < bufferSize; i++) {
            if(bytesToTransfer <= 0){
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

void sendAck(uint32_t seqNum){
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
                cout << "Sent ACK for: " << data[sIdx].header.seqNum << endl;
                // printf("sIdx: %d\n" , sIdx);
                // printf("Packet Length: %u, SeqNum: %u\n" , length[sIdx], seqNum);
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

    seqNumLock.lock();
    uint32_t seqNumExpected = seqNum;
    seqNumLock.unlock();

    // drop packet if the seqNum is smaller than expected.
    if(packet.header.seqNum < seqNumExpected){
        // ack the previous message
        cout << "DUPLICATE ACK for: " <<  seqNumExpected - 1 << endl;
        thread acker(sendAck, seqNumExpected - 1); acker.detach();
        return;
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

    thread flusher(bufferFlusher, ref(*this));
    flusher.detach();
}
