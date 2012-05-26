/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_nrec.c

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
	HfpDisableNrEc

DESCRIPTION
	Request the AG to disable its Noise Reduction (NR) / Echo Cancellation(EC)
	functions. This function must be called before an audio connection has 
	been established. The request is issued on the SLC associated with the 
	hfp profile instance passed in by the application. The message returned 
	indicates whether the command was recognised by the AG or not.

MESSAGE RETURNED
	HFP_DISABLE_NREC_CFM

RETURNS
	void
*/
void HfpDisableNrEc(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	/* Send an internal message */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_NREC_REQ, 0);
}
