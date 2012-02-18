/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_close_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_CLOSE_HANDLER_H_
#define A2DP_CLOSE_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME	
	sendCloseCfm

DESCRIPTION
    Send a confirmation to app as a result of calling A2dpClose.
*/
void sendCloseCfm(A2DP *a2dp, a2dp_status_code status);


/****************************************************************************
NAME	
	a2dpHandleCloseReq

DESCRIPTION
    Handles the A2DP_INTERNAL_CLOSE_REQ message. Internal function to handle a locally issued Close request.
*/
void a2dpHandleCloseReq(A2DP *a2dp);


#endif /* A2DP_CLOSE_HANDLER_H_ */
