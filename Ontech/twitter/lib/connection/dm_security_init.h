/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

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
