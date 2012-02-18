/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_security_attribute.c        

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
void ConnectionSmPutAttribute(uint16 ps_base, const bdaddr* bd_addr, uint16 size_psdata, const uint8* psdata)
{
	connectionAuthPutAttribute(ps_base, bd_addr, size_psdata, psdata);
}


/*****************************************************************************/
void ConnectionSmGetAttribute(uint16 ps_base, const bdaddr* bd_addr, uint16 size_psdata)
{
	{
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_GET_ATTRIBUTE_REQ);
        message->bd_addr = *bd_addr;
        message->ps_base = ps_base;
        message->size_psdata = size_psdata;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_GET_ATTRIBUTE_REQ, message);
    }
}

/*****************************************************************************/
void ConnectionSmGetIndexedAttribute(uint16 ps_base, uint16 index, uint16 size_psdata)
{
	{
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ);
        message->index = index;
        message->ps_base = ps_base;
        message->size_psdata = size_psdata;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ, message);
    }
}

