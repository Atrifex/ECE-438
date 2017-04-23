
/*
 *
 * TCP Receiver
 *
 */

#include "tcp_receiver.h"

int main(int argc, char** argv) {

	if(argc != 3){
		fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
		exit(1);
	}


	// setup receiver connection
	TCPReceiver connection(argv[1]);

	// receive file
	connection.reliableReceive(argv[2]);
}
