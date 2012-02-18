/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_sco_loopback_if.h

DESCRIPTION
   
*/

#ifndef _CSR_SCO_LOOPBACK_INTERFACE_H_
#define _CSR_SCO_LOOPBACK_INTERFACE_H_


  
#define    MESSAGE_SETMODE       0x0004 

#define    MESSAGE_SCO_CONFIG  0x2000
 

#define    SYSMODE_PSTHRGH     0

#define MINUS_45dB         0x0         /* value used with SetOutputGainNow VM trap */

#define CODEC_MUTE           MINUS_45dB

/* dsp message structure*/
typedef struct
{
    uint16 id;
    uint16 a;
    uint16 b;
    uint16 c;
    uint16 d;
} DSP_REGISTER_T;


#endif

