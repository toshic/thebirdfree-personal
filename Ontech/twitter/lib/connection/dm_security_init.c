/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    dm_security_init.c        

DESCRIPTION
    This file contains the functions to initialise the SM component of 
    the connection library	

NOTES

*/


/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include    "init.h"
#include    "dm_security_auth.h"
#include    "dm_security_init.h"

#include    <vm.h>


/****************************************************************************
NAME
    connectionSmInit

DESCRIPTION
    This Function is called to initialise SM

RETURNS
    The number of devices added to the Bluestack Security Managers' database
*/
uint16 connectionSmInit(connectionReadInfoState* infoState)
{
    uint16 noDevices = 0;
 
    /* Default security set as mode 0 */
    ConnectionSmSetSecurityMode(connectionGetCmTask(), sec_mode0_off, hci_enc_mode_off);
   
    /* Initialise the Trusted Device List.  This involves adding all devices
       in the Trusted Device List to Bluestack Security Managers' database */
    noDevices = connectionAuthInit(infoState->version);

    if(noDevices == 0)
    {
        /* There are no valid records in the Trusted Device Index therefore
           no devices were added to the Bluestack Security Managers' 
           database, initialisation is complete */
        connectionSendInternalInitCfm(connectionInitSm);
    }

    return noDevices;
}
