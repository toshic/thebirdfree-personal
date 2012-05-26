/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_init.c
@brief   Implementation of headset application initialisation functions. 
*/

#include "headset_auth.h"
#include "headset_buttons.h"
#include "headset_configmanager.h"
#include "headset_config.h"
#include "headset_debug.h"
#include "headset_init.h"
#include "headset_led_manager.h"
#include "headset_statemanager.h"
#include "headset_tones.h"
#include "headset_states.h"
#include "headset_volume.h"


#include <a2dp.h>
#include <avrcp.h>
#include <codec.h>
#include <connection.h>
#include <hfp.h>
#include <stdlib.h>
#include <memory.h>
#include <panic.h>
#include <pio.h>
#include <ps.h>


#include <csr_a2dp_decoder_common_plugin.h>
#include <csr_sbc_encoder_plugin.h>
#include "default_aac_service_record.h"


#ifdef DEBUG_INIT
#define INIT_DEBUG(x) DEBUG(x)
#else
#define INIT_DEBUG(x) 
#endif


static const sep_config_type sbc_sink_sep = { SBC_SINK_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(sbc_caps_sink), sbc_caps_sink };
static const sep_config_type sbc_source_sep = { SBC_SOURCE_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 1, 0, sizeof(sbc_caps_source_analogue), sbc_caps_source_analogue };

/****************************************************************************
  FUNCTIONS
*/

/*************************************************************************
NAME    
    headsetInitComplete
    
DESCRIPTION
    This function is called when all the libraries have been initialised,
    and the config has been loaded. The headset is now ready to be used.    
*/
static void headsetInitComplete( void )
{
    /* Enter the limbo state as we may be ON due to a charger being plugged in */
    stateManagerEnterLimboState();  

    /* Initialise the A2DP state */
    stateManagerEnterA2dpConnectableState ( TRUE );
    
	PROFILE_TIME(("InitComplete"))
	PROFILE_MEMORY(("InitComplete"))
}


/**************************************************************************/
void InitHeadsetData( void ) 
{     
	theHeadset.ProfileLibrariesInitialising = TRUE;
	theHeadset.hfp_list_index = 0xf;
	theHeadset.a2dp_list_index = 0xf;

    AuthResetConfirmationFlags();
}


/**************************************************************************/
void InitUserFeatures (void) 
{
	bool ChargerConnected = FALSE;
	
	/* Allocate the memory required for the configuration data */
	InitConfigurationMemory();
			
    /* Initialise the Tones */
	TonesInit() ;
    
    /* Initialise the Volume */    
    VolumeInit( get_config_id ( PSKEY_CONFIGURATION_ID ) ) ;
    
#ifdef ROM_LEDS	
    /* Initialise the LED Manager */
	LEDManagerInit() ;
#endif	
    
	/* Initialise the Button Manager */
	buttonManagerInit();	
    
    /* Once system Managers are initialised, load up the configuration */
    configManagerInit();
	
	/*use the microphone pre-amp*/
    CodecEnableMicInputGainA (theHeadset.features.UseLowPowerAudioCodecs ? TRUE : FALSE);
    CodecEnableMicInputGainB (theHeadset.features.UseLowPowerAudioCodecs ? TRUE : FALSE);
	
	/* Set inquiry tx power and RSSI inquiry mode */					
    ConnectionWriteInquiryTx(theHeadset.config->rssi.tx_power);
	ConnectionWriteInquiryMode(&theHeadset.task, inquiry_mode_rssi);
       
    /* The headset initialisation is almost complete. */
    headsetInitComplete();
	
#ifdef ROM_LEDS		
	/* Enable LEDs */	
	LedManagerEnableLEDS() ;
#endif	
	
	/* Automatically power on the heasdet as soon as init is complete */
	if ((theHeadset.features.AutoPowerOnAfterInitialisation)&&
        (!ChargerConnected))
	{
		MessageSend( &theHeadset.task , EventPowerOn , 0 ) ;
	}
}


/*****************************************************************************/
void InitCodec(void)
{
    CodecInitCsrInternal(&theHeadset.task) ;
}


/*****************************************************************************/
void InitConnection(void)
{
    ConnectionInit(&theHeadset.task) ;
}


/*****************************************************************************/
void InitHfp(void)
{
    hfp_init_params init;
    
    /* Initialise an HFP profile instance */
	
	switch (theHeadset.HFP_features.HFP_Version)
	{
	case headset_hfp_version_1_5:
	    init.supported_profile = hfp_handsfree_15_profile;
		break;
	case headset_hfp_version_1_0:
	default:
		init.supported_profile = hfp_handsfree_profile;
		break;
	}
    init.supported_features = theHeadset.HFP_features.HFP_Supported_Features;
    init.size_service_record = 0;
    init.service_record = 0;

    HfpInit(&theHeadset.task, &init);

    /* Initialise an HSP profile instance */
    init.supported_profile = hfp_headset_profile;
    init.supported_features = 0;
    init.size_service_record = 0;
    init.service_record = 0;
    
    HfpInit(&theHeadset.task, &init);
}


/*****************************************************************************/
void InitA2dp(void)
{
    sep_data_type seps[NUM_SEPS];
	uint16 number_of_seps = 0;
	
	seps[number_of_seps].in_use = FALSE;
	seps[number_of_seps].sep_config = &sbc_sink_sep;
	number_of_seps++;

	seps[number_of_seps].in_use = FALSE;
	seps[number_of_seps].sep_config = &sbc_source_sep;
	number_of_seps++;
	
	/* Initialise the A2DP library */
    A2dpInit(&theHeadset.task, A2DP_INIT_ROLE_SOURCE | A2DP_INIT_ROLE_SINK, NULL, number_of_seps, seps);
}


/*****************************************************************************/
void InitAvrcp(void)
{
    avrcp_init_params config;
  
	if (theHeadset.features.UseAVRCPforVolume)
	    config.device_type = avrcp_target_and_controller;
	else
		config.device_type = avrcp_controller;
   
    stateManagerSetAvrcpState(avrcpInitialising);
    
	/* Go ahead and Initialise the AVRCP library */
	AvrcpInit(&theHeadset.task, &config);
}


/*****************************************************************************/
Task InitA2dpPlugin(uint8 seid)
{
	if (seid == SBC_SINK_SEID)
		return (TaskData *)&csr_sbc_decoder_plugin;
	
	if (seid == SBC_SOURCE_SEID)
		return (TaskData *)&csr_sbc_encoder_plugin;
	
	/* No plugin found so Panic */
	Panic();
	
	return 0;
}


/*****************************************************************************/
void InitConfigurationMemory(void)
{
	uint16 lSize = 0;
    uint16 *buffer;
    uint16 pos, pos2;
	
	/* Memory slots available to the application are limited, 
		so store multiple configuration items in one slot.
	*/
	
	/*** One memory slot ***/
	/* Allocate memory for Button data and the low power tables */
	pos = sizeof(ButtonsTaskData);
	lSize = pos + (sizeof(lp_power_table) * MAX_POWER_TABLE_ENTRIES) + sizeof(uint16);
	buffer = PanicUnlessMalloc ( lSize );
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
	/* Store pointer to button task memory */
	theHeadset.theButtonTask = (ButtonsTaskData *)&buffer[0];
	/* Store pointer to user power table */
	theHeadset.user_power_table = (power_table *)&buffer[pos];
	/*************************************/

	/*** Two memory slots ***/
	/* Allocate memory for the button events */
	lSize = sizeof( ButtonEvents_t ) * BM_EVENTS_PER_BLOCK;
    theHeadset.theButtonTask->gButtonEvents[0] = (ButtonEvents_t * ) ( PanicUnlessMalloc( lSize ) ) ;
    theHeadset.theButtonTask->gButtonEvents[1]= (ButtonEvents_t * ) ( PanicUnlessMalloc( lSize ) ) ;
	INIT_DEBUG(("INIT: Malloc size [%d] [%d]\n",lSize,lSize));
	/*************************************/
	
#ifdef ROM_LEDS		
	/*** One memory slot ***/
	/* Allocate Memory for LED Patterns and Active LEDs */
	pos = (sizeof(LEDPattern_t) * LM_MAX_NUM_PATTERNS);
	lSize = pos + (sizeof(LEDActivity_t) * HEADSET_NUM_LEDS);
	buffer = PanicUnlessMalloc(lSize);
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
    theHeadset.theLEDTask.gPatterns = (LEDPattern_t*)&buffer[0];    
    theHeadset.theLEDTask.gActiveLEDS = (LEDActivity_t *)&buffer[pos];	
	/*************************************/
	
	/*** One memory slot ***/
	/* Allocate Memory for LED Filter allocation and the capabilities of 2 A2DP codecs (MP3 and FASTSTREAM) */
	pos = sizeof (LEDFilter_t ) * LM_NUM_FILTER_EVENTS;
	lSize = pos + (MAX_A2DP_CODEC_CAPS_A_SIZE * 2);
	buffer = PanicUnlessMalloc ( lSize );
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
	theHeadset.theLEDTask.gEventFilters = (LEDFilter_t *)&buffer[0];
	/*************************************/
#else
	/*** One memory slot ***/
	/* Allocate Memory for the capabilities of 2 A2DP codecs (MP3 and FASTSTREAM) */
	pos = 0;
	lSize = MAX_A2DP_CODEC_CAPS_A_SIZE * 2;	
	buffer = PanicUnlessMalloc ( lSize );
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
	/*************************************/
#endif
#ifdef ROM_LEDS			
	/*** One memory slot ***/
	/* Allocate Memory for LED State Patterns, Last Devices, and General configuration parameters */
	pos = (sizeof(LEDPattern_t *)) * LED_TOTAL_STATES;
	pos2 = sizeof(last_devices_t);
	lSize = pos + pos2 + sizeof(Configuration_t);
	buffer = PanicUnlessMalloc ( lSize );
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
	theHeadset.theLEDTask.gStatePatterns = (LEDPattern_t * * )&buffer[0];
	theHeadset.LastDevices = (last_devices_t *)&buffer[pos];
	theHeadset.config = (Configuration_t*)&buffer[pos + pos2];	
	/*************************************/
	
	/*** One memory slot ***/
	/* Allocate Memory for LED Event Patterns, and SBC codec capabilities */
	pos = (sizeof(LEDPattern_t *)) * EVENTS_MAX_EVENTS;
	lSize = pos + MAX_A2DP_CODEC_CAPS_A_SIZE;
	buffer = PanicUnlessMalloc ( lSize );
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
	theHeadset.theLEDTask.gEventPatterns = (LEDPattern_t * *)&buffer[0];
	theHeadset.sbc_caps = (sep_config_type *)&buffer[pos];
	/*************************************/
#else
	/*** One memory slot ***/
	/* Allocate Memory for Last Devices, and General configuration parameters */
	pos = sizeof(last_devices_t);
	lSize = pos + sizeof(Configuration_t);
	buffer = PanicUnlessMalloc ( lSize );
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
	theHeadset.LastDevices = (last_devices_t *)&buffer[0];
	theHeadset.config = (Configuration_t*)&buffer[pos];	
	/*************************************/
	
	/*** One memory slot ***/
	/* Allocate Memory for SBC codec capabilities */
	lSize = MAX_A2DP_CODEC_CAPS_A_SIZE;
	buffer = PanicUnlessMalloc ( lSize );
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
	theHeadset.sbc_caps = (sep_config_type *)&buffer[0];
	/*************************************/

	pos2 = 0; /* stop warning about unused variable */
#endif
	
	/*** One memory slot ***/
	/* Allocate Memory for Tones, and AAC codec capabilities */
	pos = EVENTS_MAX_EVENTS * sizeof( HeadsetTone_t);
	lSize = pos + MAX_A2DP_CODEC_CAPS_B_SIZE;
	buffer = PanicUnlessMalloc ( lSize );
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
	theHeadset.gEventTones = (HeadsetTone_t *)&buffer[0];
	/*************************************/
}

