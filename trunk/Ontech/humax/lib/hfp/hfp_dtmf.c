/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_dtmf.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"

#include <panic.h>


#ifdef HFP_DEBUG_LIB
#include <ctype.h>
#endif


/****************************************************************************
NAME	
	HfpSendDtmf

DESCRIPTION
	Request to transmit a DTMF code to the AG. The request is issued on the 
	SLC associated with the hfp profile instance passed in by the application. 
	The message returned indicates whether the command was recognised by the 
	AG or not. The dtmf character must be one from the set 0-9, #, *, A-D.

MESSAGE RETURNED
	HFP_DTMF_CFM

RETURNS
	void
*/
void HfpSendDtmf(HFP *hfp, uint8 dtmf)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));

	/* Parameter check the dtmf value */
	if (!(isdigit(dtmf) || dtmf=='#' || dtmf=='*' || dtmf=='A' || dtmf=='B' || dtmf=='C' || dtmf=='D'))
		HFP_DEBUG(("Invalid dtmf values passed in %c\n", dtmf));
#endif

	{
		/* Send an internal message so we can go through the state machine */
		MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_VTS_REQ);
		message->dtmf = dtmf;		
		MessageSend(&hfp->task, HFP_INTERNAL_AT_VTS_REQ, message);
	}
}
