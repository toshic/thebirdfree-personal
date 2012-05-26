/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_hs.c

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
	HfpSendHsButtonPress

DESCRIPTION
	The Hfp prifile library can also initialise an HSP profile instance that
	provides an implementation of the HSP specification. The HSP defines only 
	a single button press AT command (in addition to the volume/ microphone 
	control commands). Using this function the application can request to 
	send a headset profile compliant button press to the AG. The request is 
	issued on the SLC associated with the hsp profile instance passed in by 
	the application. The message returned indicates whether the command was 
	recognised by the AG or not. Please note that the profile instance passed 
	in must be initialised as HSP and not HFP.

MESSAGE RETURNED
	HFP_HS_BUTTON_PRESS_CFM,

RETURNS
	void
*/
void HfpSendHsButtonPress(HFP *hsp)
{
#ifdef HFP_DEBUG_LIB
	if (!hsp)
		HFP_DEBUG(("Null hsp task ptr passed in.\n"));
#endif

	/* Send an internal message to send a button press */
	MessageSend(&hsp->task, HFP_INTERNAL_AT_CKPD_REQ, 0);
}
