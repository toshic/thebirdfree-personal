/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_cvsd_no_dsp.c
DESCRIPTION

NOTES
*/

#include <codec.h>
#include <pcm.h>
#include <stdlib.h>
#include <panic.h>
#include <stream.h>
#include <print.h>
#include <stream.h> /*for the audio_note*/


#include "audio_plugin_if.h" /*for the audio_mode*/
#include "csr_cvsd_usb_no_dsp.h"
#include "csr_cvsd_usb_no_dsp_plugin.h"

static void DisconnectMic ( void );
static void DisconnectSpeaker ( void );
static void ConnectMic ( void );
static void ConnectSpeaker ( void ) ;

typedef struct sync_Tag
{
    unsigned volume:8 ;
    unsigned mode:8 ;
    /*! The audio sink being used*/
    Sink audio_sink ;    
    Task codec_task ;
    
}SYNC_USB_t ;

    /*the synchronous audio data structure*/
static SYNC_USB_t * SYNC_USB = NULL ;

/****************************************************************************
DESCRIPTION
	This function connects a synchronous audio stream to the pcm subsystem
*/ 
void CsrCvsdUsbNoDspPluginConnect ( Sink audio_sink , AUDIO_SINK_T sink_type, Task codec_task , uint16 volume ,uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params )
{
    if (! SYNC_USB) 
    {
        SYNC_USB = PanicUnlessNew ( SYNC_USB_t ) ;
    }
    
    SYNC_USB->codec_task = codec_task ; 
    SYNC_USB->audio_sink = audio_sink ;

    StreamDisconnect(StreamPcmSource(0), StreamPcmSink(0));
    StreamDisconnect(StreamPcmSource(1), StreamPcmSink(1));

	StreamDisconnect(StreamPcmSource(0), SYNC_USB->audio_sink);
	StreamDisconnect(StreamSourceFromSink(SYNC_USB->audio_sink), StreamPcmSink(0));

	(void)PcmClearAllRouting();
    
	if (stereo)
 		PcmRateAndRoute( 0, PCM_NO_SYNC, 8000, 8000, VM_PCM_INTERNAL_A_AND_B ) ;
 	else	
 		PcmRateAndRoute( 0, PCM_NO_SYNC, 8000, 8000, VM_PCM_INTERNAL_A ) ;
 
    /*connects the speaker and mic, which */
	/* Route port 0 to outgoing SCO audio */
	/* Route incoming SCO audio to port 1 */
    CsrCvsdUsbNoDspPluginSetMode ( mode , params ) ;
    	
}

/****************************************************************************
DESCRIPTION
	Disconnect Sync audio
*/
void CsrCvsdUsbNoDspPluginDisconnect( void ) 
{    
    if (!SYNC_USB)
        Panic() ;

    DisconnectSpeaker() ;
    DisconnectMic() ;

    free (SYNC_USB);
    SYNC_USB = NULL ; 
}

/****************************************************************************
DESCRIPTION
	Indicate the volume has changed
*/
void CsrCvsdUsbNoDspPluginSetVolume( uint16 volume ) 
{
    if (!SYNC_USB)
        Panic() ;
    
    SYNC_USB->volume = volume;    
    
    /*Set the output Gain immediately*/
    CodecSetOutputGainNow( SYNC_USB->codec_task, SYNC_USB->volume, left_and_right_ch );    
 
}

/****************************************************************************
DESCRIPTION
	Set the mode
*/
void CsrCvsdUsbNoDspPluginSetMode ( AUDIO_MODE_T mode , const void * params ) 
{
    SYNC_USB->mode = mode ;
    
	switch (SYNC_USB->mode)
    {
	    case AUDIO_MODE_MUTE_SPEAKER     :
	    {
			PRINT(("NODSP: SetMode MUTE SPEAKER\n"));			
			DisconnectSpeaker();            		
		}
	    break ;
	    case AUDIO_MODE_CONNECTED :
	    {
	        PRINT(("NODSP: Set Mode CONNECTED\n"));
	        ConnectSpeaker();            		
	        ConnectMic();            		
	    }
	    break ;
	    case AUDIO_MODE_MUTE_MIC :
	    {
	        PRINT(("NODSP: Set Mode MUTE MIC"));
	        DisconnectMic() ;
	    }
	    break ;      
	    case AUDIO_MODE_MUTE_BOTH :
	    {
	        PRINT(("NODSP: Set Mode MUTE BOTH\n"));
	        DisconnectMic() ;
	        DisconnectSpeaker() ;	        
	    }
	    break ;    
	    default :
	    {   
            /*do not send a message and return false*/    
	        PRINT(("NODSP: Set Mode Invalid [%x]\n" , mode )) ;
		}
		break ;
    }
}

/****************************************************************************
DESCRIPTION
    plays/mixes a tone as required 
    If a tone is currently playing and can_queue is TRUE then the tone is queued
*/
void CsrCvsdUsbNoDspPluginPlayTone ( audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo)  
{    
    Source lSource ;
    Sink lSink = NULL ;
       
    if (!SYNC_USB)
        Panic() ;    
   
    CodecSetOutputGainNow( SYNC_USB->codec_task, 0 , left_and_right_ch );    
                  
    if (stereo)
 		PcmRateAndRoute( 0, PCM_NO_SYNC, 8000, 8000, VM_PCM_INTERNAL_A_AND_B ) ;
 	else	
 		PcmRateAndRoute( 0, PCM_NO_SYNC, 8000, 8000, VM_PCM_INTERNAL_A ) ;    
    
    
    if ( SYNC_USB->audio_sink )
    {   
        DisconnectSpeaker() ;
    }
    
	/* connect the tone to the DACs*/
    lSink = StreamPcmSink(0) ;        
    
	/*request an indicaton that the tone has completed / been disconnected*/
    MessageSinkTask ( lSink ,  (TaskData*) &csr_cvsd_usb_no_dsp_plugin ) ;
    
	/*connect the tone*/
    lSource = StreamAudioSource ( tone ) ;    
    PanicFalse( StreamConnectAndDispose( lSource , lSink ) );
    
    if (tone_volume)
    {
        CodecSetOutputGainNow( SYNC_USB->codec_task, tone_volume, left_and_right_ch );    
    }
    else
    {
        CodecSetOutputGainNow( SYNC_USB->codec_task, SYNC_USB->volume, left_and_right_ch );    
    }
}

/****************************************************************************
DESCRIPTION
	Stop a tone from playing
RETURNS
	whether or not the tone was stopped successfully
*/
void CsrCvsdUsbNoDspPluginStopTone ( void ) 
{
    
    if (!SYNC_USB)
        Panic() ;    

    DisconnectSpeaker() ;

          /*dispose the tone stream*/
    /*    PanicFalse( StreamConnectDispose ( stream_source ) ); */

    CsrCvsdUsbNoDspPluginSetMode ( SYNC_USB->mode , NULL );
    
    AUDIO_BUSY = NULL ;      
}

/****************************************************************************
DESCRIPTION
	Reconnects the audio after a tone has completed 
*/
void CsrCvsdUsbNoDspPluginToneForceComplete ( void ) 
{
    
    AUDIO_BUSY = NULL ;
    MessageSinkTask ( StreamPcmSink(0) , NULL ) ;
 	CsrCvsdUsbNoDspPluginSetVolume( SYNC_USB->volume );     
    DisconnectSpeaker();
    DisconnectMic();
}


/****************************************************************************
DESCRIPTION
	Reconnects the audio after a tone has completed 
*/
void CsrCvsdUsbNoDspPluginToneComplete ( void ) 
{
	if (( SYNC_USB->mode != AUDIO_MODE_MUTE_BOTH )&&
        ( SYNC_USB->mode != AUDIO_MODE_MUTE_SPEAKER))
	{      
        /* check to see if the sco is still valid, if it is not then we will have received the
           message before the tone has completed playing due to some other issue, therefore
           allow tone to continue playing for an additional 1.5 seconds to allow the power off
           tone to be played to completion */
        if(StreamConnect(StreamSourceFromSink(SYNC_USB->audio_sink) , StreamPcmSink(0)))
        {    
           AUDIO_BUSY = NULL ;
           MessageSinkTask ( StreamPcmSink(0) , NULL ) ;
	       CsrCvsdUsbNoDspPluginSetVolume( SYNC_USB->volume );  
        }
        else
        {
           MessageSendLater((TaskData*) &csr_cvsd_usb_no_dsp_plugin, MESSAGE_FORCE_TONE_COMPLETE, 0, 1500);
        }
    }
    else
    {
        AUDIO_BUSY = NULL ;
        MessageSinkTask ( StreamPcmSink(0) , NULL ) ;
	    CsrCvsdUsbNoDspPluginSetVolume( SYNC_USB->volume ); 
    }

}

/****************************************************************************
DESCRIPTION
    Disconnect the microphone path
*/
static void DisconnectMic ( void )
{
    StreamDisconnect(StreamPcmSource(0), SYNC_USB->audio_sink);
}

/****************************************************************************
DESCRIPTION
    Disconnect the Speaker path
*/
static void DisconnectSpeaker ( void )
{
    StreamDisconnect(StreamSourceFromSink(SYNC_USB->audio_sink), StreamPcmSink(0));
}

/****************************************************************************
DESCRIPTION
    Connect a SCO to the Microphone
*/
static void ConnectMic ( void )
{
    if ( SYNC_USB->audio_sink )
    {
      	PanicFalse( StreamConnect(StreamPcmSource(0), SYNC_USB->audio_sink) );
    }    
}

/****************************************************************************
DESCRIPTION
    Connect a SCO to the Speaker  
*/
static void ConnectSpeaker ( void )
{
    if ( SYNC_USB->audio_sink )
    {
        PanicFalse( StreamConnect(StreamSourceFromSink(SYNC_USB->audio_sink), StreamPcmSink(0)) );
    }    
} 
