/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_call_waiting.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"

#include <panic.h>


/****************************************************************************
NAME	
	HfpCallWaitingEnableNotification

DESCRIPTION
	Enable /disable call waiting notification from the AG depending on the 
	value of the enable flag passed in from the aplpication. Once call waiting 
	notifications have been enabled at the AG they will be sent (whenever 
	there is a waiting call) until explicitly disabled or until the SLC is 
	disconnecetd. The AT command to enable / disable call waiting 
	notifications will only be sent if both the AG and headset claim support 
	for this functionality in their supported features. The request is issued 
	on the SLC associated with the hfp profile instance passed in by the 
	application. The message returned indicates whether the command was 
	recognised by the AG or not. The HFP_CALL_WAITING_IND message will be 
	used to notify the application of waiting calls.

MESSAGE RETURNED
	HFP_CALL_WAITING_ENABLE_CFM

RETURNS
	void
*/
void HfpCallWaitingEnableNotification(HFP *hfp, bool enable)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

    {
		/* Send an internal message so we can go through the state machine */
		MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_CCWA_REQ);
		message->enable = enable;
		MessageSend(&hfp->task, HFP_INTERNAL_AT_CCWA_REQ, message);
	}
}
