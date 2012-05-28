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



#endif
