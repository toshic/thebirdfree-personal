/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_avrcp_msg_handler.c
@brief    Implementation of AVRCP library message handlers.
*/


#include "headset_avrcp_event_handler.h"
#include "headset_avrcp_msg_handler.h"
#include "headset_debug.h"
#include "headset_init.h"
#include "headset_link_policy.h"
#include "headset_private.h"
#include "headset_statemanager.h"
#include "headset_volume.h"
#include "at_cmd.h"

#include <avrcp.h>
#include <panic.h>

#ifdef DEBUG_AVRCP_MSG
#define AVRCP_MSG_DEBUG(x) DEBUG(x)
#else
#define AVRCP_MSG_DEBUG(x) 
#endif


/****************************************************************************
  LOCAL MESSAGE HANDLING FUNCTIONS
*/

static void handleAVRCPInitCfm(AVRCP_INIT_CFM_T *msg)
{
	AVRCP_MSG_DEBUG(("AVRCP_INIT_CFM : "));
	if (msg->status == avrcp_success)
	{
		AVRCP_MSG_DEBUG(("Success\n"));
	    theHeadset.avrcp = msg->avrcp;
	    theHeadset.avrcp_data.pending = FALSE;
	    stateManagerSetAvrcpState(avrcpReady);
		
		/* All profile libraries are now initialised */
		theHeadset.ProfileLibrariesInitialising = FALSE;
    }
    else
    {
	    AVRCP_MSG_DEBUG(("Failure [status 0x%x]\n",msg->status));
    }
}

/****************************************************************************/
static void handleAVRCPConnectInd(AVRCP_CONNECT_IND_T *msg)
{
	bool accept = FALSE;
	AVRCP_MSG_DEBUG(("AVRCP_CONNECT_IND : "));
	
    MessageCancelAll(&theHeadset.task, APP_AVRCP_CONNECT_REQ);

    SendEvent(EVT_AVRCP_SIGNAL_CONNECT_IND,0);
    
	if ((stateManagerGetAvrcpState() == avrcpReady) && (stateManagerGetHfpState() != headsetPoweringOn))
	{ /* Accept connection */
		AVRCP_MSG_DEBUG(("Accepting connection\n"));
		accept = TRUE;
		stateManagerSetAvrcpState(avrcpConnecting);
	}
	else
	{ /* Reject connection due to incorrect state */
		AVRCP_MSG_DEBUG(("Rejecting connection [state = %d]\n", stateManagerGetAvrcpState()));
	}
	AvrcpConnectResponse(msg->avrcp, msg->connection_id, accept);
}

/****************************************************************************/
static void handleAVRCPConnectCfm(AVRCP_CONNECT_CFM_T *msg)
{
	AVRCP_MSG_DEBUG(("AVRCP_CONNECT_CFM : \n"));

    SendEvent(EVT_AVRCP_SIGNAL_CONNECT_CFM,msg->status);

	if (stateManagerGetAvrcpState() == avrcpConnecting)
	{
	    if(msg->status == avrcp_success)
	    {	
			stateManagerSetAvrcpState(avrcpConnected);
			
			/* Ensure the underlying ACL is encrypted */       
		    ConnectionSmEncrypt( &theHeadset.task , msg->sink , TRUE );
			
			/* If a request was made to send an avrcp_play when AVRCP was connected
			   then do it now. 
			*/
			theHeadset.avrcp_data.send_play = 0;
				
			/* Assume music won't be playing */
			theHeadset.PlayingState = 0;				
	    }
	    else
	    {
			MessageCancelAll( &theHeadset.task , APP_SEND_PLAY );
			stateManagerSetAvrcpState(avrcpReady);
			theHeadset.PlayingState = 1;
			if ( stateManagerGetA2dpState () == headsetA2dpPaused )
				stateManagerEnterA2dpStreamingState();
				
	    }
	    
		PROFILE_MEMORY(("AVRCPConnect"))
	}
	else
	{
		AVRCP_MSG_DEBUG(("Ignoring as in wrong state [state = %d]\n", stateManagerGetAvrcpState()));
	}
}

/****************************************************************************/
static void handleAVRCPDisconnectInd(AVRCP_DISCONNECT_IND_T *msg)
{
	PROFILE_MEMORY(("AVRCPDisco"))
	AVRCP_MSG_DEBUG(("AVRCP_DISCONNECT_IND : "));

    SendEvent(EVT_AVRCP_SIGNAL_DISCONNECT_IND,msg->status);

	if ( stateManagerIsAvrcpConnected() )
	{
		AVRCP_MSG_DEBUG(("Disconnect Result = %d\n", msg->status));
		stateManagerSetAvrcpState(avrcpReady);
		
	    /* Reset pending state as we won't get a CFM back from any sent AVRCP commands,
	       now that that connection is closed.
	    */
	    MessageCancelAll(&theHeadset.task, APP_AVRCP_CONTROLS);
	    theHeadset.avrcp_data.pending = FALSE;
	}
	else
	{
		AVRCP_MSG_DEBUG(("Ignoring as in wrong state [state = %d]\n", stateManagerGetAvrcpState()));
	}
}

/****************************************************************************/
static void handleAVRCPPassthroughCfm(void)
{
    /* 
        Clearing the pending flag should allow another
        pending event to be delivered to controls_handler 
    */
    theHeadset.avrcp_data.pending = FALSE;
}

/****************************************************************************/
static void handleAVRCPPassthroughInd(AVRCP_PASSTHROUGH_IND_T *msg)
{
    /* Acknowledge the request */
	
	if ((msg->opid == opid_volume_up) && theHeadset.features.UseAVRCPforVolume)
	{
		/* The headset should accept volume up commands as it supports AVRCP TG category 2. */
		AvrcpPassthroughResponse(msg->avrcp, avctp_response_accepted);
	
		/* Adjust the local volume only if it is a press command and the A2DP is active. */
		if (!msg->state && stateManagerIsA2dpStreaming())
			VolumeUp();
	}
	else if ((msg->opid == opid_volume_down) && theHeadset.features.UseAVRCPforVolume)
	{
		/* The headset should accept volume down commands as it supports AVRCP TG category 2. */
		AvrcpPassthroughResponse(msg->avrcp, avctp_response_accepted);	
		
		/* Adjust the local volume only if it is a press command and the A2DP is active. */
		if (!msg->state && stateManagerIsA2dpStreaming())
			VolumeDown();
	}
	else
		/* The headset won't accept any other commands. */
    	AvrcpPassthroughResponse(msg->avrcp, avctp_response_not_implemented);
}

/****************************************************************************/
static void handleAVRCPUnitInfoInd(AVRCP_UNITINFO_IND_T *msg)
{
	if (theHeadset.features.UseAVRCPforVolume)
	{
		/* Headset is a CT and TG, so send the correct response to UnitInfo requests. */
		uint32 company_id = 0xffffff; /* IEEE RAC company ID can be used here */
	    AvrcpUnitInfoResponse(msg->avrcp, TRUE, subunit_panel, 0, company_id);
	}
	else
	{
		/* Headset is only a CT not a TG, so reject UnitInfo requests. */
	    AvrcpUnitInfoResponse(msg->avrcp, FALSE, subunit_monitor, 0, (uint32) 0);
	}
}

/****************************************************************************/
static void handleAVRCPSubUnitInfoInd(AVRCP_SUBUNITINFO_IND_T *msg)
{
    if (theHeadset.features.UseAVRCPforVolume)
	{
		/* Headset is a CT and TG, so send the correct response to SubUnitInfo requests. */
		uint8 page_data[4];
		page_data[0] = 0x48; /* subunit_type: panel; max_subunit_ID: 0 */
		page_data[1] = 0xff;
		page_data[2] = 0xff;
		page_data[3] = 0xff;
	    AvrcpSubUnitInfoResponse(msg->avrcp, TRUE, page_data);
	}
	else
	{
		/* Headset is only a CT not a TG, so reject SubUnitInfo requests. */
	    AvrcpSubUnitInfoResponse(msg->avrcp, FALSE, 0);
	}
}

/****************************************************************************/
static void handleAVRCPVendorDependentInd(AVRCP_VENDORDEPENDENT_IND_T *msg)
{
    /*
        Reject all vendor requests.
    */
	AvrcpVendorDependentResponse(msg->avrcp, avctp_response_not_implemented);
}


/****************************************************************************
  INTERFACE FUNCTIONS
*/

/****************************************************************************/
void handleAVRCPMessage( Task task, MessageId id, Message message )
{
    AVRCP_MSG_DEBUG(("AVRCP msg: 0x%x\n",id));
    
    switch (id)
    {
    case AVRCP_INIT_CFM:
    	handleAVRCPInitCfm((AVRCP_INIT_CFM_T*)message);
        break;
    case AVRCP_CONNECT_IND:
    	handleAVRCPConnectInd((AVRCP_CONNECT_IND_T*)message);
        break;
    case AVRCP_CONNECT_CFM:
    	handleAVRCPConnectCfm((AVRCP_CONNECT_CFM_T*)message);
        break;
    case AVRCP_DISCONNECT_IND:
    	handleAVRCPDisconnectInd((AVRCP_DISCONNECT_IND_T*)message);
        break;

    case AVRCP_PASSTHROUGH_CFM:
    	handleAVRCPPassthroughCfm();
        break;

    case AVRCP_PASSTHROUGH_IND:
    	handleAVRCPPassthroughInd((AVRCP_PASSTHROUGH_IND_T*)message);
        break;
    case AVRCP_UNITINFO_IND:
    	handleAVRCPUnitInfoInd((AVRCP_UNITINFO_IND_T*)message);
        break;
    case AVRCP_SUBUNITINFO_IND:
    	handleAVRCPSubUnitInfoInd((AVRCP_SUBUNITINFO_IND_T*)message);
        break;
    case AVRCP_VENDORDEPENDENT_IND:
    	handleAVRCPVendorDependentInd((AVRCP_VENDORDEPENDENT_IND_T*)message);
        break;

    default:
    	AVRCP_MSG_DEBUG(("UNKNOWN MESSAGE\n"));
        break;
    }    
}



