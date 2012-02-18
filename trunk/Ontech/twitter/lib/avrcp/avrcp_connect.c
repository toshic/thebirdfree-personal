/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    avctp_connect.c
    
DESCRIPTION
*/

#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_common.h"
#include "init.h"

#include <panic.h>


/*****************************************************************************/
static void avrcpSendInternalConnectResp(AVRCP *avrcp, uint16 connection_id, bool accept)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_CONNECT_RES);
	message->connection_id = connection_id;
	message->accept = accept;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_CONNECT_RES, message);
}


/*****************************************************************************/
static void avrcpSendInternalConnectReq(AVRCP *avrcp, const bdaddr *bd_addr)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_CONNECT_REQ);
    message->bd_addr = *bd_addr;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_CONNECT_REQ, message);
}


/*****************************************************************************/
void AvrcpConnect(AVRCP *avrcp, const bdaddr *bd_addr)
{
#ifdef AVRCP_DEBUG_LIB	
    if (!bd_addr)
    {
        AVRCP_DEBUG(("Null Bluetooth address pointer\n"));
    }
#endif
    
    avrcpSendInternalConnectReq(avrcp, bd_addr);
}


/*****************************************************************************/
void AvrcpConnectLazy(Task clientTask, const bdaddr *bd_addr, const avrcp_init_params *config)
{
    AVRCP *avrcp = PanicUnlessNew(AVRCP);
    
#ifdef AVRCP_DEBUG_LIB	
    if (!config)
    {
        AVRCP_DEBUG(("Null config structure\n"));
    }
#endif
    
    avrcpInitTaskData(avrcp, clientTask, avrcpReady, config->device_type, 1);
    avrcpSendInternalConnectReq(avrcp, bd_addr);
}


/*****************************************************************************/
void AvrcpConnectResponse(AVRCP *avrcp, uint16 connection_id, bool accept)
{
    avrcpSendInternalConnectResp(avrcp, connection_id, accept);
}


/*****************************************************************************/
void AvrcpConnectResponseLazy(Task clientTask, AVRCP *avrcp, uint16 connection_id, bool accept, const avrcp_init_params *config)
{
#ifdef AVRCP_DEBUG_LIB	
    if (accept && !config)
    {
        AVRCP_DEBUG(("Null config structure\n"));
    }
#endif

    /* Finish off task instance initialisation before respnding to connect request. */
    avrcpInitTaskData(avrcp, clientTask, avrcp->state, accept ? config->device_type : avrcp_device_none, 1);
    avrcpSendInternalConnectResp(avrcp, connection_id, accept);
}


/****************************************************************************
NAME	
	AvrcpDisconnect	

DESCRIPTION
	This function is called to request an AVRCP disconnection.  

MESSAGE RETURNED
	AVRCP_DISCONNECT_IND
*/
void AvrcpDisconnect(AVRCP *avrcp)
{
	MessageSend(&avrcp->task, AVRCP_INTERNAL_DISCONNECT_REQ, 0);
}


/****************************************************************************
NAME	
	AvrcpGetSink	

DESCRIPTION
	This function is called to retrieve the connection Sink.  

PARAMETER RETURNED
	The connection sink. This will be 0 if no connection exists.
*/
Sink AvrcpGetSink(AVRCP *avrcp)
{
    if (!avrcp)
    {
#ifdef AVRCP_DEBUG_LIB
        AVRCP_DEBUG(("AvrcpGetSink NULL AVRCP instance\n"));
#endif
        return (Sink)0;
    }

    return avrcp->sink;
}
