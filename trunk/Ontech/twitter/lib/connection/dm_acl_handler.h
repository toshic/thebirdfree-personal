/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_security_handler.h
    
DESCRIPTION

*/

#ifndef	CONNECTION_DM_ACL_HANDLER_H_
#define	CONNECTION_DM_ACL_HANDLER_H_


/****************************************************************************
NAME	
    connectionHandleDmEnAclDetachReq	

DESCRIPTION
    This function is called on receipt of a CL_INTERNAL_DM_EN_ACL_DETACH_REQ message 
	from bluestack_handler and sends a DM_EN_ACL_DETACH_REQ to bluestack
RETURNS
	
*/
void connectionHandleDmEnAclDetachReq(const CL_INTERNAL_DM_EN_ACL_DETACH_REQ_T *ind);

#endif	/* CONNECTION_DM_ACL_HANDLER_H_ */
