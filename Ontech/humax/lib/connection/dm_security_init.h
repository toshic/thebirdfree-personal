/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    dm_security_init.h
    
DESCRIPTION

*/

#ifndef	CONNECTION_DM_SECURITY_INIT_H_
#define	CONNECTION_DM_SECURITY_INIT_H_


/****************************************************************************
NAME
    connectionSmInit

DESCRIPTION
    This Function is called to initialise SM

RETURNS
    The number of devices added to the Bluestack Security Managers' database
*/
uint16 connectionSmInit(connectionReadInfoState* infoState);


#endif	/* CONNECTION_DM_SECURITY_INIT_H_ */
