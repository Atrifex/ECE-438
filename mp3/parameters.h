#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "types.h"

// TODO: Use extra input from the terminal to test different window sizes
#define SWS (1)
#define RWS (SWS)
#define MAX_WINDOW_SIZE (256)

#define INIT_RTO            (1)         // in seconds

#define ACK_HEADER          (0x01)
#define SYN_HEADER          (0x02)
#define SYN_ACK_HEADER      (0x03)
#define FIN_HEADER          (0x04)
#define FIN_ACK_HEADER      (0x05)
#define DATA_HEADER         (0x06)

#define START_TIME_VEC_SIZE (100)
#define US_PER_SEC          (1000000)

#endif
