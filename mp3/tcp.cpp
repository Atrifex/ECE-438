
#include "tcp.h"

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

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("socket");
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
        fprintf(stderr, "failed to bind socket\n");
        exit(2);
    }

	// Need when you call sendto
	saddr = *(p->ai_addr);

    freeaddrinfo(servinfo);

}

void TCP::sendWindow()
{
	size_t j = buffer.startIdx;
	for(size_t i = 0; i < buffer.data.size(); i++) {
		if(buffer.state[j] == filled){
			sendto(sockfd, (char *)&(buffer.data[j]), sizeof(msg_packet_t), 0, &saddr, sizeof(saddr));
			buffer.state[j] = sent;
			j = (j + 1) % buffer.data.size();
		}
	}
}


void TCP::reliableSend(char * filename, unsigned long long int bytesToTransfer)
{
	buffer = CircularBuffer(SWS, filename, bytesToTransfer);

	// Set up TCP connection
	setupConnection();

	while(1){
		// fill
		buffer.fill();

		// send
		sendWindow();

		// wait for ack
		// TODO: increment startIdx
	}

	// tear down TCP connection
	tearDownConnection();

}


void TCP::setupConnection()
{
	// construct buffer
	msg_header_t syn;
	syn.seqNum = htonl(0);
	syn.length = htons(SYN_HEADER);

	// send
	sendto(sockfd, (char *)&syn, sizeof(msg_header_t), 0, &saddr, sizeof(saddr));

	// wait for ack + syn
    struct sockaddr_in senderAddr;
    socklen_t senderAddrLen = sizeof(senderAddr);
    if (recvfrom(sockfd, (char *)&syn, sizeof(msg_header_t), 0, (struct sockaddr*)&senderAddr, &senderAddrLen) == -1) {
        perror("connectivity listener: recvfrom failed");
        exit(1);
    }

	// send ack
	ack_packet_t ack;
	ack.seqNum = htonl(0);
	sendto(sockfd, (char *)&ack, sizeof(ack_packet_t), 0, &saddr, sizeof(saddr));
}

void TCP::tearDownConnection()
{

}

/*************** Receiver Functions ***************/
TCP::TCP(char * hostUDPport)
{
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

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
}

void TCP::receiveWindow()
{

}


void TCP::reliableReceive(char * filename)
{
	buffer = CircularBuffer(SWS, filename);

	// Set up TCP connection

	while(1){
		// receive
		receiveWindow();

		// send acks

		// flush
		buffer.flush();

	}

	// tear down TCP connection

}
