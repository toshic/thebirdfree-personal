/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    ConnectionSmEncryptionKeyRefresh.c        

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
void ConnectionSmEncryptionKeyRefresh(const bdaddr* bd_addr)
{
#ifdef CONNECTION_DEBUG_LIB
    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr)); 
    }
#endif
	
	{
		MAKE_CL_MESSAGE(CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ);
		message->bd_addr = *bd_addr;
		MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ, message); 
	}
}


