/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/


#include "aghfp.h"
#include "aghfp_private.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
	See comment in aghfp.h
*/
void AghfpSlcConnect(AGHFP *aghfp, const bdaddr *bd_addr)
{
	/* Send an internal message to kick off SLC creation */
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SLC_CONNECT_REQ);

	AGHFP_DEBUG_ASSERT(bd_addr != NULL, ("Null address ptr passed in.\n"));
	AGHFP_DEBUG_ASSERT(aghfp != NULL, ("Null aghfp task ptr passed in.\n"));

	message->addr = *bd_addr;

	MessageSend(&aghfp->task, AGHFP_INTERNAL_SLC_CONNECT_REQ, message);
}


/****************************************************************************
	See comment in aghfp.h
*/
void AghfpSlcConnectResponse(AGHFP *aghfp, bool response, const bdaddr *bd_addr)
{
	/* Send an internal message to kick off SLC creation */
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SLC_CONNECT_RES);

	AGHFP_DEBUG_ASSERT(bd_addr != NULL, ("Null address ptr passed in.\n"));
	AGHFP_DEBUG_ASSERT(aghfp != NULL, ("Null aghfp task ptr passed in.\n"));

	message->addr = *bd_addr;
	message->response = response;

	MessageSend(&aghfp->task, AGHFP_INTERNAL_SLC_CONNECT_RES, message);
}


/****************************************************************************
	This function initiates the disconnection of an SLC for a particular
	profile instance (aghfp).

MESSAGE RETURNED
	AGHFP_SLC_DISCONNECT_IND
*/
void AghfpSlcDisconnect(AGHFP *aghfp)
{
	AGHFP_DEBUG_ASSERT(aghfp != NULL, ("Null aghfp task ptr passed in.\n"));

	/* Send an internal message to kick off the disconnect */
	MessageSend(&aghfp->task, AGHFP_INTERNAL_SLC_DISCONNECT_REQ, NULL);
}
