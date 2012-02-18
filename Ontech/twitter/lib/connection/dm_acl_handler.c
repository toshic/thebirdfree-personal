/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_security_handler.c        

DESCRIPTION
    This files contains the implementation of the Security Entity API for
    the Connection Library.  The Application Task and/or Profile Libraries
    make calls to these functions to manage all Bluetooth security related
    functionality.  In order to correctly manage the Connection Library 
    state machine calls to these API's result in a private message being 
    posted to the main Connection Library task handler.  Depending upon the
    current state of the Connection Library these messages are handled
    appropriately.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "dm_acl_handler.h"

#include <bdaddr.h>
#include <vm.h>


/****************************************************************************
NAME	
    connectionHandleDmEnAclDetachReq	

DESCRIPTION
    This function is called on receipt of a CL_INTERNAL_DM_EN_ACL_DETACH_REQ message 
	from bluestack_handler and sends a DM_EN_ACL_DETACH_REQ to bluestack
RETURNS
	
*/
void connectionHandleDmEnAclDetachReq(const CL_INTERNAL_DM_EN_ACL_DETACH_REQ_T *req)
{	
	/* Send DM_EN_ACL_DETACH_REQ message to bluestack */
	MAKE_PRIM_T(DM_EN_ACL_DETACH_REQ);
	connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr); 
	prim->reason = (hci_reason_t)req->reason;
	VmSendDmPrim(prim);
}
