/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_a2dp_msg_handler.c
@brief    Handle a2dp library messages arriving at the app.
*/

#include "headset_a2dp_connection.h"
#include "headset_a2dp_msg_handler.h"
#include "headset_a2dp_stream_control.h"
#include "headset_avrcp_event_handler.h"
#include "headset_avrcp_msg_handler.h"
#include "headset_debug.h"
#include "headset_hfp_slc.h"
#include "headset_init.h"
#include "headset_inquiry.h"
#include "headset_link_policy.h"
#include "headset_private.h"
#include "headset_statemanager.h"
#include "headset_tones.h"
#include "headset_configmanager.h"

#include <a2dp.h>
#include <bdaddr.h>
#include <panic.h>
#include <ps.h>


#ifdef DEBUG_A2DP_MSG
#define A2DP_MSG_DEBUG(x) DEBUG(x)
#else
#define A2DP_MSG_DEBUG(x) 
#endif


/****************************************************************************
  ENUM DEFINITIONS
*/


/****************************************************************************
  MESSAGE DEFINITIONS
*/


/****************************************************************************
  LOCAL FUNCTIONS
*/

/****************************************************************************/
static void sendPlayOnAvrcpConnection(void)
{
	if ( stateManagerIsAvrcpConnected() )
	{
		if (theHeadset.features.autoSendAvrcp)
			avrcpSendPlay();
	}
	else
	{
		/* AVRCP is not connected yet, so send an avrcp_play once it is connected */
		if (stateManagerGetAvrcpState() == avrcpConnecting)
		{
			theHeadset.avrcp_data.send_play = 1;
			MessageSendConditionally( &theHeadset.task , APP_SEND_PLAY, 0, &theHeadset.avrcp_data.send_play );
		}
	}
}


/****************************************************************************
  LOCAL MESSAGE HANDLING FUNCTIONS
*/

static void handleA2DPInitCfm(const A2DP_INIT_CFM_T *msg)
{   
    if(msg->status == a2dp_success)
    {
        A2DP_MSG_DEBUG(("Init Success\n"));
        /* A2DP Library initialisation was a success */     
        /* Keep a record of the A2DP SEP pointer. */
		theHeadset.a2dp_data.sep_entries = msg->sep_list;
		
		/* Initialise AVRCP if enabled */
		if (theHeadset.features.UseAVRCPprofile)
			InitAvrcp();
		else
			/* All profile libraries are now initialised */
			theHeadset.ProfileLibrariesInitialising = FALSE;
    }
    else
    {
	    A2DP_MSG_DEBUG(("Init Failed [Status %d]\n", msg->status));
        Panic();
    }
}


static void handleA2DPOpenInd(Sink sink, uint8 seid)
{
    bdaddr bdaddr_ind;
    
	if (SinkGetBdAddr(sink, &bdaddr_ind))
	{
		uint8 lAttributes[ATTRIBUTE_SIZE];
		
		avrcpConnectReq(bdaddr_ind, FALSE);
					
		/* Retrieve attributes for this device */
    	if (ConnectionSmGetAttributeNow(PSKEY_ATTRIBUTE_BASE, &bdaddr_ind, ATTRIBUTE_SIZE, lAttributes))
		{
			bool write_params = FALSE;
			if (lAttributes[attribute_seid] != seid)
			{
				lAttributes[attribute_seid] = seid;
				write_params = TRUE;
			}
			if (lAttributes[attribute_a2dp_volume] != theHeadset.gAvVolumeLevel)
			{
				lAttributes[attribute_a2dp_volume] = theHeadset.gAvVolumeLevel;
				write_params = TRUE;
			}
			if (write_params)
			{
				/* Write params to PS */
				ConnectionSmPutAttribute(PSKEY_ATTRIBUTE_BASE, &bdaddr_ind, ATTRIBUTE_SIZE, lAttributes); 
				A2DP_MSG_DEBUG(("A2DP: Store A2DP attributes [%d][%d][%d][%d][%d][%d]\n",lAttributes[0],
			   		lAttributes[1],lAttributes[2],lAttributes[3],lAttributes[4],lAttributes[5])) ;
			}
		}
	    
		theHeadset.seid = seid;
		theHeadset.last_used_seid = seid;
		A2DP_MSG_DEBUG(("    Selected SEID = %d\n", seid));
		stateManagerEnterA2dpConnectedState();
		PROFILE_MEMORY(("A2DPOpen"))
	}
	else
	{
		A2DP_MSG_DEBUG(("    Can't find BDA associated with sink 0x%x\n", (uint16)sink));
	}
}

static void handleA2DPOpenCfm(a2dp_status_code status, Sink sink, uint8 seid)
{
	theHeadset.a2dpConnecting = FALSE;
	
	if (status == a2dp_success)
	{
		A2DP_MSG_DEBUG(("Open Success\n"));
		handleA2DPOpenInd(sink, seid);
		
		/* Start the Streaming */
		A2dpStart(theHeadset.a2dp);
	}
	else
	{
		A2DP_MSG_DEBUG(("Open Failure [result = %d]\n", status));
		
		/* reset flag as media connection failed */
		theHeadset.sendPlayOnConnection = FALSE;
	}
}

static void handleA2DPStartInd(A2DP_START_IND_T *msg)
{
	if (!stateManagerIsA2dpConnected())
	{
		A2DP_MSG_DEBUG(("    Not Connected - Ignoring\n"));
		return;
	}
	
    if (!A2dpGetMediaSink(theHeadset.a2dp) || (stateManagerIsA2dpStreaming()))
        return;
    
    if (HfpGetAudioSink(theHeadset.hfp_hsp) || hfpSlcIsConnecting())
    {
        /* SCO is active or currently connecting SLC so don't start AV */
        A2dpSuspend(theHeadset.a2dp);
		return;
    }

	streamControlConnectA2dpAudio();
	
	/* Always enter the streaming state, and assume the music is playing */
	stateManagerEnterA2dpStreamingState();
	
	/* Set link policy for streaming  */
	linkPolicyStreamConnect();
}

static void handleA2DPStartCfm(A2DP_START_CFM_T *msg)
{
    if (msg->status == a2dp_success)
    {   
        A2DP_MSG_DEBUG(("Start Success\n"));
        /* start Kalimba decoding if it isn't already */
        if (!stateManagerIsA2dpStreaming())
        {			
            if (HfpGetAudioSink(theHeadset.hfp_hsp) || hfpSlcIsConnecting())
            {
                /* 
                    SCO has become active while we were waiting for a START_CFM (or SLC
					is connecting).
				    AV doesn't want to be streaming now, so we must try to 
				    suspend the source again.
				*/
                A2dpSuspend(theHeadset.a2dp);
                return;
            }

			streamControlConnectA2dpAudio();
        } 
		
		/* The A2DP state needs to be set based on what the headset thinks is the playing status of the media */
		if (theHeadset.PlayingState || theHeadset.sendPlayOnConnection || IsA2dpSourceAnAg())       
		    stateManagerEnterA2dpStreamingState();
		else
			stateManagerEnterA2dpPausedState();
		
		if (theHeadset.sendPlayOnConnection && theHeadset.manualA2dpConnect)
		{
			sendPlayOnAvrcpConnection();
		}
		
		/* Set link policy for streaming  */
		linkPolicyStreamConnect();
    }
    else
    {
        A2DP_MSG_DEBUG(("Start Failed [Status %d]\n", msg->status));
		
		/* Workaraound for Samsung phones to start audio playing once media is connected.
 		   The phone sends an avdtp_start as soon as it receives the avdtp_open, so the
		   avdtp_start sent from the headset end will fail. But even though it is in the streaming
		   state we still need to send an avrcp_play here to start the music.
		*/
		if (stateManagerIsA2dpStreaming() && theHeadset.manualA2dpConnect)
		{
			sendPlayOnAvrcpConnection();
		}
    }
	/* reset flag regardless of result code */
	theHeadset.sendPlayOnConnection = FALSE;
}

static void handleA2DPSuspendInd(A2DP_SUSPEND_IND_T *msg)
{
	if (!stateManagerIsA2dpStreaming())
	{
		A2DP_MSG_DEBUG(("    Not Streaming - Ignoring\n"));
		return;
	}
    stateManagerEnterA2dpConnectedState();

    streamControlCeaseA2dpStreaming(FALSE);
	
	/* Set link policy for stop streaming  */
	linkPolicyStreamDisconnect();
}

static void handleA2DPSuspendCfm(A2DP_SUSPEND_CFM_T *msg)
{
	if (!stateManagerIsA2dpConnected())
	{
		A2DP_MSG_DEBUG(("    Not Connected - Ignoring\n"));
		return;
	}
    if (msg->status == a2dp_success)
    {
        A2DP_MSG_DEBUG(("Suspend Success\n"));
        
	    if (theHeadset.dsp_process == dsp_process_a2dp)
        {
            /* We must have had a stream restart at this end occuring so restart AV source */
            A2dpStart(theHeadset.a2dp);
			if (theHeadset.features.autoSendAvrcp)
            	avrcpSendPlay();
			return;
        }
		else
        {
            /* We have suspended the A2DP source. */
			theHeadset.a2dpSourceSuspended = TRUE;
        }
		
        stateManagerEnterA2dpConnectedState();
		
		/* Set link policy for stop streaming  */
		linkPolicyStreamDisconnect();
    }
    else
    {
        A2DP_MSG_DEBUG(("Suspend Failed [Status %d]\n", msg->status));   
    }
}

static void handleA2DPClose(void)
{
    PROFILE_MEMORY(("A2DPClose"))
			
	if (stateManagerIsA2dpStreaming())
	{
		/* Set link policy for stop streaming  */
		linkPolicyStreamDisconnect();
	}
			
    streamControlCeaseA2dpStreaming(FALSE);
	
	theHeadset.a2dpSourceSuspended = FALSE;
	
    /* Change state */	
	if (A2dpGetSignallingSink(theHeadset.a2dp))
		stateManagerEnterA2dpConnectedState();
	else
		stateManagerEnterA2dpConnectableState(FALSE);
    
	theHeadset.seid = 0;
}

static void handleA2DPCodecSettingsInd(A2DP_CODEC_SETTINGS_IND_T *msg)
{
	codec_data_type	codecData;
	
	theHeadset.a2dp_channel_mode = msg->channel_mode;
	
	switch (msg->rate)
	{
	case 48000:
		theHeadset.a2dp_rate = a2dp_rate_48_000k;
		break;
	case 44100:
		theHeadset.a2dp_rate = a2dp_rate_44_100k;
		break;
	case 32000:
		theHeadset.a2dp_rate = a2dp_rate_32_000k;
		break;
	case 24000:
		theHeadset.a2dp_rate = a2dp_rate_24_000k;
		break;
	case 22050:
		theHeadset.a2dp_rate = a2dp_rate_22_050k;
		break;
	case 16000:
	default:
		theHeadset.a2dp_rate = a2dp_rate_16_000k;
		break;
	}
	
	codecData = msg->codecData;
	
	/* Get rid of the packet_size as it isn't needed for the sink, and we're short of global space */
	theHeadset.a2dp_data.codecData.content_protection = codecData.content_protection;
	theHeadset.a2dp_data.codecData.voice_rate = codecData.voice_rate;
	theHeadset.a2dp_data.codecData.bitpool = codecData.bitpool;
	theHeadset.a2dp_data.codecData.format = codecData.format;	
	
	A2DP_MSG_DEBUG(("	chn_mode=%d rate=0x%x\n",theHeadset.a2dp_channel_mode,theHeadset.a2dp_rate));
	A2DP_MSG_DEBUG(("	content_protection=%d voice_rate=0x%lx bitpool=0x%x format=0x%x\n",
																	codecData.content_protection,
																	codecData.voice_rate,
																	codecData.bitpool,
																	codecData.format
																	));

}

static void handleA2DPSignallingConnected(a2dp_status_code status, A2DP *a2dp, Sink sink)
{
	bdaddr bdaddr_ind;
	bool bdaddr_retrieved = FALSE;
    
    theHeadset.a2dpConnecting = FALSE;	
    
    if (status != a2dp_success)
    {
        A2DP_MSG_DEBUG(("Signalling Failed [Status %d]\n", status));
		
		/* This connection failed so check if inquiry needs to resume */
		inquiryContinue();
		
		if (theHeadset.LinkLossAttemptA2dp && (++theHeadset.LinkLossAttemptA2dp <= theHeadset.features.LinkLossRetries))
		{
			/* Still reconnection attempts after link loss to try */
			A2DP_MSG_DEBUG(("A2DP : Link loss connection attempt %d \n",theHeadset.LinkLossAttemptA2dp)) ; 
			/* try next reconnect in several secs */
			MessageSendLater(&theHeadset.task, APP_CONNECT_A2DP_LINK_LOSS, 0, D_SEC(LINK_LOSS_RETRY_TIME_SECS));
			return;
		}
		theHeadset.LinkLossAttemptA2dp = 0;
		if (theHeadset.a2dp_list_index != 0xf)
		{
			/* Connect to next A2DP source in list */
			MessageSendLater(&theHeadset.task, APP_CONTINUE_A2DP_LIST_CONNECTION , 0, 0);
			return;
		}
        /* Send event to signify that reconnection attempt failed */
		MessageSend(&theHeadset.task, EventA2dpReconnectFailed, 0);
        return;
    }
	
	/* Stop any pending inquiry now */
    inquiryStop();
	
	theHeadset.LinkLossAttemptA2dp = 0;
	theHeadset.a2dp_list_index = 0xf;
	theHeadset.switch_a2dp_source = FALSE;
	
	/* If there is already a signalling channel connected, then disconnect this new connection */
	if (theHeadset.a2dp)
    {      
       A2dpDisconnectAll(a2dp);
	   return;
    }
    
    A2DP_MSG_DEBUG(("Signalling Success\n"));
	
	theHeadset.a2dp = a2dp;
    
    /* We are now connected */
    if (!stateManagerIsA2dpStreaming())
        stateManagerEnterA2dpConnectedState(); 	
	
	/* Ensure the underlying ACL is encrypted */       
    ConnectionSmEncrypt( &theHeadset.task , sink , TRUE );
	
	/* If we were in pairing mode then update HFP state also */
	if (stateManagerGetHfpState() == headsetConnDiscoverable)
		stateManagerEnterHfpConnectableState(FALSE);
	
	/* If the headset is off then disconnect */
	if (stateManagerGetHfpState() == headsetPoweringOn)
    {      
       a2dpDisconnectRequest();
    }
	else
	{
	   uint8 lAttributes[ATTRIBUTE_SIZE];
	   
	   MessageSend ( &theHeadset.task , EventA2dpConnected , 0 );
	   
	   bdaddr_retrieved = SinkGetBdAddr(sink, &bdaddr_ind);
    	/* Establish an AVRCP connection if required */
		if (bdaddr_retrieved)
	   	{
			avrcpConnectReq(bdaddr_ind, TRUE);

			/* Retrieve attributes for this device */
    		if (ConnectionSmGetAttributeNow(PSKEY_ATTRIBUTE_BASE, &bdaddr_ind, ATTRIBUTE_SIZE, lAttributes))
			{
				if (lAttributes[attribute_a2dp_profile])	
				{
					theHeadset.gAvVolumeLevel = lAttributes[attribute_a2dp_volume];
				}
				else
				{
					theHeadset.gAvVolumeLevel = theHeadset.config->gVolLevels.defaultA2dpVolLevel;
				}
				if (lAttributes[attribute_clock_mismatch])	
				{
					theHeadset.clock_mismatch_rate = lAttributes[attribute_clock_mismatch];
				}
				A2DP_MSG_DEBUG(("A2DP: Read A2DP attributes [%d][%d][%d][%d][%d][%d]\n",lAttributes[0],
				   lAttributes[1],lAttributes[2],lAttributes[3],lAttributes[4],lAttributes[5])) ;
			}
			else
			{
				theHeadset.gAvVolumeLevel = theHeadset.config->gVolLevels.defaultA2dpVolLevel;
			}
    		/* always shuffle the pdl into mru order */        
    		ConnectionSmUpdateMruDevice(&bdaddr_ind) ;
			
			/* Store last used A2DP */
			theHeadset.LastDevices->lastA2dpConnected = bdaddr_ind;
		}
		
		/* Update Link Policy as A2DP has connected. */
		linkPolicyA2dpSigConnect();
	}
}

static void handleA2DPSignallingDisconnected(A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND_T *msg)
{   	
    /* Change to ready state if no media channel open */
    if (!A2dpGetMediaSink(theHeadset.a2dp))
	    stateManagerEnterA2dpConnectableState(FALSE);
		
	theHeadset.a2dp = 0; 
	
	avrcpDisconnectReq();
		    
	if (msg->status == a2dp_disconnect_link_loss)
	{
		/* Reconnect on link loss */
		A2DP_MSG_DEBUG(("A2DP: Link Loss Detect\n")) ;
               
        MessageSend( &theHeadset.task , EventLinkLoss , 0 ) ;
		
		/* Decide on reconnection behaviour, only if the headset is configured to reconnect on link loss */
		if (theHeadset.features.LinkLossRetries)
		{
			if (theHeadset.combined_link_loss)
			{
				/* HFP on this device already suffered link loss so connect HFP first */
				theHeadset.slcConnectFromPowerOn = TRUE;
				theHeadset.LinkLossAttemptHfp = 1;
				MessageSend(&theHeadset.task, APP_CONNECT_HFP_LINK_LOSS, 0);
			}
			else if (HfpGetSlcSink(theHeadset.hfp_hsp) && !BdaddrIsZero(&theHeadset.LastDevices->lastHfpConnected) && !BdaddrIsZero(&theHeadset.LastDevices->lastA2dpConnected))
			{
				if (BdaddrIsSame(&theHeadset.LastDevices->lastHfpConnected, &theHeadset.LastDevices->lastA2dpConnected))
				{
					/* Flag to indicate that HFP should be reconnected first. It's still connected but should suffer link loss soon. */
					theHeadset.combined_link_loss = TRUE;
				}
				else
				{
					/* Connect to standalone A2DP source */
					theHeadset.LinkLossAttemptA2dp = 1;
					a2dpEstablishConnection(FALSE, FALSE);
				}
			}
			else	
			{
				/* Standard reconnect procedure for A2DP */
				theHeadset.LinkLossAttemptA2dp = 1;
				a2dpEstablishConnection(FALSE, FALSE);
			}
		}
	}
	else
	{
		theHeadset.combined_link_loss = FALSE;
	}
	
	MessageSend ( &theHeadset.task , EventA2dpDisconnected , 0 );
	
	
	if (!BdaddrIsZero(&theHeadset.LastDevices->lastA2dpConnected))
	{
		uint8 lAttributes[ATTRIBUTE_SIZE];
		/* Retrieve attributes for this device */
    	if (ConnectionSmGetAttributeNow(PSKEY_ATTRIBUTE_BASE, &theHeadset.LastDevices->lastA2dpConnected, ATTRIBUTE_SIZE, lAttributes))
		{
			bool write_params = FALSE;
			if (!lAttributes[attribute_a2dp_profile])
			{
				lAttributes[attribute_a2dp_profile] = 1;
				write_params = TRUE;
			}
			if (lAttributes[attribute_a2dp_volume] != theHeadset.gAvVolumeLevel)
			{
				lAttributes[attribute_a2dp_volume] = theHeadset.gAvVolumeLevel;
				write_params = TRUE;
			}
			if (lAttributes[attribute_clock_mismatch] != theHeadset.clock_mismatch_rate)
			{
				lAttributes[attribute_clock_mismatch] = theHeadset.clock_mismatch_rate;
				write_params = TRUE;
			}
			if (write_params)
			{
				/* Write params to PS */
				ConnectionSmPutAttribute(PSKEY_ATTRIBUTE_BASE, &theHeadset.LastDevices->lastA2dpConnected, ATTRIBUTE_SIZE, lAttributes); 
				A2DP_MSG_DEBUG(("A2DP: Store A2DP attributes [%d][%d][%d][%d][%d][%d]\n",lAttributes[0],
			   		lAttributes[1],lAttributes[2],lAttributes[3],lAttributes[4],lAttributes[5])) ;
			}
		}
	}
	
	theHeadset.clock_mismatch_rate = 0;
	
	if (theHeadset.switch_a2dp_source)
	{
		if ((stateManagerGetHfpState() != headsetPoweringOn) && !a2dpIsConnecting())
		{
			/* This was a switch source disconnect, so now connect to next A2DP source */
			theHeadset.a2dp_list_index = 0;
			MessageSendLater(&theHeadset.task, APP_CONTINUE_A2DP_LIST_CONNECTION , 0, 0);
		}
	}
	
	/* Update Link Policy as A2DP has disconnected. */
	linkPolicyA2dpSigDisconnect();
		
	PROFILE_MEMORY(("A2DPSigClose"))
}

/****************************************************************************
  INTERFACE FUNCTIONS
*/
void handleA2DPMessage( Task task, MessageId id, Message message )
{
    switch (id)
    {
    case A2DP_INIT_CFM:
        A2DP_MSG_DEBUG(("A2DP_INIT_CFM : \n"));
        handleA2DPInitCfm((A2DP_INIT_CFM_T *) message);
        break;
		
	case A2DP_SIGNALLING_CHANNEL_CONNECT_IND:
        A2DP_MSG_DEBUG(("A2DP_SIGNALLING_CHANNEL_CONNECT_IND : \n"));
	
		/* Stop any pending inquiry now */
    	inquiryStop();
	
		if (!stateManagerIsA2dpConnected() && (stateManagerGetHfpState() != headsetPoweringOn))
        {
            A2DP_MSG_DEBUG(("Accept\n"));
			A2dpConnectSignallingChannelResponse(((A2DP_SIGNALLING_CHANNEL_CONNECT_IND_T *)message)->a2dp,
											 TRUE,
											 ((A2DP_SIGNALLING_CHANNEL_CONNECT_IND_T *)message)->connection_id,
											 theHeadset.a2dp_data.sep_entries);
        }
		else
        {
            A2DP_MSG_DEBUG(("Reject\n"));
			A2dpConnectSignallingChannelResponse(((A2DP_SIGNALLING_CHANNEL_CONNECT_IND_T *)message)->a2dp,
											 FALSE,
											 ((A2DP_SIGNALLING_CHANNEL_CONNECT_IND_T *)message)->connection_id,
											 theHeadset.a2dp_data.sep_entries);
        }
		theHeadset.LinkLossAttemptA2dp = 0;
		break;
		
	case A2DP_SIGNALLING_CHANNEL_CONNECT_CFM:
        A2DP_MSG_DEBUG(("A2DP_SIGNALLING_CHANNEL_CONNECT_CFM : \n"));
		handleA2DPSignallingConnected(((A2DP_SIGNALLING_CHANNEL_CONNECT_CFM_T*)message)->status, ((A2DP_SIGNALLING_CHANNEL_CONNECT_CFM_T*)message)->a2dp, ((A2DP_SIGNALLING_CHANNEL_CONNECT_CFM_T*)message)->sink);
		break;
        
    case A2DP_OPEN_IND:
        A2DP_MSG_DEBUG(("A2DP_OPEN_IND : \n"));
    	handleA2DPOpenInd(((A2DP_OPEN_IND_T*)message)->media_sink, ((A2DP_OPEN_IND_T*)message)->seid);
        break;
    case A2DP_OPEN_CFM:
        A2DP_MSG_DEBUG(("A2DP_OPEN_CFM : \n"));
    	handleA2DPOpenCfm(((A2DP_OPEN_CFM_T*)message)->status, ((A2DP_OPEN_CFM_T*)message)->media_sink, ((A2DP_OPEN_CFM_T*)message)->seid);
        break;
        
    case A2DP_CONNECT_OPEN_CFM:
        A2DP_MSG_DEBUG(("A2DP_CONNECT_OPEN_CFM : \n"));
        handleA2DPSignallingConnected(((A2DP_CONNECT_OPEN_CFM_T*)message)->status, ((A2DP_CONNECT_OPEN_CFM_T*)message)->a2dp, ((A2DP_CONNECT_OPEN_CFM_T*)message)->signalling_sink);
    	handleA2DPOpenCfm(((A2DP_CONNECT_OPEN_CFM_T*)message)->status, ((A2DP_CONNECT_OPEN_CFM_T*)message)->media_sink, ((A2DP_CONNECT_OPEN_CFM_T*)message)->seid);
        break;
                    
    case A2DP_START_IND:
        A2DP_MSG_DEBUG(("A2DP_START_IND : \n"));
    	handleA2DPStartInd((A2DP_START_IND_T*)message);
        break;
    case A2DP_START_CFM:
        A2DP_MSG_DEBUG(("A2DP_START_CFM : \n"));
    	handleA2DPStartCfm((A2DP_START_CFM_T*)message);
        break;
        
    case A2DP_SUSPEND_IND:
        A2DP_MSG_DEBUG(("A2DP_SUSPEND_IND : \n"));
    	handleA2DPSuspendInd((A2DP_SUSPEND_IND_T*)message);
        break;
    case A2DP_SUSPEND_CFM:
        A2DP_MSG_DEBUG(("A2DP_SUSPEND_CFM : \n"));
    	handleA2DPSuspendCfm((A2DP_SUSPEND_CFM_T*)message);
        break;
        
    case A2DP_CLOSE_IND:
        A2DP_MSG_DEBUG(("A2DP_CLOSE_IND : \n"));
        handleA2DPClose();
        break;
        
    case A2DP_CLOSE_CFM:
        A2DP_MSG_DEBUG(("A2DP_CLOSE_CFM : \n"));
    	handleA2DPClose();
        break;
        
    case A2DP_CODEC_SETTINGS_IND:
        A2DP_MSG_DEBUG(("A2DP_CODEC_SETTINGS_IND : \n"));
    	handleA2DPCodecSettingsInd((A2DP_CODEC_SETTINGS_IND_T*)message);
        break;
            
	case A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND:
        A2DP_MSG_DEBUG(("A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND : \n"));
    	handleA2DPSignallingDisconnected((A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND_T*)message);
		break;
		
	case A2DP_ENCRYPTION_CHANGE_IND:
        A2DP_MSG_DEBUG(("A2DP_ENCRYPTION_CHANGE_IND : \n"));
		break;
			
    default:       
		A2DP_MSG_DEBUG(("A2DP UNHANDLED MSG: 0x%x\n",id));
        break;
    }    
}
