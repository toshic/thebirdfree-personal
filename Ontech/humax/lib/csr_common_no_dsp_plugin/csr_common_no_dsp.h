/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_common_no_dsp.h

DESCRIPTION


NOTES

*/

#ifndef _CSR_COMMON_NO_DSP_H_
#define _CSR_COMMON_NO_DSP_H_

/*plugin functions*/
void CsrNoDspPluginConnect( NoDspPluginTaskdata *task, Sink audio_sink ,AUDIO_SINK_T sink_type , Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params ) ;
void CsrNoDspPluginDisconnect(NoDspPluginTaskdata *task) ;
void CsrNoDspPluginSetVolume( uint16 volume ) ;
void CsrNoDspPluginSetMode ( AUDIO_MODE_T mode , const void * params ) ;
void CsrNoDspPluginPlayTone (NoDspPluginTaskdata *task, audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo) ;
void CsrNoDspPluginStopTone (  void) ;
void CsrNoDspPluginToneForceComplete ( void ) ;

#define MESSAGE_FORCE_TONE_COMPLETE 0001

/*internal plugin message functions*/
void CsrNoDspPluginToneComplete (NoDspPluginTaskdata *task) ;

#endif

