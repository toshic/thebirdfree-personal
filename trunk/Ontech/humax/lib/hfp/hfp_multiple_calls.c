/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_multiple_calls.c

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
	HfpMultipleCallsReleaseHeldOrRejectWaiting

DESCRIPTION
	This function is used when handling multiparty calls. This corresponds to
	sending an AT+CHLD=0 command to the AG. Depending on the state of the 
	calls it either releases all held calls or sets User Determined User Busy 
	(UDUB) for a waiting call. The request is issued on the SLC associated 
	with the hfp profile instance passed in by the application. The message 
	returned indicates whether the command was recognised by the AG or not.

MESSAGE RETURNED
	HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM

RETURNS
	void
*/
void HfpMultipleCallsReleaseHeldOrRejectWaiting(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif
	
	/* Send an internal message */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CHLD_0_REQ, 0);
}


/****************************************************************************
NAME	
	HfpMultipleCallsReleaseActiveAcceptOther

DESCRIPTION
	This function is used when handling multiparty calls. This corresponds 
	to sending an AT+CHLD=1 to the AG. All active calls (if any) are released 
	and the other (held or waiting) call is accepted. The request is issued 
	on the SLC associated with the hfp profile instance passed in by the 
	application. The message returned indicates whether the command was 
	recognised by the AG or not.

MESSAGE RETURNED
	HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM	

RETURNS
	void
*/
void HfpMultipleCallsReleaseActiveAcceptOther(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif
	
	/* Send an internal message */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CHLD_1_REQ, 0);
}


/****************************************************************************
NAME	
	HfpMultipleCallsReleaseSpecifiedAcceptOther

DESCRIPTION
	This function is used when handling multiparty calls. This corresponds 
	to sending an AT+CHLD=1,<idx> to the AG. It causes the specified call to 
	be released.  If the released call was an active call and a call is currently
	held, the AG shall retrieve	the held call.  If there are multiple held calls 
	the AG will retrieve the call with the lowest call index. The request is 
	issued on the SLC associated with the hfp profile instance passed in by the 
	application. The message returned indicates whether the command was 
	recognised by the AG or not.

MESSAGE RETURNED
	HFP_RELEASE_SPECIFIED_ACCEPT_OTHER_CALL_CFM	

RETURNS
	void
*/
void HfpMultipleCallsReleaseSpecifiedAcceptOther(HFP *hfp, uint16 call_idx)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	{
    	/* Send an internal message */
    	MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_CHLD_1X_REQ);	
    	message->call_idx = call_idx;
    	MessageSend(&hfp->task, HFP_INTERNAL_AT_CHLD_1X_REQ, message);
    }
}


/****************************************************************************
NAME	
	HfpMultipleCallsHoldActiveAcceptOther

DESCRIPTION
	This function is used when handling multiparty calls. This corresponds 
	to sending an AT+CHLD=2 to the AG. All active calls (if any) are placed 
	on hold and the other (held or waiting) call is accepted. The request is 
	issued on the SLC associated with the hfp profile instance passed in by 
	the application. The message returned indicates whether the command was 
	recognised by the AG or not.

MESSAGE RETURNED
	HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM
	
RETURNS
	void
*/
void HfpMultipleCallsHoldActiveAcceptOther(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif
	
	/* Send an internal message */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CHLD_2_REQ, 0);
}


/****************************************************************************
NAME	
	HfpMultipleCallsRequestPrivateHoldOther

DESCRIPTION
	This function is used when handling multiparty calls. This corresponds 
	to sending an AT+CHLD=2,<idx> to the AG. The <idx> is used to specify which
	call to request a private consultation with, all other calls are placed	on 
	hold. The request is issued on the SLC associated with the hfp profile 
	instance passed in by the application. The message returned indicates whether 
	the command was recognised by the AG or not.

MESSAGE RETURNED
	HFP_REQUEST_PRIVATE_HOLD_OTHER_CALL_CFM
	
RETURNS
	void
*/
void HfpMultipleCallsRequestPrivateHoldOther(HFP *hfp, uint16 call_idx)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif
	
    {
    	/* Send an internal message */
    	MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_CHLD_2X_REQ);	
    	message->call_idx = call_idx;
    	MessageSend(&hfp->task, HFP_INTERNAL_AT_CHLD_2X_REQ, message);
    }
}


/****************************************************************************
NAME	
	HfpMultipleCallsAddHeldCall

DESCRIPTION
	This function is used when handling multiparty calls. This corresponds 
	to sending an AT+CHLD=3 to the AG. The held call is added to the
	conversation. The request is issued on the SLC associated with the hfp 
	profile instance passed in by the application. The message returned 
	indicates whether the command was recognised by the AG or not.

MESSAGE RETURNED
	HFP_ADD_HELD_CALL_CFM
	
RETURNS
	void
*/
void HfpMultipleCallsAddHeldCall(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif
	
	/* Send an internal message */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CHLD_3_REQ, 0);
}


/****************************************************************************
NAME	
	HfpMultipleCallsExplicitCallTransfer	

DESCRIPTION
	This function is used when handling multiparty calls. This corresponds 
	to sending an AT+CHLD=4 to the AG. Connect the two external calls and 
	disconnect the subscriber from the conversation (Explicit Call Transfer).
	Support for this is optional for the HF. The request is issued on the 
	SLC associated with the hfp profile instance passed in by the application. 
	The message returned indicates whether the command was recognised by the 
	AG or not.

MESSAGE RETURNED
	HFP_EXPLICIT_CALL_TRANSFER_CFM

RETURNS
	void
*/
void HfpMultipleCallsExplicitCallTransfer(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif
	
	/* Send an internal message */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CHLD_4_REQ, 0);
}
