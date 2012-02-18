/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    mp3_encoder_plugin.h

DESCRIPTION
    
    
NOTES
   
*/

#ifndef _CSR_MP3_ENCODER_H_
#define _CSR_MP3_ENCODER_H_


void CsrMp3EncoderPluginConnect( Sink audio_sink , Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params ) ;
void CsrMp3EncoderPluginDisconnect( void ) ;
void CsrMp3EncoderPluginSetVolume( uint16 volume ) ;
void CsrMp3EncoderPluginSetMode ( AUDIO_MODE_T mode , const void * params ) ;
void CsrMp3EncoderPluginPlayTone ( audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo ) ;
void CsrMp3EncoderPluginStopTone ( void ) ;

    /*the tone has completed*/
void CsrMp3EncoderPluginToneComplete ( void ) ; 

#endif /* _CSR_MP3_ENCODER_H_ */

