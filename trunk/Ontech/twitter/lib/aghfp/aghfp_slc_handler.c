/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/


#include "aghfp.h"
#include "aghfp_audio_handler.h"
#include "aghfp_common.h"
#include "aghfp_indicators.h"
#include "aghfp_init.h"
#include "aghfp_ok.h"
#include "aghfp_parse.h"
#include "aghfp_private.h"
#include "aghfp_rfc.h"
#include "aghfp_sdp.h"
#include "aghfp_send_data.h"
#include "aghfp_slc_handler.h"

#include <bdaddr.h>
#include <panic.h>
#include <stdio.h>
#include <string.h>



#define stringLen(string) (sizeof((string))-1)


/****************************************************************************
	Send a AGHFP_SLC_CONNECT_CFM message to the app telling it the outcome
	of the connect attempt.
*/
void aghfpSendSlcConnectCfmToApp(aghfp_connect_status status, AGHFP *aghfp)
{
	MAKE_AGHFP_MESSAGE(AGHFP_SLC_CONNECT_CFM);
	message->status = status;
	message->aghfp = aghfp;
    message->rfcomm_sink = aghfp->rfcomm_sink;
	MessageSend(aghfp->client_task, AGHFP_SLC_CONNECT_CFM, message);

	/* If the connect succeeded need to tidy up a few things */
	if (status == aghfp_connect_success)
    {
		aghfpSetState(aghfp, aghfp_slc_connected);

		aghfp->sms_index = 0;

		/* Issue a request to unregister the service record */
		ConnectionUnregisterServiceRecord(&aghfp->task, aghfp->sdp_record_handle);
		
    }
	else if (status != aghfp_connect_failed_busy)
    {
		/* Reset the local state if we didn't fail because we're already connected. */
		aghfpSetState(aghfp, aghfp_ready);
    }
}

void aghfpSendSlcPreConnectCfmToApp(aghfp_connect_status status, AGHFP *aghfp)
{
	MAKE_AGHFP_MESSAGE(AGHFP_SLC_PRE_CONNECT_CFM);
	message->status = status;
	message->aghfp = aghfp;
    message->rfcomm_sink = aghfp->rfcomm_sink;
	MessageSendLater(aghfp->client_task, AGHFP_SLC_PRE_CONNECT_CFM, message, D_SEC(7));
}


/****************************************************************************
	Initiate the creation of a profile service level connection.
*/
void aghfpHandleSlcConnectRequest(AGHFP *aghfp, const AGHFP_INTERNAL_SLC_CONNECT_REQ_T *req)
{
    /* Update the local state to indicate we're in the middle of connecting. */
	aghfpSetState(aghfp, aghfp_slc_connecting);

	/* Perform a service attribute search to get the rfcomm channel of the remote service */
	aghfpGetProfileServerChannel(aghfp, &req->addr);
}


/****************************************************************************
	Respond to a request to create an SLC from the remote device.
*/
void aghfpHandleSlcConnectResponse(AGHFP *aghfp, const AGHFP_INTERNAL_SLC_CONNECT_RES_T *res)
{
	/* If rejecting the SLC reset the local state */
	if (!res->response)
		aghfpSetState(aghfp, aghfp_ready);

	/* Send response to the connection lib */
	aghfpHandleRfcommConnectResponse(aghfp, res->response, &res->addr, aghfp->local_rfc_server_channel, 0);
}


/****************************************************************************
	Reject the connect request outright because this profile instance is not
	in the correct state.
*/
void aghfpHandleSlcConnectIndReject(AGHFP *aghfp, const CL_RFCOMM_CONNECT_IND_T *ind)
{
	/* Issue a reject without passing this up to the app */
	aghfpHandleRfcommConnectResponse(aghfp, 0, &ind->bd_addr, ind->server_channel, 0);
}


/****************************************************************************
	Record the supported features that the HF has just reported and send our
    supported features in response.
*/
void aghfpHandleBrsfRequest(AGHFP *aghfp, uint16 supported_features)
{
	char buf[6];

	/* Record the HF's supported features */
	aghfp->hf_supported_features = supported_features;

	/* Send BRSF response */
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+BRSF:");
    sprintf(buf, "%d", aghfp->supported_features);
	aghfpAtCmdString(aghfp, buf);
    aghfpAtCmdEnd(aghfp);

    aghfpSendOk(aghfp);
}


/****************************************************************************
	Respond to AT+CIND=? command.
*/
void aghfpHandleCindSupportedRequest(AGHFP *aghfp)
{
	aghfpHandleSendCindDetails(aghfp);
    aghfpSendOk(aghfp);
}


/****************************************************************************
	Respond to AT+CIND? command.
*/
void aghfpHandleCindStatusRequest(AGHFP *aghfp)
{
	aghfpHandleSendCindStatus(aghfp);
    aghfpSendOk(aghfp);
}


/****************************************************************************
	Respond to AT+CMER command.
*/
void aghfpHandleCmerRequest(AGHFP *aghfp, uint16 mode, uint16 ind)
{
	/* B-11851: pay attention to whether the HF has just enabled or disabled
       indicator event reporting. */
	aghfpSendOk(aghfp);

	/* Now consider whether this AT+CMER marks the end of SLC establishment */
	if (aghfp->state == aghfp_slc_connecting)
	{
		if ((aghfp->supported_features & aghfp_three_way_calling) &&
			(aghfp->hf_supported_features & aghfp_hf_3_way_calling))
		{
			/* Both the AG and the HF support three way calling, so do nothing,
			   Just wait until the HF sends us an AT+CHLD */
			return;
		}
		else
		{
			/* Either the AG or the HF don't support 3 way calling. Therefore
			   SLC establishment is complete. */
			aghfpSendSlcConnectCfmToApp(aghfp_connect_success, aghfp);
		}
	}
}


/****************************************************************************
	Respond to AT+CHLD=? command.
*/
void aghfpHandleCallHoldSupportRequest(AGHFP *aghfp)
{
	aghfpSendAtCmd(aghfp, "+CHLD:(0,1,1x,2,2x,3)");
	aghfpSendOk(aghfp);

	aghfpSendSlcConnectCfmToApp(aghfp_connect_success, aghfp);
}


/****************************************************************************
	Received a disconnect request from the app when we haven't got a
	connecting and are not attempting to connect. Return error message.
*/
void aghfpHandleDisconnectRequestFail(AGHFP *aghfp)
{
	/* Send a disconnect cfm to the app with an error status */
	aghfpSendSlcDisconnectIndToApp(aghfp, aghfp_disconnect_no_slc);
}


/****************************************************************************
	Called in response to AghfpSlcDisconnect if we currently have an SLC
    connection.
*/
void aghfpHandleSlcDisconnect(AGHFP *aghfp)
{
	if (aghfp->audio_sink)
    {
		/* We have a SCO active, so we need to tear that down first */
    	AghfpAudioDisconnect(aghfp);

		/* Queue up another SLC disconnect message */
        MessageSendConditionally(&aghfp->task, AGHFP_INTERNAL_SLC_DISCONNECT_REQ,
        						 0, (uint16 *) &aghfp->audio_sink);
    }
    else
    {
		aghfpHandleRfcommDisconnectRequest(aghfp);
    }
}


/****************************************************************************
	Send an AGHFP_SLC_DISCONNECT_IND message to the app notifying it that
	the SLC has been disconnected.
*/
void aghfpSendSlcDisconnectIndToApp(AGHFP *aghfp, aghfp_disconnect_status status)
{
	MAKE_AGHFP_MESSAGE(AGHFP_SLC_DISCONNECT_IND);
	message->status = status;
	message->aghfp = aghfp;
	MessageSend(aghfp->client_task, AGHFP_SLC_DISCONNECT_IND, message);

	/* Reset the connection related state */
	aghfpResetConnectionRelatedState(aghfp);

	/* Set the profile instance state to ready as we've just had a disconnect */
	aghfpSetState(aghfp, aghfp_ready);

	/* Register the service record again */
	if (!aghfp->sdp_record_handle)
    {
		aghfpRegisterServiceRecord(aghfp, aghfp->supported_profile, aghfp->local_rfc_server_channel);
    }
}
