/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_hfp_handler.c
@brief    Functions which handle the HFP library messages.
*/

#include "headset_a2dp_connection.h"
#include "headset_a2dp_stream_control.h"
#include "headset_configmanager.h"
#include "headset_debug.h"
#include "headset_hfp_handler.h"
#include "headset_hfp_slc.h"
#include "headset_init.h"
#include "headset_inquiry.h"
#include "headset_led_manager.h"
#include "headset_link_policy.h"
#include "headset_pio.h"
#include "headset_statemanager.h"
#include "headset_volume.h"

#include <audio.h>
#include <bdaddr.h>
#include <codec.h>
#include <hfp.h>
#include <panic.h>
#include <ps.h>
#include <pio.h>
#include <stdlib.h>

#include <csr_cvc_common_plugin.h>
#include <csr_common_no_dsp_plugin.h>


#define NODSPCVSD		(TaskData *)&csr_cvsd_no_dsp_plugin 			
#define CVC1MIC	    	(TaskData *)&csr_cvsd_cvc_1mic_headset_plugin		
#define CVC2MIC     	(TaskData *)&csr_cvsd_cvc_2mic_headset_plugin			

/* The row to use is selected by user PSKEY.
   The column depends upon the audio link negotiated. */
TaskData * const gPlugins = NODSPCVSD;  
	
	
#ifdef DEBUG_HFP
#define HFP_DEBUG(x) DEBUG(x)
#else
#define HFP_DEBUG(x) 
#endif

	
/****************************************************************************
    LOCAL FUNCTION PROTOTYPES
*/
static bool audioConnectSco(AUDIO_SINK_T sink_type, uint32 bandwidth);

/****************************************************************************
  HFP MESSAGE HANDLING FUNCTIONS
*/

/****************************************************************************/
void hfpHandlerInitCfm( const HFP_INIT_CFM_T *cfm )
{
    /* Make sure the profile instance initialisation succeeded. */
    if (cfm->status == hfp_init_success)
    {
        /* Check for an hfp instance, that's registered first */
        if (!theHeadset.hfp)
        {
            /* This must be the hfp instance */ 
            theHeadset.hfp = cfm->hfp;
        }
        else
        {
            /* Its not HFP so must be HSP */
            theHeadset.hsp = cfm->hfp;
            /* HFP/HSP Library initialisation was a success, initailise the 
               next library. */
			InitA2dp();
        }
    }
    else
        /* If the profile initialisation has failed then things are bad so panic. */
        Panic();
}


/****************************************************************************/
void hfpHandlerConnectInd( const HFP_SLC_CONNECT_IND_T *ind )
{
	/* Cancel inquiry and throw away results if one is in progress */
    inquiryStop();
		
    /* We support more than one HFP so check which one this request is for */   
    if ((theHeadset.profile_connected == hfp_no_profile) &&
		(stateManagerGetHfpState() != headsetPoweringOn))
    {
        HfpSlcConnectResponse(ind->hfp, 1, &ind->addr, 0);
              
        /* See whether we are connecting as HSP or HFP */
        if (ind->hfp == theHeadset.hfp)
        {
            theHeadset.profile_connected = hfp_handsfree_profile;
        }
        else if (ind->hfp == theHeadset.hsp)
        {
            theHeadset.profile_connected = hfp_headset_profile;
        }
        else
            /* Something is wrong we should be either hfp or hsp */
            Panic();
    }
    else
    {
        /* Reject the connect attempt we're already connected */
        HfpSlcConnectResponse(ind->hfp, 0, &ind->addr, 0);
    }
	theHeadset.LinkLossAttemptHfp = 0;
}


/****************************************************************************/
void hfpHandlerConnectCfm( const HFP_SLC_CONNECT_CFM_T *cfm )
{
	theHeadset.slcConnecting = FALSE;
	
    if (stateManagerGetHfpState() == headsetPoweringOn)
    {
        if ( cfm->status == hfp_connect_success )
        {			
            /* A connection has been made and we are now logically off */
			if (cfm->hfp == theHeadset.hfp)
				theHeadset.hfp_hsp = theHeadset.hfp;
    		else
				theHeadset.hfp_hsp = theHeadset.hsp;
   
            hfpSlcDisconnect(); 
        }	
		theHeadset.slcConnectFromPowerOn = FALSE;
        return;
    }
    
    if (cfm->status == hfp_connect_success)
    {      		
        hfpSlcConnectSuccess(cfm->hfp, cfm->sink);
		/* Update Link Policy as HFP has connected. */
		linkPolicySLCconnect();
    }
    else if (cfm->status == hfp_connect_sdp_fail)
    {
        if ( !stateManagerIsHfpConnected() )  /*only continue if not already connected*/
        {
            if (cfm->hfp == theHeadset.hfp)
            {
                /* Didn't find HFP so try HSP */
				if(theHeadset.inquiry_data)
				{
					if (!hfpSlcConnectBdaddrRequest(hfp_headset_profile, &theHeadset.inquiry_data[0].bd_addr))
					{
						/* This connection failed so check if inquiry needs to resume */
						inquiryContinue();
					}
				}
            }
            else if (cfm->hfp == theHeadset.hsp)
            {
                HFP_DEBUG(("SLC: CFM HSP Fail\n")) ;
				/* This connection failed so check if inquiry needs to resume */
				inquiryContinue();
            }
            else
            {
                Panic();/* Unknown profile instance */
            }
        } 
		else
		{			
			theHeadset.slcConnectFromPowerOn = FALSE;
		}
    }
    else
    {
        if ( !stateManagerIsHfpConnected() )  /*only continue if not already connected*/
        {    /* Failed to connect */    
        }
		else
		{			
			theHeadset.slcConnectFromPowerOn = FALSE;
		}
		/* This connection failed so check if inquiry needs to resume */
		inquiryContinue();
    }       
}


/*****************************************************************************/
void hfpHandlerDisconnectInd(const HFP_SLC_DISCONNECT_IND_T *ind)
{	
	bdaddr ag_addr;
	bool last_ag = FALSE;
	
	/* Check if this was the result of an abnormal link loss */
    if (ind->status == hfp_disconnect_link_loss ) 
    {	
        HFP_DEBUG(("HFP: Link Loss Detect\n")) ;
               
        MessageSend( &theHeadset.task , EventLinkLoss , 0 ) ;
    }
    else
    {
		theHeadset.combined_link_loss = FALSE;
    }
	
	if (last_ag)
	{
		uint8 lAttributes[ATTRIBUTE_SIZE];
		/* Retrieve attributes for this device */
    	if (ConnectionSmGetAttributeNow(PSKEY_ATTRIBUTE_BASE, &ag_addr, ATTRIBUTE_SIZE, lAttributes))
		{
			bool write_params = FALSE;
			if (!lAttributes[attribute_hfp_hsp_profile])
			{
				lAttributes[attribute_hfp_hsp_profile] = ATTRIBUTE_GET_HF_PROFILE(theHeadset.profile_connected);
				write_params = TRUE;
			}
			if (lAttributes[attribute_hfp_volume] != theHeadset.gHfpVolumeLevel)
			{
				lAttributes[attribute_hfp_volume] = theHeadset.gHfpVolumeLevel;
				write_params = TRUE;
			}
			if (write_params)
			{
				/* Write params to PS */
				ConnectionSmPutAttribute(PSKEY_ATTRIBUTE_BASE, &ag_addr, ATTRIBUTE_SIZE, lAttributes); 
				HFP_DEBUG(("HFP: Store HFP attributes [%d][%d][%d][%d][%d][%d]\n",lAttributes[0],
				   lAttributes[1],lAttributes[2],lAttributes[3],lAttributes[4],lAttributes[5])) ;
			}
		}
	}
	
	/*	
        Handle the case where an incoming call is rejected using the headset profile.
		As we get no indicator info, the AV must be restarted on a SLC disconnect.
	*/
    if ((theHeadset.profile_connected == hfp_headset_profile))
    {
        streamControlResumeA2dpStreaming(0);
    }
    
    /* Update the app state if we are connected */
    if ( stateManagerIsHfpConnected() )
    {
        stateManagerEnterHfpConnectableState( FALSE ) ;
    }
    
    MessageSend(&theHeadset.task , EventSLCDisconnected , 0) ;
    
        /* Connection disconnected */
    theHeadset.profile_connected = hfp_no_profile;
	theHeadset.hfp_hsp = NULL;

	/* Reset in-band ring tone support flag */
	theHeadset.InBandRingEnabled = FALSE;
	
	/* Update Link Policy as HFP has disconnected. */
	linkPolicySLCdisconnect();
	
    PROFILE_MEMORY(("HFPDisco"))

}


/*****************************************************************************/
void hfpHandlerInbandRingInd( const HFP_IN_BAND_RING_IND_T * ind )
{    
    theHeadset.InBandRingEnabled = ind->ring_enabled;        
}


/*****************************************************************************/
void hfpHandlerCallInd ( const HFP_CALL_IND_T * pInd ) 
{
    switch (pInd->call)
    {
    case 0:
		if ( stateManagerGetHfpState() == headsetActiveCall )
        {		
			if (theHeadset.SecondIncomingCall)
			{
				theHeadset.SecondIncomingCall = FALSE;
				stateManagerEnterIncomingCallEstablishState() ;
			}
			else
			{
            	stateManagerEnterHfpConnectedState() ;
			}
        }    
      	break;

    case 1:
        stateManagerEnterActiveCallState() ;  
        break;

    default:
        break;
    }
}


/*****************************************************************************/
void hfpHandlerCallSetupInd ( const HFP_CALL_SETUP_IND_T * pInd ) 
{			
    switch (pInd->call_setup)
    {
    case (hfp_no_call_setup): /*!< No call currently being established.*/

     	/* Spec says that this is a clear indication that an incoming call has been interrupted
       	   and we can assume that the call is not going to continue....*/          
        	   
        /* However, we have come across phones that send this before a callind [1]
           on accepting a call - -interop issue*/
               
        if ( stateManagerIsHfpConnected() )
        {
            stateManagerEnterHfpConnectedState() ;
        }
               
        break;
    case (hfp_incoming_call_setup): /*!< HFP device currently ringing.*/
		/* if we are just powering up the state will still be connectable and/or discoverable so
           we need to see if it is necessary to answer the incoming call, this cannot be done until
           after the slcConnectConfirm so set a flag that will be processed in the slc cfm */
        if (!stateManagerIsHfpConnected() )
        {
        	/* set flag to accept call when the slcSyncConnectConfirm is process as it will be
               properly connected at this point */
            theHeadset.incoming_call_power_up = TRUE;
        }
        stateManagerEnterIncomingCallEstablishState() ;
        break;
    case (hfp_outgoing_call_setup): /*!< Call currently being dialed.*/
    case (hfp_outgoing_call_alerting_setup):/*!< Remote end currently ringing.*/
        stateManagerEnterOutgoingCallEstablishState() ;
        break;
    default:
        break;
        
    }
}


/*****************************************************************************/
void hfpHandlerRingInd ( void )
{
	/* Disconnect A2DP audio if it is streaming from a standalone A2DP source */
	if (!IsA2dpSourceAnAg())
		streamControlCeaseA2dpStreaming(TRUE);
	
    if ( !theHeadset.InBandRingEnabled )
    {
    	HFP_DEBUG(("HFP_DEBUG: OutBandRing\n")) ;
		
        /* Play ring tone from configuration */
    }    
	
	if ( (theHeadset.profile_connected == hfp_headset_profile) && !theHeadset.HSPCallAnswered )
    {
		/* If using HSP then use ring indication as incoming call */
		theHeadset.HSPIncomingCallInd = TRUE;
       
        stateManagerEnterIncomingCallEstablishState() ;
       
        MessageCancelAll ( &theHeadset.task , APP_CANCEL_HSP_INCOMING_CALL ) ;
        MessageSendLater ( &theHeadset.task , APP_CANCEL_HSP_INCOMING_CALL , 0 , D_SEC(6) ) ;
    }
    
}


/*****************************************************************************/
void hfpHandlerVoiceRecognitionInd( const HFP_VOICE_RECOGNITION_IND_T *ind )
{
    /* Update the local flag */
    theHeadset.voice_recognition_enabled = ind->enable;  
	
	/* If voice dial has been deactivated then try a restart A2DP if no SCO active */
	if (!theHeadset.voice_recognition_enabled && !HfpGetAudioSink(theHeadset.hfp_hsp))
		streamControlResumeA2dpStreaming(0);
}


/*****************************************************************************/
void hfpHandlerVoiceRecognitionCfm( const HFP_VOICE_RECOGNITION_ENABLE_CFM_T *cfm )
{
    if (cfm->status)
    {
        /* Voice recognition cmd not accepted - toggle state */
        theHeadset.voice_recognition_enabled ^= 1 ;
		
		/* Resume A2DP streaming. Needs extra delay because of the Nokia 8800 phone which says
			voice dial has failed then carries on with it. The media player breaks if
 			the headset sends an AVDTP_START too early.
		*/
		streamControlResumeA2dpStreaming(1000);
		
		MessageSend ( &theHeadset.task , EventError , 0 );
    }
}


/*****************************************************************************/
void hfpHandlerLastNoRedialCfm( const HFP_LAST_NUMBER_REDIAL_CFM_T *cfm )
{
    if (cfm->status)
    {
		MessageSend ( &theHeadset.task , EventError , 0 );
    }
}


/*****************************************************************************/
void hfpHandlerEncryptionChangeInd( const HFP_ENCRYPTION_CHANGE_IND_T *ind )
{
    
}


/*****************************************************************************/
void hfpHandlerSpeakerVolumeInd( const HFP_SPEAKER_VOLUME_IND_T *ind )
{
	HFP_DEBUG(("HFP: hfpHandlerSpeakerVolumeInd [%d]\n", ind->volume_gain)) ;
	theHeadset.gHfpVolumeLevel = ind->volume_gain;
	
	if (theHeadset.gHfpVolumeLevel == 0)
		MessageSend(&theHeadset.task, EventVolumeMin, 0);
	
	if (theHeadset.gHfpVolumeLevel == VOL_MAX_VOLUME_LEVEL)
		MessageSend(&theHeadset.task, EventVolumeMax, 0);
			
	if (stateManagerIsA2dpStreaming())
	    return;    
	
	VolumeSetHeadsetVolume(theHeadset.gHfpVolumeLevel, FALSE, FALSE);
	
}


/*****************************************************************************/
void hfpHandlerAudioConnectInd( const HFP_AUDIO_CONNECT_IND_T *ind )
{
    HFP *active_hfp = 0;
	hfp_audio_params audio_params_data, *audio_params = NULL;
	
    /* Accept audio if its for a task we own */
    if ((ind->hfp == theHeadset.hfp) || (ind->hfp == theHeadset.hsp))
        active_hfp = ind->hfp;    

    if (active_hfp)
    {
        /* Accept the audio connection */		
		{
			/* CVSD codec */
			if (theHeadset.HFP_features.eSCO_Parameters_Enabled )
			{
        		audio_params_data.bandwidth       = theHeadset.HFP_features.bandwidth ;     
	    		audio_params_data.max_latency     = theHeadset.HFP_features.max_latency ;
    			audio_params_data.voice_settings  = theHeadset.HFP_features.voice_settings ;
        		audio_params_data.retx_effort     = theHeadset.HFP_features.retx_effort ;  
				audio_params = &audio_params_data;
			}
			
			HfpAudioConnectResponse(active_hfp, 1, theHeadset.HFP_features.supportedSyncPacketTypes, audio_params, ind->bd_addr);
		}
    }
    else
	{
        HfpAudioConnectResponse(active_hfp, 0, sync_hv1, 0, ind->bd_addr);	
	}
}


/*****************************************************************************/
void hfpHandlerAudioConnectCfm( const HFP_AUDIO_CONNECT_CFM_T *cfm )
{
    if ( cfm->status == hfp_success)
    {
	    AUDIO_SINK_T sink_type;
		
		if (theHeadset.hfp_hsp == NULL)
		{
			/* The audio has obviously been established before being notified of SLC.
			   Update the HFP pointer accordingly. */
			if (cfm->hfp == theHeadset.hfp)
    		{
        		theHeadset.profile_connected = hfp_handsfree_profile;
				theHeadset.hfp_hsp = theHeadset.hfp;
    		}
    		else if (cfm->hfp == theHeadset.hsp)
    		{
        		theHeadset.profile_connected = hfp_headset_profile;
				theHeadset.hfp_hsp = theHeadset.hsp;
    		}    
		}
		
		/* Stop any audio transfer requests */
		MessageCancelAll(&theHeadset.task, APP_CHECK_FOR_AUDIO_TRANSFER);
	    
        switch (cfm->link_type)
        {
			case sync_link_unknown:  /* Assume SCO if we don't know */
			case sync_link_sco:
				sink_type = AUDIO_SINK_SCO;
				break;
			case sync_link_esco:
				sink_type = AUDIO_SINK_ESCO;
				break;
			default:
				sink_type = AUDIO_SINK_INVALID;
				break;
        }
		
        audioConnectSco(sink_type, cfm->tx_bandwidth) ;
        
       /* Send an event to indicate that a SCO has been opened. This indicates
          that an audio connection has been successfully created to the AG. */
    	MessageSend ( &theHeadset.task , EventSCOLinkOpen , 0 ) ;
    
	    /* If this is a headset instance, enter the active call state */
        if (theHeadset.profile_connected == hfp_headset_profile)
        {
            stateManagerEnterActiveCallState() ;
        }			
		
		/* Update Link Policy as SCO has connected. */
		linkPolicySCOconnect();
    }
}


/*****************************************************************************/
void hfpHandlerAudioDisconnectInd( const HFP_AUDIO_DISCONNECT_IND_T *ind )
{
    if (theHeadset.dsp_process == dsp_process_sco)
    {
        AudioDisconnect() ;

        theHeadset.dsp_process = dsp_process_none;
    }

    /* Try to resume A2DP streaming if not outgoing or incoming call */
    if ((stateManagerGetHfpState() != headsetIncomingCallEstablish) && (stateManagerGetHfpState() != headsetOutgoingCallEstablish))     
    {
        if ((stateManagerGetHfpState() == headsetActiveCall) && IsA2dpSourceAnAg())
        {
            /* Do nothing with A2DP_AG as the music can't start while call active */
        }
        else
        {
            streamControlResumeA2dpStreaming(0);
        }
    }

    MessageSend ( &theHeadset.task , EventSCOLinkClose , 0 ) ;
	
	theHeadset.HSPCallAnswered = FALSE;
	
    /* If this is a headset instance, end the call */    
    if ( theHeadset.profile_connected == hfp_headset_profile )
    {
        if(stateManagerIsHfpConnected())
        {
            stateManagerEnterHfpConnectedState() ;
        }
    }  

    /* If we are muted - then un mute at disconnection */
    if (theHeadset.gMuted )
    {
        MessageSend(&theHeadset.task , EventMuteOff , 0) ;   
    }    

    /* Update Link Policy as SCO has disconnected. */
    linkPolicySCOdisconnect();
}


/****************************************************************************
NAME    
    audioConnectSco
    
DESCRIPTION
	This function connects the synchronous connection to the CVC EC/NR Algorithm running
    in the Kalimba DSP. 

*/	  
static bool audioConnectSco(AUDIO_SINK_T sink_type, uint32 bandwidth)
{
    bool lResult = FALSE ;
   	
    /* The mode to connect - connected as default */
    AUDIO_MODE_T lMode = AUDIO_MODE_CONNECTED ;
	
	/* Disconnect A2DP audio if it was active */
	streamControlCeaseA2dpStreaming(TRUE);
    
    /* Mute control */
    if (theHeadset.gMuted )
    {
		if (theHeadset.features.MuteSpeakerAndMic)
    	{
        	lMode = AUDIO_MODE_MUTE_BOTH;
    	}
    	else
    	{
        	lMode = AUDIO_MODE_MUTE_MIC;
    	}      
    }    	
    
	if (HfpGetAudioSink(theHeadset.hfp_hsp))
	{
		HFP_DEBUG(("HFP: Route SCO mode=%d mic_mute=%d volindex=%d volgain=%d\n",lMode,theHeadset.gMuted,theHeadset.gHfpVolumeLevel,theHeadset.config->gVolLevels.volumes[theHeadset.gHfpVolumeLevel].hfpGain));
		lResult = AudioConnect((TaskData*)gPlugins,
		 				     HfpGetAudioSink(theHeadset.hfp_hsp),
		 				     sink_type,
							 theHeadset.theCodecTask,
							 theHeadset.config->gVolLevels.volumes[theHeadset.gHfpVolumeLevel].hfpGain,
						   	 bandwidth,
						   	 theHeadset.features.mono ? FALSE : TRUE,
		 					 lMode,
							 NULL,
							 &theHeadset.task);

		theHeadset.dsp_process = dsp_process_sco;
	}	   

	return (lResult=TRUE);
}

/****************************************************************************
NAME
    hfpHandlerCallWaitingInd

DESCRIPTION
    Handle indication that we have a call waiting.

*/
void hfpHandlerCallWaitingInd( const HFP_CALL_WAITING_IND_T* ind )
{
    headsetHfpState hfp_state = stateManagerGetHfpState();

    if (headsetActiveCall == hfp_state)
    {
        stateManagerEnterTWCWaitingState();
    }
    else
    {
        HFP_DEBUG(("HFP: Got call waiting indication in state [%d]\n", hfp_state));
    }
}

/****************************************************************************
NAME    
    hfpHandlerCallWaitingEnableCfm
    
DESCRIPTION
    Handles HFP_CALL_WAITING_ENABLE_CFM_T message from HFP library

*/
void hfpHandlerCallWaitingEnableCfm( const HFP_CALL_WAITING_ENABLE_CFM_T* cfm )
{

}

/****************************************************************************
NAME    
    hfpHandlerReleaseHeldRejectWaitingCallCfm

DESCRIPTION
    Handles HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM_T message from HFP library

*/
void hfpHandlerReleaseHeldRejectWaitingCallCfm ( const HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM_T *cfm )
{
    /* if successful we're just in a normal active call now */
    if (cfm->status == hfp_success)
    {
        stateManagerEnterActiveCallState();
    }
    else
    {
        HFP_DEBUG(("HFP: Failure to ReleaseHeldRejectWaiting - status[%d]\n", cfm->status));
    }
}

/****************************************************************************
NAME    
    hfpHandlerReleaseActiveAcceptOtherCallCfm

DESCRIPTION
    Handles HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM_T message from HFP library

*/
void hfpHandlerReleaseActiveAcceptOtherCallCfm ( const HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM_T *cfm )
{
    /* if successful we're just in a normal active call now */
    if (cfm->status == hfp_success)
    {
        stateManagerEnterActiveCallState();
    }
    else
    {
        HFP_DEBUG(("HFP: Failure to ReleaseActiveAcceptOther - status[%d]\n", cfm->status));
    }
}

/****************************************************************************
NAME    
    hfpHandlerHoldActiveAcceptOtherCallCfm

DESCRIPTION
    Handles HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM_T message from HFP library

*/
void hfpHandlerHoldActiveAcceptOtherCallCfm ( const HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM_T *cfm )
{
    headsetHfpState hfp_state = stateManagerGetHfpState();
    
    if (cfm->status == hfp_success)
    {
        /* Workaround for ambiguity in HFP spec.
           Only change state if we're in a threeway calling mode.
           This is so that when CHLD=2 is used in an active call (put it on hold), we'll just stay
           in the active call state and therefore generate the correct CHLD=2 (unhold) then CHUP (hangup)
           messages */
        if (hfp_state == headsetTWCWaiting || hfp_state == headsetTWCOnHold)
            stateManagerEnterTWCOnHoldState();
    }
    else
    {
        HFP_DEBUG(("HFP: Failure to HoldActiveAcceptOther - status[%d]\n", cfm->status));
    }
}

/****************************************************************************
NAME    
    hfpHandlerAddHeldCallCfm

DESCRIPTION
    Handles HFP_ADD_HELD_CALL_CFM_T message from HFP library

*/
void hfpHandlerAddHeldCallCfm ( const HFP_ADD_HELD_CALL_CFM_T *cfm )
{
    if (cfm->status == hfp_success)
    {
        stateManagerEnterTWCMulticallState();
    }
    else
    {
        HFP_DEBUG(("HFP: Failure to AddHeldCall - status[%d]\n", cfm->status));
    }
}

/****************************************************************************
NAME    
    hfpHandlerExplicitCallTransferCfm

DESCRIPTION
    Handles HFP_EXPLICIT_CALL_TRANSFER_CFM_T message from HFP library

*/
void hfpHandlerExplicitCallTransferCfm ( const HFP_EXPLICIT_CALL_TRANSFER_CFM_T *cfm )
{
    if (cfm->status == hfp_success)
    {
        stateManagerEnterHfpConnectedState();
    }
    else
    {
        HFP_DEBUG(("HFP: Failure to ExplicitCallTransfer - status[%d]\n", cfm->status));
    }
}

/****************************************************************************
NAME    
    hfpHandlerThreeWayCallInd

DESCRIPTION
    Handles HFP_CALL_IND_T message from HFP library during threeway calling
    scenario

*/
void hfpHandlerThreeWayCallInd ( const HFP_CALL_IND_T *ind )
{
    headsetHfpState state = stateManagerGetHfpState();

    HFP_DEBUG(("Handling CallInd[%d] in threeway calling state[%d]\n", ind->call, state));

    switch (ind->call)
    {
        case 0: /* not in a call */
        {
            /* last call in the conference has hung up on us go to connected ||
               last call hung up after we were handling held calls */
            if (state == headsetTWCMulticall || state == headsetTWCOnHold)
            {
                stateManagerEnterHfpConnectedState();
            }
            /* callInd:0 in headsetTWCWaiting means active call has ended
               go to incoming as AG will be ringing us now with call that was waiting */
            else if (state == headsetTWCWaiting)
            {
                stateManagerEnterIncomingCallEstablishState();
            }
            break;
        }
        case 1:
            break;
        default:
        break;
    }
}

/****************************************************************************
NAME    
    hfpHandlerThreeWayCallSetupInd

DESCRIPTION
    Handles HFP_CALL_SETUP_IND_T message from HFP library during threeway calling
    scenario

*/
void hfpHandlerThreeWayCallSetupInd ( const HFP_CALL_SETUP_IND_T *ind )
{
    headsetHfpState state = stateManagerGetHfpState();

    HFP_DEBUG(("Handling CallSetupInd[%d] in threeway calling state[%d]\n", ind->call_setup, state));

    switch (ind->call_setup)
    {
        case hfp_no_call_setup:
        {
            if (state == headsetTWCWaiting)
            {
                /* this is fundamentally broken in the HFP spec...

                   a no_call_setup value in headsetTWCWaiting state can mean any 1 of 3 things:-

                    - the waiting call (that called us) has hung up on us
                    - the waiting call (that we called) has hung up on us
                    - the waiting call (that we called) has accepted our call
                   
                  we'll - arbitrarily - choose to believe the 3rd case, as this'll pass the PTS test */
                stateManagerEnterTWCOnHoldState();
            }
            break;
        }
        case hfp_outgoing_call_setup:
            if (state == headsetActiveCall)
            {
                /* we're in an active call and making an outgoing call ourselves
                   get into threeway waiting state */
                stateManagerEnterTWCWaitingState();
            }
            break;
        case hfp_outgoing_call_alerting_setup:
            /* should only get here if already in TWCWaiting state and want to stay there so ignore */
        case hfp_incoming_call_setup:
			/* Record event if TWC not supported so we can check when currently active call ends */
			if (!(theHeadset.HFP_features.HFP_Supported_Features & HFP_THREE_WAY_CALLING))
			{
				theHeadset.SecondIncomingCall = TRUE;
				return;
			}
			break;
			
        default:
            break;
    }
	
	theHeadset.SecondIncomingCall = FALSE;
}
