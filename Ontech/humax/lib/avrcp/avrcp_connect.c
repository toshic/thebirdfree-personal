/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avctp_connect.c
    
DESCRIPTION
*/

#include "avrcp.h"
#include "avrcp_private.h"

#include <panic.h>


/****************************************************************************
NAME
	AvrcpConnect	

DESCRIPTION
	This function is called to initiate an AVRCP connection to a remote 
	device.

MESSAGE RETURNED
	AVRCP_CONNECT_CFM
*/
void AvrcpConnect(AVRCP *avrcp, const bdaddr *bd_addr)
{	
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_CONNECT_REQ);
    
#ifdef AVRCP_DEBUG_LIB	
    if (!bd_addr)
    {
        AVRCP_DEBUG(("Null Bluetooth address pointer\n"));
    }
#endif
    
	message->bd_addr = *bd_addr;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_CONNECT_REQ, message);
}


/****************************************************************************
NAME	
	AvrcpConnectResponse	

DESCRIPTION
	This function is called on receipt of an AVRCP_CONNECT_IND message. It 
	is used to either accept or reject the incoming connection from the 
	remote device whose address is in the AVRCP_CONNECT_IND message.

MESSAGE RETURNED
	AVRCP_CONNECT_IND
*/
void AvrcpConnectResponse(AVRCP *avrcp, uint16 connection_id, bool accept)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_CONNECT_RES);
	message->connection_id = connection_id;
	message->accept = accept;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_CONNECT_RES, message);
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
