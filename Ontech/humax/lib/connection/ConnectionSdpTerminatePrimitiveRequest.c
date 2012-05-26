/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    ConnectionSdpTerminatePrimitiveRequest.c        

DESCRIPTION
		

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <panic.h>
#include <string.h>


/*****************************************************************************/
void ConnectionSdpTerminatePrimitiveRequest(Task appTask)
{
	MAKE_CL_MESSAGE(CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ);
	message->theAppTask = appTask;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ, message);
}
