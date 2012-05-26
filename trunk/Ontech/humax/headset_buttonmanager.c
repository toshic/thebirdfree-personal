/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_buttonmanager.c
@brief    Implementation of the button manager.
    
This file provides a configurable wrapper for the button messages and
converts them into the standard system messages which are passed to the
main message handler - main.c
*/
#include "headset_private.h"
#include "headset_buttonmanager.h"
#include "headset_statemanager.h"
#include "headset_buttons.h"
#include "headset_volume.h"
#include "headset_debug.h"

#include <stddef.h>
#include <csrtypes.h>
#include <panic.h>
#include <stdlib.h>


#include "headset_events.h"

#ifdef DEBUG_BUT_MAN
    #define BM_DEBUG(x) DEBUG(x)
    
    const char * const gDebugTimeStrings[13] = {"Inv" , 
    											"Short", 
                                                "Long" ,
                                                "VLong" , 
                                                "Double" ,
                                                "Rpt" , 
                                                "LToH" , 
                                                "HToL" , 
                                                "ShSingle",
                                                "Long Release",
                                                "VLong Release",
                                                "V V Long" ,
                                                "VV Long Release"} ;
                 
#else
    #define BM_DEBUG(x) 
#endif

/*
 LOCAL FUNCTION PROTOTYPES
 */
static void BMCheckForButtonMatch ( uint32 pButtonMask , ButtonsTime_t  pDuration  )  ;

static void BMCheckForButtonPatternMatch ( uint32 pButtonMask ) ;

/****************************************************************************
VARIABLES  
*/

#define BM_NUM_BLOCKS (2)
#define BM_NUM_CONFIGURABLE_EVENTS (BM_EVENTS_PER_BLOCK * BM_NUM_BLOCKS)

#define BUTTON_PIO_DEBOUNCE_NUM_CHECKS  (4)
#define BUTTON_PIO_DEBOUNCE_TIME_MS     (15)

/****************************************************************************
  FUNCTIONS
*/


/****************************************************************************
NAME 
 	buttonManagerInit
    
DESCRIPTION
 	Initialises the Button Module parameters
RETURNS
 	void
*/  
void buttonManagerInit ( void )
{   
	uint16 lIndex = 0 ;
    uint16 lButtonIndex = 0; 
	
	ButtonsTaskData *pButtonsTask = theHeadset.theButtonTask;
 
    /* initialise the edge and level detect mask values */
    pButtonsTask->gPerformEdgeCheck = 0;
    pButtonsTask->gPerformLevelCheck = 0;    
  
    for (lIndex = 0 ; lIndex < BM_NUM_BUTTON_MATCH_PATTERNS ; lIndex++ )
    {
		/*set the progress to the beginning*/
        pButtonsTask->gButtonMatchProgress[lIndex] = 0 ;
        
            /*set the button match patterns to known vals*/
        pButtonsTask->gButtonPatterns[lIndex].NumberOfMatches = 0 ;
        pButtonsTask->gButtonPatterns[lIndex].EventToSend = 0 ;        
        
        for (lButtonIndex = 0 ; lButtonIndex < BM_NUM_BUTTONS_PER_MATCH_PATTERN ; lButtonIndex++)
        {
            pButtonsTask->gButtonPatterns[lIndex].ButtonToMatch[lButtonIndex] = B_INVALID ;
        }   
    }
	
    ButtonsInit() ; 
}

/****************************************************************************

DESCRIPTION
	Wrapper method for the button Duration Setup
	configures the button durations to the user values

*/   
void buttonManagerConfigDurations ( button_config_type pButtons )
{
	ButtonsTaskData *pButtonsTask = theHeadset.theButtonTask;
	
    pButtonsTask->button_config = pButtons ;
	
	if ((pButtons.debounce_number == 0 ) || ( pButtons.debounce_period_ms == 0 ) )
	{
		/*use defaults*/
		BM_DEBUG(("BM: DEFAULT button debounce\n")) ;
		pButtonsTask->button_config.debounce_number =  BUTTON_PIO_DEBOUNCE_NUM_CHECKS;
		pButtonsTask->button_config.debounce_period_ms = BUTTON_PIO_DEBOUNCE_TIME_MS ;		
	}
	else
	{
		BM_DEBUG(("BM: Debounce[%x][%x]\n" , pButtonsTask->button_config.debounce_number ,pButtonsTask->button_config.debounce_period_ms)) ;
	}
	BM_DEBUG(("BM: Key Test PIOs [%lx]\n",pButtonsTask->button_config.key_test_pios)) ;
}

/****************************************************************************
NAME	
	buttonManagerAddMapping

DESCRIPTION
	Maps a button event to a system event
        
    pButtonMask - 
    mask of the buttons that when pressed will generate an event
    e.g.  0x0001 = button PIO 0
    
          0x0003 = combination of PIO 0  and PIO 1
    pSystemEvent
        The Event to be signalled as define in headset_events.h
    pStateMask
        the states as defined in headset_states that the event will be generated in
    pDuration
        the Duration of the button press as defined in headset_buttons.h
        B_SHORT , B_LONG , B_VLONG, B_DOUBLE
          
RETURNS
 bool to indicate success of button being added to map
    
*/     
void buttonManagerAddMapping ( event_config_type * event_config, 
							   uint8 index ) 
{
    ButtonEvents_t * lButtonEvent ;
	ButtonsTaskData *pButtonsTask = theHeadset.theButtonTask;

    /* obtain next free button position in array */
    if(index < BM_EVENTS_PER_BLOCK)
    {
        lButtonEvent = &pButtonsTask->gButtonEvents[0][index];
    }
    /* second set of button events as first set of events is now full*/
    else
    {
        lButtonEvent = &pButtonsTask->gButtonEvents[1][index - BM_EVENTS_PER_BLOCK];
    }
	
    if ( lButtonEvent )
    {
    
        lButtonEvent->ButtonMask = ((uint32)event_config->pio_mask_16_to_31 << 16) | event_config->pio_mask_0_to_15 ;
        lButtonEvent->Duration   = (ButtonsTime_t)event_config->type ;
        lButtonEvent->Event      = event_config->event ;
        lButtonEvent->HfpStateMask  = event_config->hfp_state_mask ;
		lButtonEvent->A2dpStateMask  = event_config->a2dp_state_mask ;
    
        /* look for edge detect config and add the pio's used to the check for edge detect */
        if((lButtonEvent->Duration == B_LOW_TO_HIGH)||(lButtonEvent->Duration == B_HIGH_TO_LOW))
        {
            /* add pio mask bit to U16 mask store */
            pButtonsTask->gPerformEdgeCheck |= lButtonEvent->ButtonMask;
        }
        /* otherwise must be a level check */
        else
        {
            pButtonsTask->gPerformLevelCheck |= lButtonEvent->ButtonMask;
        }
                        
        /*register the buttons we are interested in with the buttons task*/               
        pButtonsTask->gButtonLevelMask |= lButtonEvent->ButtonMask ; 
    }
    else
    {
        BM_DEBUG(("_!BM1\n")) ;
    }
}


/****************************************************************************
DESCRIPTION
 	add a new button pattern mapping
*/
bool buttonManagerAddPatternMapping ( uint16 pSystemEvent , button_pattern_type * pButtonsToMatch ) 
{
    uint16 lMapIndex = 0 ;
    uint16 lButtonIndex = 0 ;
	
	ButtonsTaskData *pButtonsTask = theHeadset.theButtonTask;

    /*adds a button pattern map*/
    for (lMapIndex = 0 ; lMapIndex < BM_NUM_BUTTON_MATCH_PATTERNS ; lMapIndex++)
    {
        if (pButtonsTask->gButtonPatterns[lMapIndex].EventToSend == B_INVALID )
        {
            pButtonsTask->gButtonPatterns[lMapIndex].EventToSend = pSystemEvent ;
        
            for (lButtonIndex = 0 ; lButtonIndex < BM_NUM_BUTTONS_PER_MATCH_PATTERN ; lButtonIndex++)
            {
                pButtonsTask->gButtonPatterns[lMapIndex].ButtonToMatch[lButtonIndex] = ((uint32)pButtonsToMatch[lButtonIndex].pio_mask_16_to_31 << 16) | pButtonsToMatch[lButtonIndex].pio_mask_0_to_15;
            
                if (pButtonsTask->gButtonPatterns[lMapIndex].ButtonToMatch[lButtonIndex] != 0)
                {
                    pButtonsTask->gButtonPatterns[lMapIndex].NumberOfMatches = lButtonIndex + 1;
                }
            }
            
            BM_DEBUG(("BM: But Pat Added[%d] [%x] ,[%lx][%lx][%lx][%lx][%lx][%lx] [%d]\n" , lMapIndex , pButtonsTask->gButtonPatterns[lMapIndex].EventToSend
                                                                            , pButtonsTask->gButtonPatterns[lMapIndex].ButtonToMatch[0]
                                                                            , pButtonsTask->gButtonPatterns[lMapIndex].ButtonToMatch[1]
                                                                            , pButtonsTask->gButtonPatterns[lMapIndex].ButtonToMatch[2]
                                                                            , pButtonsTask->gButtonPatterns[lMapIndex].ButtonToMatch[3]
                                                                            , pButtonsTask->gButtonPatterns[lMapIndex].ButtonToMatch[4]
                                                                            , pButtonsTask->gButtonPatterns[lMapIndex].ButtonToMatch[5]
                                                                            
                                                                            , pButtonsTask->gButtonPatterns[lMapIndex].NumberOfMatches
                                                                            
                                                                                )) ;
            return TRUE ;
        }    
    }
    
    return FALSE ;
}

/****************************************************************************
NAME	
	BMButtonDetected

DESCRIPTION
	function call for when a button has been detected 
RETURNS
	void
    
*/
void BMButtonDetected ( uint32 pButtonMask , ButtonsTime_t pTime  )
{
    BM_DEBUG(("BM : But [%lx] [%s]\n" ,pButtonMask ,  gDebugTimeStrings[pTime]  )) ;
 
        /*perform the search over both blocks*/
    BMCheckForButtonMatch ( pButtonMask  , pTime ) ;
    
        /*only use regular button presses for the pattern matching to make life simpler*/
    if ( ( pTime == B_SHORT ) || (pTime == B_LONG ) )
    {
        BMCheckForButtonPatternMatch ( pButtonMask ) ;
    }   
}


/****************************************************************************
NAME 
 BMCheckForButtonMatch
    
DESCRIPTION
 function to check a button for a match in the button events map - sends a message
    to a connected task with the corresponding event
    
RETURNS

    void
*/   
static void BMCheckForButtonMatch ( uint32 pButtonMask , ButtonsTime_t  pDuration ) 
{
    uint16 lHfpStateBit = ( 1 << stateManagerGetHfpState () ) ; 
    uint16 lA2dpStateBit = ( 1 << stateManagerGetA2dpState () ) ;  
    uint16 lBlockIndex = 0 ; 
    uint16 lEvIndex = 0 ;

    ButtonsTaskData *pButtonsTask = theHeadset.theButtonTask;

    BM_DEBUG(("BM : BMCheckForButtonMatch [%x][%x][%lx][%x]\n" , lHfpStateBit , lA2dpStateBit , pButtonMask, pDuration)) ;

    /*each block*/
    for (lBlockIndex = 0 ; lBlockIndex < BM_NUM_BLOCKS ; lBlockIndex++)
    {       /*Each Entry*/        
        for (lEvIndex = 0 ; lEvIndex < BM_EVENTS_PER_BLOCK ; lEvIndex ++)
        { 
            ButtonEvents_t * lButtonEvent = &pButtonsTask->gButtonEvents [lBlockIndex] [ lEvIndex ] ;
            /*if the event is valid*/
            if ( lButtonEvent != NULL)
            {            
                if (lButtonEvent->ButtonMask == pButtonMask )
                {                          
                    /*we have a button match*/
                    if ( lButtonEvent->Duration == pDuration )
                    {           
#ifdef SD_SUPPORT
                        /* if SD player enabled OR matched HFP and A2DP state */
                        if ( (theHeadset.sd_player != NULL && theHeadset.sd_player->sd_mode == 1) ||
                             (((lButtonEvent->HfpStateMask) & (lHfpStateBit)) && ((lButtonEvent->A2dpStateMask) & (lA2dpStateBit))) )
#else
                        if ( ((lButtonEvent->HfpStateMask) & (lHfpStateBit)) && ((lButtonEvent->A2dpStateMask) & (lA2dpStateBit)) )
#endif
                        {
                            BM_DEBUG(("BM : State Match [%lx][%x]\n" , pButtonMask , lButtonEvent->Event)) ;

                            /* due to the slow processing of messages when trying to change volume
                               check for volume up and down events and call the volume change functions
                               directly instead of using messages, this gives up to approx 1 second 
                               quicker turn around */
                            if(lButtonEvent->Event == EventVolumeUp)
                            {                              
                                if (!theHeadset.buttons_locked || (stateManagerGetHfpState() == headsetActiveCall))
                                    VolumeUp( ) ;
                                else
                                    BM_DEBUG(("BM : Buttons Locked\n"));
                            }
                            /* also check for volume down presses */
                            else if(lButtonEvent->Event == EventVolumeDown)
                            {                              
                                if (!theHeadset.buttons_locked || (stateManagerGetHfpState() == headsetActiveCall))
                                    VolumeDown() ; 
                                else
                                    BM_DEBUG(("BM : Buttons Locked\n"));
                            }
                            else
                            {
                                /*we have fully matched an event....so tell the main task about it*/
                                MessageSend( &theHeadset.task, lButtonEvent->Event , 0 ) ;
#ifdef SD_SUPPORT
                                return;
#endif
                            }
                        }
                    }
                }
            }
        }
    }
}
  
/****************************************************************************
DESCRIPTION
 	check to see if a button pattern has been matched
*/
static void BMCheckForButtonPatternMatch ( uint32 pButtonMask  ) 
{
    uint16 lIndex = 0 ;
	
	ButtonsTaskData *pButtonsTask = theHeadset.theButtonTask;
    
    BM_DEBUG(("BM: Pat[%lx]\n", pButtonMask )) ;
    
    for (lIndex = 0; lIndex < BM_NUM_BUTTON_MATCH_PATTERNS ; lIndex++ )
    {
        if ( pButtonsTask->gButtonPatterns[lIndex].ButtonToMatch[pButtonsTask->gButtonMatchProgress[lIndex]] == pButtonMask )
        {
                    /*we have matched a button*/
            pButtonsTask->gButtonMatchProgress[lIndex]++ ;
            
            BM_DEBUG(("BM: Pat Prog[%d][%x]\n", lIndex , pButtonsTask->gButtonMatchProgress[lIndex]  )) ;
                    
                
            if (pButtonsTask->gButtonMatchProgress[lIndex] >= pButtonsTask->gButtonPatterns[lIndex].NumberOfMatches)
            {
                        /*we have matched a pattern*/
                BM_DEBUG(("BM: Pat Match[%d] Ev[%x]\n", lIndex ,pButtonsTask->gButtonPatterns[lIndex].EventToSend)) ;
                
                pButtonsTask->gButtonMatchProgress[lIndex] = 0 ;
                
                MessageSend( &theHeadset.task, pButtonsTask->gButtonPatterns[lIndex].EventToSend , 0 ) ;
            }
            
        }       
        else
        {
            pButtonsTask->gButtonMatchProgress[lIndex] = 0 ;
                /*special case = if the last button pressed was the same as the first button*/
            if ( pButtonsTask->gButtonPatterns [ lIndex ].ButtonToMatch[0]== pButtonMask)            
            {
                pButtonsTask->gButtonMatchProgress[lIndex] = 1 ;
            
            }
        }
    }
}


/****************************************************************************
DESCRIPTION
 	perform an initial read of pios following configuration reading as it is possible
    that pio states may have changed whilst the config was being read and now needs
    checking to see if any relevant events need to be generated. 
*/
void BMCheckButtonsAfterReadingConfig( void )
{
	ButtonsCheckForChangeAfterInit();
}
