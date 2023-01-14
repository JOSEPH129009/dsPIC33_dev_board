/* 
 * File:   main.h
 * Author: josephjoe1209
 *
 * Created on November 20, 2022, 9:48 AM
 */

#ifndef MAIN_H
#define	MAIN_H
#include "uart.h"
#include "variables.h"
#include "can.h"
#include "constants.h"
#include <xc.h> // include processor files - each processor file is guarded.
typedef enum{
    PINSET,
    PINRESET
}PINSTATE;
    
#define         SW1           _RC7
//40M , Fcyc = 1/2Fpll
#define BAUD38400 ((40000000/38400)/16)-1 // UART2 Baud
#define         LIN_ID          0x23
#define         LIN_TXE     _LATA8
#define         LIN_CS      _LATA7
#define         ANSEL_LIN   _ANSA7
#define         TRISLINTXE  _TRISA8
#define         TRISLINCS   _TRISA7
    
#define         LIN_BIT_TIME   ((1000000/4800) + 0.5)
#define         LIN_BIT_STUFF                       0x4
#define         LIN_BRGVAL     ((40000000/4800)/16) - 1 //baud 4800
    
    
    
void Delayus(int delay);
void Calc_Checksum(int data);
int read_pinstatus(volatile uint16_t* port , uint16_t pin);
PINSTATE Sub_ReadPin(volatile uint16_t* port , uint16_t pin);
#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

