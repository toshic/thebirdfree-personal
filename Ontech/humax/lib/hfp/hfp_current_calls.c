/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
	hfp_current_calls.c        

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
	HfpGetCurrentCalls

DESCRIPTION
	Requests list of current calls from the AG. An SLC for the supplied
	profile instance (hfp) must already be established before calling this
	function. Each individual response sent by the AG will be sent to the	
	application using a HFP_CURRENT_CALL_IND message.  If there are no current
    calls the application will not receive any HFP_CURRENT_CALL_IND messages.  
    When the application receives a HFP_CURRENT_CALL_CFM message it will indicate 
    that the request has been completed.

MESSAGE RETURNED
	HFP_CURRENT_CALL_CFM

RETURNS
	void
*/
void HfpGetCurrentCalls(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	{
		/* Send an internal message */
		MessageSend(&hfp->task, HFP_INTERNAL_AT_CLCC_REQ, 0);
    }
}
