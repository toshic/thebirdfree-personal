/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_open_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_OPEN_HANDLER_H_
#define A2DP_OPEN_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME	
    a2dpSendConnectOpenCfm

DESCRIPTION
    Send an A2DP_CONNECT_OPEN_CFM message to the client task informing it of the 
    outcome of the request to open an AVDTP signalling connection and media channel with a remote device.

*/
void a2dpSendConnectOpenCfm(A2DP *a2dp, Task client, a2dp_status_code status);


/****************************************************************************
NAME
	a2dpSendOpenCfm

DESCRIPTION
    Send an A2DP_OPEN_CFM message to the client task informing it of the 
    outcome of the request to open an a2dp connection to a remote device.
*/
void a2dpSendOpenCfm(A2DP *a2dp, Task client, a2dp_status_code status);


/***************************************************************************
NAME
	a2dpHandleOpenReq

DESCRIPTION
    Handle a A2DP_INTERNAL_OPEN_REQ message sent internally by the library
    to kick off creating an a2dp connection to a remote device.
*/
void a2dpHandleOpenReq(A2DP *a2dp, const A2DP_INTERNAL_OPEN_REQ_T *req);


#endif /* A2DP_OPEN_HANDLER_H_ */
