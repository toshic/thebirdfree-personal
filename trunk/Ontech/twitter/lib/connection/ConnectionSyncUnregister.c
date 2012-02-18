/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    ConnectionSyncUnregister.c        

DESCRIPTION
    This file contains the functions responsible for managing the setting up 
    and tearing down of Synchronous connections.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"

/*****************************************************************************/
void ConnectionSyncUnregister(Task theAppTask)
{
    /* Send an internal unregister request message */
	MAKE_CL_MESSAGE(CL_INTERNAL_SYNC_UNREGISTER_REQ);
	message->theAppTask = theAppTask;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_SYNC_UNREGISTER_REQ, message);
}


