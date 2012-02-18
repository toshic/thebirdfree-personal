/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    ConnectionSmChangeLinkKey.c        

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
void ConnectionSmChangeLinkKey(Sink sink)
{   
    MAKE_CL_MESSAGE(CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ);
    message->sink = sink;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ, message);
}

