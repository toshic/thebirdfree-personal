/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_voice.c

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
	HfpVoiceRecognitionEnable

DESCRIPTION
	Enable / disable voice recognition function at the AG. An SLC for the 
	supplied profile instance (hfp) must already be established before calling 
	this function. The enable flag determines whether a request will be made 
	to the AG to enabe or disable its voice recognition function. The message 
	returned indicates whether the command was recognised by the AG or not. 
	
	The AG may autonomously notify the HFP device of a change in the state of
	its voice recognition function. This notification will be passed on to the 
	application using a HFP_VOICE_RECOGNITION_IND message.

MESSAGE RETURNED
	HFP_VOICE_RECOGNITION_ENABLE_CFM

RETURNS
	void
*/
void HfpVoiceRecognitionEnable(HFP *hfp, bool enable)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
#endif

	{
		/* Send an internal message to kick off SLC creation */
		MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_BVRA_REQ);
		message->enable = enable;
		MessageSend(&hfp->task, HFP_INTERNAL_AT_BVRA_REQ, message);
	}
}
