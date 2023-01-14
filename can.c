
/* 
 * File:   can.c
 * Author: bug70
 *
 * Created on 2022?1?2?, ?? 11:57
 */

#include <stdio.h>
#include <stdlib.h>
#include "can.h"
#include "constants.h"
#include "variables.h"

/* 
 * 
 */



void InitCAN(void)
{
   //
   // drive the CAN STANDBY driver pin low
   //
   _TRISG9 = 0;
   _LATG9 = 0;
   _TRISF1 = 0;
   _TRISF0 = 1;

   //
   // remap the CAN module to the proper pins on the board
   //
   RPINR26 = 0x60;         // connect CAN RX to RPI96
   RPOR9 = 0x000E;         // connect CAN TX to RP97

   C1CTRL1bits.REQOP = 4;//CAN controller register, set configuration mode

   while (C1CTRL1bits.OPMODE != 4);
   C1CTRL1bits.WIN = 0; // 0 = uses buffer window, 1 = uses filter window

   /* Set up the CAN module for 250kbps speed with 10 Tq per bit. */


   C1CFG1 = 0x71; //SJW = 2 TQ, 50*2*1/Fcyc
   /*1TQ = 25us
    * CAN Baud = 40khz
    */
   
   C1CFG2 = 0x292; //Bus line sampled once at sample point
   /*
    * Prop = 3TQ
    * Phase Seg1 = 3TQ
    * Phase Seg1 = 3TQ
    * Sample point = 70%
    */
   C1FCTRL = 0xC01F; //no FIFO, 32 buffers
   /*  when C1FCTRL = 0xC01F
    *  32 buffers in RAM
    *  FIFO area starts with buffer bits
    *  Receive Buffer RB31 = 32buffers
    */
   
   //
   // set up the CAN DMA0 for the Transmit Buffer
   //
   DMA0CONbits.SIZE = 0x0;
   DMA0CONbits.DIR = 0x1;
   DMA0CONbits.AMODE = 0x2;
   DMA0CONbits.MODE = 0x0;
   DMA0REQ = 70;
   DMA0CNT = 7;
   DMA0PAD = (volatile unsigned int)&C1TXD;
   DMA0STAL = (unsigned int)&ecan1MsgBuf;
   DMA0STAH = (unsigned int)&ecan1MsgBuf;

   C1TR01CONbits.TXEN0 = 0x1;          // Buffer 0 is the Transmit Buffer = TRB0 is a transmit buffer
   C1TR01CONbits.TX0PRI = 0x3;         // transmit buffer priority: Highest Message Priortiy

   DMA0CONbits.CHEN = 0x1;

    /* initialise the DMA channel 2 for ECAN Rx */

    /* setup channel 2 for peripheral indirect addressing mode
    normal operation, word operation and select as Rx to peripheral */
    DMA2CON = 0x0020;
    /* setup the address of the peripheral ECAN1 (C1RXD) */
    DMA2PAD = (volatile unsigned int)&C1RXD;
    /* Set the data block transfer size of 8 */
    DMA2CNT = 7;
    /* automatic DMA Rx initiation by DMA request */
    DMA2REQ = 0x0022;
    /* start adddress offset value */
    DMA2STAL=(unsigned int)(&ecan1MsgBuf);
    DMA2STAH=(unsigned int)(&ecan1MsgBuf);
    /* enable the channel */
    DMA2CONbits.CHEN=1;

    /*000 = 4 buffers in RAM, cab be set up to 32 buffers*/
    /* 4 CAN Messages to be buffered in DMA RAM */
    C1FCTRLbits.DMABS=0b000;

    
    /* Filter configuration */
    /* enable window to access the filter configuration registers */
    C1CTRL1bits.WIN = 0b1;
    
/*======Filter 0 Mask0 for buffer1===============================*/
    /*0 = select acceptance mask 0 for this filter 0, only three registers for acceptance mask0~2*/
    /* select acceptance mask 0 filter 0 buffer 1 */
    C1FMSKSEL1bits.F0MSK = 0;

    /* setup the mask to check every bit of the standard message, the macro when called as */
    /* CAN_FILTERMASK2REG_SID(0x7FE) will write the register C1RXM0SID to include every bit in */
    /* filter comparison except last bit*/
    C1RXM0SID=CAN_FILTERMASK2REG_SID(0x7FE); 
   
    /* configure accpetence filter 0
    setup the filter to accept a standard id of 0x123,
    the macro when called as CAN_FILTERMASK2REG_SID(0x123) will
    write the register C1RXF0SID to accept only standard id of 0x123 & 0x122
    */
    C1RXF0SID = CAN_FILTERMASK2REG_SID(MSG_SID01);
    /* set filter to check for standard ID and accept standard id only */
    C1RXM0SID = CAN_SETMIDE(C1RXM0SID); //add IDE bit to bit4 to care about this bit
    C1RXF0SID = CAN_FILTERSTD(C1RXF0SID); //clear IDE bit so it will only take standard frame
    /* acceptance filter 0 to use buffer 1 for incoming messages */
    C1BUFPNT1bits.F0BP = 0b0001;
    /* enable filter 0 */
    C1FEN1bits.FLTEN0 = 1;
/*=====================================================*/

/*======Filter 1 Mask 1 for buffer2===============================*/
    /*1 = select acceptance mask 1 for this filter 1, only three registers for acceptance mask0~2*/
    /* select acceptance mask 1 filter 1 buffer 2 */
    C1FMSKSEL1bits.F1MSK = 1;

    /* setup the mask to check every bit of the standard message, the macro when called as */
    /* CAN_FILTERMASK2REG_SID(0x7FF) will write the register C1RXM1SID to include every bit in */
    /* filter comparison */
    C1RXM1SID=CAN_FILTERMASK2REG_SID(0x7FF); //care about all SID bits
   
    /* configure accpetence filter 1
    setup the filter to accept a standard id of 0x19,
    the macro when called as CAN_FILTERMASK2REG_SID(0x19) will
    write the register C1RXF1SID to accept only standard id of 0x19
    */
    C1RXF1SID = CAN_FILTERMASK2REG_SID(MSG_SID02);
    /* set filter to check for standard ID and accept standard id only */
    C1RXM1SID = CAN_SETMIDE(C1RXM1SID); //add IDE bit to bit4 to care about this bit
    C1RXF1SID = CAN_FILTERSTD(C1RXF1SID); //clear IDE bit so it will only take standard frame
    /* acceptance filter 1 to use buffer 2 for incoming messages */
    C1BUFPNT1bits.F1BP = 0b0010;
    /* enable filter 1 */
    C1FEN1bits.FLTEN1 = 1;
/*=====================================================*/
    
    
    
    /* clear window bit to access ECAN control registers */
    C1CTRL1bits.WIN = 0;

    /* ECAN1, Buffer 1 is a Receive Buffer */
    C1TR01CONbits.TXEN1 = 0;

    /* ECAN1, Buffer 2 is a Receive Buffer */
    C1TR23CONbits.TXEN2 = 0;


    /* clear the buffer and overflow flags */
    C1RXFUL1=C1RXFUL2=C1RXOVF1=C1RXOVF2=0x0000;

   // Place the ECAN module in Normal mode.
   C1CTRL1bits.REQOP = 0;
   while (C1CTRL1bits.OPMODE != 0);

   //
   // CAN RX interrupt enable - 'double arm' since 2-level nested interrupt
   //
   C1INTEbits.RBIE = 1;
   IEC2bits.C1IE = 1;
}

/******************************************************************************
*
*    Function:			rxECAN
*    Description:       moves the message from the DMA memory to RAM
*
*    Arguments:			*message: a pointer to the message structure in RAM
*						that will store the message.
******************************************************************************/
void rxECAN(mID *message)
{
	unsigned int ide=0;
	unsigned int rtr=0;
	unsigned long id=0;

	/*
	Standard Message Format:
	Word0 : 0bUUUx xxxx xxxx xxxx
			     |____________|||
 					SID10:0   SRR IDE(bit 0)
	Word1 : 0bUUUU xxxx xxxx xxxx
			   	   |____________|
						EID17:6
	Word2 : 0bxxxx xxx0 UUU0 xxxx
			  |_____||	     |__|
			  EID5:0 RTR   	  DLC
	word3-word6: data bytes
	word7: filter hit code bits

	Remote Transmission Request Bit for standard frames
	SRR->	"0"	 Normal Message
			"1"  Message will request remote transmission
	Substitute Remote Request Bit for extended frames
	SRR->	should always be set to "1" as per CAN specification

	Extended  Identifier Bit
	IDE-> 	"0"  Message will transmit standard identifier
	   		"1"  Message will transmit extended identifier

	Remote Transmission Request Bit for extended frames
	RTR-> 	"0"  Message transmitted is a normal message
			"1"  Message transmitted is a remote message
	Don't care for standard frames
	*/

	/* read word 0 to see the message type */
	ide=ecan1MsgBuf[message->buffer][0] & 0x0001;

	/* check to see what type of message it is */
	/* message is standard identifier */
	if(ide==0)
	{
		message->id=(ecan1MsgBuf[message->buffer][0] & 0x1FFC) >> 2;
		message->frame_type=CAN_FRAME_STD;
		rtr=ecan1MsgBuf[message->buffer][0] & 0x0002;
	}
	/* mesage is extended identifier */
	else
	{
		id=ecan1MsgBuf[message->buffer][0] & 0x1FFC;
		message->id=id << 16;
		id=ecan1MsgBuf[message->buffer][1] & 0x0FFF;
		message->id=message->id+(id << 6);
		id=(ecan1MsgBuf[message->buffer][2] & 0xFC00) >> 10;
		message->id=message->id+id;
		message->frame_type=CAN_FRAME_EXT;
		rtr=ecan1MsgBuf[message->buffer][2] & 0x0200;
	}
	/* check to see what type of message it is */
	/* RTR message */
	if(rtr==1)
	{
		message->message_type=CAN_MSG_RTR;
	}
	/* normal message */
	else
	{
		message->message_type=CAN_MSG_DATA;
		message->data[0]=(unsigned char)ecan1MsgBuf[message->buffer][3];
		message->data[1]=(unsigned char)((ecan1MsgBuf[message->buffer][3] & 0xFF00) >> 8);
		message->data[2]=(unsigned char)ecan1MsgBuf[message->buffer][4];
		message->data[3]=(unsigned char)((ecan1MsgBuf[message->buffer][4] & 0xFF00) >> 8);
		message->data[4]=(unsigned char)ecan1MsgBuf[message->buffer][5];
		message->data[5]=(unsigned char)((ecan1MsgBuf[message->buffer][5] & 0xFF00) >> 8);
		message->data[6]=(unsigned char)ecan1MsgBuf[message->buffer][6];
		message->data[7]=(unsigned char)((ecan1MsgBuf[message->buffer][6] & 0xFF00) >> 8);
		message->data_length=(unsigned char)(ecan1MsgBuf[message->buffer][2] & 0x000F);
	}
	clearRxFlags(message->buffer);
}

/******************************************************************************
*
*    Function:			clearRxFlags
*    Description:       clears the rxfull flag after the message is read
*
*    Arguments:			buffer number to clear
******************************************************************************/
void clearRxFlags(unsigned char buffer_number)
{
	if((C1RXFUL1bits.RXFUL1) && (buffer_number==1))
		/* clear flag */
		C1RXFUL1bits.RXFUL1=0;
	/* check to see if buffer 2 is full */
	else if((C1RXFUL1bits.RXFUL2) && (buffer_number==2))
		/* clear flag */
		C1RXFUL1bits.RXFUL2=0;
	/* check to see if buffer 3 is full */
	else if((C1RXFUL1bits.RXFUL3) && (buffer_number==3))
		/* clear flag */
		C1RXFUL1bits.RXFUL3=0;
	else;

}

#define MSG_SID 0x22

void CAN_Transmit(void)
{
    ecan1MsgBuf[0][0] = MSG_SID << 2;

    ecan1MsgBuf[0][1] = 0x0000;
    /* CiTRBnDLC = 0b0000 0000 xxx0 1111
    EID<17:6> = 0b000000
    RTR = 0b0
    RB1 = 0b0
    RB0 = 0b0
    DLC = 6 */
    ecan1MsgBuf[0][2] = 0x0006;

    // Write message 6 data bytes as follows:
    //
    // POTH POTL TEMPH TEMPL 0000 S3S2S1
    //
    ecan1MsgBuf[0][3] = 0x9876;
    ecan1MsgBuf[0][4] = 0x5432;
    ecan1MsgBuf[0][5] = 0x10FE;

    Nop();
    Nop();
    Nop();
    /* Request message buffer 0 transmission */
    C1TR01CONbits.TXREQ0 = 0x1;
    /* The following shows an example of how the TXREQ bit can be polled to check if transmission
    is complete. */
    Nop();
    Nop();
    Nop();
    while (C1TR01CONbits.TXREQ0 == 1);
    // Message was placed successfully on the bus, return
}