/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_config.h
@brief    Interface to functions for configing the headset.
*/

#ifndef _HEADSET_CONFIG_H_
#define _HEADSET_CONFIG_H_


#include "headset_configmanager.h"
#include "headset_states.h"


typedef struct
{
 	uint16     length;
 	uint16     value[1];
}config_uint16_type;


typedef struct
{
  	uint16         length;
  	uint16         value[sizeof(battery_config_type)];
}config_battery_type;


typedef struct
{
   	uint16         length;
   	uint16         value[sizeof(button_config_type)];
}config_button_type;

typedef struct
{
    uint16         length ;
    uint16         value[sizeof(button_pattern_config_type) * BM_NUM_BUTTON_MATCH_PATTERNS]; 
}config_button_pattern_type ;


typedef struct
{
    uint16 blank;   
}HFP_1_5_features_type;


typedef struct
{
    uint16 length ;
    uint16      value[sizeof(HFP_1_5_features_type)];
}config_HFP_1_5_type ;


typedef struct
{
 	uint16     length;
 	uint16     value[sizeof(vol_table_t)];
}config_volume_type;


typedef struct
{
 	uint16     length;
 	uint16     value[sizeof(led_filter_config_type) * MAX_LED_FILTERS];
}config_led_filters_type;


typedef struct
{
 	uint16     length;
 	uint16     value[sizeof(led_config_type) * MAX_LED_STATES];
}config_led_states_type;


typedef struct
{
 	uint16     length;
 	uint16     value[sizeof(led_config_type) * MAX_LED_EVENTS];
}config_led_events_type;


typedef struct
{
 	uint16     length;
 	uint16     value[sizeof(event_config_type) * MAX_EVENTS]; 
}config_events_type;


typedef struct
{
 	uint16     length;
 	uint16     value[sizeof(tone_config_type) * MAX_EVENTS]; 
}config_tone_events_type;


typedef struct
{
	uint16     length;
	uint16     value[sizeof(Timeouts_t)] ; 
}config_timeouts ;

typedef struct
{
	uint16     length;
	uint16     value[sizeof(Amp_t)] ; 
}config_amp ;

typedef struct
{
	uint16     length;
	uint16     value[sizeof(Features_t)] ; 
}config_features ;

typedef struct
{
        uint16     length;
        uint16     value[sizeof(subrate_data)];
}config_ssr_params_type;

typedef struct
{
        uint16     length;
        uint16     value[sizeof(HFP_features_type)];
}config_hfp_features_params_type;

typedef struct
{
	uint16		length;
	uint16		value[sizeof(auristream_t)];
} config_auristream_type;

typedef struct
{
    uint16         length ;
    uint16         value[sizeof(rssi_pairing_t)]; 
} config_rssi_type ;


typedef struct
{

    const config_battery_type*  	battery_config;     	    /* Battery configuration */
 	const config_button_type*  		button_config;     		    /* Button configuration */
 	const config_button_pattern_type*   button_pattern_config;  /* Button Sequence Patterns*/
	const config_hfp_features_params_type*   hfp_features_config;/* HFP features */
	const config_auristream_type*  	auristream;			        /* Auristream parameters */
    const config_timeouts*  		timeouts_config;  		    /* Timeouts */
	const config_amp* 				amp_config;   		    	/* Amp configuration */
 	const config_uint16_type*  		no_led_filters;         	/* Number of LED filters */
 	const config_led_filters_type* 	led_filters;     	    	/* LED filter configuration */
 	const config_uint16_type*  		no_led_states_a;   	    	/* Number of LED states */
 	const config_led_states_type* 	led_states_a;      	    	/* LED state configuration */
	const config_uint16_type*  		no_led_states_b;   	    	/* Number of LED states */
 	const config_led_states_type* 	led_states_b;      	    	/* LED state configuration */
 	const config_uint16_type*  		no_led_events;     		    /* Number of LED events */
 	const config_led_events_type* 	led_events;      		    /* LED event configuration */
 	const config_events_type*  		events_a;     			    /* System event configuration */
 	const config_events_type*  		events_b;       	        /* System event configuration */
 	const config_uint16_type*  		no_tone_events;         	/* Number of tone events */
 	const config_tone_events_type* 	tones;       		    	/* Tone event configuration */
 	const config_volume_type* 		vol_gains;       		    /* Volume Gains */
	const config_features* 			features;       		    /* Features */
    const config_ssr_params_type*   ssr_config;                 /* Sniff Subrate parameters */
	const config_rssi_type*  		rssi;         				/* RSSI Configuration parameters */

}config_type;


/****************************************************************************
  FUNCTIONS
*/


/****************************************************************************
NAME 
 	get_config_id

DESCRIPTION
 	This function is called to read the configuration ID
 
RETURNS
 	Defaults to config 0 if the key doesn't exist
*/
uint16 get_config_id(uint16 key);
		

/****************************************************************************
NAME 
 	ConfigRetrieve

DESCRIPTION
 	This function is called to read a configuration key.  If the key exists
 	in persistent store it is read from there.  If it does not exist then
 	the default is read from constant space.
 
RETURNS
 	0 if no data was read otherwise the length of data.
    
*/
uint16 ConfigRetrieve(uint16 config_id, uint16 key, void* data, uint16 len);


#endif /* _HEADSET_CONFIG_H_ */
