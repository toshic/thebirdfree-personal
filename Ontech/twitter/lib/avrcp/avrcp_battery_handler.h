/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_battery_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_BATTERY_HANDLER_H_
#define AVRCP_BATTERY_HANDLER_H_

#include "avrcp.h"
#include "avrcp_private.h"


/****************************************************************************
NAME	
	avrcpHandleInformBatteryStatusCommand

DESCRIPTION
	Handle a InformBatteryStatusOfCT PDU command received from the CT.
*/
void avrcpHandleInformBatteryStatusCommand(AVRCP *avrcp, uint16 transaction, uint16 battery_status);


#endif /* AVRCP_BATTERY_HANDLER_H_ */
