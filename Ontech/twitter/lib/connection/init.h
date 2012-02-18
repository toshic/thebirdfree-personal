/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    init.h
    
DESCRIPTION

*/

#ifndef	CONNECTION_INIT_H_
#define	CONNECTION_INIT_H_


/* Connection Library components. Used to control initialisation */
typedef enum
{
    connectionInit      = 0x00,
    connectionInitDm    = 0x01,
    connectionInitRfc   = 0x02,
    connectionInitL2cap = 0x04,
    connectionInitUdp   = 0x08,
    connectionInitTcp   = 0x10,
    connectionInitSdp   = 0x20,
	connectionInitVer	= 0x40,
    connectionInitSm    = 0x80
}connectionInitMask;

#define connectionInitComplete  (connectionInitDm | connectionInitRfc | connectionInitL2cap | connectionInitTcp | connectionInitUdp | connectionInitSdp | connectionInitVer | connectionInitSm)

/* Initailisation timeout period */
#define INIT_TIMEOUT    (10000)


/****************************************************************************

NAME	
    connectionHandleInternalInit	

DESCRIPTION
    This function is called to control the initialsation process.  To avoid race
    conditions at initialisation, the process is serialised.

RETURNS
    void
*/
void connectionHandleInternalInit(connectionInitMask mask);


/****************************************************************************

NAME	
    connectionSendInternalInitCfm	

DESCRIPTION
    This function is callled to send a CL_INTERNAL_INIT_CFM message to the 
    Connection Library task

RETURNS
    void
*/
void connectionSendInternalInitCfm(connectionInitMask mask);


/****************************************************************************

NAME	
    connectionSendInitCfm

DESCRIPTION
    This function is called from the main Connection Library task handler to 
    indicate to the Client application the result of the request to initialise
    the Connection Library.  The application task is passed in as the first
    parameter

RETURNS
    void
*/
void connectionSendInitCfm(Task task, connection_lib_status state, cl_dm_bt_version version);


#endif	/* CONNECTION_INIT_H_ */
