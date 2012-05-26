/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_call.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"


/****************************************************************************
NAME	
	HfpAnswerCall

DESCRIPTION
	This function is used to answer an incoming call. The AT command to 
	answer the call will be sent out on the SLC of the hfp profile instance
	passed into the function from the application. The message returned 
	indicates whether the command was recognised by the AG or not. 

MESSAGE RETURNED
	HFP_ANSWER_CALL_CFM

RETURNS
	void
*/
void HfpAnswerCall(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

    /* Send an internal message to answer an incoming call. */
	MessageSend(&hfp->task, HFP_INTERNAL_ATA_REQ, 0);
}


/****************************************************************************
NAME	
	HfpRejectCall

DESCRIPTION
	This function is used to reject an incoming call. The AT command to 
	reject the call will be sent out on the SLC of the hfp profile instance
	passed into the function from the application. The message returned 
	indicates whether the command was recognised by the AG or not. 

MESSAGE RETURNED
	HFP_REJECT_CALL_CFM

RETURNS
	void
*/
void HfpRejectCall(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	/* Send an internal message to reject an incoming call. */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CHUP_REJECT_REQ, 0);
}


/****************************************************************************
NAME	
	HfpTerminateCall

DESCRIPTION
	This function is used to hang up an active call or terminate an outgoing 
	call process before it has been completed. The AT command will be sent out 
	on the SLC of the hfp profile instance passed into this function from the
	application. The message returned indicates whether the command was 
	recognised by the AG or not. 

MESSAGE RETURNED
	HFP_TERMINATE_CALL_CFM

RETURNS
	void
*/
void HfpTerminateCall(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	/* Send an internal message to terminate an ongoing call process. */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CHUP_TERMINATE_REQ, 0);
}
