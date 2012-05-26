/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    ConnectionGetRole.c        

DESCRIPTION
    This file contains the implementation of the link policy management 
    entity. This is responsible for arbitrating between the different low 
    power mode requirements of the connection library clients.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"


/*****************************************************************************/
void ConnectionGetRole(Task task, Sink sink)
{
	MAKE_CL_MESSAGE(CL_INTERNAL_DM_GET_ROLE_REQ);
	message->theAppTask = task;
	message->sink = sink;
	MessageSendConditionally(connectionGetCmTask(), CL_INTERNAL_DM_GET_ROLE_REQ, message, (const uint16 *)&theCm.linkPolicyState.roleLock);
}


