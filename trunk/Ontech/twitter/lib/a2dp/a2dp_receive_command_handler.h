/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_receive_command_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_RECEIVE_COMMAND_HANDLER_H_
#define A2DP_RECEIVE_COMMAND_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME	
	handleReceiveCommand

DESCRIPTION
	This function is called to process a command received over the signalling
	channel.
	
*/
bool handleReceiveCommand(A2DP* a2dp, const uint8 *ptr, uint16 packet_size);


#endif /* A2DP_RECEIVE_COMMAND_HANDLER_H_ */
