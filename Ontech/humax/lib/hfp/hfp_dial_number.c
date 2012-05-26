/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_dial_number.c

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
	HfpDialNumber

DESCRIPTION
	This function issues a request to the AG to dial the supplied number.
	The request is issued on the SLC associated with the hfp profile instance 
	passed in by the application. 
	
	The length argument specifies the length in bytes of the number to be sent. 
	
	The number is specified as an array and can include valid dial characters 
	such as '+'. The message returned indicates whether the command was 
	recognised by the AG or not. 

MESSAGE RETURNED
	HFP_DIAL_NUMBER_CFM

RETURNS
	void
*/
void HfpDialNumber(HFP *hfp, uint16 length, const uint8 *number)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));

	if (!length)
		HFP_DEBUG(("Zero length passed in.\n"));

	if (!number)
		HFP_DEBUG(("Null number ptr passed in.\n"));
#endif

	/* Send an internal message */
	{
		MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_ATD_NUMBER_REQ);
		message->length = length;
		message->number = (uint8 *) PanicUnlessMalloc(length);
		memmove(message->number, number, length);
		MessageSend(&hfp->task, HFP_INTERNAL_AT_ATD_NUMBER_REQ, message);
	}
}
