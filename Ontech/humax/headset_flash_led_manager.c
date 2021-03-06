/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009

FILE NAME
    headset_flash_led_manager.c
    
*/

/*!

@file	headset_flash_led_manager.h
@brief converts the user event and state handling into led patterns to be used
	for LED indications.
	
	LEDs can be easily added to specific user events and states without the ned
	for configuring PSKEYS.
	
	To add a new LED pattern for a given event or state
		create the pattern in the .led file
		the led pattern will then be generated automatically using ledpars 
			(see ledparse documentation for details)
		The led pattern will then be available for use in the application.
		(the pattern name (enum) will be in the autogenerated .h file
		
		To add a new state pattern, add the pattern enum name to the 
		headsetStatePatterns table in this file.
		
		On entry to this state, then the desired pattern will display
		
		To add a new event patternm, add the pattern enum name to the 
		headsetEventPatterns table in this file.
		On receiving the event in the headset application, then the 
		desired pattern will play 
		
		
		n.b. state patterns are usually repeating patterns RPT
			 event patterns are usually non repeating patterns
			
    
*/

#ifndef ROM_LEDS


#include "headset_events.h"
#include "headset_led_manager.h"
#include "headset_statemanager.h"
#include "leds.h"

#include <stdio.h>

/*! 
    @brief a led state pattern type
	
	a different led pattern can 
*/
typedef struct
{
	unsigned normal:8;  
	unsigned low_battery:8 ;
	unsigned full_battery:8 ;
	unsigned charger_connected:8 ;
} ledStatePattern_t ;
 

/*! 
    @brief table of LED state patterns 
	 
	To change the pattern for a particular state, change this table
*/
static const ledStatePattern_t ledStatePatterns [ LED_TOTAL_STATES ] = 
{
									/*normal,				low battery						full battery			charger connected*/	
/*powering on*/						{ LEDS_OFF, 			LEDS_OFF,						BLUE_ON_RPT,			RED_ON_RPT						},
/*conndiscoverable*/				{ BLUE_FAST_FLASH_RPT,	BLUE_FAST_FLASH_ALT_RED_RPT,	BLUE_FAST_FLASH_RPT,	BLUE_FAST_FLASH_ALT_RED_RPT		},
/*testmode*/						{ RED_BLUE_ON_RPT, 		RED_BLUE_ON_RPT,				RED_BLUE_ON_RPT,		RED_BLUE_ON_RPT					},
									
/*hfp conn'ble, a2dp conn'ble*/		{ BLUE_SHORT_ON_RPT, 	BLUE_SHORT_ON_ALT_RED_RPT,		BLUE_SHORT_ON_RPT,		BLUE_SHORT_ON_ALT_RED_RPT		},
/*hfp conn'ted, a2dp conn'ble*/		{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp outgoing, a2dp conn'ble*/		{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp incoming, a2dp conn'ble*/		{ BLUE_MEDIUM_FLASH_RPT,BLUE_MEDIUM_FLASH_ALT_RED_RPT,	BLUE_MEDIUM_FLASH_RPT,	BLUE_MEDIUM_FLASH_ALT_RED_RPT	},
/*hfp active, a2dp conn'ble*/		{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp TWC waiting, a2dp conn'ble*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp TWC onhold, a2dp conn'ble*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp multicall, a2dp conn'ble*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},	
									
/*hfp conn'ble, a2dp conn'ted*/		{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp conn'ted, a2dp conn'ted*/		{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp outgoing, a2dp conn'ted*/		{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp incoming, a2dp conn'ted*/		{ BLUE_MEDIUM_FLASH_RPT,BLUE_MEDIUM_FLASH_ALT_RED_RPT,	BLUE_MEDIUM_FLASH_RPT,	BLUE_MEDIUM_FLASH_ALT_RED_RPT	},
/*hfp active, a2dp conn'ted*/		{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp TWC waiting, a2dp conn'ted*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp TWC onhold, a2dp conn'ted*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp multicall, a2dp conn'ted*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
									
/*hfp conn'ble, a2dp streaming*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp conn'ted, a2dp streaming*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp outgoing, a2dp streaming*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp incoming, a2dp streaming*/	{ BLUE_MEDIUM_FLASH_RPT,BLUE_MEDIUM_FLASH_ALT_RED_RPT,	BLUE_MEDIUM_FLASH_RPT,	BLUE_MEDIUM_FLASH_ALT_RED_RPT	},
/*hfp active, a2dp streaming*/		{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp TWC waiting, a2dp streaming*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp TWC onhold, a2dp streaming*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
/*hfp multicall, a2dp streaming*/	{ BLUE_LONG_OFF_RPT, 	BLUE_LONG_OFF_ALT_RED_RPT,		BLUE_LONG_OFF_RPT,		BLUE_LONG_OFF_ALT_RED_RPT 		},
} ;
	

typedef struct 
{
	unsigned event:8 ;
	unsigned pattern:8 ;
						  
} ledEventPattern_t ;

#define NUM_LED_EVENTS (9) 
/*! 
    @brief table of LED event patterns
	
	To adda particular led pattern to a user event, add it here
*/
static const ledEventPattern_t ledEventPatterns [NUM_LED_EVENTS] = 
{
	{ EventPowerOn ,				LEDS_EVENT_POWER_ON				},
	{ EventPowerOff,				LEDS_EVENT_POWER_OFF			},
	{ EventAnswer, 					LEDS_EVENT_CALL					},
	{ EventEndOfCall, 				LEDS_EVENT_CALL					},
	{ EventResetPairedDeviceList,	LEDS_EVENT_RESET_PAIRED_DEVICES	},
	{ EventLinkLoss, 				LEDS_EVENT_LINK_LOSS			},
	{ EventToggleMute, 				LEDS_EVENT_TOGGLE_MUTE			},
	{ EventSLCConnected, 			LEDS_EVENT_CONNECTED			},
	{ EventA2dpConnected, 			LEDS_EVENT_CONNECTED			}
} ;


/*! 
    @brief the leds information stored and used to ccontrol the currently playing led pattern
*/
typedef struct ledsInfoTag
{
	unsigned charger_connected:1 ;
	unsigned low_battery:1 ;
	unsigned full_battery:1 ;
	unsigned hfp_state:4 ;	
	unsigned a2dp_state:2 ;	
	unsigned reserved:7 ;			
}ledsInfo_t ;



static ledsInfo_t ledsInfo ;


/*!
	@brief coverts headset user events into led patterns to be played

	makes use of the auto generated LED patterns.
	
	Patterns can be defined using the .led file and autogenerated by ledparse
	
	Once generated, patterns can be assigned to events in the ledStatePatterns
	table at the top of this file
	
*/
void ledsIndicateEvent ( headsetEvents_t event )  
{
	int i = 0 ;
	
	for (i = 0 ; i < NUM_LED_EVENTS ; i++ )
	{
		if (ledEventPatterns[i].event == event )
		{
			ledsPlay( ledEventPatterns[i].pattern ) ;
		}
	}
	
	/*change the stored state based upon the events recieved - 
		used to update the state indication*/
	switch (event)
	{
		case (EventLowBattery) :
			if (ledsInfo.low_battery)
				return;
			ledsInfo.low_battery = TRUE ;
			ledsInfo.full_battery = FALSE; 					
			break ;
		case (EventFastCharge) :
			if (!ledsInfo.full_battery && !ledsInfo.low_battery)
				return;
			ledsInfo.low_battery = FALSE ;
			ledsInfo.full_battery = FALSE; 							
			break ;
		case (EventTrickleCharge) :
			if (ledsInfo.full_battery)
				return;
			ledsInfo.low_battery = FALSE ;
			ledsInfo.full_battery = TRUE; 							
			break ;
		case (EventChargerConnected):
			if (ledsInfo.charger_connected)
				return;
			ledsInfo.charger_connected = TRUE ;
			break ;
		case (EventChargerDisconnected):
			if (!ledsInfo.charger_connected)
				return;
			ledsInfo.charger_connected = FALSE ;
			break ;
		default:
			return;
	}		
	
		/*the state may need changing after this event has been received*/
	ledsIndicateState( ledsInfo.hfp_state, ledsInfo.a2dp_state );
}


/*!
	@brief coverts headset user states into led patterns to be played

	makes use of the auto generated LED patterns.
	
	Patterns can be defined using the .led file and autogenerated by ledparse
	
	Once generated, patterns can be assigned to states in the ledStatePatterns
	table at the top of this file
	
*/
void ledsIndicateState ( headsetHfpState pState , headsetA2dpState pA2dpState )
{		
	uint16 state = stateManagerGetCombinedLEDState(pState, pA2dpState) ;
		/*store the current state
		used for changing led pattern after event changes*/
	ledsInfo.hfp_state = pState;
	ledsInfo.a2dp_state = pA2dpState;
		
	/*switch event to play based on current state*/	
	if (ledsInfo.charger_connected)
	{
		if (ledsInfo.full_battery)
			ledsPlay( ledStatePatterns[state].full_battery ) ;	
		else
			ledsPlay( ledStatePatterns[state].charger_connected ) ;	
	}	
	else
	if (ledsInfo.low_battery)
	{
		ledsPlay( ledStatePatterns[state].low_battery ) ;
	}
	else
	{
		ledsPlay( ledStatePatterns[state].normal ) ;
	}
}

#else /* ROM_LEDS */
	static const int temp ;
#endif /* ROM_LEDS */

