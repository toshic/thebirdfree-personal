/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    ConnectionSmAddAuthDevice.c        

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
#include	<string.h>
#include    <vm.h>

/*****************************************************************************/
void ConnectionSmAddAuthDevice(Task theAppTask, const bdaddr *peer_bd_addr, uint16 trusted, uint16 bonded, uint8 key_type, uint16 size_link_key, const uint8* link_key)
{
#ifdef CONNECTION_DEBUG_LIB
	if (size_link_key != SIZE_LINK_KEY)
	{
		CL_DEBUG(("Link key size %d not supported\n", size_link_key))
	}
#endif

	{
		MAKE_CL_MESSAGE(CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ);
		message->theAppTask = theAppTask;
		message->bd_addr = *peer_bd_addr;
		message->link_key_type = key_type;
		memmove(message->link_key, link_key, SIZE_LINK_KEY);
		message->trusted = trusted;
                message->bonded = bonded;
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ, message, &theCm.smState.deviceReqLock);
	}
}
