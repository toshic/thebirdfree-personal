/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    tone.h
DESCRIPTION
    plugin implentation which plays tones
NOTES
*/

#include <codec.h>
#include <pcm.h>
#include <stdlib.h>
#include <panic.h>
#include <stream.h>
#include <print.h>
#include <stream.h> 
#include <message.h> 
                                                                                                                    
#include "audio_plugin_if.h" /*for the audio_mode*/
#include "csr_tone.h"
#include "csr_tone_plugin.h"

/****************************************************************************
DESCRIPTION
    plays a tone using the audio plugin    
*/
void CsrTonePluginPlayTone ( audio_note * tone, Task codec_task, uint16 tone_volume , bool stereo) 
{    
    Source lSource ;
    Sink lSink = NULL ;
    
    CodecSetOutputGainNow( codec_task, 0 , left_and_right_ch );   

        /*clear all routing to the PCM* subsysytem*/
    PcmClearAllRouting() ;
    
    if (stereo)
 		PcmRateAndRoute( 0, PCM_NO_SYNC, 8000, 8000, VM_PCM_INTERNAL_A_AND_B ) ;
 	else	
 		PcmRateAndRoute( 0, PCM_NO_SYNC, 8000, 8000, VM_PCM_INTERNAL_A ) ;    
    
	StreamDisconnect(StreamPcmSource(0), 0); 
        /* connect the tone to the DACs*/
    lSink = StreamPcmSink(0) ;        
        /*request an indication that the tone has completed / been disconnected*/
    MessageSinkTask ( lSink , (TaskData*)&csr_tone_plugin ) ;
        
    	/*connect the tone*/		
	lSource = StreamAudioSource ( (const audio_note *) tone ) ;    
            
    PanicFalse( StreamConnect( lSource , lSink ) );
    
    CodecSetOutputGainNow( codec_task, tone_volume, left_and_right_ch );                
}

/****************************************************************************
DESCRIPTION
	Stop a tone from currently playing
*/
void CsrTonePluginStopTone ( void ) 
{  
	
    PRINT(("TONE: Terminated\n"));
	StreamDisconnect(0, StreamPcmSink(0)); 
}

/****************************************************************************
DESCRIPTION
	a tone has completed
*/
void CsrTonePluginToneComplete (void )
{
        /*we no longer want stream indications*/
    MessageSinkTask ( StreamPcmSink(0) , NULL ) ;
}

