/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_receive_packet_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_RECEIVE_PACKET_HANDLER_H_
#define A2DP_RECEIVE_PACKET_HANDLER_H_

#include "a2dp.h"


/****************************************************************************
NAME	
	a2dpHandleSignalPacket

DESCRIPTION
	This function is called on receipt of an MESSAGE_MORE_DATA
	message indicating that a peer device has send data over the
	signalling channel.

*/
void a2dpHandleSignalPacket(const A2DP *a2dp, Source source);


/****************************************************************************
NAME	
	a2dpHandleNewSignalPacket

DESCRIPTION
	This function is called on receipt of an A2DP_INTERNAL_SIGNAL_PACKET_IND
	message indicating that a peer device has send data over the signalling
	channel.

*/
void a2dpHandleNewSignalPacket(A2DP *a2dp);


#endif /* A2DP_RECEIVE_PACKET_HANDLER_H_ */
