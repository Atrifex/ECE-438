#ifndef TYPES_H
#define TYPES_H

#include "parameters.h"

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
#include <condition_variable>
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
#include <list>
#include <deque>
#include <cmath>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

using std::sqrt;
using std::thread;
using std::ref;
using std::list;
using std::cout;
using std::endl;
using std::vector;
using std::numeric_limits;
using std::priority_queue;
using std::pair;
using std::make_pair;
using std::greater;
using std::deque;
using std::queue;
using std::stack;
using std::string;
using std::to_string;
using std::mutex;
using std::unique_lock;
using std::ifstream;
using std::ofstream;
using std::min;
using std::max;
using std::unique_lock;
using std::condition_variable;

typedef pair<int,int> int_pair;

#pragma pack(1)
typedef struct {
    uint8_t type;
    int seqNum;
} msg_header_t;

#define PAYLOAD (1472 - sizeof(msg_header_t))

#pragma pack(1)
typedef struct {
    msg_header_t header;
    char msg[PAYLOAD];
} msg_packet_t;

#pragma pack(1)
typedef struct {
    uint8_t type;
    int seqNum;
} ack_packet_t;

typedef struct {
    ack_packet_t ack;
    struct timeval time;
} ack_process_t;

typedef enum : uint8_t {
    /***** Sender States *****/
    AVAILABLE, FILLED, RETRANSMIT, SENT, ACKED,

    /***** Receiver States *****/
    WAITING, RECEIVED
} packet_state_t;

typedef enum : uint8_t {
    CLOSED, LISTEN,
    SYN_SENT,       // sender side
    SYN_RECVD,      // receiver side
    ESTABLISHED,    // both
    FIN_SENT,
    FIN_RCVD,
    CLOSING, TIME_WAIT
} tcp_state_t;

typedef enum : uint8_t {
    WAITING_TO_SEND,
    SLOW_START,
    AIMD
} send_state_t;

#endif
