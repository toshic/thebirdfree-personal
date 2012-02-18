/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    tcp_init.c        

DESCRIPTION
    This file contains the functions to initialise the TCP component of 
    the connection library	

NOTES

*/

/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include    "init.h"
#include    "tcp_init.h"

#include    <vm.h>


/****************************************************************************
NAME
connectionTcpInit

DESCRIPTION
This Function is called to initialise TCP

RETURNS

*/
void connectionTcpInit(void)
{
     /* TODO B-4408 For now just confirm object has been initialised */
    connectionSendInternalInitCfm(connectionInitTcp);
}
