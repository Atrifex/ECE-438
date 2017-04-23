
#include "tcp_sender.h"

TCPSender::TCPSender(char * hostname, char * hostUDPport)
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

void TCPSender::sendWindow()
{
	for(size_t i = 0; i < buffer.data.size(); i++) {
		if(buffer.state[i] == filled){
			sendto(sockfd, (char *)&(buffer.data[i]), sizeof(msg_packet_t), 0, &saddr, sizeof(saddr));
			buffer.state[i] = sent;
		}
	}
}


void TCPSender::reliableSend(char * filename, unsigned long long int bytesToTransfer)
{
	buffer = SendBuffer(SWS, filename, bytesToTransfer);


	// fill

	// send

	// wait for ack

}
