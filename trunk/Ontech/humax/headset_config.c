/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_config.c
@brief   Implementation of headset configuration functionality. 
*/


#include "headset_config.h"

#include <panic.h>
#include <ps.h>
#include <stdlib.h>
#include <string.h>


/****************************************************************************/
/* Reference the default configuartion definition files */

extern const config_type csr_default_config_1645;




/* Increase this define as new configs are added to default_configs below */
#define LAST_CONFIG_ID	5


/****************************************************************************/
/* Table of default configurations */
const config_type* const default_configs[] = 
{

	&csr_default_config_1645,
			
	0,
	
 	0,

 	0,

 	0,
};

 
/****************************************************************************/

/* Key defintion structure */
typedef struct
{
 	uint16    length;
 	const uint16* data;
}key_type;


/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************
NAME 
 	getkey

DESCRIPTION
 	Access specified key in constant space.
 
*/
static key_type getkey(uint16 config_id, uint16 key_id)
{
 	key_type key = {0, 0};
 
 	switch(key_id)
 	{
        case PSKEY_BATTERY_CONFIG:
      		key.length = default_configs[config_id]->battery_config->length;
      		key.data = (uint16*)default_configs[config_id]->battery_config->value;
      		break;
            
  		case PSKEY_BUTTON_CONFIG:
        	key.length = default_configs[config_id]->button_config->length;
        	key.data = (uint16*)default_configs[config_id]->button_config->value;
        	break;
            
        case PSKEY_BUTTON_PATTERN_CONFIG:
        	key.length = default_configs[config_id]->button_pattern_config->length;
        	key.data = (uint16*)default_configs[config_id]->button_pattern_config->value;
        	break;
  
  		case PSKEY_NO_LED_FILTERS:
   			key.length = default_configs[config_id]->no_led_filters->length;
   			key.data = (uint16*)default_configs[config_id]->no_led_filters->value;
   			break;
   
  		case PSKEY_LED_FILTERS:
   			key.length = default_configs[config_id]->led_filters->length;
   			key.data = (uint16*)default_configs[config_id]->led_filters->value;
   			break;
   
  		case PSKEY_NO_LED_STATES_A:
   			key.length = default_configs[config_id]->no_led_states_a->length;
   			key.data = (uint16*)default_configs[config_id]->no_led_states_a->value;
   			break;
   
  		case PSKEY_LED_STATES_A:
   			key.length = default_configs[config_id]->led_states_a->length;
   			key.data = (uint16*)default_configs[config_id]->led_states_a->value;
   			break;
			
		case PSKEY_NO_LED_STATES_B:
   			key.length = default_configs[config_id]->no_led_states_b->length;
   			key.data = (uint16*)default_configs[config_id]->no_led_states_b->value;
   			break;
   
  		case PSKEY_LED_STATES_B:
   			key.length = default_configs[config_id]->led_states_b->length;
   			key.data = (uint16*)default_configs[config_id]->led_states_b->value;
   			break;
   
  		case PSKEY_NO_LED_EVENTS:
   			key.length = default_configs[config_id]->no_led_events->length;
   			key.data = (uint16*)default_configs[config_id]->no_led_events->value;
   			break;
   
  		case PSKEY_LED_EVENTS:
   			key.length = default_configs[config_id]->led_events->length;
   			key.data = (uint16*)default_configs[config_id]->led_events->value;
   			break;
   
  		case PSKEY_EVENTS_A:
   			key.length = default_configs[config_id]->events_a->length;
   			key.data = (uint16*)default_configs[config_id]->events_a->value;
   			break;
   
  		case PSKEY_EVENTS_B:
   			key.length = default_configs[config_id]->events_b->length;
   			key.data = (uint16*)default_configs[config_id]->events_b->value;
   			break;
   
  		case PSKEY_NO_TONES:
   			key.length = default_configs[config_id]->no_tone_events->length;
   			key.data = (uint16*)default_configs[config_id]->no_tone_events->value;
   			break;
   
  		case PSKEY_TONES:
   			key.length = default_configs[config_id]->tones->length;
   			key.data = (uint16*)default_configs[config_id]->tones->value;
   			break;
            
        case PSKEY_TIMEOUTS:
        	key.length = default_configs[config_id]->timeouts_config->length;
   			key.data = (uint16*)default_configs[config_id]->timeouts_config->value;
   			break; 
			
		case PSKEY_AMP:
        	key.length = default_configs[config_id]->amp_config->length;
   			key.data = (uint16*)default_configs[config_id]->amp_config->value;
   			break; 

		case PSKEY_VOLUME_GAINS:
        	key.length = default_configs[config_id]->vol_gains->length;
   			key.data = (uint16*)default_configs[config_id]->vol_gains->value;
   			break; 
			
		case PSKEY_FEATURES:
			key.length = default_configs[config_id]->features->length;
   			key.data = (uint16*)default_configs[config_id]->features->value;
   			break;
                
        case PSKEY_SSR_PARAMS:
            key.length = default_configs[config_id]->ssr_config->length;
            key.data = (uint16*)default_configs[config_id]->ssr_config->value;
            break;
			
		case PSKEY_HFP_FEATURES:
			key.length = default_configs[config_id]->hfp_features_config->length;
            key.data = (uint16*)default_configs[config_id]->hfp_features_config->value;
            break;			
			
		case PSKEY_AURISTREAM:
   			key.length = default_configs[config_id]->auristream->length;
   			key.data = (uint16*)default_configs[config_id]->auristream->value;
            break;
			
		case PSKEY_RSSI_PAIRING:
   			key.length = default_configs[config_id]->rssi->length;
   			key.data = (uint16*)&default_configs[config_id]->rssi->value;
   			break;

  		default:
   			break;
 	}

 	return key;
}


/*****************************************************************************/
uint16 get_config_id(uint16 key)
{
 	/* Default to CSR standard configuration */
 	uint16 id = 0;
 
 	/* Read the configuration ID.  This identifies the configuration held in
       constant space */
 	if(PsRetrieve(key, &id, sizeof(uint16)))
 	{
  		if(id >= LAST_CONFIG_ID)
  		{
   			id = 0;
  		}
 	}
 
 	return id;
}


/*****************************************************************************/
uint16 ConfigRetrieve(uint16 config_id, uint16 key_id, void* data, uint16 len)
{
 	uint16 ret_len;
 
 	 	/* Read requested key from PS if it exists */
 	ret_len = PsRetrieve(key_id, data, len);
 
 	/* If no key exists then read the parameters from the default configuration
       held in constant space */
 	if(!ret_len)
 	{
  		/* Access the required configuration */
  		if( default_configs[ config_id ] )
  		{
   			/* Read the key */
   			key_type key = getkey( config_id , key_id);
   
   			/* Providing the retrieved length is not zero. */
   			if(key.length == 0)
			{
				/* This will indicate an error. */
				ret_len = 0;
			}
			else
			{
	   			if(key.length == len)
	   			{
	    			/* Copy from constant space */
	    			memmove(data, key.data, len);
	    			ret_len = len;
	   			}
	   			else
	   			{
					if(key.length > len)
					{
						Panic() ;
					}
					else
					{
		   				/* (key.length < len) && (key.length != 0) here since we're comparing unsigned numbers. */

		   				/* We have more space than the size of the key in constant space.
		   				   Just copy the data for the key.
		   				   The length returned will let the caller know about the length mismatch. */
		    			/* Copy from constant space */
		    			memmove(data, key.data, key.length);
		    			ret_len = key.length;
	    			}
	   			}
   			}
  		}
 	}
 
 	return ret_len;
}
