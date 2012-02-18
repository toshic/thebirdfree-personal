/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    ConnectionL2capUnregisterRequest.c        

DESCRIPTION
	File containing the l2cap API function implementations.	

NOTES

*/


/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"


/*****************************************************************************/
void ConnectionL2capUnregisterRequest(Task appTask, uint16 psm)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_UNREGISTER_REQ);
	message->theAppTask = appTask;
	message->app_psm = psm;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_UNREGISTER_REQ, message);
}


