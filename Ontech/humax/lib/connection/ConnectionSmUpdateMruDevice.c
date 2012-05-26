/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    ConnectionSmUpdateMruDevice.c        

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
void ConnectionSmUpdateMruDevice(const bdaddr *bd_addr)
{
	/* Update the Trusted Device Index so that the specified device is recorded
	   as the most recently used device */
	connectionAuthUpdateMru(bd_addr);
}


