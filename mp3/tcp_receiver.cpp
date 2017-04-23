
#include "tcp_receiver.h"

TCPReceiver::TCPReceiver(char * hostname, char * hostUDPport)
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

void TCPReceiver::receiveWindow()
{

}


void TCPReceiver::reliableReceive(char * filename)
{
	buffer = ReceiveBuffer(SWS, filename);

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

//
