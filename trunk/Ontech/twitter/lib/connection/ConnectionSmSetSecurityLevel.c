/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    ConnectionSmSetSecurityLevel.c        

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


/*****************************************************************************/
void ConnectionSmSetSecurityLevel(dm_protocol_id protocol_id, uint32 channel, dm_ssp_security_level ssp_sec_level, bool outgoing_ok, bool authorised, bool disable_legacy)
{
#ifdef CONNECTION_DEBUG_LIB
	if (ssp_sec_level >= secl_level_unknown)
	{
		CL_DEBUG(("Out of range security level 0x%x\n", ssp_sec_level));
	}
#endif
	{
    MAKE_CL_MESSAGE(CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ);
	message->protocol_id = protocol_id;
	message->channel = channel;
	message->psm = 0;
	message->ssp_sec_level = ssp_sec_level;
	message->outgoing_ok = outgoing_ok;
	message->authorised = authorised;
	message->disable_legacy = disable_legacy;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ, message);
	}
}

