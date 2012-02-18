/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_send_command_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_SEND_COMMAND_HANDLER_H_
#define A2DP_SEND_COMMAND_HANDLER_H_

#include "a2dp.h"


/****************************************************************************
NAME	
    a2dpGetNextTransactionLabel

DESCRIPTION
    This function returns the next transaction label.  This is used to route
	responses from the remote device to the correct local initiator.  Note
	that zero is a special case, indicating no command is pending.

*/
uint16 a2dpGetNextTransactionLabel(A2DP* a2dp);


/****************************************************************************
NAME	
	a2dpSendDiscover

DESCRIPTION
	Send AVDTP_DISCOVER discover command.

*/
bool a2dpSendDiscover(A2DP* a2dp);


/****************************************************************************
NAME	
	gavdpSendGetCapabilities

DESCRIPTION
	Send AVDTP_GET_CAPABILITIES command.

*/
bool a2dpSendGetCapabilities(A2DP* a2dp, uint8 seid);


/****************************************************************************
NAME	
	a2dpSendSetConfiguration

DESCRIPTION
	Sends a AVDTP_SET_CONFIGURATION command.

*/
bool a2dpSendSetConfiguration(A2DP* gavdp, const uint8* config, uint16 config_size);


/****************************************************************************
NAME	
	a2dpSendOpen

DESCRIPTION
	Send AVDTP_OPEN command.

RETURNS
	void
*/
bool a2dpSendOpen(A2DP* a2dp);


/****************************************************************************
NAME	
	a2dpSendReconfigure

DESCRIPTION
	Send AVDTP_RECONFIGURE command.
RETURNS
	void
*/
bool a2dpSendReconfigure(A2DP* a2dp, const uint8* config, uint16 config_size);


/****************************************************************************
NAME	
	a2dpSendStart

DESCRIPTION
	Send AVDTP_START command.

RETURNS
	void
*/
bool a2dpSendStart(A2DP* a2dp);


/****************************************************************************
NAME	
	a2dpSendSuspend

DESCRIPTION
	Send AVDTP_SUSPEND command.

RETURNS
	void
*/
bool a2dpSendSuspend(A2DP* a2dp);


/****************************************************************************
NAME	
	a2dpSendClose

DESCRIPTION
	Send AVDTP_CLOSE command.

RETURNS
	void
*/
bool a2dpSendClose(A2DP* a2dp);


/****************************************************************************
NAME	
	a2dpSendAbort

DESCRIPTION
	Send AVDTP_ABORT command.

RETURNS
	bool
*/
bool a2dpSendAbort(A2DP* a2dp);


#endif /* A2DP_SEND_COMMAND_HANDLER_H_ */
