/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    sdp_mtu.c        

DESCRIPTION
		

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <panic.h>
#include <string.h>


/*****************************************************************************/
void ConnectionSetSdpServerMtu(uint16 mtu)
{
#ifdef CONNECTION_DEBUG_LIB	
	if (mtu < MINIMUM_MTU)
		CL_DEBUG(("sdp - mtu too small 0x%x\n", mtu));
#endif

	/* Send an internal message */
	{
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ);
		message->mtu = mtu;
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ, message, &theCm.sdpState.sdpLock);
	}
}


/*****************************************************************************/
void ConnectionSetSdpClientMtu(uint16 mtu)
{
#ifdef CONNECTION_DEBUG_LIB	
	if (mtu < MINIMUM_MTU)
		CL_DEBUG(("sdp - mtu too small 0x%x\n", mtu));
#endif

	/* Send an internal message */
	{
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ);
		message->mtu = mtu;
		MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ, message, &theCm.sdpState.sdpLock);
	}
}


