/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_send_packet_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_SEND_PACKET_HANDLER_H_
#define A2DP_SEND_PACKET_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME	
	a2dpGrabSink

DESCRIPTION
	Allocates space in the Sink for writing data.

RETURNS
	NULL on failure.
*/
uint8 *a2dpGrabSink(Sink sink, uint16 size);


/****************************************************************************
NAME	
	a2dpSendPacket

DESCRIPTION
	Flushes an AVDTP packet already written into the passed Sink and
    performs any required fragmentation.

RETURNS
	bool - TRUE on success, FALSE on failure.
*/
bool a2dpSendPacket(Sink sink, uint16 mtu, uint16 length);


#endif /* A2DP_SEND_PACKET_HANDLER_H_ */
