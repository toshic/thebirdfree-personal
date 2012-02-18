/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1
*/

/*!
@file    headset_statemanager.c
@brief    State machine helper functions used for state changes etc - provide single state change points etc for the headset app.
*/

#include "audioAdaptor_led.h"
#include "audioAdaptor_statemanager.h"

#include <audio.h>
#include <bdaddr.h>
#include <pio.h>
#include <ps.h>
#include <stdlib.h>


#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_STATES
static const char * const s_a2dp_states[] =
{
    "A2dpStateUninitialised",
    "A2dpStateDisconnected",
    "A2dpStatePaged",
    "A2dpStateConnected",
    "A2dpStateOpening",
    "A2dpStateOpen",
    "A2dpStateDisconnecting",
    "A2dpStateStarting",
    "A2dpStateStreaming",
    "A2dpStateSuspending",
    "A2dpStateClosing"
};
static const char * const s_avrcp_states[] =
{
    "AvrcpStateUninitialised",
    "AvrcpStateDisconnected",
    "AvrcpStatePaging",
    "AvrcpStatePaged",
    "AvrcpStateConnected",
    "AvrcpStateDisconnecting"
};
static const char * const s_aghfp_states[] =
{
    "AghfpStateUninitialised",
    "AghfpStateDisconnected",
    "AghfpStatePaging",
    "AghfpStatePaged",
    "AghfpStateConnected",
    "A2dpStateDisconnecting",
    "AghfpStateAudioOpening",
    "AghfpStateAudioStreaming",
    "AghfpStateAudioClosing",
    "AghfpStateCallSetup",
    "AghfpStateCallActive",
    "AghfpStateCallShutdown"
};
static const char * const s_app_states[] =
{
    "AppStateUninitialised",
    "AppStateInitialising",
    "AppStateIdle",
    "AppStateInquiring",
    "AppStateSearching",
    "AppStateConnecting",
	"AppStateConnected",
    "AppStateStreaming",
    "AppStateInCall",
    "AppStateEnteringDfu",
    "AppStateLowBattery",
    "AppStatePoweredOff"
};
#endif /* ENABLE_DEBUG */


/****************************************************************************
  LOCAL FUNCTIONS
*/
      
/****************************************************************************
NAME
    setA2dpState

DESCRIPTION
    Sets a new A2DP state for the specified device instance.
    
*/
void setA2dpState(devInstanceTaskData *theInst, mvdA2dpState state)
{
    DEBUG_STATES(("A2DP STATE old:[%s] new:[%s] inst:[0x%x]\n", s_a2dp_states[theInst->a2dp_state], s_a2dp_states[state],(uint16)theInst));
    theInst->a2dp_state = state;

	if(state ==	A2dpStateDisconnected)
		ledSetProfile(LedTypeA2DP,FALSE);
	else if(state == A2dpStateConnected || state == A2dpStateOpen)
		ledSetProfile(LedTypeA2DP,TRUE);
}


/****************************************************************************
NAME
    setAvrcpState

DESCRIPTION
    Sets a new AVRCP state for the specified device instance.
    
*/
void setAvrcpState(devInstanceTaskData *theInst, mvdAvrcpState state)
{
    DEBUG_STATES(("AVRCP STATE old:[%s] new:[%s] inst:[0x%x]\n", s_avrcp_states[theInst->avrcp_state], s_avrcp_states[state],(uint16)theInst));
    theInst->avrcp_state = state;
}


/****************************************************************************
NAME
    setAghfpState

DESCRIPTION
    Sets a new AGHFP state for the specified device instance.
    
*/
void setAghfpState(devInstanceTaskData *theInst, mvdAghfpState state)
{
    DEBUG_STATES(("AGHFP STATE old:[%s] new:[%s] inst:[0x%x]\n", s_aghfp_states[theInst->aghfp_state], s_aghfp_states[state],(uint16)theInst));
    theInst->aghfp_state = state;

	if(state == AghfpStateDisconnected)
		ledSetProfile(LedTypeHFP,FALSE);
	else if(state == AghfpStateConnected)
		ledSetProfile(LedTypeHFP,TRUE);
}


/****************************************************************************
NAME
    setAppState

DESCRIPTION
    Sets a new APP state.
    
*/
void setAppState (mvdAppState state)
{
    DEBUG_STATES(("APP STATE old:[%s] new:[%s]\n", s_app_states[the_app->app_state], s_app_states[state]));
    /* Change the LED flash corresponding to the new state      */
    /* Low battery and battcharging state just change LED flash */
    if(state < AppStateLowBattery)
        the_app->app_state = state;
    
    ledPlayPattern(state);
}


/****************************************************************************
NAME
    getAghfpState

DESCRIPTION
    Returns the current AGHFP state for the specified device instance.
    
*/
mvdAghfpState getAghfpState(devInstanceTaskData *theInst)
{
    return theInst->aghfp_state;
}


/****************************************************************************
NAME
    getA2dpState

DESCRIPTION
    Returns the current A2DP state for the specified device instance.
    
*/
mvdA2dpState getA2dpState(devInstanceTaskData *theInst)
{
    return theInst->a2dp_state;
}


/****************************************************************************
NAME
    getAvrcpState

DESCRIPTION
    Returns the current AVRCP state for the specified device instance.
    
*/
mvdAvrcpState getAvrcpState(devInstanceTaskData *theInst)
{
    return theInst->avrcp_state;
}


