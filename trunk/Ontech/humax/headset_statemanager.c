/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_statemanager.c
@brief    State machine helper functions used for state changes etc - provide single state change points etc for the headset app.
*/

#include "headset_a2dp_connection.h"
#include "headset_a2dp_stream_control.h"
#include "headset_auth.h"
#include "headset_avrcp_event_handler.h"
#include "headset_configmanager.h"
#include "headset_debug.h"
#include "headset_hfp_slc.h"
#include "headset_inquiry.h"
#include "headset_led_manager.h"
#include "headset_scan.h"
#include "headset_statemanager.h"
#include "headset_volume.h"
#include "uart.h"

#include <audio.h>
#include <bdaddr.h>
#include <pio.h>
#include <ps.h>
#include <stdlib.h>

#ifdef TEST_HARNESS
#include "test_bc5_stereo.h"
#endif

#ifdef DEBUG_STATES

#include <panic.h>

#define SM_DEBUG(x) DEBUG(x)
#define SM_ASSERT(c, x) { if (!(c)) { DEBUG(x); Panic();} }

const char * const gHSStateStrings [ 11 ] = {
                               "Limbo",
                               "ConnDisc",
                               "Connectable",
                               "Connected",
                               "Out",
                               "Inc",
                               "ActiveCall",
                               "TESTMODE",
                               "TWCWaiting",
                               "TWCOnHold",
                               "TWCMulti"} ;

const char * const gA2DPStateStrings [HEADSET_NUM_A2DP_STATES] = {
                               "Connectable",
                               "Connected",
                               "Streaming",
                               "Paused"} ;

#else
#define SM_DEBUG(x) 
#define SM_ASSERT(c, x)

#endif


#define SM_LIMBO_TIMEOUT_SECS	(5)


typedef struct
{
    /* The hfp headset state variable - accessed only from below fns */
    headsetHfpState gTheHfpState:4;
    /* The a2dp headset state variable - accessed only from below fns */
    headsetA2dpState gTheA2dpState:3 ;
    /* The avrcp headset state variable - accessed only from below fns */
    headsetAvrcpState gTheAvrcpState:3 ;
} headsetAppStates;

static headsetAppStates appStates;


/****************************************************************************
    LOCAL FUNCTION PROTOTYPES
*/

static void stateManagerSetHfpState ( headsetHfpState pNewState ) ;

static void stateManagerSetA2dpState ( headsetA2dpState pNewState ) ;


/****************************************************************************
    FUNCTIONS
*/

/*****************************************************************************/
headsetHfpState stateManagerGetHfpState ( void )
{
    return appStates.gTheHfpState ;
}


/*****************************************************************************/
headsetA2dpState stateManagerGetA2dpState ( void )
{
    return appStates.gTheA2dpState ;
}


/*****************************************************************************/
uint16 stateManagerGetCombinedLEDState ( headsetHfpState pState, headsetA2dpState pA2dpState )
{
	uint16 combined_state = 0;
	
	/* Not all HFP\A2DP state pairs are required as for example, if the hfpState is headsetConnDiscoverable then
	   it doesn't matter what the A2DP state is (should always be disconnected). Combine the states so they are 
	   more compact and memory space is saved.
	*/
	if ((pState == headsetPoweringOn)||(pState == headsetConnDiscoverable)||(pState == headsetTestMode))
	{
		/* Move the dedicated headset states to the beginning, states 0, 1, 2. */
		if (pState == headsetTestMode)
			combined_state = LED_HEADSET_STATES - 1;
		else
			combined_state = pState;
	}
	else
	{
		/* Move hfpStates down as headset states have been moved to the beginning. */ 
		if (pState > headsetTestMode)
			pState = pState - LED_HEADSET_STATES;
		else
			pState = pState - LED_HEADSET_STATES + 1;
		
		/* Count streaming and paused as the same state for LED purposes */
		if (pA2dpState == headsetA2dpPaused)
			pA2dpState = headsetA2dpStreaming;
		
		combined_state = LED_HEADSET_STATES + pState + (pA2dpState * LED_HFP_STATES);
	}
    return combined_state;
}


/*****************************************************************************/
void stateManagerEnterHfpConnectableState ( bool req_disc )
{   
    headsetHfpState lOldHfpState = stateManagerGetHfpState() ;
    
    if ( stateManagerIsHfpConnected() && req_disc )
    {       /*then we have an SLC active*/
       hfpSlcDisconnect();
    }
    
    /* Make the headset connectable */
    headsetEnableConnectable();
    
    stateManagerSetHfpState ( headsetHfpConnectable ) ;
    
    /*determine if we have got here after a DiscoverableTimeoutEvent*/
    if ( lOldHfpState == headsetConnDiscoverable )
    {   
        /*disable the discoverable mode*/
        headsetDisableDiscoverable () ;
        MessageCancelAll ( &theHeadset.task , EventPairingFail ) ;        
    }
}


/*****************************************************************************/
void stateManagerEnterA2dpConnectableState ( bool req_disc  )
{   	    
    if ( stateManagerIsA2dpConnected() && req_disc )
    {      
       a2dpDisconnectRequest();
    }
	
	if ( stateManagerIsA2dpStreaming() && stateManagerIsAvrcpConnected() )
	{
	    /* Store the current playing status of the media, so the playing status
		  can now be tracked independently of the A2DP state. This is because 
		  AVRCP commands can be sent from the headset while not in the streaming state.
		*/
		if (stateManagerGetA2dpState() == headsetA2dpStreaming)
			theHeadset.PlayingState = 1;
		else
			theHeadset.PlayingState = 0;
	}
    
    /* Make the headset connectable if it's turned on */
	if (stateManagerGetHfpState() != headsetPoweringOn)
    	headsetEnableConnectable();
	
	stateManagerSetA2dpState(headsetA2dpConnectable);
}

void stateManagerEnterA2dpConnectedState(void)
{
	if ( stateManagerIsA2dpSignallingActive() )
	{
   		stateManagerSetA2dpState(headsetA2dpConnected);
		
		if ( stateManagerIsHfpConnected())
            headsetDisableConnectable() ;
	}
	
	if ( stateManagerIsA2dpStreaming() && stateManagerIsAvrcpConnected() )
	{
		/* Store the current playing status of the media, so the playing status
		  can now be tracked independently of the A2DP state. This is because 
		  AVRCP commands can be sent from the headset while not in the streaming state.
		*/
		if (stateManagerGetA2dpState() == headsetA2dpStreaming)
			theHeadset.PlayingState = 1;
		else
			theHeadset.PlayingState = 0;
	}
}

/*****************************************************************************/
void stateManagerEnterA2dpStreamingState(void)
{
   	stateManagerSetA2dpState(headsetA2dpStreaming);
}

/*****************************************************************************/
void stateManagerEnterA2dpPausedState(void)
{
   	stateManagerSetA2dpState(headsetA2dpPaused);
}


/*****************************************************************************/
void stateManagerEnterConnDiscoverableState ( void )
{
    if ( stateManagerIsHfpConnected() )
    {       
        hfpSlcDisconnect();
    }
	
	if ( stateManagerIsA2dpConnected() )
    {      
       a2dpDisconnectRequest();
    }
	
	/* Don't connect back to last devices if pairing mode is entered */
	theHeadset.slcConnectFromPowerOn = FALSE;
    
    /* Make the headset connectable */
    headsetEnableConnectable();
    
    /* Make the headset discoverable */  
    headsetEnableDiscoverable();    
    
    /* The headset is now in the connectable/discoverable state */
    stateManagerSetHfpState ( headsetConnDiscoverable ) ;
           
}


/*****************************************************************************/
void stateManagerEnterHfpConnectedState ( void )
{
	headsetHfpState hfpState = stateManagerGetHfpState (); 
			
    if (hfpState != headsetHfpConnected )
    {
        headsetDisableDiscoverable() ;
        
        if ( stateManagerIsA2dpSignallingActive() )
            headsetDisableConnectable() ;
		
		if ( (hfpState == headsetActiveCall) && (theHeadset.dsp_process == dsp_process_sco) && theHeadset.features.audio_plugin )
        {
			AudioSetMode (AUDIO_MODE_MUTE_SPEAKER , NULL) ;
		}
		
		/* If we are muted - then un mute after call has ended */
    	if ( (hfpState == headsetActiveCall) && theHeadset.gMuted )
    	{
         	MessageSend(&theHeadset.task , EventMuteOff , 0) ;   
    	}    
        
        switch ( hfpState )
        {
            case headsetActiveCall:
            case headsetOutgoingCallEstablish:
            case headsetIncomingCallEstablish:
                /* We have just ended a call */
			
				streamControlResumeA2dpStreaming(0);
				
                MessageSend ( &theHeadset.task , EventEndOfCall , 0 ) ;
                break;
            default:
                break;                      
        }
        
        MessageCancelAll ( &theHeadset.task , EventPairingFail ) ;
        
        stateManagerSetHfpState ( headsetHfpConnected ) ;
    }
}


/*****************************************************************************/
void stateManagerEnterIncomingCallEstablishState ( void )
{   
    stateManagerSetHfpState ( headsetIncomingCallEstablish ) ;
}


/*****************************************************************************/
void stateManagerEnterOutgoingCallEstablishState ( void )
{
    stateManagerSetHfpState ( headsetOutgoingCallEstablish ) ;
}


/*****************************************************************************/
void stateManagerEnterActiveCallState ( void )   
{
    stateManagerSetHfpState ( headsetActiveCall ) ;
	
	/* Update the SCO audio mode if the SCO is already running */
	if ( (theHeadset.dsp_process == dsp_process_sco) && theHeadset.features.audio_plugin )
	{
		if (theHeadset.gMuted)
			AudioSetMode ( AUDIO_MODE_MUTE_MIC , NULL ) ;
		else
			AudioSetMode( AUDIO_MODE_CONNECTED, NULL);
	}
}


/*****************************************************************************/
void stateManagerEnterPoweringOffState ( void )
{
	if ( stateManagerIsA2dpConnected() )
    {
		/* A2DP is connected, so disconnect it now */
      	a2dpDisconnectRequest();
    }
	else 
	{
		/* See if a standalone AVRCP channel needs disconnecting */
		avrcpDisconnectReq();
	}
	
	if ( stateManagerIsHfpConnected() )
    {       
		/* HFP is connected, so disconnect it now */
        hfpSlcDisconnect();
    }
	
	headsetDisableDiscoverable() ;    
    headsetDisableConnectable() ;
    
    /* Now just waiting for switch off */
    stateManagerEnterLimboState() ;
}


/*****************************************************************************/
void stateManagerPowerOff ( void ) 
{
    /* Update state in case we are debugging */
    stateManagerSetHfpState ( headsetPoweringOn) ;
}


/*****************************************************************************/
void stateManagerPowerOn ( void ) 
{
    /* Cancel the event message if there was one so it doesn't power off */
    MessageCancelAll ( &theHeadset.task , APP_LIMBO_TIMEOUT ) ;
    
    /* Reset sec mode config - always turn off debug keys on power on */
    ConnectionSmSecModeConfig(&theHeadset.task,
                             cl_sm_wae_acl_owner_none,
                             FALSE,
                             TRUE);
	
	theHeadset.slcConnectFromPowerOn = TRUE;
					
	stateManagerEnterHfpConnectableState ( TRUE );
    UartPrintf("\r\nReady\r\n");
}


/*****************************************************************************/
void stateManagerEnterLimboState ( void )
{
    /* Set a timeout so that we will turn off eventually anyway */
    MessageSendLater ( &theHeadset.task , APP_LIMBO_TIMEOUT , 0 , D_SEC(5) ) ;
	
	/* Cancel inquiry if in progress */
    inquiryStop();

    stateManagerSetHfpState ( headsetPoweringOn) ;
	
	/* Reset A2DP states */
	theHeadset.connect_a2dp_when_no_call = FALSE;
	theHeadset.LinkLossAttemptA2dp = 0;
	
	/* Reset HFP states */
	theHeadset.slcConnecting = FALSE;
	theHeadset.slcConnectFromPowerOn = FALSE;	
	theHeadset.LinkLossAttemptHfp = 0;
}


/*****************************************************************************/
void stateManagerUpdateLimboState ( void ) 
{
     /* We are entering here as a result of a power off */
    stateManagerPowerOff() ;
}


/*****************************************************************************/
bool stateManagerIsHfpConnected ( void )
{
    bool lIsConnected = FALSE ;
    
    switch (stateManagerGetHfpState() )
    {
        case headsetPoweringOn:
        case headsetHfpConnectable:
        case headsetConnDiscoverable:  
        case headsetTestMode:
            lIsConnected = FALSE ;    
            break ;
        
        default:
            lIsConnected = TRUE ;
            break ;
    }
    return lIsConnected ;
}


/*****************************************************************************/
bool stateManagerIsA2dpConnected ( void )
{
    bool lIsConnected = FALSE ;
    
    switch (stateManagerGetA2dpState() )
    {
        case headsetA2dpConnectable:   
            lIsConnected = FALSE ;    
            break ;
            
        default:
            lIsConnected = TRUE ;
            break ;
    }
    return lIsConnected ;
}


/*****************************************************************************/
bool stateManagerIsAvrcpConnected ( void )
{
	return (stateManagerGetAvrcpState() == avrcpConnected);
}


/****************************************************************************/
bool stateManagerIsA2dpStreaming(void)
{
	bool lRet = FALSE;
	if ((stateManagerGetA2dpState() == headsetA2dpStreaming) || (stateManagerGetA2dpState() == headsetA2dpPaused))
		lRet = TRUE;
		
	return lRet;
}

/****************************************************************************/
bool stateManagerIsA2dpSignallingActive(void)
{
	bool lRet = FALSE;
	
	if (A2dpGetSignallingSink(theHeadset.a2dp))
		lRet = TRUE;
		
	return lRet;
}


/*****************************************************************************/
void stateManagerEnterTestModeState ( void )
{
	/* Cancel the event message if there was one so it doesn't power off */
    MessageCancelAll ( &theHeadset.task , APP_LIMBO_TIMEOUT ) ;
	
    stateManagerSetHfpState ( headsetTestMode ) ;
}


/*****************************************************************************/
void stateManagerSetAvrcpState ( headsetAvrcpState state )
{
	SM_DEBUG(("SM (AVRCP): AvrcpSetState : From %d to %d\n", appStates.gTheAvrcpState, state));	
	appStates.gTheAvrcpState = state;
#ifdef TEST_HARNESS
    vm2host_send_avrcp_state(state);
#endif
}


/*****************************************************************************/
headsetAvrcpState stateManagerGetAvrcpState ( void )
{
	return appStates.gTheAvrcpState;
}

/****************************************************************************
NAME
    stateManagerEnterTWCWaitingState

DESCRIPTION
    Single point of entry for entering three-way calling call waiting state

*/
void stateManagerEnterTWCWaitingState( void )
{
    stateManagerSetHfpState( headsetTWCWaiting); 
}

/*****************************************************************************
NAME
    stateManagerEnterTWCOnHoldState

DESCRIPTION
    Single point of entry for entering three-way calling call on hold state

*/
void stateManagerEnterTWCOnHoldState( void )
{
    stateManagerSetHfpState( headsetTWCOnHold); 
}

/*****************************************************************************
NAME
    stateManagerEnterTWCMulticallState

DESCRIPTION
    Single point of entry for entering three-way calling multicall state

*/
void stateManagerEnterTWCMulticallState( void )
{
    stateManagerSetHfpState( headsetTWCMulticall); 
}

/****************************************************************************
NAME	
	stateManagerSetHfpState

DESCRIPTION
	Helper function to Set the current hfp headset state
    provides a single state change point and passes the information
    on to the managers requiring state based responses. 
    
*/
static void stateManagerSetHfpState ( headsetHfpState pNewState )
{
	SM_DEBUG(("SM (HFP):[%s]->[%s][%d]\n",gHSStateStrings[stateManagerGetHfpState()] , gHSStateStrings[pNewState] , pNewState ));
    
    if ( pNewState < HEADSET_NUM_HFP_STATES )
    {

        if (pNewState != appStates.gTheHfpState )
        {
            /* Inform the LED manager of the current state to be displayed */
            LEDManagerIndicateState ( pNewState , stateManagerGetA2dpState () ) ;
		#ifdef TEST_HARNESS
            vm2host_send_hfp_state(pNewState);
        #endif
        }
        else
        {
            /* We are already indicating this state no need to set */
        }
   
        appStates.gTheHfpState = pNewState ;
   
    }
    else
    {
        SM_DEBUG(("SM (HFP): ? [%s] [%x]\n",gHSStateStrings[ pNewState] , pNewState)) ;
    }
}


/****************************************************************************
NAME	
	stateManagerSetA2dpState

DESCRIPTION
	Helper function to Set the current a2dp headset state
    provides a single state change point and passes the information
    on to the managers requiring state based responses. 
    
*/
static void stateManagerSetA2dpState ( headsetA2dpState pNewState )
{
	SM_ASSERT((pNewState < HEADSET_NUM_A2DP_STATES), ("SM (A2DP): Invalid New State [%d]\n", pNewState));
	
	if (pNewState != appStates.gTheA2dpState )
    {
        /* Inform the LED manager of the current state to be displayed */
        LEDManagerIndicateState ( stateManagerGetHfpState () , pNewState ) ;
	#ifdef TEST_HARNESS
        vm2host_send_a2dp_state(pNewState);
    #endif
    }
    else
    {
        /* We are already indicating this state no need to set */
    }
	
    SM_DEBUG(("SM (A2DP):[%s]->[%s][%d]\n",gA2DPStateStrings[stateManagerGetA2dpState()] , gA2DPStateStrings[pNewState] , pNewState ));
	appStates.gTheA2dpState = pNewState ;
}



