/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_security_authorise.c        

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
#include    <vm.h>


/*****************************************************************************/
void ConnectionDmAclDetach(bdaddr* bd_addr, uint8 reason)
{   
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_EN_ACL_DETACH_REQ);
    message->bd_addr = *bd_addr;
	message->reason = reason;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_EN_ACL_DETACH_REQ, message);
}
