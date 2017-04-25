#ifndef PARAMETERS_H
#define PARAMETERS_H

// TODO: Use extra input from the terminal to test different window sizes
#define SWS (1)
#define RWS (SWS)
#define MAX_WINDOW_SIZE (256)

#define PAYLOAD (1472)

#define INIT_RTT (1000000)  // 1000 ms is

#define ACK_HEADER          (0x01)
#define SYN_HEADER          (0x02)
#define SYN_ACK_HEADER      (0x03)
#define FIN_HEADER          (0x04)
#define FIN_ACK_HEADER      (0x05)
#define DATA_HEADER         (0x06)

#endif
