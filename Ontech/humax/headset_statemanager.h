/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_statemanager.h
@brief    main headset state information
*/
#ifndef _HEADSET_STATE_MANAGER_H
#define _HEADSET_STATE_MANAGER_H

#include "headset_private.h"
#include "headset_states.h"


/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************
NAME	
	stateManagerGetHfpState

DESCRIPTION
	Helper function to get the current hfp headset state.

RETURNS
	The Headset State information.
    
*/
headsetHfpState stateManagerGetHfpState ( void ) ;


/****************************************************************************
NAME	
	stateManagerGetA2dpState

DESCRIPTION
	Helper function to get the current a2dp headset state.

RETURNS
	The Headset State information.
    
*/
headsetA2dpState stateManagerGetA2dpState ( void ) ;


/****************************************************************************
NAME	
	stateManagerGetCombinedLEDState

DESCRIPTION
	Helper function to get the current combined hfp and a2dp headset states.
    Used to find the correct place in the LED state array.

RETURNS
	The resulting headset state value.
    
*/
uint16 stateManagerGetCombinedLEDState ( headsetHfpState pState, headsetA2dpState pA2dpState ) ;


/****************************************************************************
NAME	
	stateManagerEnterHfpConnectableState

DESCRIPTION
	Single point of entry for the HFP connectable state.

*/
void stateManagerEnterHfpConnectableState ( bool req_disc ) ;


/****************************************************************************
NAME	
	stateManagerEnterA2dpConnectableState

DESCRIPTION
	Single point of entry for the A2DP connectable state.

*/
void stateManagerEnterA2dpConnectableState ( bool req_disc ) ;


/****************************************************************************
NAME	
	stateManagerEnterA2dpConnectedState

DESCRIPTION
	Single point of entry for the A2DP connected state.

*/
void stateManagerEnterA2dpConnectedState (void) ;


/****************************************************************************
NAME	
	stateManagerEnterA2dpStreamingState

DESCRIPTION
	Single point of entry for the A2DP streaming state.

*/
void stateManagerEnterA2dpStreamingState(void) ;


/****************************************************************************
NAME	
	stateManagerEnterA2dpPausedState

DESCRIPTION
	Single point of entry for the A2DP paused state.

*/
void stateManagerEnterA2dpPausedState(void) ;


/****************************************************************************
NAME	
	stateManagerEnterConnDiscoverableState

DESCRIPTION
	Single point of entry for the connectable / discoverable state 
    uses timeout if configured.

*/
void stateManagerEnterConnDiscoverableState ( void ) ;


/****************************************************************************
NAME	
	stateManagerEnterHfpConnectedState

DESCRIPTION
	Single point of entry for the HFP connected state - disables discoverable mode.

*/
void stateManagerEnterHfpConnectedState ( void ) ;


/****************************************************************************
NAME	
	stateManagerEnterIncomingCallEstablishState

DESCRIPTION
	Single point of entry for the incoming call establish state.

*/
void stateManagerEnterIncomingCallEstablishState ( void ) ;


/****************************************************************************
NAME	
	stateManagerEnterOutgoingCallEstablishState

DESCRIPTION
	Single point of entry for the outgoing call establish state.

*/
void stateManagerEnterOutgoingCallEstablishState ( void ) ;


/****************************************************************************
NAME	
	stateManagerEnterActiveCallState

DESCRIPTION
	Single point of entry for the active call state.

*/
void stateManagerEnterActiveCallState ( void ) ;


/****************************************************************************
NAME	
	stateManagerEnterPoweringOffState

DESCRIPTION
	Single point of entry for the powering off state - enables power off.

*/
void stateManagerEnterPoweringOffState ( void ) ;


/****************************************************************************
NAME	
	stateManagerPowerOff

DESCRIPTION
	Actually power down the device.

*/
void stateManagerPowerOff ( void )  ;


/****************************************************************************
NAME	
	stateManagerPowerOn

DESCRIPTION
	Power on the device by latching on the power regs.
    
*/
void stateManagerPowerOn ( void ) ;


/****************************************************************************
NAME	
	stateManagerEnterLimboState

DESCRIPTION
    Method to provide a single point of entry to the limbo /poweringOn state.
    
*/
void stateManagerEnterLimboState ( void ) ;


/****************************************************************************
NAME	
	stateManagerUpdateLimboState

DESCRIPTION
    Method to update the limbo state.
    
*/
void stateManagerUpdateLimboState ( void ) ;


/****************************************************************************
NAME	
	stateManagerIsHfpConnected

DESCRIPTION
    Helper method to see if we are connected using the HFP profile or not.
    
RETURNS
	bool
    
*/
bool stateManagerIsHfpConnected ( void ) ;


/****************************************************************************
NAME	
	stateManagerIsA2dpConnected

DESCRIPTION
    Helper method to see if we are connected using the A2DP profile or not.
    
RETURNS
	bool
    
*/
bool stateManagerIsA2dpConnected ( void ) ;


/****************************************************************************
NAME	
	stateManagerIsAvrcpConnected

DESCRIPTION
    Helper method to see if we are connected using the AVRCP profile or not.
    
RETURNS
	bool
    
*/
bool stateManagerIsAvrcpConnected ( void ) ;


/****************************************************************************
NAME	
	stateManagerIsA2dpStreaming

DESCRIPTION
    Helper method to see if we are streaming using the A2DP profile or not.
    
RETURNS
	bool
    
*/
bool stateManagerIsA2dpStreaming(void) ;


/****************************************************************************
NAME	
	stateManagerIsA2dpSignallingActive

DESCRIPTION
    Helper method to see if we are streaming using the A2DP profile or not.
    
RETURNS
	bool
    
*/
bool stateManagerIsA2dpSignallingActive(void) ;


/****************************************************************************
NAME	
	stateManagerEnterTestModeState

DESCRIPTION
    Method to provide a single point of entry to the test mode state.
    
*/
void stateManagerEnterTestModeState ( void ) ;


/****************************************************************************
NAME	
	stateManagerSetAvrcpState

DESCRIPTION
    Updates the AVRCP state.
    
*/
void stateManagerSetAvrcpState ( headsetAvrcpState state );


/****************************************************************************
NAME	
	stateManagerGetAvrcpState

DESCRIPTION
    Return the current AVRCP state.
	
RETURNS
	headsetAvrcpState
    
*/ 
headsetAvrcpState stateManagerGetAvrcpState ( void );

/****************************************************************************
NAME
    stateManagerEnterTWCWaitingState

DESCRIPTION
    Single point of entry for entering three-way calling call waiting state

*/
void stateManagerEnterTWCWaitingState( void );

/*****************************************************************************
NAME
    stateManagerEnterTWCOnHoldState

DESCRIPTION
    Single point of entry for entering three-way calling call on hold state

*/
void stateManagerEnterTWCOnHoldState( void );

/*****************************************************************************
NAME
    stateManagerEnterTWCMulticallState

DESCRIPTION
    Single point of entry for entering three-way calling multicall state

*/
void stateManagerEnterTWCMulticallState( void );

#endif

