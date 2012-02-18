/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_current_sep_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp_current_sep_handler.h"
#include "a2dp_private.h"

#include <string.h>


/****************************************************************************/
void sendGetCurrentSepCapabilitiesCfm(A2DP *a2dp, a2dp_status_code status, const uint8 *caps, uint16 size_caps)
{
	MAKE_A2DP_MESSAGE_WITH_LEN(A2DP_GET_CURRENT_SEP_CAPABILITIES_CFM, size_caps);

	message->a2dp = a2dp;
	message->status = status;

	if (status == a2dp_success)
	{
		memmove(message->caps, caps, size_caps);   
		message->size_caps = size_caps;
	}
	else
	{
		message->caps[0] = 0;  
		message->size_caps = 0;
	}

	MessageSend(a2dp->clientTask, A2DP_GET_CURRENT_SEP_CAPABILITIES_CFM, message);
}

