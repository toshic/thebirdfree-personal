/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_hfp_slc.h
@brief    Handles HFP SLC
*/
#ifndef HEADSET_HFP_SLC_H
#define HEADSET_HFP_SLC_H


#include "headset_private.h"



/****************************************************************************
  FUNCTIONS
*/


/****************************************************************************
NAME    
    hfpSlcIsConnecting
    
DESCRIPTION
    Method to tell if the headset is currently connecting or not.

RETURNS
    bool
*/
bool hfpSlcIsConnecting (void);


/****************************************************************************
NAME    
    hfpSlcConnectSuccess
    
DESCRIPTION
    Indicate that the HFP/HSP profile has been connected. 
    
*/
void hfpSlcConnectSuccess (HFP * pProfile, Sink sink);


/****************************************************************************
NAME    
    hfpSlcConnectFail
    
DESCRIPTION
    SLC failed to connect. If this function returns FALSE then another connection attempt should be made.

*/
bool hfpSlcConnectFail( void );
        

/****************************************************************************
NAME    
    hfpSlcLastConnectRequest
    
DESCRIPTION
    Request to create a connection to a remote AG.

*/
bool hfpSlcLastConnectRequest( hfp_profile pProfile );


/****************************************************************************
NAME    
    hfpSlcConnectBdaddrRequest
    
DESCRIPTION
    Request to create a connection to a remote AG with supplied address.

*/
bool hfpSlcConnectBdaddrRequest( hfp_profile pProfile, bdaddr * pAddr );


/****************************************************************************
NAME    
    hfpSlcAttemptConnect
    
DESCRIPTION
    Attempt connection to a remote AG.

*/
void hfpSlcAttemptConnect( hfp_profile pProfile , bdaddr * pAddr );


/****************************************************************************
NAME    
    hfpSlcDisconnect
    
DESCRIPTION
    Disconnect the SLC associated with this profile instance.

*/
void hfpSlcDisconnect(void);


/****************************************************************************
NAME    
    hfpSlcGetLastUsedAG
    
DESCRIPTION
    Retrieve the bdaddr of the last used AG (paired or connected). 
	
RETURNS
	If the return value is TRUE then the function will have returned a valid bdaddr. 
	If the return value is FALSE then there is no last used AG.
*/
bool hfpSlcGetLastUsedAG(bdaddr *addr, uint16 *profile);
		

/****************************************************************************
NAME    
    hfpSlcGetLastConnectedAG
    
DESCRIPTION
    Retrieve the bdaddr of the last connected AG. 
	
RETURNS
	If the return value is TRUE then the function will have returned a valid bdaddr. 
	If the return value is FALSE then there is no last connected AG.
*/
bool hfpSlcGetLastConnectedAG(bdaddr *addr);


/****************************************************************************
NAME    
    hfpSlcGetListNextAG
    
DESCRIPTION
    Retrieve the next AG in the device list. 
*/
bool hfpSlcGetListNextAG(uint16 *current_index, bdaddr *addr, uint16 *profile);


/****************************************************************************
NAME    
    hfpSlcListConnection
    
DESCRIPTION
    Continue connecting to HFP devices from the device list. 
*/
bool hfpSlcListConnection(void);


#endif
