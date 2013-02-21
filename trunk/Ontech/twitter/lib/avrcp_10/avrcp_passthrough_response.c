/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	avrcp_signal.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_signal_handler.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME	
	AvrcpPassthroughResponse

DESCRIPTION
	This function is called in response to an AVRCP_PASSTHROUGH_IND message
	to verify the data that was sent.  

MESSAGE RETURNED
	
*/
void AvrcpPassthroughResponse(AVRCP *avrcp, avrcp_response_type response)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_PASSTHROUGH_RES);
    
#ifdef AVRCP_DEBUG_LIB	
	if ((response < avctp_response_not_implemented) || (response > avctp_response_bad_profile))
	{
		AVRCP_DEBUG(("Out of range response  0x%x\n", response));
	}
#endif
    
	message->response = response;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_PASSTHROUGH_RES, message);
}


