/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_connect_handler.h
    
DESCRIPTION
    
*/


#ifndef AVRCP_CONNECT_HANDLER_H_
#define AVRCP_CONNECT_HANDLER_H_

#include "avrcp_common.h"



/****************************************************************************
NAME    
    avrcpHandleInternalConnectReq

DESCRIPTION
    This function handles an internally generated connect request message.
*/
void avrcpHandleInternalConnectReq(AVRCP *avrcp, const AVRCP_INTERNAL_CONNECT_REQ_T *req);


/****************************************************************************
NAME    
    avrcpHandleInternalConnectRes

DESCRIPTION
    This function handles an internally generated connect response message.
*/
void avrcpHandleInternalL2capConnectRes(AVRCP *avrcp, const AVRCP_INTERNAL_CONNECT_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalDisconnectReq

DESCRIPTION
    This function handles an internally generated disconnect request message.
*/
void avrcpHandleInternalDisconnectReq(AVRCP *avrcp);


#endif /* AVRCP_CONNECT_HANDLER_H_ */
