
#include "tcp.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
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

	sendAddr  = *(servinfo->ai_addr);
	sendAddrLen = servinfo->ai_addrlen;

	char s[INET6_ADDRSTRLEN];
	printf("Sending to: %s\n",
		inet_ntop(servinfo->ai_family,
			get_in_addr((struct sockaddr *)&sendAddr),
			s, sizeof s));

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
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&rtt,sizeof(rtt)) < 0) {
		perror("setsockopt");
		exit(3);
	}

}

void TCP::receiveSynAck()
{
	struct sockaddr_storage receiverAddr;
    socklen_t receiverAddrLen = sizeof(receiverAddr);

	msg_header_t syn;
	int seqNum = 1;
	syn.length = htons(SYN_HEADER);

	msg_header_t syn_ack;


	while(1){
		if(recvfrom(sockfd, (char *)&syn_ack, sizeof(msg_header_t), 0, (struct sockaddr*)&receiverAddr, &receiverAddrLen) == -1){
			printf("No SYN_ACK yet.\n");
			// rtt.tv_sec *= 2;
			// rtt.tv_usec = 0;
			// setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&rtt,sizeof(rtt));
			// retransmit send
			syn.seqNum = htonl(seqNum++);
			sendto(sockfd, (char *)&syn, sizeof(msg_header_t), 0, &sendAddr, sendAddrLen);
		}
		else break;
	}
	printf("SYN ACK received, with seqNum: %d", ntohl(syn.seqNum));
}

void TCP::senderSetupConnection()
{
	// construct buffer
	msg_header_t syn;
	syn.seqNum = htonl(0);
	syn.length = htons(SYN_HEADER);

	// send
	sendto(sockfd, (char *)&syn, sizeof(msg_header_t), 0, &sendAddr, sendAddrLen);

	// wait for ack + syn
	receiveSynAck();

	printf("GOT THIS THIS POINT\n");

	// send ack
	ack_packet_t ack;
	ack.seqNum = htonl(0);
	sendto(sockfd, (char *)&ack, sizeof(ack_packet_t), 0, &sendAddr, sendAddrLen);
}

void TCP::reliableSend(char * filename, unsigned long long int bytesToTransfer)
{
    buffer = new CircularBuffer(SWS, filename, bytesToTransfer);

	// Set up TCP connection
	senderSetupConnection();

	while(1){
		// fill
		buffer->fill();

		// send
		sendWindow();

		// wait for ack
		// TODO: increment startIdx
	}

	// tear down TCP connection
	senderTearDownConnection();

}

void TCP::senderTearDownConnection()
{

}

void TCP::sendWindow()
{
	size_t j = buffer->startIdx;
	for(size_t i = 0; i < buffer->data.size(); i++) {
		if(buffer->state[j] == filled){
			sendto(sockfd, (char *)&(buffer->data[j]), sizeof(msg_packet_t), 0, &sendAddr, sizeof(sendAddr));
			buffer->state[j] = sent;
			j = (j + 1) % buffer->data.size();
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
}

void TCP::receiverSetupConnection()
{
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	msg_header_t syn;
	msg_header_t syn_ack;

	// receive syn
	recvfrom(sockfd, (char *)&syn, sizeof(msg_header_t) , 0, (struct sockaddr *)&their_addr, &addr_len);

	printf("GOT THIS THIS POINT, %d\n", ntohs(syn.seqNum));

	// send syn + ack
	syn_ack.seqNum = syn.seqNum;
	syn_ack.length = htons(SYN_ACK_HEADER);
	sendto(sockfd, (char *)&syn_ack, sizeof(msg_header_t), 0, (struct sockaddr*)&their_addr, addr_len);

	// wait for ack + syn
}

void TCP::reliableReceive(char * filename)
{
	buffer = new CircularBuffer(SWS, filename);

	// Set up TCP connection
	receiverSetupConnection();

	while(1){
		// receive
		receiveWindow();

		// send acks

		// flush
		buffer->flush();

	}

	// tear down TCP connection

}



void TCP::receiveWindow()
{
}
