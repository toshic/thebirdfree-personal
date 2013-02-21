/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    avrcp_connect_handler.c
    
DESCRIPTION
*/

#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_common.h"
#include "avrcp_connect_handler.h"

#include <panic.h>


/****************************************************************************
NAME	
	avrcpHandleInternalConnectReq

DESCRIPTION
	This function handles an internally generated connect request message.
*/
void avrcpHandleInternalConnectReq(AVRCP *avrcp, const AVRCP_INTERNAL_CONNECT_REQ_T *req)
{
	/* Set the state to connecting */
	avrcpSetState(avrcp, avrcpConnecting);

	/* Initiate the L2CAP connection */
	ConnectionL2capConnectRequest(&avrcp->task, &req->bd_addr, AVCTP_PSM, AVCTP_PSM, 0);
}


/****************************************************************************
NAME	
	avrcpHandleInternalConnectRes

DESCRIPTION
	This function handles an internally generated connect response message.
*/
void avrcpHandleInternalL2capConnectRes(AVRCP *avrcp, const AVRCP_INTERNAL_CONNECT_RES_T *res)
{
	/* Send the response to the connection lib */
	ConnectionL2capConnectResponse(&avrcp->task, res->accept, AVCTP_PSM, res->connection_id, 0);

	/* If the connection has been rejected reset the local state to ready */
	if (!res->accept)
    {
        if (avrcp->lazy)
            MessageSend(&avrcp->task, AVRCP_INTERNAL_TASK_DELETE_REQ, 0);
        else
		    avrcpSetState(avrcp, avrcpReady);
    }
}


/****************************************************************************
NAME	
	avrcpHandleInternalDisconnectReq

DESCRIPTION
	This function handles an internally generated disconnect request message.
*/
void avrcpHandleInternalDisconnectReq(AVRCP *avrcp)
{
	/* Make sure we have been passed a valid sink */
	if (avrcp->sink)
		ConnectionL2capDisconnectRequest(&avrcp->task, avrcp->sink);
	else
	{
		avrcpSendCommonCfmMessageToApp(AVRCP_DISCONNECT_IND, avrcp_invalid_sink, 0, avrcp);
	}
}
