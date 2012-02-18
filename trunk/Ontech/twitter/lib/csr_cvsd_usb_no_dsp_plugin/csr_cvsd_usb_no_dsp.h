/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_cvsd_no_dsp.h

DESCRIPTION
    
    
NOTES
   
*/

#ifndef _CSR_CVSD_USB_NO_DSP_H_
#define _CSR_CVSD_USB_NO_DSP_H_

#include <message.h>

/*plugin functions*/
void CsrCvsdUsbNoDspPluginConnect( Sink audio_sink , AUDIO_SINK_T sink_type, Task codec_task , uint16 volume , uint32 rate, bool stereo , AUDIO_MODE_T mode , const void * params );
void CsrCvsdUsbNoDspPluginDisconnect( void ) ;
void CsrCvsdUsbNoDspPluginSetVolume( uint16 volume ) ;
void CsrCvsdUsbNoDspPluginSetMode ( AUDIO_MODE_T mode , const void * params ) ;
void CsrCvsdUsbNoDspPluginPlayTone ( audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo) ;
void CsrCvsdUsbNoDspPluginStopTone ( void ) ;
void CsrCvsdUsbNoDspPluginToneForceComplete ( void ) ;

/*Internal plugin functions*/         
void CsrCvsdUsbNoDspPluginToneComplete ( void ) ;

#define MESSAGE_FORCE_TONE_COMPLETE 0001

#endif

