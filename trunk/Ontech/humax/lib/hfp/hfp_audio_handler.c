/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_audio_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_audio_handler.h"
#include "hfp_common.h"

#include <bdaddr.h>
#include <panic.h>

/* Default parameters for an S1 esco connection.  Configured to obtain best ESCO link possible. */
static const sync_config_params default_s1_sync_config_params =
{
    8000,                    /* tx_bandwidth   */
    8000,                    /* rx_bandwidth   */
    0x7,                     /* max_latency    */
    sync_air_coding_cvsd,    /* voice_settings */
    sync_retx_power_usage,   /* retx_effort    */
    (sync_ev3 | sync_all_edr_esco)   /* packet_type    */
};


/* Set hfp's audio parameters to imply no connection */
static void resetAudioParams (HFP *hfp)
{
    hfp->audio_connection_state = hfp_audio_disconnected;
    hfp->audio_sink = 0;
}


/* Inform the app of the status of the audio (Synchronous) connection */
static void sendAudioConnectCfmFailToApp(HFP *hfp, hfp_audio_connect_status status)
{
	/* Send a cfm message to the application. */
	MAKE_HFP_MESSAGE(HFP_AUDIO_CONNECT_CFM);
	message->hfp = hfp;
    message->status = status;
	message->audio_sink = hfp->audio_sink;
	message->rx_bandwidth = 0;
	message->tx_bandwidth = 0;
	message->link_type = sync_link_unknown;
	MessageSend(hfp->clientTask, HFP_AUDIO_CONNECT_CFM, message);
    
}


/* Inform the app of the status of the audio (Synchronous) disconnection */
static void sendAudioDisconnectIndToApp(HFP *hfp, hfp_audio_disconnect_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_AUDIO_DISCONNECT_IND, hfp, status);
}


/* Attempt to create an audio (Synchronous) connection.  Due to firmware operation, eSCO and SCO must be requested
   separately. */
static void startAudioConnectRequest(HFP *hfp, sync_pkt_type packet_type, const hfp_audio_params *audio_params)
{
    sync_config_params config_params;
    
        
    /* Store connection parameters for later use */
    hfp->audio_packet_type = packet_type;
    hfp->audio_packet_type_to_try = packet_type;
    hfp->audio_params = *audio_params;

    /* determine if esco or edr (inverted logic) packet types requested */
    if ( packet_type & sync_all_esco )  
        {
        /* Attempt to open an eSCO connection */
        hfp->audio_connection_state = hfp_audio_connecting_esco;
        /* set packet type as the passed in esco and edr bits */
        config_params.packet_type = packet_type & (sync_all_esco | sync_all_edr_esco);
        config_params.retx_effort = audio_params->retx_effort;
    }
    else
    {   /* Attempt to open a SCO connection */
        hfp->audio_connection_state = hfp_audio_connecting_sco;
        /* mask out esco and add edr bits (inverted logic) */
        config_params.packet_type = ((packet_type & sync_all_sco) | sync_all_edr_esco);
        /* No re-transmissions for SCO */            
        config_params.retx_effort = sync_retx_disabled;           
    }
        
    config_params.tx_bandwidth = audio_params->bandwidth;
    config_params.rx_bandwidth = audio_params->bandwidth;
    config_params.max_latency = audio_params->max_latency;
    config_params.voice_settings = audio_params->voice_settings;
        
    /* Issue a Synchronous connect request to the connection lib */
    ConnectionSyncConnectRequest(&hfp->task, hfp->sink, &config_params);
}


/* Continue with attempt to create an audio (Synchronous) connection.  Due to firmware operation, eSCO and SCO must be requested
   separately. */
static void continueAudioConnectRequest(HFP *hfp)
{
    sync_config_params config_params;
    
  
    /* On entry, hfp->audio_packet_type will contain the packet types last attempted.
       Continue with connection attempt if we tried a packet type > hv1 last time around. */
    if ( hfp->audio_packet_type_to_try != (sync_hv1 | sync_all_edr_esco) )
    {         
        /* if EDR bits attempted, try esco only by removing SCO and EDR bits */
        if(((hfp->audio_packet_type_to_try & sync_all_edr_esco) != sync_all_edr_esco) && 
		   (hfp->audio_packet_type_to_try != (sync_ev3 | sync_all_edr_esco)))
        {
        	/* if ESCO bits attempted that weren't EV3 only try EV3 with S1 settings */
            hfp->audio_packet_type_to_try = (sync_ev3 | sync_all_edr_esco);            
        }
        /* now try all enabled sco packet bits */
        else if(hfp->audio_packet_type_to_try == (sync_ev3 | sync_all_edr_esco))            
        {
            /* set to the passed in sco enable bits */
            hfp->audio_packet_type_to_try = ((hfp->audio_packet_type & sync_all_sco) | sync_all_edr_esco);
        }
        /* now down to SCO packet types, try HV1 only before giving up */
        else
        {
            hfp->audio_packet_type_to_try = (sync_hv1 | sync_all_edr_esco);            
        }   
                   
        hfp->audio_connection_state = hfp_audio_connecting_sco;
        config_params.tx_bandwidth = 8000;
        config_params.rx_bandwidth = 8000;
        config_params.retx_effort = sync_retx_disabled;
        config_params.packet_type = hfp->audio_packet_type_to_try;
        config_params.max_latency = hfp->audio_params.max_latency;
        config_params.voice_settings = hfp->audio_params.voice_settings;
        
        /* when trying EV3 S1 settings, substitute the S1 settings instead of user supplied settings */
        if(hfp->audio_packet_type_to_try == (sync_ev3 | sync_all_edr_esco))
        {    
            /* Issue a Synchronous connect request to the connection lib */
            ConnectionSyncConnectRequest(&hfp->task, hfp->sink, &default_s1_sync_config_params);
        }
        /* all non S1 EV3 attempts */
        else
        {
            /* Issue a Synchronous connect request to the connection lib */
            ConnectionSyncConnectRequest(&hfp->task, hfp->sink, &config_params);
        }
    }
    else
    {   /* All connection attempts have failed - give up */
        resetAudioParams(hfp);
        
        /* Inform app that connect failed */
	    sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_failure);
    }
}


/* Accept/reject an incoming audio connect request */
static void audioConnectResponse(HFP *hfp, bool response, sync_pkt_type packet_type, const hfp_audio_params *audio_params, bdaddr bd_addr)
{    
    sync_config_params config_params;
    
    /* If connection request is being rejected, don't worry about validating params being returned */
    if ( !response )
    {
        /* To send the response we need the bd addr of the underlying connection. */
        ConnectionSyncConnectResponse(&hfp->task, &bd_addr, FALSE, 0);

		resetAudioParams(hfp);
			
		/* Inform app that connect failed */
	    sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_failure);
    }
    else 
    {
        config_params.tx_bandwidth = audio_params->bandwidth;
        config_params.rx_bandwidth = audio_params->bandwidth;
        config_params.max_latency = audio_params->max_latency;
        config_params.voice_settings = audio_params->voice_settings;
        config_params.packet_type = packet_type;

        config_params.retx_effort = audio_params->retx_effort;      
    
        /* To send the response we need the bd addr of the underlying connection. */
        ConnectionSyncConnectResponse(&hfp->task, &bd_addr, TRUE, &config_params);
    }
}


/* Disconnect an existing audio (Synchronous) connection */
static void audioDisconnectRequest(HFP *hfp)
{
	/* Send a disconnect request to the connection lib */
    hfp->audio_connection_state = hfp_audio_disconnecting;
    ConnectionSyncDisconnect(hfp->audio_sink, hci_error_oetc_user);
}


/****************************************************************************
NAME	
	hfpManageSyncDisconnect

DESCRIPTION
    Used to inform hfp of a synchronous (audio) disconnection.
    
RETURNS
	void
*/
void hfpManageSyncDisconnect(HFP *hfp)
{
    if ( hfp->audio_connection_state!=hfp_audio_disconnected )
    {
        /* Reset the audio handle */
        resetAudioParams(hfp);
        sendAudioDisconnectIndToApp(hfp, hfp_audio_disconnect_success);
    }
}


/****************************************************************************
NAME	
	hfpHandleSyncConnectInd

DESCRIPTION
	Incoming audio notification, accept if we recognise the sink reject
	otherwise.

RETURNS
	void
*/
void hfpHandleSyncConnectInd(HFP *hfp, const CL_DM_SYNC_CONNECT_IND_T *ind)
{
    uint16 i=0;
    uint16 my_audio = 0;
    uint16 num_sinks = 5;

    /* Sink array to store the sinks on the acl. */
    Sink *all_sinks = (Sink *)PanicNull(calloc(num_sinks, sizeof(Sink))); 

    if(StreamSinksFromBdAddr(&num_sinks, all_sinks, &ind->bd_addr))
    {
        for (i=0; i<num_sinks; i++)
        {
            /* Make sure this profile instance owns the unlerlying sink */
            if (all_sinks[i] && all_sinks[i] == hfp->sink ) 
            {
                    /*we can only accept an incoming audio conn if one is not already
                    in progress */                    
               	if ( hfp->audio_connection_state!=hfp_audio_connecting_esco &&
   	                 hfp->audio_connection_state!=hfp_audio_connecting_sco &&
                     hfp->audio_connection_state!=hfp_audio_accepting )
                { 
                    my_audio = 1;
                    break;
                }
                
            }
        }
    }

    free(all_sinks);    

    /* If this is our audio connection then ask the app, otherwise reject outright */
    if (my_audio)
    {
        MAKE_HFP_MESSAGE(HFP_AUDIO_CONNECT_IND);
        /* also add the bd_addr address to the message to remove the need for SinkGetBdAddr 
           which fails due to a race condition */
    	message->hfp = hfp;
        message->bd_addr = ind->bd_addr;                
    	MessageSend(hfp->clientTask, HFP_AUDIO_CONNECT_IND, message);
        hfp->audio_connection_state = hfp_audio_accepting;
    }
    else
    {
        ConnectionSyncConnectResponse(&hfp->task, &ind->bd_addr, FALSE, 0);
    }
}


/****************************************************************************
NAME	
	hfpHandleSyncConnectIndReject

DESCRIPTION
	Incoming audio notification, reject outright, profile is in the wrong state.
	This is probably a audio ind for a different task.

RETURNS
	void
*/
void hfpHandleSyncConnectIndReject(HFP *hfp, const CL_DM_SYNC_CONNECT_IND_T *ind)
{
    /* Reject the Synchronous connect ind outright, we're in the wrong state */
    ConnectionSyncConnectResponse(&hfp->task, &ind->bd_addr, FALSE, 0);
}


/****************************************************************************
NAME	
	hfpHandleSyncConnectCfm

DESCRIPTION
	Confirmation in response to an audio (SCO/eSCO) open request indicating 
    the outcome of the Synchronous connect attempt.

RETURNS
	void
*/
void hfpHandleSyncConnectCfm(HFP *hfp, const CL_DM_SYNC_CONNECT_CFM_T *cfm)
{
   	if ( hfp->audio_connection_state==hfp_audio_connecting_esco ||
   	     hfp->audio_connection_state==hfp_audio_connecting_sco ||
         hfp->audio_connection_state==hfp_audio_accepting )
    {
    	/* Informs us of the outcome of the Synchronous connect attempt */
    	if (cfm->status == hci_success)
    	{
    	    /* Construct cfm message for the application. */
    	    MAKE_HFP_MESSAGE(HFP_AUDIO_CONNECT_CFM);
        	message->hfp = hfp;
            message->status = hfp_audio_connect_success;
    	    message->audio_sink = cfm->audio_sink;
    	    message->rx_bandwidth = cfm->rx_bandwidth;
    	    message->tx_bandwidth = cfm->tx_bandwidth;
    	    message->link_type = cfm->link_type;
    
    	    /* store the audio sink */
    	    hfp->audio_sink = cfm->audio_sink;
    	    hfp->audio_connection_state = hfp_audio_connected;
    
    	    /* Inform app of connection success */	
    		MessageSend(hfp->clientTask, HFP_AUDIO_CONNECT_CFM, message);
    	
    	    if (supportedProfileIsHsp(hfp->hfpSupportedProfile))
    	    {
    		    hfpSetState(hfp, hfpActiveCall);
    	    }
    	}
    	else 
    	{
        	/* Give up if we are either attempting to accept in incoming connection or error code
        	   indicates it is pointless to continue. */
        	if ( hfp->audio_connection_state==hfp_audio_accepting )
        	{
                resetAudioParams(hfp);
        	
        	    /* Inform app that connect failed */
        	    sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_failure);
        	}
        	else
        	{
        	    /* This step failed, move onto next stage of connection attempt */
    		    continueAudioConnectRequest(hfp);
    	    }
    	}
	}
    else
    {
        /* Should never get here */
        HFP_DEBUG(("hfpHandleSyncConnectCfm invalid state %d",hfp->audio_connection_state));
        
        resetAudioParams(hfp);
	
	    /* Inform app that connect failed */
	    sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_failure);
        
    }
}

/****************************************************************************
NAME	
	hfpHandleSyncDisconnectInd

DESCRIPTION
	Audio (Synchronous) connection has been disconnected 

RETURNS
	void
*/
void hfpHandleSyncDisconnectInd(HFP *hfp, const CL_DM_SYNC_DISCONNECT_IND_T *ind)
{
    /* If it's not our sink, silently ignore this indication */
    if ( ind->audio_sink == hfp->audio_sink )
    {
        if ( hfp->audio_connection_state==hfp_audio_connected ||
             hfp->audio_connection_state==hfp_audio_disconnecting )
        { 
    	    /* Inform the app */ 
    	    if (ind->status == hci_success)
            {
                hfpManageSyncDisconnect(hfp);
            }
    	    else
            {
                /* Disconnect has failed, we are still connected - inform the app */
        	    sendAudioDisconnectIndToApp(hfp, hfp_audio_disconnect_failure);
            }
            
    	    /* Update the local state. Check current state in case SLC disc has beaten the SCO/eSCO disc */
    	    if (supportedProfileIsHsp(hfp->hfpSupportedProfile) && (hfp->state != hfpReady))
            {
    		    hfpSetState(hfp, hfpSlcConnected);
            }
        }
        else
        {
            /* Should never get here */
            HFP_DEBUG(("hfpHandleSyncDisconnectInd invalid state %d\n",hfp->audio_connection_state));
            
            resetAudioParams(hfp);
	
	        /* Inform app that connect failed */
	        sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_failure);
        }
    }
}


/****************************************************************************
NAME	
	hfpHandleAudioConnectReq

DESCRIPTION
	Transfer the audio from the AG to the HF or vice versa depending on which
	device currently has it.

RETURNS
	void
*/
void hfpHandleAudioConnectReq(HFP *hfp, const HFP_INTERNAL_AUDIO_CONNECT_REQ_T *req)
{
	/* If we don't have a valid audio handle, open a Synchronous connection */
    switch ( hfp->audio_connection_state )
    {
    case hfp_audio_disconnected:
		startAudioConnectRequest(hfp, req->packet_type, &req->audio_params);
        break;
    case hfp_audio_connecting_esco:
    case hfp_audio_connecting_sco:
    case hfp_audio_accepting:
		/* Already attempting to create an audio connection - indicate a fail for this attempt */
		sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_in_progress);
        break;
    case hfp_audio_disconnecting:   /* Until disconnect is complete, assume we have a connection */
    case hfp_audio_connected:
		/* Already have an audio connection - indicate a fail for this attempt */
		sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_have_audio);
		break;
	default:
       HFP_DEBUG(("hfpHandleAudioConnectReq invalid state %d\n",hfp->audio_connection_state));
       break;
    }
}


/****************************************************************************
NAME	
	hfpHandleAudioConnectReqError

DESCRIPTION
	The app has requested that the audio be transferred (either to or from the AG).
	If the profile instance was in the wrong state for this operation to be 
	performed we need to send an immediate response to the app indicating an
	error has ocurred. We send the a audio status message with the status code 
	set to hfp_fail. We determine which message to send by looking at the action
	the app has requested and the current state of the HFP instance.

RETURNS
	void
*/
void hfpHandleAudioConnectReqError(HFP *hfp, const HFP_INTERNAL_AUDIO_CONNECT_REQ_T *req)
{
    /* Send error message to inform the app we didn't open a Synchronous connection */
    sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_error);
}


/****************************************************************************
NAME	
	hfpHandleAudioConnectRes

DESCRIPTION
    Accept or reject to an incoming audio connection request from remote device.

RETURNS
	void
*/
void hfpHandleAudioConnectRes(HFP *hfp, const HFP_INTERNAL_AUDIO_CONNECT_RES_T *res)
{
    switch ( hfp->audio_connection_state )
    {
    case hfp_audio_disconnected:
    case hfp_audio_accepting:
        audioConnectResponse(hfp, res->response, res->packet_type, &res->audio_params, res->bd_addr);
        break;
    case hfp_audio_connecting_esco:
    case hfp_audio_connecting_sco:
		/* Already attempting to create an audio connection - indicate a fail for this attempt */
		sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_in_progress);
        break;
    case hfp_audio_disconnecting:   /* Until disconnect is complete, assume we have a connection */
    case hfp_audio_connected:
		/* Already have an audio connection - indicate a fail for this attempt */
		sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_have_audio);
		break;
	default:
        HFP_DEBUG(("hfpHandleAudioConnectRes invalid state %d\n",hfp->audio_connection_state));
        break;
    }
}

    
/****************************************************************************
NAME	
	hfpHandleAudioConnectResError

DESCRIPTION
    Attempt has been made to accept/reject an incoming audio connection request.  However,
    HFP library is not in the correct state to process the response.

RETURNS
	void
*/
void hfpHandleAudioConnectResError(HFP *hfp, const HFP_INTERNAL_AUDIO_CONNECT_RES_T *res)
{
    /* Send error message to inform the app we didn't respond to an incoming Synchronous connect request */
    sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_error);
}


/****************************************************************************
NAME	
	hfpHandleAudioDisconnectReq

DESCRIPTION
	Attempt to disconnect the audio (Synchronous) connection.

RETURNS
	void
*/
void hfpHandleAudioDisconnectReq(HFP *hfp)
{
    switch ( hfp->audio_connection_state )
    {
    case hfp_audio_disconnected:
    case hfp_audio_connecting_esco:  /* Until connect is complete, assume we don't have a connection */
    case hfp_audio_connecting_sco:
    case hfp_audio_accepting:
		/* Audio already with AG - indicate a fail for this attempt */
		sendAudioDisconnectIndToApp(hfp, hfp_audio_disconnect_no_audio);
        break;
    case hfp_audio_disconnecting:   
		/* Already attempting to close an audio connection - indicate a fail for this attempt */
		sendAudioDisconnectIndToApp(hfp, hfp_audio_disconnect_in_progress);
        break;
    case hfp_audio_connected:
		audioDisconnectRequest(hfp);
		break;
	default:
        HFP_DEBUG(("hfpHandleAudioDisconnectReq invalid state %d\n",hfp->audio_connection_state));
        break;
    }
}


/****************************************************************************
NAME	
	hfpHandleAudioDisconnectReqError

DESCRIPTION
    Attempt has been made to disconnect an audio connection request.  However,
    HFP library is not in the correct state to process the request.

RETURNS
	void
*/
void hfpHandleAudioDisconnectReqError(HFP *hfp)
{
    sendAudioDisconnectIndToApp(hfp, hfp_audio_disconnect_error);
}


/****************************************************************************
NAME	
	hfpHandleAudioTransferReq

DESCRIPTION
	Transfer the audio from the AG to the HF or vice versa depending on which
	device currently has it.

RETURNS
	void
*/
void hfpHandleAudioTransferReq(HFP *hfp, const HFP_INTERNAL_AUDIO_TRANSFER_REQ_T *req)
{
	switch (req->direction)
	{
	case hfp_audio_to_hfp:
        switch ( hfp->audio_connection_state )
        {
        case hfp_audio_disconnected:
    		startAudioConnectRequest(hfp, req->packet_type, &req->audio_params);
            break;
        case hfp_audio_connecting_esco:
        case hfp_audio_connecting_sco:
        case hfp_audio_accepting:
    		/* Already attempting to create an audio connection - indicate a fail for this attempt */
    		sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_in_progress);
            break;
        case hfp_audio_disconnecting:   /* Until disconnect is complete, assume we have a connection */
        case hfp_audio_connected:
    		/* Already have an audio connection - indicate a fail for this attempt */
    		sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_have_audio);
    		break;
    	default:
            HFP_DEBUG(("hfpHandleAudioTransferReq invalid state %d\n",hfp->audio_connection_state));
            break;
        }
		break;

	case hfp_audio_to_ag:
        switch ( hfp->audio_connection_state )
        {
        case hfp_audio_disconnected:
        case hfp_audio_connecting_esco:  /* Until connect is complete, assume we don't have a connection */
        case hfp_audio_connecting_sco:
        case hfp_audio_accepting:
        	/* Audio already with AG - indicate a fail for this attempt */
    		sendAudioDisconnectIndToApp(hfp, hfp_audio_disconnect_no_audio);
            break;
        case hfp_audio_disconnecting:   
    		/* Already attempting to close an audio connection - indicate a fail for this attempt */
    		sendAudioDisconnectIndToApp(hfp, hfp_audio_disconnect_in_progress);
            break;
        case hfp_audio_connected:
    		audioDisconnectRequest(hfp);
            break;
    	default:
            HFP_DEBUG(("hfpHandleAudioTransferReq invalid state %d\n",hfp->audio_connection_state));
            break;
        }
		break;

	case hfp_audio_transfer:
        switch ( hfp->audio_connection_state )
        {
        case hfp_audio_disconnected:
    		startAudioConnectRequest(hfp, req->packet_type, &req->audio_params);
            break;
        case hfp_audio_connecting_esco:
        case hfp_audio_connecting_sco:
        case hfp_audio_accepting:
    		/* Already attempting to create an audio connection - indicate a fail for this attempt */
    		sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_in_progress);
            break;
        case hfp_audio_disconnecting:   
    		/* Already attempting to close an audio connection - indicate a fail for this attempt */
    		sendAudioDisconnectIndToApp(hfp, hfp_audio_disconnect_in_progress);
            break;
        case hfp_audio_connected:
    		audioDisconnectRequest(hfp);
            break;
    	default:
            HFP_DEBUG(("hfpHandleAudioTransferReq invalid state %d\n",hfp->audio_connection_state));
            break;
        }
		break;

	default:
		HFP_DEBUG(("Unknown audio transfer direction\n"));
	}
}


/****************************************************************************
NAME	
	hfpHandleAudioTransferReqError

DESCRIPTION
	The app has requested that the audio be transferred (either to or from the AG).
	If the profile instance was in the wrong state for this operation to be 
	performed we need to send an immediate response to the app indicating an
	error has ocurred. We send the a audio status message with the status code 
	set to hfp_fail. We determine which message to send by looking at the action
	the app has requested and the current state of the HFP instance.

RETURNS
	void
*/
void hfpHandleAudioTransferReqError(HFP *hfp, const HFP_INTERNAL_AUDIO_TRANSFER_REQ_T *req)
{
	switch (req->direction)
	{
	case hfp_audio_to_hfp:
        /* Send error message to inform the app we didn't open a Synchronous connection */
        sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_error);
		break;

	case hfp_audio_to_ag:
        /* Send error message to inform the app we didn't close the Synchronous connection */
        sendAudioDisconnectIndToApp(hfp, hfp_audio_disconnect_error);
		break;

	case hfp_audio_transfer:
        /* Need to inform the app that the request failed. */
        if (hfp->audio_sink)
        {
			sendAudioDisconnectIndToApp(hfp, hfp_audio_disconnect_error);
        }
		else
        {
			sendAudioConnectCfmFailToApp(hfp, hfp_audio_connect_error);
        }
		break;

	default:
		HFP_DEBUG(("Unknown audio transfer direction\n"));
	}
}


/****************************************************************************
NAME
	HfpGetAudioSink

DESCRIPTION
	Returns the audio sink of the HFP instance.

RETURNS
	Sink if valid, or 0 otherwise.
*/
Sink HfpGetAudioSink(HFP *hfp)
{
    if (!hfp)
        return (Sink)0;

    return hfp->audio_sink;
}
