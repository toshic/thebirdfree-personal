/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    ConnectionSmSetSdpSecurityOut.c        

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


/******************************************************************************
 This function configures the security settings for SDP service record 
 browsing. It function will configure outgoing connections to permit service 
 record to be browsed without the need for the devices concerned to be paired.
*/	
void ConnectionSmSetSdpSecurityOut(bool enable, const bdaddr *bd_addr)
{
#ifdef CONNECTION_DEBUG_LIB
    if ((enable != TRUE) && (enable != FALSE))
    {
        CL_DEBUG(("Out of range enable 0x%x\n", enable));
    }
    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void *)bd_addr)); 
    }
#endif

	if (enable)
	{
        ConnectionSmRegisterOutgoingService(bd_addr, protocol_l2cap, UUID16_SDP, secl_none);
	}
	else
	{
        ConnectionSmUnRegisterOutgoingService(bd_addr, protocol_l2cap, UUID16_SDP);
	}
}
