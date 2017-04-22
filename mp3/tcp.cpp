
#include "tcp.h"

TCP::TCP()
{
    char myAddr[100];
    struct sockaddr_in bindAddr;

    //socket() and bind() our socket. We will do all sendto()ing and recvfrom()ing on this one.
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    // setup address for binding purposes
    sprintf(myAddr, "10.1.1.%d", myNodeID);         // prints ip address to my addr
    memset(&bindAddr, 0, sizeof(bindAddr));         // clears bindaddr
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(GLOBAL_COM_PORT);     // port for communication
    inet_pton(AF_INET, myAddr, &bindAddr.sin_addr); // writing addr to sin_addr

    // bind for listining
    if(bind(sockfd, (struct sockaddr*)&bindAddr, sizeof(struct sockaddr_in)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

}
