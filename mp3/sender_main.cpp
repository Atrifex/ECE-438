/*
 *
 * TCP Sender
 *
 */

#include "tcp.h"

int main(int argc, char** argv) {
	if(argc != 5) {
		fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n", argv[0]);
		exit(1);
	}

	// setup sender connection
	TCP sender(argv[1], argv[2]);

	// send file
	// const auto start = std::chrono::high_resolution_clock::now();
	sender.reliableSend(argv[3], atoll(argv[4]));
	// const auto end = std::chrono::high_resolution_clock::now();

	// cout << "Sent: " << argv[3] << "\nTime Elapsed: "
		// << std::chrono::duration<double, std::milli>(end - start).count() << " milliseconds.\n";

}
