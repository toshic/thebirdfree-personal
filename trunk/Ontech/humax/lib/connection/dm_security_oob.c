/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    dm_security_oob.c        

DESCRIPTION
    This file contains the management entity responsible for device security

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "dm_security_auth.h"

#include    <message.h>
#include    <vm.h>


/*****************************************************************************/
void ConnectionSmReadLocalOobData(Task theAppTask)
{
	MAKE_CL_MESSAGE(CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ);
	message->task = theAppTask;
	MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ, message, &theCm.smState.authReqLock);
}

