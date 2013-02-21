/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_battery_handler.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_battery_handler.h"
#include "avrcp_common.h"
#include "avrcp_private.h"
#include "avrcp_send_response.h"


/*****************************************************************************/
void avrcpHandleInformBatteryStatusCommand(AVRCP *avrcp, uint16 transaction, uint16 battery_status)
{
	/*	
		Process the command PDU. If app settings are not enabled at this side then reject the 
		command immediately. If they are enabled then send an indication up to the app
		so it can respond. 
	*/
	if (!avrcpMetadataAppSettingsEnabled(avrcp))
	{
		avrcpSendInformBatteryResponse(avrcp, avctp_response_not_implemented); 	
	}
	else
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INFORM_BATTERY_STATUS_IND);
		message->avrcp = avrcp;
		message->transaction = transaction;
		message->battery_status = battery_status;
		MessageSend(avrcp->clientTask, AVRCP_INFORM_BATTERY_STATUS_IND, message);
	}
}

