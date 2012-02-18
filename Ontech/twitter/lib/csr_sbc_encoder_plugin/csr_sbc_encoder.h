/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    sbc_encoder_plugin.h

DESCRIPTION
    
    
NOTES
   
*/

#ifndef _CSR_SBC_ENCODER_H_
#define _CSR_SBC_ENCODER_H_


void CsrSbcEncoderPluginConnect( Sink audio_sink , Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params ) ;
void CsrSbcEncoderPluginDisconnect( void ) ;
void CsrSbcEncoderPluginSetVolume( uint16 volume ) ;
void CsrSbcEncoderPluginSetMode ( AUDIO_MODE_T mode , const void * params ) ;
void CsrSbcEncoderPluginPlayTone ( audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo ) ;
void CsrSbcEncoderPluginStopTone ( void ) ;

    /*the tone has completed*/
void CsrSbcEncoderPluginToneComplete ( void ) ; 

#endif /* _CSR_SBC_ENCODER_H_ */

