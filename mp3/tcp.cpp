
#include "tcp.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

TCP::~TCP(){
	delete buffer;
	close(sockfd);
}

/*************** Sender Functions ***************/
TCP::TCP(char * hostname, char * hostUDPport)
{
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(hostname, hostUDPport, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	if (servinfo == NULL) {
		fprintf(stderr, "talker: failed to resolve addr\n");
		exit(2);
	}

	receiverAddr  = *(servinfo->ai_addr);
	receiverAddrLen = servinfo->ai_addrlen;

	freeaddrinfo(servinfo);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, hostUDPport, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		exit(2);
	}

	freeaddrinfo(servinfo);

	// Initial time out estimation
	rtt.tv_sec = 1;
	rtt.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &rtt, sizeof(rtt)) < 0) {
		perror("setsockopt");
		exit(3);
	}

	state = CLOSED;
}

void TCP::senderSetupConnection()
{
	// construct buffer
	msg_header_t syn;
	syn.type = SYN_HEADER;
	syn.seqNum = htonl(0);

	// send
	sendto(sockfd, (char *)&syn, sizeof(msg_header_t), 0, &receiverAddr, receiverAddrLen);
	state = SYN_SENT;

	// wait for ack + syn
	ack_packet_t ack;
	ack.type = ACK_HEADER;
	ack.seqNum = receiveStartSynAck();

	// send ack
	sendto(sockfd, (char *)&ack, sizeof(ack_packet_t), 0, &receiverAddr, receiverAddrLen);
}


void bufferFiller(CircularBuffer & buffer) {
	buffer.fill();
	cout << "All data has been read from the buffer";
}

void packetSender(TCP & connection) {
	connection.sendWindow();
}

void TCP::reliableSend(char * filename, unsigned long long int bytesToTransfer)
{
    buffer = new CircularBuffer(SWS, filename, bytesToTransfer);

	state = LISTEN;

	// Set up TCP connection
	senderSetupConnection();

	state = ESTABLISHED;

	// send data
	thread bufferHandler(bufferFiller, ref(*(this->buffer))); bufferHandler.detach();
	thread sendHandler(packetSender, ref(*(this))); sendHandler.detach();
	processAcks();

	// tear down TCP connection
	senderTearDownConnection();

}

void TCP::senderTearDownConnection()
{

}


void TCP::processAcks()
{
	while(1);
}

void TCP::sendWindow()
{
	while(state == ESTABLISHED){
		int bufferSize = buffer->data.size();
		for(int i = 0; i < bufferSize; i++) {
			unique_lock<mutex> lkSend(buffer->pktLocks[i]);
			buffer->senderCV.wait(lkSend, [=]{
					return (buffer->state[i] == FILLED || buffer->state[i] == RETRANSMIT);
			});

			if(buffer->state[i] == FILLED){
				sendto(sockfd, (char *)&(buffer->data[i]), buffer->length[i], 0, &receiverAddr, receiverAddrLen);
				buffer->state[i] = SENT;
			}else if(buffer->state[i] == RETRANSMIT){
				sendto(sockfd, (char *)&(buffer->data[i]), buffer->length[i], 0, &receiverAddr, receiverAddrLen);
				buffer->state[i] = SENT;
				// drop lock asap
				lkSend.unlock();

				uint j = (i + 1) % bufferSize;
				for(int k = 0; k < bufferSize; k++){
					if(buffer->state[j] == RETRANSMIT){
						unique_lock<mutex> lkRetrans(buffer->pktLocks[j]);
						sendto(sockfd, (char *)&(buffer->data[j]), buffer->length[j], 0, &receiverAddr, receiverAddrLen);
						buffer->state[i] = SENT;
					}
					j = (j + 1) % bufferSize;
				}
				// Retransmission so can't move forward
				i--;
			}
		}
    }
}

/*************** Receiver Functions ***************/
TCP::TCP(char * hostUDPport)
{
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, hostUDPport, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		exit(2);
	}

	freeaddrinfo(servinfo);

	state = CLOSED;
}

void TCP::receiverSetupConnection()
{
	msg_header_t syn_ack;

	// receive SYN
	syn_ack.seqNum = receiveStartSyn();
	state = SYN_RECVD;

	// send SYN + ACK
	syn_ack.type = SYN_ACK_HEADER;
 	sendto(sockfd, (char *)&syn_ack, sizeof(msg_header_t), 0, (struct sockaddr *)&senderAddr, senderAddrLen);

	// receive ACK
	receiveStartAck(syn_ack);
}

void TCP::reliableReceive(char * filename)
{

	buffer = new CircularBuffer(SWS, filename);

	state = LISTEN;

	// Set up TCP connection
	receiverSetupConnection();

	state = ESTABLISHED;

	while(1){
		if(receivePacket() == false) break;
	}

	// tear down TCP connection

}



bool TCP::receivePacket()
{
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	msg_packet_t packet;
	addr_len = sizeof(their_addr);

	if ((numbytes = recvfrom(sockfd, (char *)&packet, sizeof(msg_packet_t) , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	if(packet.header.type != DATA_HEADER) return false;

	buffer->storeReceivedPacket(packet, numbytes);

	return true;
}


/*************** Startup Handshake Functions ***************/
int TCP::receiveStartSyn()
{
	struct sockaddr theirAddr;
    socklen_t theirAddrLen = sizeof(theirAddr);
	msg_header_t syn;
	int numbytes;

	while(1){
		if((numbytes = recvfrom(sockfd, (char *)&syn, sizeof(msg_header_t), 0, (struct sockaddr*)&theirAddr, &theirAddrLen)) == -1){
			perror("recvfrom");
		}

		if((syn.type == SYN_HEADER) && (numbytes == sizeof(msg_header_t))){
			break;
		}
	}

	senderAddr = theirAddr;
	senderAddrLen = theirAddrLen;
	buffer->setSocketAddrInfo(sockfd, senderAddr, senderAddrLen);

	return syn.seqNum;
}

int TCP::receiveStartSynAck()
{
	struct sockaddr theirAddr;
    socklen_t theirAddrLen = sizeof(theirAddr);
	msg_header_t syn, syn_ack;

	syn.type = SYN_HEADER;
	int seqNum = 1;

	while(1){
		if((recvfrom(sockfd, (char *)&syn_ack, sizeof(msg_header_t), 0, (struct sockaddr*)&theirAddr, &theirAddrLen) == -1)
			|| (syn_ack.type != SYN_ACK_HEADER)){
			syn.seqNum = htonl(seqNum++);
			sendto(sockfd, (char *)&syn, sizeof(msg_header_t), 0, &receiverAddr, receiverAddrLen);
		} else{
			break;
		}
	}

	return syn_ack.seqNum;
}

void TCP::receiveStartAck(msg_header_t syn_ack)
{
	struct sockaddr theirAddr;
	socklen_t theirAddrLen = sizeof(theirAddr);
	msg_packet_t packet;
	int numbytes;

	while(1){
		if ((numbytes = recvfrom(sockfd, (char *)&packet, sizeof(msg_packet_t) , 0, (struct sockaddr *)&theirAddr, &theirAddrLen)) == -1) {
			perror("recvfrom");
		}

		// write message into buffer if ACK lost and message seen first
		if(packet.header.type == DATA_HEADER && numbytes > (int)sizeof(msg_header_t)){
			buffer->storeReceivedPacket(packet, numbytes);
			break;
		} else if(packet.header.type == ACK_HEADER){
			break;
		} else{
			sendto(sockfd, (char *)&syn_ack, sizeof(msg_header_t), 0, &theirAddr, theirAddrLen);
		}
	}
}

/*************** Teardown Handshake Functions ***************/
