/* 
 * File:   can.h
 * Author: josephjoe1209
 *
 * Created on November 20, 2022, 9:46 AM
 */

#ifndef CAN_H
#define	CAN_H

#include <xc.h> // include processor files - each processor file is guarded.  
/*CAN receive struct in RAM*/
typedef struct{
    /*keep track of buffer status*/
    unsigned char buffer_status;
    /* RTR message or data message*/
    unsigned char message_type;
    /* frame type extended or standard*/
    unsigned char frame_type;
    /*buffer being used to send and receive message*/
    unsigned char buffer;
    /*
     * 29 bit id max of 0x1FFF FFFF
     * 11 bit id max of 0x7FF
     */
    unsigned long id;
    unsigned int data[8];
    unsigned char data_length;
}mID;


void InitCAN(void);
void rxECAN(mID *message);
void clearRxFlags(unsigned char buffer_number);
void CAN_Transmit(void);

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* CAN_H */

