
/* 
 * File:   uart.c
 * Author: bug70
 *
 * Created on 2021?11?14?, ?? 5:40
 */

#include <stdio.h>
#include <stdlib.h>
#include "p33EV256GM106.h"
#include "main.h"



void UART2_Init(void)
{
    //digital output
    TRISBbits.TRISB4 =0;
    //digital input
    TRISBbits.TRISB3 = 1;
    /* map the pin to RB4, UART2 is remappable to RP36*/

    RPOR1bits.RP36R = 0x03; //map UART2 TXD to pin RB4
    RPINR19bits.U2RXR = 0x23; //tied to RP35 = RB3
     //
    // set up the UART for default baud, 1 start, 1 stop, no parity
    //
    U2MODEbits.STSEL = 0;       // 1-Stop bit
    U2MODEbits.PDSEL = 0;       // No Parity, 8-Data bits
    U2MODEbits.ABAUD = 0;       // Auto-Baud disabled
    U2MODEbits.BRGH = 0;        // Standard-Speed mode
    U2BRG = BAUD38400;          // Baud Rate setting for 38400 (default)
    U2STAbits.UTXISEL0 = 1;    
    U2STAbits.UTXISEL1 = 0;
    
    /*
     * UTXISEL[1:0]: UARTx Transmission Interrupt Mode Selection bits
     * 11 = Reserved; do not use
10 = Interrupt when a character is transferred to the Transmit Shift Register (TSR), and as a result,
the transmit buffer becomes empty
01 = Interrupt when the last character is shifted out of the Transmit Shift Register; all transmit
operations are completed
00 = Interrupt when a character is transferred to the Transmit Shift Register (this implies there is at
least one character open in the transmit buffer)
     */
  
    IEC1bits.U2TXIE = 0;        // disable UART TX interrupt
    U2MODEbits.UARTEN = 1;      // Enable UART (this bit must be set *BEFORE* UTXEN)   
    U2STAbits.UTXEN = 1;
}

//void tx_process(int c)
//{
//    while(U2STAbits.UTXBF); //wait while Tx buffer full
//    U2TXREG = c;
//}
//
//void uart2_putc(char *s)
//{
//    while(*s) // loop until *s=\0, end of string
//        tx_process(*s++); //send the character and point to the next one
//}

void uart2_tx(char byte)
{
    while(U2STAbits.TRMT == 0) //while shift register is not empty
        ;
    U2TXREG = byte;
}

/* the LIN UART is UART1 of the 'GM106*/
void InitLIN_TX(void)
{
    ANSEL_LIN = 0;         //Analog off
    TRISLINTXE = 0;        //set as output
    TRISLINCS = 0;         //set as output
    LIN_TXE = 1;            //enable LIN transmitter, set to zero for transmitter off mode enabling receiver
    LIN_CS = 1;              //enable LIN interface MCP2021A
    
    //
    // map LIN_TX pin to port RD6, which is remappable RP70
    // map LIN_RX pin to port RD8, which is remappable RPI72
    //
    RPOR8bits.RP70R = 0x01; // map LIN transmitter to pin RD6, hi byte
    _TRISD6 = 0;            // digital output pin

    RPINR18 = 0x48;         // map LIN receiver to pin RD8
    _TRISD8 = 1;            // digital input pin
    //
    // set up the UART for LIN_BRGVAL baud, 1 start, 1 stop, no parity
    //
    U1MODEbits.STSEL = 0;   // 1-Stop bit
    U1MODEbits.PDSEL = 0;   // No Parity, 8-Data bits
    U1MODEbits.ABAUD = 0;   // Auto-Baud disabled
    U1MODEbits.BRGH = 0;    // Standard-Speed mode
    U1BRG = LIN_BRGVAL;     // Baud Rate setting for 9600
    U1STAbits.UTXISEL0 = 1; // Interrupt after one TX done
    U1STAbits.UTXISEL1 = 0;
    IEC0bits.U1TXIE = 1;    // Enable UART TX interrupt
    U1MODEbits.UARTEN = 1;  // Enable UART (this bit must be set *BEFORE* UTXEN)

}
//
//void InitLIN_RX(void)
////
//// the LIN UART is UART1 of the 'GM106
////
//{
//    ANSEL_LIN = 0;
//    TRISLINCS = 0;
//    LIN_TXE = 0;                    // disable LIN transmitter
//    LIN_CS = 1;                     // enable LIN interface MCP2021A
//    //
//    // map LIN_RX pin to port RD8, which is remappable RPI72
//    RPINR18 = 0x48;                 // map LIN receiver to pin RD8
//    _TRISD8 = 1;                    // digital input pin
//    //
//    // set up the UART for LIN_BRGVAL baud, 1 start, 1 stop, no parity
//    //
//    U1MODEbits.STSEL = 0;           // 1-Stop bit
//    U1MODEbits.PDSEL = 0;           // No Parity, 8-Data bits
//    U1MODEbits.ABAUD = 0;           // Auto-Baud disabled
//    U1MODEbits.BRGH = 0;            // Standard-Speed mode
//    U1BRG = LIN_BRGVAL;             // Baud Rate setting
//    U1STAbits.URXISEL = 0;          // Interrupt after one RX done
//    IEC0bits.U1RXIE = 1;            // Enable UART1 RX interrupt
//    IEC4bits.U1EIE = 1;             // Enable Error (Framing) Interrupt for BREAK
//    U1MODEbits.UARTEN = 1;          // Enable UART1
//
//}

void LIN_Transmit(void)
{
    //
    // send break followed by 0x55 'autobaud' byte
    //
    while (U1STAbits.TRMT == 0);    // wait for transmitter empty
    while (U1STAbits.UTXBRK == 1);  // wait for HW to clear the previous BREAK
    U1STAbits.UTXEN = 1;            // Enable UART TX
    U1STAbits.UTXBRK = 1;           // set the BREAK bit
    /*Start bit, followed by twelve ?0? bits, followed by Stop
    bit; cleared by hardware upon completion   */
    /*    note that the total idle bit stuffing time cannot exceed 40% of the nominal time per LIN specification
           in this example, the nominal message frame is 92 total bits, with 28 (4 x 7) idle stuffed bits or 30.4%
           This time can be increased/decreased by changing the value for LIN_BIT_STUFF
     * checksum is inverted and includes IDbyte
     */
    U1TXREG = 0;                    // dummy write to trigger UART transmit
    Nop();                          // must wait 1 instruction cycle
    databyte = 0x55;
    U1TXREG = databyte;                 // AUTO-BAUD sync character per J2602 spec
    while (U1STAbits.TRMT == 0);    // wait for transmitter empty
    Delayus(LIN_BIT_TIME * LIN_BIT_STUFF);
    U1TXREG = LIN_ID;
    
    checksum = LIN_ID; 
    
    while (U1STAbits.TRMT == 0);    // wait for transmitter empty
    Delayus(LIN_BIT_TIME * LIN_BIT_STUFF);
    databyte = 0x12;
    U1TXREG = databyte;
    Calc_Checksum(databyte);
    while (U1STAbits.TRMT == 0);    // wait for transmitter empty
    Delayus(LIN_BIT_TIME * LIN_BIT_STUFF);
    databyte = 0x34;
    U1TXREG = databyte;
    Calc_Checksum(databyte);
    

    
    while (U1STAbits.TRMT == 0);    // wait for transmitter empty
    Delayus(LIN_BIT_TIME * LIN_BIT_STUFF);
    U1TXREG = (~checksum & 0xFF); //invert bit value
    
    /* above does the same thing as 
         checksum = (~checksum) & 0xFF; //precedence
    */
}