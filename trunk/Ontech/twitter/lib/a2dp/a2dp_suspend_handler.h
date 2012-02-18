/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_suspend_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_SUSPEND_HANDLER_H_
#define A2DP_SUSPEND_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME
	sendSuspendCfm

DESCRIPTION
    Send cfm message to app as a result of calling the A2dpSuspend API.
*/
void sendSuspendCfm(A2DP *a2dp, Sink sink, a2dp_status_code status);


/****************************************************************************
NAME
	a2dpHandleSuspendReq

DESCRIPTION
    Handles a local A2DP_INTERNAL_SUSPEND_REQ message.
*/
void a2dpHandleSuspendReq(A2DP *a2dp);


#endif /* A2DP_SUSPEND_HANDLER_H_ */
