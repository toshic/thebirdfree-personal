/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
	hfp_extended_error.c        

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
	HfpEnableExtendedErrors

DESCRIPTION
	Requests that the AG sends extended error result codes. An SLC for the 
	supplied profile instance (hfp) must already be established before calling 
	this function.   The application will receive a HFP_EXTENDED_ERROR_CFM message 
	to indicate that the request has been acknowledged.  The application will then
	received HFP_EXTENDED_ERROR_IND notification when the AG sends an extended error
	code.

MESSAGE RETURNED
	HFP_EXTENDED_ERROR_CFM

RETURNS
	void
*/
void HfpEnableExtendedErrors(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
	{
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
	}
#endif

	{
		/* Send an internal message */
		MessageSend(&hfp->task, HFP_INTERNAL_AT_CMEE_REQ, 0);
    }
}
