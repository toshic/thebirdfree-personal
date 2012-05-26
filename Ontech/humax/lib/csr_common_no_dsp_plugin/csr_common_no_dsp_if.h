/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_common_no_dsp_if.h

DESCRIPTION

*/

#ifndef _CSR_COMMON_NO_DSP_INTERFACE_H_
#define _CSR_COMMON_NO_DSP_INTERFACE_H_



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

/* Values for the selecting the plugin variant in the NoDspPluginTaskdata structure  */
typedef enum
{

    CVSD_NO_DSP         =   0,

    AURI_2BIT_NO_DSP    =   1,

    AURI_4BIT_NO_DSP    =   2

}NO_DSP_PLUGIN_TYPE_T;

/* Different types of SCO stream data encoders  */
typedef enum
{
    SCO_ENCODING_CVSD       =   0,

    SCO_ENCODING_AURI       =   1,

    SCO_ENCODING_SBC        =   2

}SCO_ENCODING_TYPE_T;

/* The CODEC type selected from VM  application */
typedef enum
{
    AUDIO_CODEC_CVSD        =   1,

    AUDIO_CODEC_2BIT_AURI   =   2,

    AUDIO_CODEC_4BIT_AURI   =   4

}CODEC_TYPE_T;

#endif

