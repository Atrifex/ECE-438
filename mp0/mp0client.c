/*
 * MP0 Client Program
 * Author: Rishi Thakkar
 * Assignment: MP0
 * Date: 1/27/2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>


// Default connection information in the case the user does not provide the correct input
#define DEFAULT_PORT "5900"
#define DEFAULT_SERVER "cs438.cs.illinois.edu"

// Variable Attributes
#define MAXDATASIZE 100 			// max number of bytes we can get at once
#define NUMBER_MSGS_RECV 10			// number of transmission to recieve
#define RECV_STRG_BASE_LEN 10
#define DATA_BASE_LEN 12

// Messages and constant strings
#define INIT_MESSAGE "HELO\n"
#define	TERMINATING_MESSAGE "BYE\n"
#define RECV_MESSAGE "RECV\n"
#define USERNAME "USERNAME "
#define RECV_STRING "Received: "



int main(int argc, char *argv[])
{
	int sockfd, bytes_read, bytes_sent, length, i;
	char buffer[MAXDATASIZE] = INIT_MESSAGE;
	char * data = buffer + DATA_BASE_LEN;
	char string_printout[MAXDATASIZE] = RECV_STRING;
	struct addrinfo hints, * servinfo, * p;
	int gai_rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 4) {
	    fprintf(stderr,"usage: mp0client hostname port data\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	/********* Connection Setup Phase *********/
	if ((gai_rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		// get a socket to be able to communicate
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		// connect with server
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	// if the addresses are all invalid
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure


	/********* Handshake Phase *********/
	// send initial handshake message
	if ((bytes_sent = send(sockfd, buffer, strlen(buffer), 0)) == -1){
	    perror("send");
        close(sockfd);
		exit(0);
	}

	// recieve message from server | Message: 100 - OK\n
	if ((bytes_read = recv(sockfd, buffer, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
		close(sockfd);
	    exit(1);
	}
	buffer[bytes_read] = '\0';

	// uncomment and print recieved message during debuging
	// printf("%s", buffer);

	// prepare buffer to contain the second pahse of the handshake message
	strcpy(buffer, USERNAME);
	length = strlen(buffer);
	strncpy(buffer+length, argv[3], strlen(argv[3]));
	length = length+strlen(argv[3]);
	buffer[length] = '\n';
	buffer[length+1] = '\0';

	// send mesage to server | Message: USERNAME <username>\n
	if ((bytes_sent = send(sockfd, buffer, strlen(buffer), 0)) == -1){
	    perror("send");
        close(sockfd);
		exit(0);
	}

	// recieve message from server | Message: 200 - Username: <username>\n
	if ((bytes_read = recv(sockfd, buffer, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
		close(sockfd);
	    exit(1);
	}
	buffer[bytes_read] = '\0';

	// uncomment and print recieved message during debuging
	// printf("%s", buffer);

	/********* Recieving Transmission Phase *********/
	for(i = 0; i < NUMBER_MSGS_RECV; i++){

		// ask to recieve data | Message: RECV\n
		if ((bytes_sent = send(sockfd, RECV_MESSAGE, strlen(RECV_MESSAGE), 0)) == -1){
		    perror("send");
	        close(sockfd);
			exit(0);
		}

		// recieve data from server | Message: 300 - DATA: <some_string>\n
		if((bytes_read = recv(sockfd, buffer, MAXDATASIZE, 0)) == -1){
			perror("recv");
			close(sockfd);
			exit(1);
		}
		buffer[bytes_read] = '\0';

		// concatinate the strings, add new line, and add null character
		strcat(string_printout, data);

		// Print message for user
		printf("%s", string_printout);

		// clean up
		string_printout[RECV_STRG_BASE_LEN] = '\0';
	}

	/********* Closing Connection Phase *********/
	// Set up closing connection message
	strcpy(buffer, TERMINATING_MESSAGE);

	// Initiate closing connection | Message: BYE\n
	if ((bytes_sent = send(sockfd, buffer, strlen(buffer), 0)) == -1){
	    perror("send");
        close(sockfd);
		exit(0);
	}

	// Recieve message from server | Message: 400 – Bye\n
	if ((bytes_read = recv(sockfd, buffer, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
		close(sockfd);
	    exit(1);
	}
	buffer[bytes_read] = '\0';

	// uncomment and print recieved message during debuging
	// printf("%s", buffer);

	close(sockfd);

	return 0;
}
