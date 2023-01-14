/* 
 * File:   variables.h
 * Author: josephjoe1209
 *
 * Created on November 20, 2022, 9:49 AM
 */

#ifndef VARIABLES_H
#define	VARIABLES_H
#include <xc.h> // include processor files - each processor file is guarded.  
#include "constants.h"
#include "can.h"

extern volatile int databyte, checksum, s_tick;
extern int debounce_timer;

extern volatile unsigned int ecan1MsgBuf[NUM_OF_ECAN_BUFFERS][8]
__attribute__((aligned(NUM_OF_ECAN_BUFFERS * 16)));

extern mID CanRxMessage[NUM_OF_ECAN_BUFFERS];

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* VARIABLES_H */

