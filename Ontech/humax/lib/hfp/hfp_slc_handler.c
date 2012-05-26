/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_slc_handler.c        

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
#include "hfp_parse.h"
#include "hfp_rfc.h"
#include "hfp_ring_handler.h"
#include "hfp_sdp.h"
#include "hfp_send_data.h"
#include "hfp_slc_handler.h"
#include "init.h"

#include <bdaddr.h>
#include <panic.h>
#include <stdio.h>
#include <string.h>


#define stringLen(string) (sizeof((string))-1)


static void slcConnectSuccessTidy(HFP *hfp)
{
	/* 
		Update the local state to indicate we're connected. If there is
		an active call at the AG the state will have been set to 
		activeCall by now so only update the local state if we're still 
		in hfpSlcConnecting mode.
	*/
	if (hfp->state == hfpSlcConnecting) 
		hfpSetState(hfp, hfpSlcConnected);

	/* Issue a request to unregister the service record */
	ConnectionUnregisterServiceRecord(&hfp->task, hfp->sdp_record_handle);

	/* If this HFP instance supports caller id then enable it. */
	if (hfp->hfpSupportedFeatures & HFP_CLI_PRESENTATION)
		HfpCallerIdEnable(hfp, 1);

	/* If this HFP instance supports three way calling enable it */
	if ((hfp->hfpSupportedFeatures & HFP_THREE_WAY_CALLING) &&
		(hfp->agSupportedFeatures & AG_THREE_WAY_CALLING))
		HfpCallWaitingEnableNotification(hfp, 1);

    /* Request version of HFP that AG supports */
    hfpGetAgProfileVersion(hfp);
}


static void sendSinkCfmToApp(HFP *hfp, hfp_lib_status status, Sink sink)
{
	MAKE_HFP_MESSAGE(HFP_SINK_CFM);
	message->status = status;
	message->sink = sink;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_SINK_CFM, message);
}


/****************************************************************************
NAME	
	hfpSendSlcConnectCfmToApp

DESCRIPTION
	Send a HFP_SLC_CONNECT_CFM message to the app telling it the outcome
	of the connect attempt.

RETURNS
	void
*/
void hfpSendSlcConnectCfmToApp(hfp_connect_status status, HFP *hfp)
{
	MAKE_HFP_MESSAGE(HFP_SLC_CONNECT_CFM);
	message->status = status;
	message->sink = hfp->sink;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_SLC_CONNECT_CFM, message);

	/* If the connect succeeded need to tidy up a few things */
	if (status == hfp_connect_success)
		slcConnectSuccessTidy(hfp);
	else if (status != hfp_connect_failed_busy)
    {
       	/* Reset the connection related state */
	    hfpResetConnectionRelatedState(hfp);

		/* Reset the local state if we didn't fail because we're already connected. */
		hfpSetState(hfp, hfpReady);
        hfp->agSupportedProfile = hfp_no_profile;
    }
}

/****************************************************************************
NAME	
	hfpSendRemoteAGProfileVerIndToApp

DESCRIPTION
	Send a HFP_REMOTE_AG_PROFILE15_IND message to the app telling it the remote
	AG supports profile hfp version 1.5.

RETURNS
	void
*/
void hfpSendRemoteAGProfileVerIndToApp(hfp_profile profile, HFP *hfp)
{
	MAKE_HFP_MESSAGE(HFP_REMOTE_AG_PROFILE15_IND);

	message->hfp = hfp;
	message->agSupportedProfile = profile;
								  
	MessageSend(hfp->clientTask, HFP_REMOTE_AG_PROFILE15_IND, message);
}

/****************************************************************************
NAME	
	hfpSendSlcDisconnectIndToApp

DESCRIPTION
	Send a HFP_SLC_DISCONNECT_IND message to the app notifying it that
	the SLC has been disconnected.

RETURNS
	void
*/
void hfpSendSlcDisconnectIndToApp(HFP *hfp, hfp_disconnect_status status)
{
	MAKE_HFP_MESSAGE(HFP_SLC_DISCONNECT_IND);
	message->status = status;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_SLC_DISCONNECT_IND, message);

	/* Reset the connection related state */
	hfpResetConnectionRelatedState(hfp);

	/* Set the profile instance state to ready as we've just had a disconnect */
	hfpSetState(hfp, hfpReady);
    hfp->agSupportedProfile = hfp_no_profile;

	/* Cancel the AT response timeout message because we'll have no more AT cmds being sent */
	(void) MessageCancelAll(&hfp->task, HFP_INTERNAL_WAIT_AT_TIMEOUT_IND);

	/* Register the service record again */
	if (!hfp->sdp_record_handle)
		hfpRegisterServiceRecord(hfp);
}


/****************************************************************************
NAME	
	hfpHandleSlcConnectRequest

DESCRIPTION
	Initiate the creation of a profile service level connection.

RETURNS
	void
*/
void hfpHandleSlcConnectRequest(HFP *hfp, const HFP_INTERNAL_SLC_CONNECT_REQ_T *req)
{
    /* Update the local state to indicate we're in the middle of connecting. */
	hfpSetState(hfp, hfpSlcConnecting);
    hfp->agSupportedProfile = hfp_no_profile;

	/* Set the unhandled indicator flag */	
	hfp->indicator_status.extra_indicators = req->extra_indicators;

	if (req->extra_indicators)
		hfp->indicator_status.extra_inds_enabled = 1;

	/* Perform a service attrribute search to get the rfcomm channel of the remote service */
	hfpGetProfileServerChannel(hfp, &req->addr);
}


/****************************************************************************
NAME	
	hfpHandleSlcConnectResponse

DESCRIPTION
	Respond to a request to create an SLC from the remote device.

RETURNS
	void
*/
void hfpHandleSlcConnectResponse(HFP *hfp, const HFP_INTERNAL_SLC_CONNECT_RES_T *res)
{
	if (!res->response)
	{	/* Rejecting the SLC, reset the local state */
		hfpSetState(hfp, hfpReady);
	}
	else
	{	/* Accepting the SLC, reset AG profile type */
        hfp->agSupportedProfile = hfp_no_profile;
    }
		
	/* Set the unhandled indicator flag */	
	hfp->indicator_status.extra_indicators = res->extra_indicators;

	if (res->extra_indicators)
		hfp->indicator_status.extra_inds_enabled = 1;

	/* Send response to the connection lib */
	hfpHandleRfcommConnectResponse(hfp, res->response, &res->addr, hfp->local_rfc_server_channel, 0);
}


/****************************************************************************
NAME	
	hfpHandleSlcConnectIndReject

DESCRIPTION
	Reject the connect request outright because this profile instance is not 
	in the correct state.

RETURNS
	void
*/
void hfpHandleSlcConnectIndReject(HFP *hfp, const CL_RFCOMM_CONNECT_IND_T *ind)
{
	/* Issue a reject without passing this up to the app */
	hfpHandleRfcommConnectResponse(hfp, 0, &ind->bd_addr, ind->server_channel, 0);
}


/****************************************************************************
NAME	
	hfpHandleBrsfRequest

DESCRIPTION
	Send AT+BRSF to the AG.

RETURNS
	void
*/
void hfpHandleBrsfRequest(HFP *hfp)
{
	char brsf[15];

	/* Create the AT cmd we're sending */
	sprintf(brsf, "AT+BRSF=%d\r", hfp->hfpSupportedFeatures);

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(brsf), brsf);
}


/****************************************************************************
NAME	
	hfpSendInternalBrsfMessage

DESCRIPTION
	Send an internal message that we've got the AG's supported features.

RETURNS
	void
*/
void hfpHandleSupportedFeaturesNotification(HFP *hfp, uint16 features)
{
	/* Store the AG's supported features */
	hfp->agSupportedFeatures = features;

	/* Send a message to the app telling it the in-band ring setting of the AG */
	if (hfp->agSupportedFeatures & AG_IN_BAND_RING)
		hfpSendInBandRingIndToApp(hfp, 1);
	else
		hfpSendInBandRingIndToApp(hfp, 0);
}


/****************************************************************************
NAME	
	hfpHandleSupportedFeaturesInd

DESCRIPTION
	Handle the supported features sent by the AG.

AT INDICATION
	+BRSF

RETURNS
	void
*/
void hfpHandleSupportedFeaturesInd(Task profileTask, const struct hfpHandleSupportedFeaturesInd *ind)
{
	HFP* hfp = (HFP*)profileTask;
	
	/* Only allowed if we are an HFP device */
	checkHfpProfile(hfp->hfpSupportedProfile);
	/* Deal with supported Features */
	hfpHandleSupportedFeaturesNotification(hfp, ind->supportedfeat);
}


/****************************************************************************
NAME	
	hfpHandleBrsfAtAck

DESCRIPTION
	Called when we receive OK/ ERROR in response to the AT+BRSF cmd. This 
	indicates whether the AG recognised the cmd.

RETURNS
	void
*/
void hfpHandleBrsfAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Did the AG recognise the BRSF cmd */
	if (status == hfp_success)
	{
		/* Yes it did, we can proceed with the SLC */
		MessageSend(&hfp->task, HFP_INTERNAL_AT_CIND_TEST_REQ, 0);
	}
	else
	{
		/* No it didn't so we need to perform SDP search */
		hfpGetAgSupportedFeatures(hfp);

		/* Proceed with SLC establishment in parallel with the SDP search */
		MessageSend(&hfp->task, HFP_INTERNAL_AT_CIND_TEST_REQ, 0);
	}
}


/****************************************************************************
NAME	
	hfpHandleCindTestRequest

DESCRIPTION
	Send AT+CIND=? to the AG.

RETURNS
	void
*/
void hfpHandleCindTestRequest(HFP *hfp)
{
	static const char cind_test[] = "AT+CIND=?\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, stringLen(cind_test), cind_test);
}


/****************************************************************************
NAME	
	hfpHandleCindReadRequest

DESCRIPTION
	Send AT+CIND? to the AG.

RETURNS
	void
*/
void hfpHandleCindReadRequest(HFP *hfp)
{
	static const char cind_read[] = "AT+CIND?\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, stringLen(cind_read), cind_read);
}


/****************************************************************************
NAME	
	hfpHandleCmerRequest

DESCRIPTION
	Send AT+CMER to the AG.

RETURNS
	void
*/
void hfpHandleCmerRequest(HFP *hfp)
{
	char *cmer = 0;

	/* If three way calling is supported by both ends get call hold settings from AG */
    if ((hfp->hfpSupportedFeatures & HFP_THREE_WAY_CALLING) &&
            (hfp->agSupportedFeatures & AG_THREE_WAY_CALLING))
		cmer = "AT+CMER=3, 0, 0, 1\rAT+CHLD=?\r";
    else
		cmer = "AT+CMER=3, 0, 0, 1\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(cmer), cmer);
}


/****************************************************************************
NAME	
	hfpHandleCmerAtAck

DESCRIPTION
	Called when we receive OK/ ERROR in response to the AT+CMER cmd. If we're
	not getting call hold params from the AG then the SLC is complete.

RETURNS
	void
*/
void hfpHandleCmerAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Check the cmd succeeded */
	if (status == hfp_success)
	{
		/* 
			If the local device does not support three way calling then we 
			would not have requested the AG's call hold settings so the SLC 
			is complete. Tell the app.
		*/
		if (!(hfp->hfpSupportedFeatures & HFP_THREE_WAY_CALLING) ||
			!(hfp->agSupportedFeatures & AG_THREE_WAY_CALLING))
			hfpSendSlcConnectCfmToApp(hfp_connect_success, hfp);
	}
	else
	{
		/* Failed, tell the app */
		hfpSendSlcConnectCfmToApp(hfp_connect_slc_failed, hfp);
	}
}


/****************************************************************************
NAME	
	hfpHandleDisconnectRequestFail

DESCRIPTION
	Received a disconnect request from the app when we haven't got a 
	connecting and are not attempting to connect. Return error message.

RETURNS
	void
*/
void hfpHandleDisconnectRequestFail(HFP *hfp)
{
	/* Send a disconnect cfm to the app with an error status */
	hfpSendSlcDisconnectIndToApp(hfp, hfp_disconnect_no_slc);
}


/****************************************************************************
NAME	
	hfpHandleDisconnectRequest

DESCRIPTION
	We're in the right state and have received a disconnect request, 
	handle it here.

RETURNS
	void
*/
void hfpHandleDisconnectRequest(HFP *hfp)
{	
	if (hfp->audio_sink)
	{
		/* If we have a SCO/eSCO active need to tear that down first */
		hfpHandleAudioDisconnectReq(hfp);

		/* Queue up the SLC disconnect message */
		MessageSendConditionally(&hfp->task, HFP_INTERNAL_SLC_DISCONNECT_REQ, 0, (uint16 *) &hfp->audio_sink);	/*lint !e740 */
	}
	else
	{
		/* Issue an SLC disconnect */
		hfpHandleRfcommDisconnectRequest(hfp);
	}
}


/****************************************************************************
NAME	
	hfpHandleSlcDisconnectIndication

DESCRIPTION
	Handle disconnection of a profile service level connection.

RETURNS
	void
*/
void hfpHandleSlcDisconnectIndication(HFP *hfp, const HFP_INTERNAL_SLC_DISCONNECT_IND_T *ind)
{
	if (hfp->audio_sink)
	{
		/* Reconstruct the SLC disconnect indication for when the HFP has tidied up*/
    	MAKE_HFP_MESSAGE(HFP_INTERNAL_SLC_DISCONNECT_IND);
	    message->status = ind->status;
	    
		/* We have a SCO/eSCO active - tidy up and inform app of that disappearing first */
		hfpManageSyncDisconnect(hfp);
		
		MessageSendConditionally(&hfp->task, HFP_INTERNAL_SLC_DISCONNECT_IND, message, (uint16 *) &hfp->audio_sink);
	}
	else
	{
    	/* Throw away any data arriving on the source */ 
		StreamConnectDispose(StreamSourceFromSink(hfp->sink));
		/* Inform app of SLC disconnection */
    	hfpSendSlcDisconnectIndToApp(hfp, ind->status);
	}
}


/****************************************************************************
NAME	
	hfpHandleSinkRequest

DESCRIPTION
	If we have a valid sink return it otherwise send back an error

RETURNS
	void
*/
void hfpHandleSinkRequest(HFP *hfp)
{
	/* Check if we have a valid sink */
	if (hfp->sink)
		sendSinkCfmToApp(hfp, hfp_success, hfp->sink);
	else
		sendSinkCfmToApp(hfp, hfp_fail, 0);
}


/****************************************************************************
NAME	
	hfpHandleSinkRequestFail

DESCRIPTION
	Return an error, the profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleSinkRequestFail(HFP *hfp)
{
	sendSinkCfmToApp(hfp, hfp_fail, 0);
}


/****************************************************************************
NAME
	HfpGetSlcSink

DESCRIPTION
	Returns SLC sink of the HFP instance.

RETURNS
	Sink if valid, or 0 otherwise.
*/
Sink HfpGetSlcSink(HFP *hfp)
{
    if (!hfp)
        return (Sink)0;

    return hfp->sink;
}

