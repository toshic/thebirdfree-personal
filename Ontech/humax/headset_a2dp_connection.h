/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_a2dp_connection.h
@brief    Handles a2dp connection.
*/
#ifndef HEADSET_A2DP_CONNECTION_H
#define HEADSET_A2DP_CONNECTION_H

#include <message.h>
#include "headset_private.h"


/****************************************************************************
  FUNCTIONS
*/


/*************************************************************************
NAME    
    a2dpGetLastUsedSource
    
DESCRIPTION
    Retrieve bdaddr and sep of last used A2DP source. 
	Return value of FALSE indicates no such device.
*/
bool a2dpGetLastUsedSource(bdaddr *addr, uint8 *seid);


/*************************************************************************
NAME    
     a2dpEstablishConnection
    
DESCRIPTION
     Establish A2DP connection to last used devices.
*/
bool a2dpEstablishConnection(bool a2dp_ag_connect_signalling_only, bool manual_connect);


/*************************************************************************
NAME    
     a2dpConnectRequest
    
DESCRIPTION
     Connect to the last AV source the headset was connected to.
*/
bool a2dpConnectRequest(bool connect_media, bool manual_connect);


/*************************************************************************
NAME    
     a2dpConnectBdaddrRequest
    
DESCRIPTION
     Connect to the last AV source with the requested address.
*/
bool a2dpConnectBdaddrRequest(bdaddr *pAddr);


/*************************************************************************
NAME    
     a2dpDisconnectRequest
    
DESCRIPTION
     Disconnect from the AV source.
*/
void a2dpDisconnectRequest(void);


/*************************************************************************
NAME    
     a2dpIsConnecting
    
DESCRIPTION
     Is the headset currently connecting A2DP.
*/
bool a2dpIsConnecting(void);
		

/****************************************************************************
NAME    
    a2dpGetListNextA2dpSource
    
DESCRIPTION
    Retrieve the next A2DP source in the device list. 
*/
bool a2dpGetListNextA2dpSource(uint16 *current_index, bdaddr *addr, uint8 *seid);


/****************************************************************************
NAME    
    a2dpListConnection
    
DESCRIPTION
    Connect to A2DP source in the device list. 
*/
bool a2dpListConnection(void);


/****************************************************************************
NAME    
    a2dpSwitchSource
    
DESCRIPTION
    Switches to the next A2DP source in the device list. 
*/
void a2dpSwitchSource(void);


#endif
