#ifndef PARAMETERS_H
#define PARAMETERS_H

// TODO: Use extra input from the terminal to test different window sizes
#define SWS (16)
#define RWS (SWS)
#define MAX_WINDOW_SIZE (128)

#define PAYLOAD (1472)

#define RTT (1000000)  // 1000 ms is

#define COM_RANGE           (32768)
#define SYN_HEADER          (COM_RANGE - 1)
#define SYN_ACK_HEADER      (COM_RANGE - 2)
#define FIN_HEADER          (COM_RANGE - 3)

#endif
