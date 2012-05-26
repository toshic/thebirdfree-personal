/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_voice_tag.c

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
	HfpRequestNumberForVoiceTag

DESCRIPTION
	Request a number from the AG to attach to a voice tag. The request is 
	issued on the SLC associated with the hfp profile instance passed in by 
	the application. The message returned indicates whether the command was 
	recognised by the AG or not and the number supplied by the AG (if any).

MESSAGE RETURNED
	HFP_VOICE_TAG_NUMBER_CFM

RETURNS
	void
*/
void HfpRequestNumberForVoiceTag(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	/* Send an internal message */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_BINP_REQ, 0);
}
