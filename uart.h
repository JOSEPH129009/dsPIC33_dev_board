/* 
 * File:   uart.h
 * Author: josephjoe1209
 *
 * Created on November 20, 2022, 9:48 AM
 */

#ifndef UART_H
#define	UART_H
#include <xc.h> // include processor files - each processor file is guarded.  


void UART2_Init(void);
//void uart2_putc(char *s);
void uart2_tx(char byte);
void InitLIN_TX(void);
void LIN_Transmit(void);


#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

