/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_hfp_call.c
@brief    Implementation of HFP call functionality.
*/


#include "headset_a2dp_stream_control.h"
#include "headset_debug.h"
#include "headset_hfp_call.h"
#include "headset_statemanager.h"
#include "headset_tones.h"

#include <hfp.h>
#include <panic.h>
#include <ps.h>


#ifdef DEBUG_HFP_CALL
#define HFP_CALL_DEBUG(x) DEBUG(x)
#else
#define HFP_CALL_DEBUG(x) 
#endif


static void headsetQueueEvent (headsetEvents_t pEvent) ;

static headsetEvents_t gMessageQueued = EventInvalid ;


/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************/
bool hfpCallInitiateVoiceDial ( void )
{
    HFP_CALL_DEBUG(("CM: VD\n")) ;
    if (!stateManagerIsHfpConnected())
    {
        MessageSend ( &theHeadset.task , EventEstablishSLC , 0 ) ;
		/* Only do voice dial if this wasn't initiated from a power on event */
		if (!theHeadset.slcConnectFromPowerOn)
	        headsetQueueEvent( EventInitateVoiceDial ) ;
        
        theHeadset.voice_recognition_enabled = FALSE ;
		
		return FALSE;
    }
    else
    {    
        HFP_CALL_DEBUG(("CM: VD Connected\n")) ;
		
		/* Pause A2DP streaming on a separate A2DP source.*/
		if (!IsA2dpSourceAnAg())
			streamControlCeaseA2dpStreaming(TRUE);
    
        if ( theHeadset.profile_connected == hfp_handsfree_profile )
        {           
            HfpVoiceRecognitionEnable(theHeadset.hfp, TRUE);    
                
            theHeadset.voice_recognition_enabled = TRUE ;
        }
        else
        {
            HfpSendHsButtonPress ( theHeadset.hsp ) ;
        }	    
    }    	
	return TRUE;
}


/****************************************************************************/
void hfpCallCancelVoiceDial ( void )
{
    if ( theHeadset.profile_connected == hfp_handsfree_profile )
    {      
        /*if we believe voice dial is currently active*/
        if ( theHeadset.voice_recognition_enabled)
        {
            HfpVoiceRecognitionEnable(theHeadset.hfp, FALSE);    
                
            theHeadset.voice_recognition_enabled = FALSE ;
        }
    }
}


/****************************************************************************/
bool hfpCallInitiateLNR ( void )
{
    HFP_CALL_DEBUG(("CM: LNR\n")) ;

    if (!stateManagerIsHfpConnected() )
    {
        MessageSend ( &theHeadset.task , EventEstablishSLC , 0 ) ;
		/* Only do LNR if this wasn't initiated from a power on event */
		if (!theHeadset.slcConnectFromPowerOn)
	        headsetQueueEvent( EventLastNumberRedial) ;
		
		return FALSE;
    }
    else
    {
        
        HFP_CALL_DEBUG(("CM: LNR Connected\n")) ;
    
        if ( theHeadset.profile_connected == hfp_handsfree_profile )
        {
            HfpLastNumberRedial( theHeadset.hfp );    
        }
        else
        {
            HfpSendHsButtonPress ( theHeadset.hsp ) ;
        }
    }
	return TRUE;
}


/****************************************************************************/
void hfpCallAnswer ( void )
{
    /* 
        Call the HFP lib function, this will determine the AT cmd to send
        depending on whether the profile instance is HSP or HFP compliant.
    */ 
    if ( theHeadset.profile_connected == hfp_handsfree_profile )
    {
        HfpAnswerCall ( theHeadset.hfp );
        
        /* Terminate the ring tone */
        ToneTerminate()  ;
    }
    else
    {
		HfpSendHsButtonPress ( theHeadset.hsp ) ;
 		/* if answer call when using HSP and we already have SCO then this is the best
		   indication we're going to get that the call has been answered so change state */
    	if(theHeadset.HSPIncomingCallInd && HfpGetAudioSink(theHeadset.hsp))
	   	{
			 stateManagerEnterActiveCallState();	
			 theHeadset.HSPCallAnswered = TRUE;
    	}  
    } 
}


/****************************************************************************/
void hfpCallReject ( void )
{
    /* 
        Reject incoming call - only valid for instances of HFP in HSP mode
        send a button press.
    */
    if ( theHeadset.profile_connected == hfp_handsfree_profile )
    {
        HfpRejectCall ( theHeadset.hfp );
        
        /* Terminate the ring tone */
        ToneTerminate() ;        
    }
    else
    {
        HfpSendHsButtonPress ( theHeadset.hsp ) ;
    }
    
}


/****************************************************************************/
void hfpCallHangUp ( void )
{
    /* Terminate the current ongoing call process */
    if ( theHeadset.profile_connected == hfp_handsfree_profile )
    {
        HfpTerminateCall ( theHeadset.hfp );
    }
    else
    {
        HfpSendHsButtonPress ( theHeadset.hsp ) ;
    }    
}


/****************************************************************************/
void hfpCallTransferToggle ( void )
{
	hfp_audio_params audio_params_data, *audio_params = 0;
    hfp_audio_transfer_direction lTransferDirection = hfp_audio_to_ag ;

	if ( !HfpGetAudioSink(theHeadset.hfp_hsp) )
    {
        lTransferDirection = hfp_audio_to_hfp ;
    }
	
	/* If the most recent codec connection that has been established was for Auristream we need
	   to set up the same Auristream link again. Set the audio parameters appropriately. */
	switch(theHeadset.SCO_codec_selected)
	{
		case(audio_codec_auristream_2_bit):
		case(audio_codec_auristream_4_bit):
			if(theHeadset.SCO_codec_selected == audio_codec_auristream_2_bit)
			{
				audio_params_data.bandwidth = theHeadset.config->Auristream.bw_2bits;
			}
			else
			{
				audio_params_data.bandwidth = theHeadset.config->Auristream.bw_4bits;
			}

			audio_params_data.max_latency = theHeadset.config->Auristream.max_latency;
			audio_params_data.voice_settings = theHeadset.config->Auristream.voice_settings;
			audio_params_data.retx_effort = theHeadset.config->Auristream.retx_effort;
			
			audio_params = &audio_params_data;
			break;
		default:
			/* Default SCO will be opened. */
			break;
	}
   
	/* call the transfer function - a hsp transfer call will generate the HSP button press */
	if ((!theHeadset.features.UseSCOforAudioTransfer) || (lTransferDirection != hfp_audio_to_hfp)) 
    {
		HfpAudioTransferConnection(theHeadset.hfp, lTransferDirection, theHeadset.HFP_features.supportedSyncPacketTypes, audio_params );
	} 
    else 
    {
		/* See B-34179: Transfer audio from ag to headset directly with a SCO */
		HfpAudioTransferConnection(theHeadset.hfp, lTransferDirection, (theHeadset.HFP_features.supportedSyncPacketTypes & sync_all_sco), audio_params);
    }
}


/*****************************************************************************/
void hfpCallRecallQueuedEvent ( void )
{
        /*this is currently only applicable to LNR and voice Dial but does not care */
    if (gMessageQueued != EventInvalid)
    {
        switch (stateManagerGetHfpState() )
        {
            case headsetIncomingCallEstablish:
            case headsetOutgoingCallEstablish:
            case headsetActiveCall:            
                /* Do Nothing Message Gets ignored*/
            break ;
            default:
                if ( theHeadset.profile_connected == hfp_handsfree_profile )
                {
                    MessageSend ( &theHeadset.task , gMessageQueued , 0 ) ; 
                
                    HFP_CALL_DEBUG(("CM: Queued Message ? [%x] Sent\n" , gMessageQueued))    
                }
                else
                {
                    HfpSendHsButtonPress ( theHeadset.hsp ) ;
                    HFP_CALL_DEBUG(("CM: Queued Msg - HSP Butt Sent\n"))    
                } 
            break;
        }
    }    
    
        /*reset the queued event*/
    gMessageQueued = EventInvalid ;
}


/*****************************************************************************/
void hfpCallClearQueuedEvent ( void )
{
    /* this resets the queue - on a conenction fail / power off etc */
    gMessageQueued = EventInvalid ;
}


/*****************************************************************************/
void headsetCheckForAudioTransfer ( void )
{
    headsetHfpState lState = stateManagerGetHfpState() ;
   
    switch (lState)
    {
        case headsetIncomingCallEstablish :
        case headsetTWCWaiting :
        case headsetTWCOnHold :
        case headsetTWCMulticall :
        case headsetActiveCall :      
            if (!HfpGetAudioSink(theHeadset.hfp_hsp) && (theHeadset.profile_connected == hfp_handsfree_profile))
            {
				{
					hfp_audio_params audio_params_data, *audio_params = 0;

					/* If the most recent codec connection that has been established was for Auristream we need
						   to set up the same Auristream link again. Set the audio parameters appropriately. */
					switch(theHeadset.SCO_codec_selected)
					{
						case(audio_codec_auristream_2_bit):
						case(audio_codec_auristream_4_bit):
							if(theHeadset.SCO_codec_selected == audio_codec_auristream_2_bit)
							{
								audio_params_data.bandwidth = theHeadset.config->Auristream.bw_2bits;
							}
							else
							{
								audio_params_data.bandwidth = theHeadset.config->Auristream.bw_4bits;
							}
						
							audio_params_data.max_latency = theHeadset.config->Auristream.max_latency;
							audio_params_data.voice_settings = theHeadset.config->Auristream.voice_settings;
							audio_params_data.retx_effort = theHeadset.config->Auristream.retx_effort;
								
							audio_params = &audio_params_data;
							break;
						default:
							/* Default SCO will be opened. */
							break;
					}

                    HfpAudioTransferConnection(theHeadset.hfp, hfp_audio_to_hfp , theHeadset.HFP_features.supportedSyncPacketTypes, audio_params);
                }
            }    
        	break ;
        default:
        	break;
    }
}


/****************************************************************************
NAME    
    headsetQueueEvent
    
DESCRIPTION
    Queues an event to be sent once the headset is connected.

*/
static void headsetQueueEvent ( headsetEvents_t pEvent)
{
    HFP_CALL_DEBUG(("CM: QQ Ev[%x]\n", pEvent)) ;
    gMessageQueued = pEvent ;
}


