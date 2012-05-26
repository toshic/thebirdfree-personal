/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    dm_info.c        

DESCRIPTION
    This file contains the management entity responsible for arbitrating 
    access to functionality in BlueStack that provides information on
    the setup of the local device version

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"


/*****************************************************************************/
void ConnectionReadLocalVersion(Task theAppTask)
{
 	/* All requests are sent through the internal state handler */   
 	MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ);
 	message->theAppTask = theAppTask;
 	MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ, message, &theCm.infoState.stateInfoLock);
}

