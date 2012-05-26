/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    ConnectionReadClassOfDevice.c        

DESCRIPTION
	This file contains the implementation of the entity responsible for 
    configuring the local baseband. This includes making the device
    discoverable, connectable etc.

NOTES

*/


/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include	"common.h"

#include <message.h>
#include <panic.h>
#include <string.h>
#include <vm.h>


/*****************************************************************************/
void ConnectionReadClassOfDevice(Task theAppTask)
{
	/* Create internal message and sent to the CL */
	MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ);
	message->theAppTask = theAppTask;
	MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ, message, &theCm.infoState.stateInfoLock);
}


