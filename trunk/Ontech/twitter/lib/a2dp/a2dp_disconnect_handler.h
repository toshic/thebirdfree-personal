/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_disconnect_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_DISCONNECT_HANDLER_H_
#define A2DP_DISCONNECT_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME
	a2dpHandleDisconnectAllReq

DESCRIPTION
    Handle an internal A2DP_INTERNAL_DISCONNECT_ALL_REQ message to handle a locally initiated  disconnect all request.
*/
void a2dpHandleDisconnectAllReq(A2DP *a2dp);


#endif /* A2DP_DISCONNECT_HANDLER_H_ */
