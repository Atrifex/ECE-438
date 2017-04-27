#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "types.h"

#define SWS (1)
#define RWS (SWS)
#define MAX_WINDOW_SIZE (256)

#define INIT_RTO            (1)         // in seconds

// Header Flags
#define ACK_HEADER          (0x01)
#define SYN_HEADER          (0x02)
#define SYN_ACK_HEADER      (0x03)
#define FIN_HEADER          (0x04)
#define FIN_ACK_HEADER      (0x05)
#define DATA_HEADER         (0x06)

// Timing Information
#define START_TIME_VEC_SIZE (100)
#define US_PER_SEC          (1000000)


// RTT
#define MAX_RTT_HISTORY     (100)

// SRTT
#define ALPHA               ((double)0.125)                  // (1/8) is goood, acts like a low-pass filter

// RTO
#define MAX_SRTT_WEIGHT    ((double)4.0)                     // good between 2 - 4  --> questions how much you can trust initial results
#define MIN_SRTT_WEIGHT    ((double)1.0)
#define SRTT_SLOPE         ((MIN_SRTT_WEIGHT - MAX_SRTT_WEIGHT)/((double)(MAX_RTT_HISTORY)))

#define MAX_STD_WEIGHT     ((double)4.0)                     // good at 4 --> questions how much you can trust instantaneous changes
#define STD_SLOPE          ((MAX_STD_WEIGHT)/((double)(MAX_RTT_HISTORY)))


#endif
