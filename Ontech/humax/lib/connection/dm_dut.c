/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    dm_dut.c        

DESCRIPTION	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"


/*****************************************************************************/
void ConnectionEnterDutMode(void)
{	
	/* All requests are sent through the internal state handler */	
	MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_DUT_REQ, 0);
}

