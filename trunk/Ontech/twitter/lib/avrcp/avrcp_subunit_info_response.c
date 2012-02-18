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
	AvrcpSubUnitInfoResponse

DESCRIPTION	
	The SubUnitInfo command is used to obtain information about the subunit(s)
	of a device.
	
	accept					- Flag accepting or rejecting request for SubUnitInfo
	page_data				- Four entries from the subunit table for the requested
	   						  page on the target device
			
MESSAGE RETURNED

*/
void AvrcpSubUnitInfoResponse(AVRCP *avrcp, bool accept, const uint8 *page_data)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SUBUNITINFO_RES);
	message->accept= accept;

	if (accept)
		memmove(message->page_data, page_data, PAGE_DATA_LENGTH);
	else
		memset(message->page_data, 0, PAGE_DATA_LENGTH);

	MessageSend(&avrcp->task, AVRCP_INTERNAL_SUBUNITINFO_RES, message);
}

