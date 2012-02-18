/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_faststream_source.h

DESCRIPTION
    
    
NOTES
   
*/

#ifndef _CSR_FASTSTREAM_SOURCE_H_
#define _CSR_FASTSTREAM_SOURCE_H_


void CsrFaststreamSourcePluginConnect( Sink audio_sink , Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params ) ;
void CsrFaststreamSourcePluginDisconnect( void ) ;
void CsrFaststreamSourcePluginSetVolume( uint16 volume ) ;
void CsrFaststreamSourcePluginSetMode ( AUDIO_MODE_T mode , const void * params ) ;
void CsrFaststreamSourcePluginPlayTone ( audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo ) ;
void CsrFaststreamSourcePluginStopTone ( void ) ;

    /*the tone has completed*/
void CsrFaststreamSourcePluginToneComplete ( void ) ; 

#endif

