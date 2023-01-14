
/*
 * File:   main.c
 * Author: bug70
 *
 * Created on 2021?10?24?, ?? 3:36
 */

/*int size: 16bit*/

#include "xc.h"
#include "main.h"
#include "p33EV256GM106.h"
#include "constants.h"
#include "stdio.h"
#include "uart.h"
#include <libpic30.h>  //for link to __30_UART

/*above includes the header p33EV256GM106.h*/

/*Device is running at 40Mhz = 40MIPS*/
//  Macros for Configuration Fuse Registers 
#pragma config FNOSC = PRIPLL  //Primary Oscillator are based on OSC1 & OCS2 
#pragma config FCKSM = CSDCMD
#pragma config OSCIOFNC = OFF
#pragma config POSCMD = XT
// Startup directly into XT + PLL
//  XT = medium speed freqency 3.5mhz - 10mhz 

#pragma config FWDTEN = OFF //Watchdog Timer disable
#pragma config ICS = PGD2     //PGD3 for external PK3/ICD3/RealIce, use PGD2 fir PKOB
#pragma config BOREN0 = OFF // no brownout detect
#pragma config DMTEN = DISABLE //no deadman timer <<<< **** New feature, important tto DISABLE



void Init_HW(void);
void ClrIntrFlag(void);
void ocsConfig(void);


uint32_t delay = 1000000;
int DLC = 0;

int main(void) {
    int i, j;
    ocsConfig();
    ClrIntrFlag();
    Init_HW();
    InitCAN();
    UART2_Init();
    InitLIN_TX();
    __C30_UART = 2; // direct UART2 to printf 
    while (1) {
        for (i = 0; i < NUM_OF_BUFFERS_IN_USE; i++)
            if (CanRxMessage[i].buffer_status == CAN_BUF_FULL) {
                rxECAN(&CanRxMessage[i]);
                /* reset the flag when done */
                CanRxMessage[i].buffer_status = CAN_BUF_EMPTY;

                if (CanRxMessage[i].id == 0x123) {
                    /*LED testing*/
                    if (CanRxMessage[i].data[0] == 0x12)
                        LED1 ^= 1;
                    if (CanRxMessage[i].data[0] == 0x34)
                        LED2 ^= 1;
                    if (CanRxMessage[i].data[0] == 0x56)
                        LED3 ^= 1;
                }

//                printf("ID: %X", (int) CanRxMessage[i].id);
//                DLC = CanRxMessage[i].data_length;
//                for (j = 0; j < DLC; j++)
//                    printf(" %2X", CanRxMessage[i].data[j]);
//                printf("\r\n");
            }


        //        /*U2TXREG is an 8 bit register*/
        //        U2STAbits.UTXEN = 1;
        //        U2TXREG = 0x34;
        //        while(U2STAbits.TRMT == 0);
        //        U2TXREG = 0x78;
        //        while(U2STAbits.TRMT == 0); //transmit shift register is not empty

        //        if(s_tick>=2) // 500ms
        //        {
        //                LIN_Transmit();
        //                s_tick = 0;
        //        } 
        if (read_pinstatus(&PORTC, 0x80))
            CAN_Transmit();
//            LIN_Transmit();

    }


    return 0;
}

void ocsConfig(void) {
    //

    /*
       Configure Oscillator to operate the device at 80MHz/40MIPs
        Fosc= Fin*M/(N1*N2), Fcy=Fosc/2,
     * Fcyc = 40MHz for ECAN baud timer
        Fosc= 8M*40/(2*2)=80Mhz for 8M input clock
       To be safe, always load divisors before feedback
     *M stands for PLLDIV[8:0], default at 50? or 40?
     */
    CLKDIVbits.PLLPOST = 0; //N1 =2
    CLKDIVbits.PLLPRE = 0; //N2 =2
    PLLFBD = 38; // M=(40-2)
    // Disable Watch Dog Timer
    RCONbits.SWDTEN = 0;

}

void Init_HW(void) {
    ANSELC = ANSELC & 0xFC3F; // (re)set the 3 switch bits + CAN due to error in v1.20 header
    //
    // Timer 1 to generate an interrupt every 250ms
    //
    T1CONbits.TON = 1; // enable Timer1
    T1CONbits.TCS = 0; // Select internal instruction cycle clock
    T1CONbits.TGATE = 0; // Disable Gated Timer mode
    T1CONbits.TCKPS = 0x3; // Select 1:256 Prescaler
    PR1 = 39062; // Load the period value (250ms/(256*25ns))
    IPC0bits.T1IP = 0x03; // Set Timer 1 Interrupt Priority Level
    IFS0bits.T1IF = 0; // Clear Timer 1 Interrupt Flag
    IEC0bits.T1IE = 1; // Enable Timer1 interrupt

    /*set output for LED1 ~ 3 */
    TRISCbits.TRISC4 = 0;
    TRISCbits.TRISC5 = 0;
    TRISCbits.TRISC6 = 0;
    /*initialize to all off*/
    LED1 = 0;
    LED2 = 0;
    LED3 = 0;

}

void ClrIntrFlag(void) {

    /*Clear Interrupt Flags*/
    IFS0 = 0;
    IFS1 = 0;
    IFS2 = 0;
    IFS3 = 0;
    IFS4 = 0;
    /*set interrupt priority*/
    IPC16bits.U1EIP = 6; // service the LIN framing error before the RX
    IPC2bits.U1RXIP = 4;
}

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    while (U1STAbits.TRMT == 0); // wait for transmitter empty
    IFS0bits.U1TXIF = 0; // Clear TX1 Interrupt flag
}

//void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt(void)
//{
//    while (U2STAbits.TRMT == 0); // wait for transmitter empty
//    IFS1bits.U2TXIF = 0; // Clear TX2 Interrupt flag
//}

//void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void)
//{
//    //
//    // LIN message received
//    //
//    //
//    // is it a SYNCH (0x55) which is start of message?
//    //
//    // A Framing Error *must* immediately proceed!!
//    //
//       datal = U1RXREG;
//    if ((datal == 0x55) && (lin_start == 1) && (lin_index == 0))
//    {
//        lin_start = 2;
//        LIN_RXBUF[lin_index] = datal & 0xFF;    // get the SYNCH byte for fun
//        lin_index++;                            // ready for ID byte reveive
//
//    }
//        //
//        // is part of the LIN message
//        //
//
//    else if (lin_start == 2)
//    {
//        LIN_RXBUF[lin_index] = datal & 0x00FF;   // get the data byte
//        lin_index++;
//        if (lin_index == LIN_MESSAGE_SIZE)
//        {
//            lin_rx = 1;
//            lin_index = 0;
//            lin_start = 0;
//        }
//    }
//    IFS0bits.U1RXIF = 0;                // Clear RX1 Interrupt flag
//}

//void __attribute__((interrupt, no_auto_psv)) _U1ErrInterrupt(void)
//{
//    //
//    // a LIN 'BREAK' (13 consecutive '0's) will generate a Framing Error
//    // ***NOTE*** This ISR MUST be at a higher priority than U1RX ISR in order
//    // to test for framing error prior to testing for SYNC byte
//    //
//
//    if (U1STAbits.FERR == 1)
//    {
//        lin_start = 1;          // first message detection phase
//    }
//    IFS4bits.U1EIF = 0;         // Clear LIN Error Flag
//}

/* code for Timer1 ISR, called every 250ms*/
void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    s_tick++; // increment the 'slow tick'

    IFS0bits.T1IF = 0; //Clear Timer1 interrupt flag

}

void __attribute__((interrupt, no_auto_psv))_C1Interrupt(void) {
    IFS2bits.C1IF = 0; // clear interrupt flag
    if (C1INTFbits.TBIF) {
        C1INTFbits.TBIF = 0;
    }

    if (C1INTFbits.RBIF) {

        /*check to see if buffer 1 is full */
        if (C1RXFUL1bits.RXFUL1) {
            /* set the buffer full flag and the buffer received flag */
            CanRxMessage[0].buffer_status = CAN_BUF_FULL;
            CanRxMessage[0].buffer = 1; //buffer 1 for ecanMsgBuf
        }

        /*check to see if buffer 2 is full */
        if (C1RXFUL1bits.RXFUL2) {
            /* set the buffer full flag and the buffer received flag */
            CanRxMessage[1].buffer_status = CAN_BUF_FULL;
            CanRxMessage[1].buffer = 2; //buffer 2 for ecanMsgBuf
        }
        C1INTFbits.RBIF = 0;
    }
}
//------------------------------------------------------------------------------
//    DMA interrupt handlers
//------------------------------------------------------------------------------

void __attribute__((interrupt, no_auto_psv)) _DMA0Interrupt(void) {
    IFS0bits.DMA0IF = 0; // Clear the DMA0 Interrupt Flag;
}

void __attribute__((interrupt, no_auto_psv)) _DMA1Interrupt(void) {
    IFS0bits.DMA1IF = 0; // Clear the DMA1 Interrupt Flag;
}

void __attribute__((interrupt, no_auto_psv)) _DMA2Interrupt(void) {
    IFS1bits.DMA2IF = 0; // Clear the DMA2 Interrupt Flag;
}

void __attribute__((interrupt, no_auto_psv)) _DMA3Interrupt(void) {
    IFS2bits.DMA3IF = 0; // Clear the DMA3 Interrupt Flag;
}

void Delayus(int delay) {
    int i;
    for (i = 0; i < delay; i++) {
        __asm__ volatile ("repeat #39");
        __asm__ volatile ("nop");
    }
}

void Calc_Checksum(int data) {
    checksum = checksum + data;
    if (checksum > 0xFF) {
        checksum = (checksum & 0xFF) + 1; // addd one carry bit
    }
}

int read_pinstatus(volatile uint16_t* port, uint16_t pin) {
    static uint8_t flag = 0;
    if (Sub_ReadPin(port, pin) == PINSET && flag == 0) {
        debounce_timer = s_tick;
        flag = 1;
    }


    if (s_tick > debounce_timer && flag == 1) {
        if (s_tick - debounce_timer >= 2 && (Sub_ReadPin(port, pin) == PINSET)) {
            flag = 0;
            return 1;
        }
    } else if (s_tick < debounce_timer && flag == 1) {

        if (((0xFFFF - s_tick + 1) - debounce_timer) >= 2 && (Sub_ReadPin(port, pin) == PINSET)) {
            flag = 0;
            return 1;
        }
    }

    return 0;
}

PINSTATE Sub_ReadPin(volatile uint16_t* port, uint16_t pin) {
    PINSTATE bitstatus;
    if ((*port & pin) == 0) {
        bitstatus = PINSET;
    } else
        bitstatus = PINRESET;

    return bitstatus;
}


