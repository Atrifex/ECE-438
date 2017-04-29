
/*
 *
 * TCP Receiver
 *
 */

#include "tcp.h"

int main(int argc, char** argv) {

	if(argc != 3){
		fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
		exit(1);
	}


	// setup receiver connection
	TCP receiver(argv[1]);

	// receive file
	const auto start = std::chrono::high_resolution_clock::now();
	receiver.reliableReceive(argv[2]);
	const auto end = std::chrono::high_resolution_clock::now();

	cout << "Received in: " << std::chrono::duration<double, std::milli>(end - start).count() << " milliseconds.\n";
}
