/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_response_hold_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_response_hold_handler.h"
#include "hfp_common.h"
#include "hfp_send_data.h"

#include <panic.h>
#include <string.h>


static void sendResponseHoldStatusCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_RESPONSE_HOLD_STATUS_CFM, hfp, status);
}

static void sendResponseHoldHeldCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_RESPONSE_HOLD_HELD_CFM, hfp, status);
}

static void sendResponseHoldAcceptCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_RESPONSE_HOLD_ACCEPT_CFM, hfp, status);
}

static void sendResponseHoldRejectCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_RESPONSE_HOLD_REJECT_CFM, hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleBtrhStatusReq

DESCRIPTION
	Request response hold status from the AG.

RETURNS
	void
*/
void hfpHandleBtrhStatusReq(HFP *hfp)
{
	static const char btrh[] = "AT+BTRH?\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(btrh), btrh);
}


/****************************************************************************
NAME	
	hfpHandleBtrhStatusReqError

DESCRIPTION
	Response hold status can't be obtained from the AG because the 
    profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleBtrhStatusReqError(HFP *hfp)
{
	/* Send a cfm indicating an error has occurred */
	sendResponseHoldStatusCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleBtrhStatusReqAtAck

DESCRIPTION
	Response hold status request command has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleBtrhStatusAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Tell the app about this */
	sendResponseHoldStatusCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleBtrhHoldReq

DESCRIPTION
	Request to AG to hold incoming call.

RETURNS
	void
*/
void hfpHandleBtrhHoldReq(HFP *hfp)
{
	static const char btrh[] = "AT+BTRH=0\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(btrh), btrh);
}


/****************************************************************************
NAME	
	hfpHandleBtrhHoldReqError

DESCRIPTION
	Request to AG to hold incoming call can't be made because the profile 
    instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleBtrhHoldReqError(HFP *hfp)
{
	/* Send a cfm indicating an error has occurred */
	sendResponseHoldHeldCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleBtrhHoldReqAtAck

DESCRIPTION
	Request to AG to hold incoming call has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleBtrhHoldAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Tell the app about this */
	sendResponseHoldHeldCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleBtrhAcceptReq

DESCRIPTION
	Request to AG to accept the currently held incoming call.

RETURNS
	void
*/
void hfpHandleBtrhAcceptReq(HFP *hfp)
{
	static const char btrh[] = "AT+BTRH=1\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(btrh), btrh);
}


/****************************************************************************
NAME	
	hfpHandleBtrhAcceptReqError

DESCRIPTION
	Request to AG to accept the currently held incoming call can't be made 
    because the profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleBtrhAcceptReqError(HFP *hfp)
{
	/* Send a cfm indicating an error has occurred */
	sendResponseHoldAcceptCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleBtrhAcceptReqAtAck

DESCRIPTION
	Request to AG to accept the currently held incoming call has been 
    acknowledged (completed) by the AG.

RETURNS
	void
*/
void hfpHandleBtrhAcceptAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Tell the app about this */
	sendResponseHoldAcceptCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleBtrhRejectReq

DESCRIPTION
	Request to AG to reject the currently held incoming call.

RETURNS
	void
*/
void hfpHandleBtrhRejectReq(HFP *hfp)
{
	static const char btrh[] = "AT+BTRH=2\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(btrh), btrh);
}


/****************************************************************************
NAME	
	hfpHandleBtrhRejectReqError

DESCRIPTION
	Request to AG to reject the currently held incoming call can't be made 
    because the profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleBtrhRejectReqError(HFP *hfp)
{
	/* Send a cfm indicating an error has occurred */
	sendResponseHoldRejectCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleBtrhRejectReqAtAck

DESCRIPTION
	Request to AG to reject the currently held incoming call has been 
    acknowledged (completed) by the AG.

RETURNS
	void
*/
void hfpHandleBtrhRejectAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Tell the app about this */
	sendResponseHoldRejectCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleResponseHold

DESCRIPTION
	Response hold indication received from the AG

AT INDICATION
	+BTRH

RETURNS
	void
*/
void hfpHandleResponseHold(Task profileTask, const struct hfpHandleResponseHold *ind)
{
	HFP *hfp = (HFP *) profileTask;

	/* Silently ignore AT command if we are not HFP v.15 */
	if (supportedProfileIsHfp15(hfp->hfpSupportedProfile))
	{
		checkHfpProfile15(hfp->hfpSupportedProfile);
		/* If message parameter not within specified range, silently ignore the message */
    	if ( ind->state <= hfp_held_call_rejected )
    	{
	    MAKE_HFP_MESSAGE(HFP_RESPONSE_HOLD_STATUS_IND);
	    message->hfp = hfp;
	    message->state = (hfp_response_hold_state)ind->state;
	    MessageSend(hfp->clientTask, HFP_RESPONSE_HOLD_STATUS_IND, message);
    	}
	}
}
