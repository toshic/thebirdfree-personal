/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    ConnectionReadRemoteName.c        

DESCRIPTION
    This file contains the management entity responsible for device security

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"

#include    <message.h>
#include	<string.h>
#include    <vm.h>

/*****************************************************************************/
void ConnectionReadRemoteName(Task theAppTask, const bdaddr *bd_addr)
{
   MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_REMOTE_NAME_REQ);
   message->theAppTask = theAppTask;
   message->bd_addr = *bd_addr;
   MessageSendConditionally(connectionGetCmTask(), CL_INTERNAL_DM_READ_REMOTE_NAME_REQ, message, (const uint16 *)&theCm.inqState.nameLock);
}
