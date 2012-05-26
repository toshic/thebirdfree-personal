/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	avrcp_common.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_common.h"

#include <panic.h>


/****************************************************************************
NAME	
	avrcpSetState

DESCRIPTION
	Update the avrcp state.
*/
void avrcpSetState(AVRCP *avrcp, avrcpState state)
{
	avrcp->state = state;
}


/****************************************************************************
NAME	
	avrcpSendCommonCfmMessageToApp

DESCRIPTION
	Create a common cfm message (many messages sent to the app
	have the form of the message below and a common function can be used to
	allocate them). Send the message not forgetting to set the correct 
	message id.

RETURNS
	void
*/
void avrcpSendCommonCfmMessageToApp(uint16 message_id, avrcp_status_code status, Sink sink, AVRCP *avrcp)
{
	MAKE_AVRCP_MESSAGE(AVRCP_COMMON_CFM_MESSAGE);
	message->status = status;
	message->sink = sink;
	message->avrcp = avrcp;
	MessageSend(avrcp->clientTask, message_id, message);
}
