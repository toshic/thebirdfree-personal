/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    avrcp_l2cap_handler.c
    
DESCRIPTION

*/

#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_common.h"
#include "avrcp_l2cap_handler.h"
#include "avrcp_sdp_handler.h"
#include "avrcp_signal_handler.h"
#include "init.h"

#include <panic.h>


/* Reset the local state values to their initial states */
static void resetAvrcpValues(AVRCP* avrcp)
{
	/* Set the local state to ready */
	avrcpSetState(avrcp, avrcpReady);

	/* Reset the remaining local state */
	avrcp->pending = avrcp_none;
    avrcp->block_received_data = 0;
	avrcp->watchdog = 0;
	avrcp->sink = 0;
	avrcp->transaction_label = 0;
	avrcp->fragmented = avrcp_frag_none;

	/* Free any memory that may be allocated */
	if (avrcp->identifier)
	{
		free(avrcp->identifier);
		avrcp->identifier = 0;
	}
}

static avrcp_status_code convertL2capDisconnectStatus(l2cap_disconnect_status l2cap_status)
{
    switch(l2cap_status)
    {
        case l2cap_disconnect_successful: return avrcp_success;
        case l2cap_disconnect_timed_out: return avrcp_timeout;		
        case l2cap_disconnect_error: return avrcp_fail;
        case l2cap_disconnect_no_connection: return avrcp_device_not_connected;
        case l2cap_disconnect_link_loss: return avrcp_link_loss;
        default: return avrcp_fail;
    }
}

/****************************************************************************
NAME	
	avrcpHandleL2capRegisterCfm

DESCRIPTION
	This function is called on receipt of an CL_L2CAP_REGISTER_CFM.
*/
void avrcpHandleL2capRegisterCfm(AVRCP *avrcp, const CL_L2CAP_REGISTER_CFM_T *cfm)
{
	if (cfm->status == success)
		/* Register the AVRCP service record */
		avrcpRegisterServiceRecord(avrcp);
	else
        avrcpSendInitCfmToClient(avrcp->clientTask, avrcp, avrcp_fail);
}


/****************************************************************************
NAME	
	avrcpHandleL2capConnectCfm

DESCRIPTION
	This function is called on receipt of a CL_L2CAP_CONNECT_CFM message
	indicating the outcome of the connect attempt.
*/
void avrcpHandleL2capConnectCfm(AVRCP *avrcp, const CL_L2CAP_CONNECT_CFM_T *cfm)
{	
	avrcp_status_code status = avrcp_fail;

	if (cfm->status == l2cap_connect_success)
	{
		status = avrcp_success;
		
		/* Break any Source connections. A bug has been seen where the firmware sees the Source 
		   already to be in use, even on first connect. This resulted in no data being received. 
			See B-47773 for details. 
		*/
		StreamDisconnect(StreamSourceFromSink(cfm->sink),0);

		/* Update the profile state to indicate we're connected */
		avrcpSetState(avrcp, avrcpConnected);

		/* Store the connection sink */
		avrcp->sink = cfm->sink;

		/* Store the mtu negotiated */
		avrcp->l2cap_mtu = cfm->mtu_remote;

		/* Check for data in the buffer */
		avrcpHandleReceivedData(avrcp);
	}
	else
	{
        if (cfm->status == l2cap_connect_timeout)
            status = avrcp_timeout;
		else if(cfm->status == l2cap_connect_failed_key_missing)
			status = avrcp_key_missing;
        else
		    status = avrcp_fail;

		/* If the connect attempt failed, reset the local state. */
		resetAvrcpValues(avrcp);
	}	

	/* Send the cfm message to the client */
	avrcpSendCommonCfmMessageToApp(AVRCP_CONNECT_CFM, status, cfm->sink, avrcp);
}


/****************************************************************************
NAME	
	avrcpHandleL2capConnectInd

DESCRIPTION
	This function is called on receipt of a CL_L2CAP_CONNECT_IND message.
	This message indicates that a remote device is attempting to establish
	an L2CAP connection to this device on the AVCTP PSM.
*/
void avrcpHandleL2capConnectInd(AVRCP *avrcp, const CL_L2CAP_CONNECT_IND_T *ind)
{		
	MAKE_AVRCP_MESSAGE(AVRCP_CONNECT_IND);
	message->bd_addr = ind->bd_addr;
	message->connection_id = ind->connection_id;
	message->avrcp = avrcp;
	MessageSend(avrcp->clientTask, AVRCP_CONNECT_IND, message);

	/* Update the local state */
	avrcpSetState(avrcp, avrcpConnecting);
}


/****************************************************************************
NAME	
	avrcpHandleL2capConnectIndReject

DESCRIPTION
	The profile instance is in the wrong state, automatically reject the 
	connect request.
*/
void avrcpHandleL2capConnectIndReject(AVRCP *avrcp, const CL_L2CAP_CONNECT_IND_T *ind)
{
	/* Send a connect response rejecting the connection. */
	ConnectionL2capConnectResponse(&avrcp->task, 0, ind->psm, ind->connection_id, 0);
}


/****************************************************************************
NAME	
	avrcpHandleL2capDisconnectInd

DESCRIPTION
	This function is called on receipt of a CL_L2CAP_DISCONNECT_IND message.
	This message indicates that an L2CAP connection has been disconnected.
*/
void avrcpHandleL2capDisconnectInd(AVRCP *avrcp, const CL_L2CAP_DISCONNECT_IND_T *ind)
{		
	/* Send message to the application */
	avrcpSendCommonCfmMessageToApp(AVRCP_DISCONNECT_IND, convertL2capDisconnectStatus(ind->status), ind->sink, avrcp);

    if (avrcp->lazy)
        MessageSend(&avrcp->task, AVRCP_INTERNAL_TASK_DELETE_REQ, 0);
    else
	    /* Reset the local state */
	    resetAvrcpValues(avrcp);
}
