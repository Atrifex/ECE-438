#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "types.h"

// Window Properties
#define INIT_SWS                    (100)
#define BUFFER_SIZE                 (512)
#define MAX_WINDOW_SIZE             (BUFFER_SIZE/2)
#define MIN_WINDOW_SIZE             (10)

#define INIT_RTO                    (80000)     // in microseconds
#define FIN_TO                      (100000)         // in seconds
#define MAX_RTO                     (2000000)   // in microseconds

// Header Flags
#define ACK_HEADER                  (0x01)
#define SYN_HEADER                  (0x02)
#define SYN_ACK_HEADER              (0x03)
#define FIN_HEADER                  (0x04)
#define FIN_ACK_HEADER              (0x05)
#define DATA_HEADER                 (0x06)
#define DATA_RETRANS_HEADER         (0x07)

// Timing Information
#define START_TIME_VEC_SIZE         (100)
#define US_PER_SEC                  (1000000)
#define RETRANS_CHECK_TIME          (50)                              // 50 microseconds

// RTT
#define MAX_RTT_HISTORY             (BUFFER_SIZE)
#define DROP_HIST_WEIGHT            (10)                              // drop a multiple of 10 things everytime there is a timeout

// SRTT
#define ALPHA                       ((double)0.125)                   // (1/8) is goood, acts like a low-pass filter
#define ALPHA_TO_SCALAR             ((double)0)
#define ALPHA_MAX                   ((double)0.75)

// RTO
#define MAX_SRTT_WEIGHT             ((double)3.0)                     // good between 2 - 4  --> questions how much you can trust initial results
#define MIN_SRTT_WEIGHT             ((double)1.0)
#define SRTT_SLOPE                  ((MIN_SRTT_WEIGHT - MAX_SRTT_WEIGHT)/((double)(MAX_RTT_HISTORY)))

#define MAX_STD_WEIGHT              ((double)4.0)                     // good at 4 --> questions how much you can trust instantaneous changes
#define STD_SLOPE                   ((MAX_STD_WEIGHT)/((double)(MAX_RTT_HISTORY)))

// Duplicate
#define DUP_MAX_COUNTER             (3)

#endif
