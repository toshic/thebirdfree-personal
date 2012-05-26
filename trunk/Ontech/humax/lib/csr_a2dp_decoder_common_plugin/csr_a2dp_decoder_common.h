/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_a2dp_decoder_common.h

DESCRIPTION
    
    
NOTES
   
*/
 
#ifndef _CSR_A2DP_DECODER_COMMON_H_
#define _CSR_A2DP_DECODER_COMMON_H_


/*plugin functions*/
void CsrA2dpDecoderPluginConnect( A2dpPluginTaskdata *task, Sink audio_sink, Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params , Task app_task ) ;
void CsrA2dpDecoderPluginDisconnect( void ) ;
void CsrA2dpDecoderPluginSetVolume(A2dpPluginTaskdata *task, uint16 volume ) ;
void CsrA2dpDecoderPluginSetMode( AUDIO_MODE_T mode , A2dpPluginTaskdata *task , const void * params ) ;
void CsrA2dpDecoderPluginPlayTone ( A2dpPluginTaskdata *task, audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo) ;
void CsrA2dpDecoderPluginStopTone ( void ) ;
void CsrA2dpDecoderPluginToneComplete ( void ) ;

/*internal plugin message functions*/
void CsrA2dpDecoderPluginInternalMessage( A2dpPluginTaskdata *task ,uint16 id , Message message ) ;

#endif


