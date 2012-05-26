/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    dm_baseband_scan.c        

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
void ConnectionWriteScanEnable(hci_scan_enable mode)
{
	/* Check params are within allowed values - debug build only */
#ifdef CONNECTION_DEBUG_LIB	
	if (mode > hci_scan_enable_inq_and_page)
	{
		CL_DEBUG(("Out of range scan enable 0x%x\n", mode));
	}
#endif
	
	{
		/* All requests are sent through the internal state handler */
		MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ);
		message->mode = mode;
		MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ, message);
	}
}

