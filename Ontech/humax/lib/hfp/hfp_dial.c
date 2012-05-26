/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_dial.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"

#include <panic.h>
#include <string.h>



/****************************************************************************
NAME	
	HfpLastNumberRedial

DESCRIPTION
	This function issues a request to the AG to perform a last number redial.
	The request is issued on the SLC associated with the hfp profile instance 
	passed in by the application. The message returned indicates whether 
	the command was recognised by the AG or not.

MESSAGE RETURNED
	HFP_LAST_NUMBER_REDIAL_CFM

RETURNS
	void
*/
void HfpLastNumberRedial(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	/* Send an internal message requesting this action */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_BLDN_REQ, 0);
}
