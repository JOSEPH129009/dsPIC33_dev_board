/* 
 * File:   constants.h
 * Author: josephjoe1209
 *
 * Created on November 20, 2022, 9:47 AM
 */

#ifndef CONSTANTS_H
#define	CONSTANTS_H

#include <xc.h>

#include "p33EV256GM106.h" // include processor files - each processor file is guarded.  


#define LED1    LATCbits.LATC4
#define LED2    LATCbits.LATC5
#define LED3    LATCbits.LATC6

enum{
    CAN_MSG_DATA,
    CAN_MSG_RTR,
    CAN_FRAME_EXT,
    CAN_FRAME_STD,
    CAN_BUF_FULL,
    CAN_BUF_EMPTY
};

#define NUM_OF_BUFFERS_IN_USE  2
#define NUM_OF_ECAN_BUFFERS 32
#define MSG_SID01 0x123              // the arbitrary CAN SID of the transmitted message
#define MSG_SID02 0x19 

/* CAN filter and mask defines */
/* Macro used to write filter/mask ID to Register CiRXMxSID and
CiRXFxSID. For example to setup the filter to accept a value of
0x123, the macro when called as CAN_FILTERMASK2REG_SID(0x123) will
write the register space to accept message with ID 0x123
USE FOR STANDARD MESSAGES ONLY */
/*shift ID bits to 5 position to match the register*/
#define CAN_FILTERMASK2REG_SID(x) ((x & 0x07FF)<< 5)
/* the Macro will set the "MIDE" bit in CiRXMxSID */
#define CAN_SETMIDE(sid) (sid | 0x0008)
/* the macro will set the EXIDE bit in the CiRXFxSID to
accept extended messages only */
#define CAN_FILTERXTD(sid) (sid | 0x0008)
/* the macro will clear the EXIDE bit in the CiRXFxSID to
accept standard messages only */
#define CAN_FILTERSTD(sid) (sid & 0xFFF7)
// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

// TODO Insert declarations

// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation
#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* CONSTANTS_H */

