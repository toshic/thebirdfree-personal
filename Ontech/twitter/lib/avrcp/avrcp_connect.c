/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avctp_connect.c
    
DESCRIPTION
*/

#include "avrcp.h"
#include "avrcp_common.h"
#include "avrcp_connect_handler.h"
#include "avrcp_private.h"
#include "avrcp_send_response.h"
#include "avrcp_init.h"

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
    if (!config)
    {
        /* Client must pass down a valid config so report connect fail */
        MAKE_AVRCP_MESSAGE(AVRCP_CONNECT_CFM);
	    message->status = avrcp_fail;
	    message->sink = 0;
	    message->avrcp = 0;
	    MessageSend(clientTask, AVRCP_CONNECT_CFM, message);

        /* 
            TODO use fun below but it needs updating
        avrcpSendCommonCfmMessageToApp(AVRCP_CONNECT_CFM, avrcp_fail, 0, avrcp);
        */
    }
    else
    {
        AVRCP *avrcp = PanicUnlessNew(AVRCP);
        avrcpInitTaskData(avrcp, clientTask, avrcpReady, config->device_type, config->supported_controller_features, config->supported_target_features, config->profile_extensions, 1);
        avrcpSendInternalConnectReq(avrcp, bd_addr);
    }
}


/*****************************************************************************/
void AvrcpConnectResponseLazy(AVRCP *avrcp, uint16 connection_id, bool accept, const avrcp_init_params *config)
{
    if (!config)
    {
        /* Client must pass down a valid config so report connect fail */
        avrcpSendCommonCfmMessageToApp(AVRCP_CONNECT_CFM, avrcp_fail, 0, avrcp);

        /* Reject the connection and destroy the task instance */
        avrcpSendInternalConnectResp(avrcp, connection_id, 0);
        MessageSend(&avrcp->task, AVRCP_INTERNAL_TASK_DELETE_REQ, 0);
    }
    else
    {
        /* Finish off task instance initialisation before respnding to connect request. */
        avrcpInitTaskData(avrcp, avrcp->clientTask, avrcp->state, config->device_type, config->supported_controller_features, config->supported_target_features, config->profile_extensions, 1);
        avrcpSendInternalConnectResp(avrcp, connection_id, accept);
    }
}


/*****************************************************************************/
void AvrcpConnectResponse(AVRCP *avrcp, uint16 connection_id, bool accept)
{
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
