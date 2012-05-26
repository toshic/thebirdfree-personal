/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_cvc_common.h

DESCRIPTION
    
    
NOTES
   
*/

#ifndef _CSR_CVC_COMMON_H_
#define _CSR_CVC_COMMON_H_

/*plugin functions*/
void CsrCvcPluginConnect( CvcPluginTaskdata *task, Sink audio_sink ,AUDIO_SINK_T sink_type , Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params, bool mic_switch ) ;
void CsrCvcPluginDisconnect(CvcPluginTaskdata *task) ;
void CsrCvcPluginSetVolume( uint16 volume ) ;
void CsrCvcPluginSetMode ( AUDIO_MODE_T mode , const void * params ) ;
void CsrCvcPluginPlayTone (CvcPluginTaskdata *task, audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo) ;
void CsrCvcPluginStopTone ( void ) ;
void CsrCvcPluginMicSwitch ( CvcPluginTaskdata *task ) ;

/*internal plugin message functions*/
void CsrCvcPluginInternalMessage( CvcPluginTaskdata *task ,uint16 id , Message message ) ;

void CsrCvcPluginToneComplete (void ) ;
#endif

