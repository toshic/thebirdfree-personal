/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_call_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_call_handler.h"
#include "hfp_common.h"
#include "hfp_indicators_handler.h"
#include "hfp_send_data.h"

#include <panic.h>
#include <string.h>


/* The CHUP cmd is used to reject and terminate a call */
static void sendChup(HFP *hfp)
{
	static const char chup[] = "AT+CHUP\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(chup), chup);
}


/****************************************************************************
NAME	
	hfpHandleAnswerCall

DESCRIPTION
	Answer an incoming call.

RETURNS
	void
*/
void hfpHandleAnswerCall(HFP *hfp)
{
	static const char ata[] = "ATA\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(ata), ata);
}


/****************************************************************************
NAME	
	hfpHandleAnswerCallError

DESCRIPTION
	Received an answer call request but the profile lib is in the wrong 
	state. Send back an error message.

RETURNS
	void
*/
void hfpHandleAnswerCallError(HFP *hfp)
{
	/* Send an cfm message with status set to fail */
	hfpSendCommonCfmMessageToApp(HFP_ANSWER_CALL_CFM, hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleAtaAtAck

DESCRIPTION
	Tell the app whether the AG accepted or rejected the ATA cmd.

RETURNS
	void
*/
void hfpHandleAtaAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_ANSWER_CALL_CFM, hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleRejectCall

DESCRIPTION
	Reject an incoming call.

RETURNS
	void
*/
void hfpHandleRejectCall(HFP *hfp)
{
	/* Send the hang up AT cmd to the AG */
	sendChup(hfp);
}


/****************************************************************************
NAME	
	hfpHandleRejectCallError

DESCRIPTION
	Received a request to reject a call when the profile instance is in the 
	wrong state, so send back an error message.

RETURNS
	void
*/
void hfpHandleRejectCallError(HFP *hfp)
{
	/* Send an error message */
	hfpSendCommonCfmMessageToApp(HFP_REJECT_CALL_CFM, hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleChupAtAck

DESCRIPTION
	Tell the app whether the AG accepted or rejected the AT+CHUP cmd.

RETURNS
	void
*/
void hfpHandleChupAtAck(HFP *hfp, hfp_lib_status status)
{
	/* 
		This function needs to handle both reject call and terminate call 
		because they both result in the AT+CHUP command being issued.
	*/
	if (hfp->state == hfpIncomingCallEstablish)
	{
		/* Send a cfm message to the application. */
		hfpSendCommonCfmMessageToApp(HFP_REJECT_CALL_CFM, hfp, status);
	}
	else if (hfp->state == hfpOutgoingCallEstablish ||
			hfp->state == hfpOutgoingCallAlerting ||
			hfp->state == hfpActiveCall)
	{
		/* Send a cfm message to the application. */
		hfpSendCommonCfmMessageToApp(HFP_TERMINATE_CALL_CFM, hfp, status);
	}

	/* This is necessary when dealing with 0.96 AGs */
	if ((status == hfp_success) && !hfp->indicator_status.indexes.call_setup &&
        supportedProfileIsHfp(hfp->hfpSupportedProfile))
    {
		hfpSendIndicatorCallSetupToApp(hfp, hfp_no_call_setup);
    }
}


/****************************************************************************
NAME	
	hfpHandleTerminateCall

DESCRIPTION
	Terminate an ongoing call process.

RETURNS
	void
*/
void hfpHandleTerminateCall(HFP *hfp)
{
	/* Send the hang up AT cmd to the AG */
	sendChup(hfp);
}


/****************************************************************************
NAME	
	hfpHandleTerminateCallError

DESCRIPTION
	Send an error message because we received the terminate call request
	when the profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleTerminateCallError(HFP *hfp)
{
	/* Send an error message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_TERMINATE_CALL_CFM, hfp, hfp_fail);
}
