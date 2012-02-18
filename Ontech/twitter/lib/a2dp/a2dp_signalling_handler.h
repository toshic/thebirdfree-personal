/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_signalling_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_SIGNALLING_HANDLER_H_
#define A2DP_SIGNALLING_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME	
    a2dpHandleConnectSignallingReq

DESCRIPTION
    Handles the request to connect an AVDTP signalling channel.


*/
void a2dpHandleConnectSignallingReq(A2DP *a2dp, const A2DP_INTERNAL_CONNECT_SIGNALLING_REQ_T *req);


/****************************************************************************
NAME	
    a2dpHandleConnectSignallingRes

DESCRIPTION
    Handles the response to an AVDTP signalling channel request.

*/
void a2dpHandleConnectSignallingRes(A2DP *a2dp, const A2DP_INTERNAL_CONNECT_SIGNALLING_RES_T *res);


/****************************************************************************
NAME	
    a2dpSetSignallingState

DESCRIPTION
    Changes the signalling state (discover, set_config etc.).

*/
void a2dpSetSignallingState(A2DP *a2dp, avdtp_state state);


/****************************************************************************
NAME	
    a2dpHandleWatchdogInd

DESCRIPTION
    Function called when the signalling watchdog times out.

*/
void a2dpHandleWatchdogInd(A2DP *a2dp);


/****************************************************************************
NAME	
    a2dpAbortSignalling

DESCRIPTION
    Aborts the current A2DP connection.

*/
void a2dpAbortSignalling(A2DP *a2dp, bool link_loss, bool key_missing);


/****************************************************************************
NAME	
    a2dpResetSignalling

DESCRIPTION
    Resets the current A2DP connection.

*/
void a2dpResetSignalling(A2DP *a2dp, bool link_loss, bool key_missing);


/****************************************************************************
NAME	
    a2dpHandleSignalConnectionTimeoutInd

DESCRIPTION
    Signalling connection timer has fired, so the connection must be closed.

*/
void a2dpHandleSignalConnectionTimeoutInd(A2DP* a2dp);


/****************************************************************************
NAME	
    a2dpGetCapsTimeout

DESCRIPTION
    A get capabilities signalling request has timed out.

*/
void a2dpGetCapsTimeout(A2DP* a2dp);


#endif /* A2DP_SIGNALLING_HANDLER_H_ */
