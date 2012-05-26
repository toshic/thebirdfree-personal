/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
	hfp_subscriber_num.c        

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
	HfpGetSubscriberNumbers

DESCRIPTION
	Requests subscriber number information from the AG. An SLC for the supplied
	profile instance (hfp) must already be established before calling this
	function. Each individual subscriber number received from the AG will be
    sent to the	application using a HFP_SUBSCRIBER_NUMBER_IND message.  If no
    subscriber number information is available the application will not receive
    any HFP_SUBSCRIBER_NUMBER_IND messages.  When the application receives a 
    HFP_SUBSCRIBER_NUMBER_CFM message it will indicate that the request has been
    completed.

MESSAGE RETURNED
	HFP_SUBSCRIBER_NUMBER_CFM

RETURNS
	void
*/
void HfpGetSubscriberNumbers(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
	{
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
	}
#endif

	{
		/* Send an internal message */
		MessageSend(&hfp->task, HFP_INTERNAL_AT_CNUM_REQ, 0);
    }
}
