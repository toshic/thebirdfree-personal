/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_init.c
@brief   Implementation of headset application initialisation functions. 
*/

#include "headset_auth.h"
#include "headset_configmanager.h"
#include "headset_config.h"
#include "headset_debug.h"
#include "headset_init.h"
#include "headset_led_manager.h"
#include "headset_statemanager.h"
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
	/* Allocate the memory required for the configuration data */
	InitConfigurationMemory();
			
    /* Initialise the Volume */    
    VolumeInit( get_config_id ( PSKEY_CONFIGURATION_ID ) ) ;
    
    /* Once system Managers are initialised, load up the configuration */
    configManagerInit();
	
	/*use the microphone pre-amp*/
    CodecEnableMicInputGainA (theHeadset.features.UseLowPowerAudioCodecs ? TRUE : FALSE);
    CodecEnableMicInputGainB (theHeadset.features.UseLowPowerAudioCodecs ? TRUE : FALSE);
	
	/* Set inquiry tx power and RSSI inquiry mode */					
    ConnectionWriteInquiryTx(theHeadset.config->rssi.tx_power);
	ConnectionWriteInquiryMode(&theHeadset.task, inquiry_mode_eir);
       
    /* The headset initialisation is almost complete. */
    headsetInitComplete();
	
	/* Automatically power on the heasdet as soon as init is complete */
	MessageSend( &theHeadset.task , EventPowerOn , 0 ) ;
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
	
    init.supported_profile = hfp_handsfree_15_profile;

    init.supported_features =  HFP_VOICE_RECOGNITION | HFP_REMOTE_VOL_CONTROL | HFP_THREE_WAY_CALLING;
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
	
	/* Memory slots available to the application are limited, 
		so store multiple configuration items in one slot.
	*/
	
	/*** One memory slot ***/
	/* Allocate memory for Button data and the low power tables */
	lSize = (sizeof(lp_power_table) * MAX_POWER_TABLE_ENTRIES) + sizeof(uint16);
	buffer = PanicUnlessMalloc ( lSize );
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
	/* Store pointer to user power table */
	theHeadset.user_power_table = (power_table *)&buffer[0];
	/*************************************/

	/*** One memory slot ***/
	/* Allocate Memory for Last Devices, and General configuration parameters */
	lSize = sizeof(Configuration_t);
	buffer = PanicUnlessMalloc ( lSize );
	INIT_DEBUG(("INIT: Malloc size [%d]\n",lSize));
	theHeadset.config = (Configuration_t*)&buffer[0];	
	/*************************************/
}

