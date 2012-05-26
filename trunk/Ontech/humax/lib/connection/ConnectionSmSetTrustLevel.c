/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    ConnectionSmSetTrustLevel.c        

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
void ConnectionSmSetTrustLevel(const bdaddr* bd_addr, uint16 trusted)
{
    /* Update the Trusted Device List */
    MAKE_CL_MESSAGE(CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ)
	message->bd_addr = *bd_addr;
	message->trusted = trusted;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ, message);
}


