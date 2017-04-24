#ifndef TYPES_H
#define TYPES_H

#include <cstdlib>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <queue>
#include <stack>
#include <utility>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using std::thread;
using std::ref;

using std::cout;
using std::endl;
using std::vector;
using std::numeric_limits;
using std::priority_queue;
using std::pair;
using std::make_pair;
using std::greater;
using std::queue;
using std::stack;
using std::string;
using std::to_string;
using std::mutex;
using std::ifstream;
using std::ofstream;
using std::min;

typedef pair<int,int> int_pair;


typedef struct {
    uint32_t seqNum;
    uint16_t length;
} msg_header_t;

typedef struct {
    msg_header_t header;
    char msg[PAYLOAD - sizeof(msg_header_t)];
} msg_packet_t;

typedef struct {
    uint32_t seqNum;
} ack_packet_t;


typedef enum : uint8_t {
    /***** Sender States *****/
    available,
    filled,
    sent,
    acked,

    /***** Receiver States *****/
    waiting,
    received
} packet_state_t;

typedef enum : uint8_t {
    closed
} tcp_state_t;




#endif
