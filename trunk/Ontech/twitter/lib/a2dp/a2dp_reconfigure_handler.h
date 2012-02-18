/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_reconfigure_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_RECONFIGURE_HANDLER_H_
#define A2DP_RECONFIGURE_HANDLER_H_

#include "a2dp_private.h"


/***************************************************************************
NAME
	a2dpSendReconfigureCfm

DESCRIPTION
    Send a reconfigure confirmation message to the client as a result of calling the A2dpReconfigure API.
*/
void a2dpSendReconfigureCfm(A2DP *a2dp, a2dp_status_code status);


/***************************************************************************
NAME
	a2dpHandleReconfigureReq

DESCRIPTION
    Handle a A2DP_INTERNAL_RECONFIGURE_REQ message sent internally by the library
    to reconfigure the currently open stream.
*/
void a2dpHandleReconfigureReq(A2DP *a2dp, const A2DP_INTERNAL_RECONFIGURE_REQ_T *req);


#endif /* A2DP_RECONFIGURE_HANDLER_H_ */
