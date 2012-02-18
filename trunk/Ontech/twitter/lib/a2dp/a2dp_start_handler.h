/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_start_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_START_HANDLER_H_
#define A2DP_START_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME
	sendStartCfm

DESCRIPTION
    Send cfm message to app as a result of calling the A2dpStart API.
*/
void sendStartCfm(A2DP *a2dp, Sink sink, a2dp_status_code status);


/****************************************************************************
NAME
	a2dpHandleStartReq

DESCRIPTION
    Handles a local A2DP_INTERNAL_START_REQ message.
*/
void a2dpHandleStartReq(A2DP *a2dp);


#endif /* A2DP_START_HANDLER_H_ */
