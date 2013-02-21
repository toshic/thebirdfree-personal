/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_signal_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_SIGNAL_HANDLER_H_
#define AVRCP_SIGNAL_HANDLER_H_

#include "avrcp.h"


/****************************************************************************
NAME	
	avrcpHandleReceivedData

DESCRIPTION
	This function is called to process data received over the L2cap connection.
*/
void avrcpHandleReceivedData(AVRCP *avrcp);


/****************************************************************************
NAME	
	avrcpHandleCommand

DESCRIPTION
	This function is called to process command received over 
    the L2cap connection.
*/
void avrcpHandleCommand(AVRCP *avrcp, Source source, uint16 packet_size);


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME	
	avrcpHandleSingleResponse

DESCRIPTION
	This function is called to process a response received over 
    the L2cap connection.
*/
void avrcpHandleResponse(AVRCP *avrcp, Source source, uint16 packet_size);

/****************************************************************************
NAME	
	avrcpHandleInternalWatchdogTimeout

DESCRIPTION
	Called if the watchdog times out by the CT not receiving a response within
	the alloted time.
*/
void avrcpHandleInternalWatchdogTimeout(AVRCP *avrcp);


#endif

/****************************************************************************
NAME	
	avrcpSendResponse
	
DESCRIPTION
	This function is used to send a response back to the CT device.
*/
void avrcpSendResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size, avrcp_response_type response);


/****************************************************************************
NAME
    avrcpBlockReceivedData

DESCRIPTION
    Stop handling any more received data until a response has been sent.
*/
void avrcpBlockReceivedData(AVRCP *avrcp, avrcpPending pending_command, uint16 data);


/****************************************************************************
NAME
    avrcpUnblockReceivedData

DESCRIPTION
    Restart handling received data.
*/
void avrcpUnblockReceivedData(AVRCP *avrcp);


/****************************************************************************
NAME	
	avrcpHandleInternalSendResponseTimeout

DESCRIPTION
	Called if the TG has not sent a response in the required time.
*/
void avrcpHandleInternalSendResponseTimeout(AVRCP *avrcp, const AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT_T *res);


#endif /* AVRCP_SIGNAL_HANDLER_H_ */
