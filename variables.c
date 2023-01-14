
/*
 * File:   variables.c
 * Author: bug70
 *
 * Created on 2021?11?21?, ?? 12:17
 */
#include "variables.h"
/*interger only has two bytes width*/

volatile int databyte = 0, checksum = 0, s_tick = 0;
int debounce_timer = 0;

volatile unsigned int ecan1MsgBuf[NUM_OF_ECAN_BUFFERS][8]
__attribute__((aligned(NUM_OF_ECAN_BUFFERS * 16)));

mID CanRxMessage[NUM_OF_ECAN_BUFFERS] = {{0}};


