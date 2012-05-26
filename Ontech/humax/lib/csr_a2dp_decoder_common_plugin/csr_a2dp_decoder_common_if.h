/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_a2dp_decoder_common_if.h

DESCRIPTION
    Message definitions for A2DP plugins.           
   
*/

/*!
@file csr_a2dp_decoder_common_if.h
  
@brief 
   Definitions for A2DP plugins.
  
   @description
    
      
*/

#ifndef _CSR_A2DP_DECODER_COMMON_INTERFACE_H_
#define _CSR_A2DP_DECODER_COMMON_INTERFACE_H_


typedef enum
{
	SBC_DECODER		= 1,
	MP3_DECODER		= 2,
	AAC_DECODER		= 3,
	FASTSTREAM_SINK	= 4
}A2DP_DECODER_PLUGIN_TYPE_T;


#define MESSAGE_SET_SAMPLE_RATE		(0x7050)

#define MUSIC_READY_MSG           (0x1000)
#define MUSIC_SETMODE_MSG         (0x1001)
#define MUSIC_VOLUME_MSG          (0x1002)
#define MUSIC_CODEC_MSG           (0x1006)
#define MUSIC_LOADPARAMS_MSG      (0x1012)
#define MUSIC_PS_BASE             8822

#define MUSIC_SYSMODE_PASSTHRU      1
#define MUSIC_SYSMODE_FULLPROC      2


#endif

