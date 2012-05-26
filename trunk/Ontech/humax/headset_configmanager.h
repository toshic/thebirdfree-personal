/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_configmanager.h
@brief    Configuration manager for the headset - resoponsible for extracting user information out of the PSKEYs and initialising the configurable nature of the headset components.
*/
#ifndef HEADSET_CONFIG_MANAGER_H
#define HEADSET_CONFIG_MANAGER_H

#include "headset_private.h"

/* Persistent store key allocation */
#define PSKEY_BASE  (0)
enum
{
	/* Configuration PS Keys */
 	PSKEY_BATTERY_CONFIG     		= PSKEY_BASE,
 	PSKEY_BUTTON_CONFIG     		= 1,
 	PSKEY_BUTTON_PATTERN_CONFIG     = 2,
	PSKEY_HFP_FEATURES				= 3,
	PSKEY_AURISTREAM				= 4,
 	PSKEY_TIMEOUTS                  = 5,
 	PSKEY_AMP	                    = 6,
 	PSKEY_NO_LED_FILTERS     		= 7,
 	PSKEY_LED_FILTERS      			= 8,
 	PSKEY_NO_LED_STATES_A     		= 9,
 	PSKEY_LED_STATES_A     			= 10,
	PSKEY_NO_LED_STATES_B     		= 11,
 	PSKEY_LED_STATES_B     			= 12,
 	PSKEY_NO_LED_EVENTS     		= 13,
 	PSKEY_LED_EVENTS      			= 14,
 	PSKEY_EVENTS_A      			= 15,
 	PSKEY_EVENTS_B    				= 16,
 	PSKEY_NO_TONES                  = 17,
    PSKEY_TONES                     = 18,
	PSKEY_VOLUME_GAINS				= 19,
	PSKEY_FEATURES					= 20,
	PSKEY_SSR_PARAMS				= 21,
	PSKEY_A2DP_CODEC_CAPS_A	        = 22,
	PSKEY_A2DP_CODEC_CAPS_B	        = 23,
	PSKEY_RSSI_PAIRING		 		= 24,
	PSKEY_CONFIGURATION_ID			= 25,
	PSKEY_CODEC_ENABLED				= 26,
	/* Test mode PS Keys */
	PSKEY_SW_VERSION_NUMBER    		= 27,
	PSKEY_DEFAULT_BDADDR    		= 28,
	/* PS Keys used by the application */
	PSKEY_ATTRIBUTE_BASE			= 29,
	/* PSKeys 29 - 36 used by devices attributes - DO NOT USE */
	PSKEY_A2DP_TONE_VOLUME 			= 38
};

/* Bit field define for CODEC Enabled key */
#define MASK_MP3_ENABLED (1<<0)
#define MASK_AAC_ENABLED (1<<1)

#define MAX_A2DP_CODEC_CAPS_A_SIZE (20)
#define MAX_A2DP_CODEC_CAPS_B_SIZE (22)
#define MAX_POWER_TABLE_ENTRIES (9)

#define MAX_EVENTS ( EVENTS_MAX_EVENTS )


/* Persistent store LED configuration definition */
typedef struct
{
 	unsigned 	state:8;
    unsigned 	a2dp_state:8;
 	unsigned 	on_time:8;
 	unsigned 	off_time:8;
 	unsigned  	repeat_time:8;
 	unsigned  	dim_time:8;
 	unsigned  	timeout:8;
 	unsigned 	number_flashes:4;
 	unsigned 	led_a:4;
 	unsigned 	led_b:4;
    unsigned    overide_disable:1;
 	unsigned 	colour:3;
    unsigned    unused:8;
}led_config_type;

typedef struct
{
 	unsigned 	event:8;
 	unsigned 	speed:8;
     
    unsigned 	active:1;
    unsigned    dummy:1 ;
 	unsigned 	speed_action:2;
 	unsigned  	colour:4;
    unsigned    filter_to_cancel:4 ;
    unsigned    overide_led:4 ;   
    
    unsigned    overide_led_active:1 ;
    unsigned    dummy2:2;
    unsigned    follower_led_active:1 ;
    unsigned    follower_led_delay_50ms:4;
    unsigned    overide_disable:1 ;
    unsigned    dummy3:7 ;
    
    
}led_filter_config_type;

#define MAX_STATES (HEADSET_NUM_HFP_STATES)
#define MAX_LED_EVENTS (20)
#define MAX_LED_STATES (HEADSET_NUM_HFP_STATES * HEADSET_NUM_A2DP_STATES)
#define MAX_LED_FILTERS (LM_NUM_FILTER_EVENTS)

/* LED patterns */
typedef enum
{
 	led_state_pattern,
 	led_event_pattern
}configType;


typedef struct 
{
    unsigned event:8;
    unsigned tone:8;
}tone_config_type ;


typedef struct
{
    uint16 event;
    button_pattern_type pattern[6];
}button_pattern_config_type ;


/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************
NAME 
  	configManagerInit

DESCRIPTION
  	The Configuration Manager is responsible for reading the user configuration
  	from the persistent store are setting up the system.  Each system component
  	is initialised in order.  Where appropriate, each configuration parameter
  	is limit checked and a default assigned if found to be out of range.

*/
void configManagerInit (void);


/***************************************************************************
NAME 
  	configManagerSetupVolumeGains

DESCRIPTION
  	Setup the volume gain levels.
    
*/ 
void configManagerSetupVolumeGains(uint16 pConfigID, uint16 *gains, uint16 size);

/****************************************************************************
NAME 
  	configManagerReset

DESCRIPTION
    Resets the Paired Device List - Reboots if PSKEY says to do so.
    
*/ 
void configManagerReset(void) ;     


#endif   
