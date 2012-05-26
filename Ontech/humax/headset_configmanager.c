/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_configmanager.c
@brief    Configuration manager for the headset - resoponsible for extracting user information out of the PSKEYs and initialising the configurable nature of the headset components.
*/


#include "headset_amp.h"
#include "headset_auth.h"
#include "headset_buttonmanager.h"
#include "headset_configmanager.h"
#include "headset_config.h"
#include "headset_debug.h"
#include "headset_events.h"
#include "headset_led_manager.h"
#include "headset_statemanager.h"
#include "headset_private.h"
#include "headset_tones.h"

#include <bdaddr.h>
#include <csrtypes.h>
#include <ps.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <panic.h>

#ifdef DEBUG_CONFIG
#define CONF_DEBUG(x) DEBUG(x)
#else
#define CONF_DEBUG(x) 
#endif

#define HCI_PAGESCAN_INTERVAL_DEFAULT  (0x800)
#define HCI_PAGESCAN_WINDOW_DEFAULT   (0x12)
#define HCI_INQUIRYSCAN_INTERVAL_DEFAULT  (0x800)
#define HCI_INQUIRYSCAN_WINDOW_DEFAULT   (0x12)


#define DEFAULT_VOLUME_MUTE_REMINDER_TIME_SEC 10
#define BUTTON_EVENTS_PER_BLOCK (25)


/****************************************************************************
    LOCAL FUNCTIONS
*/
static void 	configManagerSetupSupportedFeatures	(uint16 pConfigID);
static void 	configManagerButtons				(uint16 pConfigID);
static void  	configManagerButtonDurations        (uint16 pConfigID);
static void     configManagerEventTones             (uint16 pConfigID);
static void     configManagerButtonPatterns         (uint16 pConfigID);
static void     configManagerTimeouts               (uint16 pConfigID);
static void     configManagerAmp	                (uint16 pConfigID);
static void     configManagerFeatures               (uint16 pConfigID);
static void     configManagerSsr                    (uint16 pConfigID);
static void 	configManagerA2DPcodecCaps			(uint16 ps_id, uint16 max_size, sep_config_type **caps_a, sep_config_type **caps_b/*, uint16 optional_bit, uint16 optional_bit_b*/);
static void 	configManagerPowerTables			(void);
static void 	configManagerRetrieveLastDevices	(void);

#ifdef ROM_LEDS
static void  	configManagerLEDS                   (uint16 pConfigID);
#endif /* ROM_LEDS */

/****************************************************************************
  FUNCTIONS
*/

/*****************************************************************************/
void configManagerInit(void)  
{ 
	uint8 codec_enabled;
	uint16 lConfigID  = get_config_id ( PSKEY_CONFIGURATION_ID ) ;
	
		/* Read and configure the headset features */
    configManagerFeatures(lConfigID);
	
  	    /* Read and configure the button durations */
  	configManagerButtonDurations(lConfigID);
    			
  	    /* Read the system event configuration and configure the buttons */
    configManagerButtons(lConfigID);

        /*configures the pattern button events*/
    configManagerButtonPatterns(lConfigID) ;

        /*Read and configure the event tones*/
    configManagerEventTones(lConfigID) ;

#ifdef ROM_LEDS	
  	    /* Read and configure the LEDs */
    configManagerLEDS(lConfigID);
#endif	
    
         /* Read and configure the timeouts */
    configManagerTimeouts(lConfigID);
	
		/* Read and configure the audio amp control */
    configManagerAmp(lConfigID);

    	/* Read and configure the Sniff Subrate parameters */
    configManagerSsr(lConfigID);
	
		/* Read and configure HFP features */
	configManagerSetupSupportedFeatures(lConfigID);
	
		/* Read and configure A2DP codec caps */
	configManagerA2DPcodecCaps(PSKEY_A2DP_CODEC_CAPS_A, MAX_A2DP_CODEC_CAPS_A_SIZE + MAX_A2DP_CODEC_CAPS_B_SIZE, &theHeadset.sbc_caps, &theHeadset.optional_caps[AAC_CODEC_BIT]);
	configManagerA2DPcodecCaps(PSKEY_A2DP_CODEC_CAPS_B, MAX_A2DP_CODEC_CAPS_A_SIZE * 2, &theHeadset.optional_caps[MP3_CODEC_BIT], &theHeadset.optional_caps[FASTSTREAM_CODEC_BIT]);
	
		/* Read the optional A2DP codecs that are enabled */
	if (PsRetrieve(PSKEY_CODEC_ENABLED, &codec_enabled, sizeof(uint8)))
	{
		theHeadset.a2dpCodecsEnabled = codec_enabled;
	}

	CONF_DEBUG(("Co: User A2DP caps sbc[%x] aac[%x] mp3[%x] faststream[%x]\n",
				(uint16)theHeadset.sbc_caps,
				(uint16)theHeadset.optional_caps[AAC_CODEC_BIT],
				(uint16)theHeadset.optional_caps[MP3_CODEC_BIT],
				(uint16)theHeadset.optional_caps[FASTSTREAM_CODEC_BIT]));
	
		/* Read and configure HFP and A2DP power tables */
	configManagerPowerTables();
	
		/* Read and configure Auristream params */
	ConfigRetrieve(lConfigID, PSKEY_AURISTREAM, (void*)&theHeadset.config->Auristream, sizeof(auristream_t));
	
		/* Read and configure RSSI params */
	ConfigRetrieve(lConfigID , PSKEY_RSSI_PAIRING, (void*)&theHeadset.config->rssi, sizeof(rssi_pairing_t));
    
	/* Retrieve the last used devices */
	configManagerRetrieveLastDevices();
	
	/* Find out the Paired Device List (PDL) entries */
	theHeadset.PDLEntries = AuthGetPDLEntries();
			   
}


/*****************************************************************************/ 
static void configManagerSetupSupportedFeatures(uint16 pConfigID)
{
	ConfigRetrieve( pConfigID, PSKEY_HFP_FEATURES , &theHeadset.HFP_features , sizeof( HFP_features_type ) ) ;

    {
           /*make sure the mask we have read in is within range*/
        uint16 lAllPktTypes = ( sync_hv1  | sync_hv2  |  sync_hv3 | 
                                sync_ev3  | sync_ev4  |  sync_ev5 | 
                                sync_2ev3 | sync_3ev3 | sync_2ev5 | sync_3ev5 ) ;
                                 
        uint16 lPSPktTypes = theHeadset.HFP_features.supportedSyncPacketTypes ;
                
        theHeadset.HFP_features.supportedSyncPacketTypes =  (lAllPktTypes & lPSPktTypes) ;
		CONF_DEBUG(("Co: HFPsuppFeatures[%x] HFP version[%d] packet types[%x] eSCO_Parameters_Enabled[%x] bandwidth[%lx] max_latency[%x] voice_settings[%x] retx_effort[%x]\n" , 
					theHeadset.HFP_features.HFP_Supported_Features,
					theHeadset.HFP_features.HFP_Version,
					theHeadset.HFP_features.supportedSyncPacketTypes,
					theHeadset.HFP_features.eSCO_Parameters_Enabled,
					theHeadset.HFP_features.bandwidth,
					theHeadset.HFP_features.max_latency,
					theHeadset.HFP_features.voice_settings,
					theHeadset.HFP_features.retx_effort)) ;
    }
}

/*****************************************************************************/ 
void configManagerSetupVolumeGains(uint16 pConfigID, uint16 *gains, uint16 size)
{
	/* ConfigRetrieve cannot fail since we have volume levels in the default config. */
	(void)ConfigRetrieve(pConfigID, PSKEY_VOLUME_GAINS, gains, size);
}

/*****************************************************************************/ 
void configManagerReset(void) 
{
	uint16 i;
	
    CONF_DEBUG(("CO: Reset\n")) ;
        
    /* Reset the Last Used Devices */
	memset(theHeadset.LastDevices, 0, sizeof(last_devices_t));
	
	theHeadset.gHfpVolumeLevel = theHeadset.config->gVolLevels.defaultHfpVolLevel ; 
   	theHeadset.gAvVolumeLevel = theHeadset.config->gVolLevels.defaultA2dpVolLevel ; 
    
    /* Delete the Connection Libs Paired Device List */
    ConnectionSmDeleteAllAuthDevices ( 0 );
	
	/* Remove devices attributes */
	for (i=0; i<MAX_PAIRED_DEVICES; i++)
		(void)PsStore ( PSKEY_ATTRIBUTE_BASE + i , 0 , 0 ) ;
		
	/* Store that the PDL is now empty */
	theHeadset.PDLEntries = 0;
}
 

/****************************************************************************
NAME 
  	configManagerButtons

DESCRIPTION
 	Read the button config from PS Store.
*/
static void configManagerButtons(uint16 pConfigID)
{
	/* Allocate enough memory to hold event configuration */
    event_config_type* configA = (event_config_type*) PanicUnlessMalloc(BM_EVENTS_PER_BLOCK * sizeof(event_config_type));
    event_config_type* configB = (event_config_type*) PanicUnlessMalloc(BM_EVENTS_PER_BLOCK * sizeof(event_config_type));
   
    uint16 n;
    uint8  i = 0;
 
    ConfigRetrieve(pConfigID , PSKEY_EVENTS_A, configA, BM_EVENTS_PER_BLOCK * sizeof(event_config_type)) ;
    ConfigRetrieve(pConfigID , PSKEY_EVENTS_B, configB, BM_EVENTS_PER_BLOCK * sizeof(event_config_type)) ; 
  
        /* Now we have the event configuration, map required events to system events */
    for(n = 0; n < BM_EVENTS_PER_BLOCK; n++)
    { 
        CONF_DEBUG(("Co : AddMap Ev[%x][%x]\n", configA[n].event , configB[n].event )) ;
                       
           /* check to see if a valid pio mask is present, this includes the upper 2 bits of the state
              info as these are being used for bc5 as vreg enable and charger detect */
        if ( configA[n].pio_mask_0_to_15 | configA[n].pio_mask_16_to_31 )
            buttonManagerAddMapping ( &configA[n], i++ ); 
               
        if ( configB[n].pio_mask_0_to_15 | configB[n].pio_mask_16_to_31 )
            buttonManagerAddMapping ( &configB[n], i++ ); 
                                                           
   	}
    
    free(configA) ;
    free(configB) ; 
    
    /* perform an initial pio check to see if any pio changes need processing following the completion
       of the configuration ps key reading */
    BMCheckButtonsAfterReadingConfig();
}   


/****************************************************************************
NAME 
  	configManagerButtonPatterns

DESCRIPTION
  	Read and configure any buttonpattern matches that exist.
    
*/
static void configManagerButtonPatterns(uint16 pConfigID) 
{  
      		/* Allocate enough memory to hold event configuration */
    button_pattern_config_type* config = (button_pattern_config_type*) PanicUnlessMalloc(BM_NUM_BUTTON_MATCH_PATTERNS * sizeof(button_pattern_config_type));
   
    CONF_DEBUG(("Co: No Button Patterns - %d\n", BM_NUM_BUTTON_MATCH_PATTERNS));
   
    /* Now read in event configuration */
    if(ConfigRetrieve(pConfigID,PSKEY_BUTTON_PATTERN_CONFIG, config, BM_NUM_BUTTON_MATCH_PATTERNS * sizeof(button_pattern_config_type)))
    {
        uint16 n;
 
       /* Now we have the event configuration, map required events to system events */
        for(n = 0; n < BM_NUM_BUTTON_MATCH_PATTERNS ; n++)
        {	 
 	      CONF_DEBUG(("Co : AddPattern Ev[%x]\n", config[n].event )) ;
                    
      			   /* Map PIO button event to system events in specified states */
      	    buttonManagerAddPatternMapping ( config[n].event , config[n].pattern ) ;
        }
    }
    else
	    {
	      CONF_DEBUG(("Co: !EvLen\n")) ;
    }
    free (config) ;
}


#ifdef ROM_LEDS
/****************************************************************************
NAME 
  	config

DESCRIPTION
  	Read the LED configuration from persistent store and configure the LEDS.
 
RETURNS
  	TRUE or FALSE
    
*/ 
static bool config(uint16 pConfigID, configType type, uint16 pskey_no, uint16 pskey_config, uint16 max) 
{ 	
  	bool success = FALSE;
  	uint16 no_events = 0;
 
  	/* First read the number of states/events configured */
  	if(ConfigRetrieve(pConfigID, pskey_no, &no_events, sizeof(uint16)))
  	{	  
		CONF_DEBUG(("Co: no_events:%d max:%d\n",no_events,max)) ;
    	/* Providing there are states to configure */
    	if((no_events > 0) && (no_events <= max))
    	{
      		/* Allocate enough memory to hold state/event configuration */
      		led_config_type* config = (led_config_type*) PanicUnlessMalloc(no_events * sizeof(led_config_type));
   
      		/* Now read in configuration */
   			if(ConfigRetrieve(pConfigID, pskey_config, config, no_events * sizeof(led_config_type)))
   			{
     			uint16    n;
     			LEDPattern_t  pattern;                   

     			/* Now we have the configuration, map to system states/events */
     			for(n = 0; n < no_events; n++)
     			{ 
       				pattern.LED_A           = config[n].led_a;
       				pattern.LED_B           = config[n].led_b;
         			pattern.OnTime          = ((config[n].on_time * 10) << theHeadset.features.LedTimeMultiplier);
         			pattern.OffTime         = ((config[n].off_time * 10) << theHeadset.features.LedTimeMultiplier);
           			pattern.RepeatTime      = ((config[n].repeat_time * 50) << theHeadset.features.LedTimeMultiplier);
       				pattern.NumFlashes      = config[n].number_flashes;           		
      				pattern.TimeOut         = config[n].timeout;
      				pattern.DimTime         = config[n].dim_time;
      				
       				pattern.Colour          = config[n].colour;
                    pattern.OverideDisable  = config[n].overide_disable;
                    
     				    
       				switch(type)
       				{
         				case led_state_pattern:
          					LEDManagerAddLEDStatePattern(config[n].state, config[n].a2dp_state , &pattern);
          					break;
         				case led_event_pattern:
          					LEDManagerAddLEDEventPattern(config[n].state, &pattern);
          					break;
       				}       
     			}
     			success = TRUE;
   			}
            else
            {
                CONF_DEBUG(("Co: !LedLen\n")) ;
            }
            /* Free up memory */
   			free(config);
  		}
  	}
  	return success;
}


/****************************************************************************
NAME 
 	config_led_filter

DESCRIPTION
 	Read the LED filter configuration from persistent store and configure the
 	LED filters.
 
RETURNS
 	TRUE or FALSE
    
*/ 
static bool config_filter(uint16 pConfigID, uint16 pskey_no, uint16 pskey_filter, uint16 max)
{
 	bool success = FALSE;
 	uint16 no_filters = 0;
 
  	/* First read the number of filters configured */
  	if(ConfigRetrieve(pConfigID, pskey_no, &no_filters, sizeof(uint16)))
  	{  
    	/* Providing there are states to configure */
    	if((no_filters > 0) && (no_filters <= max))
    	{
      		/* Allocate enough memory to hold filter configuration */
      		led_filter_config_type* config = (led_filter_config_type*) PanicUnlessMalloc(no_filters * sizeof(led_filter_config_type));
   
      		/* Now read in configuration */
   			if(ConfigRetrieve(pConfigID, pskey_filter, config, no_filters * sizeof(led_filter_config_type)))
   			{
     			uint16    n;
     			LEDFilter_t  filter;

     			/* Now we have the configuration, map to system states/events */
     			for(n = 0; n < no_filters; n++)
     			{ 
       				filter.Event                = config[n].event;
       				filter.Speed                = config[n].speed;
       				filter.IsFilterActive       = config[n].active;
       				filter.SpeedAction          = config[n].speed_action;
       				filter.Colour               = config[n].colour;
                    filter.FilterToCancel       = config[n].filter_to_cancel ;
                    filter.OverideLED           = config[n].overide_led ;

                    filter.OverideLEDActive     = config[n].overide_led_active ;
                    filter.FollowerLEDActive    = config[n].follower_led_active ;

                    filter.FollowerLEDDelay     = ((config[n].follower_led_delay_50ms) << theHeadset.features.LedTimeMultiplier) ;
                    filter.OverideDisable       = config[n].overide_disable;

                        /*add the filter*/
      				LEDManagerAddLEDFilter(&filter);
                                			} 
   			}
            else
            {
                CONF_DEBUG(("Co :!FilLen\n")) ;
            }
    		/* Free up memory */
   			free(config);

       		success = TRUE;
    	}
  	} 
  	return success;
}


/****************************************************************************
NAME 
  	configManagerLEDS

DESCRIPTION
  	Read the system LED configuration from persistent store and configure
  	the LEDS. 
    
*/ 
static void configManagerLEDS(uint16 pConfigID)
{ 
  	/* 1. LED state configuration */
  	config(pConfigID, led_state_pattern, PSKEY_NO_LED_STATES_A, PSKEY_LED_STATES_A, 16);
 	config(pConfigID, led_state_pattern, PSKEY_NO_LED_STATES_B, PSKEY_LED_STATES_B, LED_TOTAL_STATES - 16);	
	
  	/* 2. LED event configuration */
  	config(pConfigID, led_event_pattern, PSKEY_NO_LED_EVENTS, PSKEY_LED_EVENTS, MAX_LED_EVENTS);
 
  	/* 3. LED event filter configuration */
  	config_filter(pConfigID, PSKEY_NO_LED_FILTERS, PSKEY_LED_FILTERS, MAX_LED_FILTERS);         
}
#endif /* ROM_LEDS */


/****************************************************************************
NAME 
  	configManagerButtonDurations

DESCRIPTION
  	Read the button configuration from persistent store and configure
  	the button durations.
    
*/ 
static void configManagerButtonDurations(uint16 pConfigID)
{
 	button_config_type  buttons;
  
   	if(ConfigRetrieve(pConfigID, PSKEY_BUTTON_CONFIG, &buttons, sizeof(button_config_type)))
 	{
  		buttonManagerConfigDurations(buttons);
 	}                  
}
    

/****************************************************************************
NAME 
  	configManagerEventTone

DESCRIPTION
  	Configure an event tone only if one is defined.
 
*/ 
static void configManagerEventTones (uint16 pConfigID)
{ 
    uint16 no_tones = 0;
	uint16 ret_len = 0;
	uint32 data;
 
  	/* First read the number of events configured */
  	if(ConfigRetrieve(pConfigID, PSKEY_NO_TONES, &no_tones, sizeof(uint16)))
  	{
        /* Allocate enough memory to hold event configuration */
    	tone_config_type * config = (tone_config_type *) PanicUnlessMalloc(no_tones * sizeof(tone_config_type));
 
     	/* Now read in tones configuration */
    	if(ConfigRetrieve(pConfigID, PSKEY_TONES, config, no_tones * sizeof(tone_config_type)))
    	{
      		uint16 n;
   
       		/* Now we have the event configuration, map required events to system events */
        	for(n = 0; n < no_tones; n++)
        	{
                CONF_DEBUG(("CO: Ev[%x]Tone[%x] \n" , config[n].event, config[n].tone )) ;
                TonesConfigureEvent ((config[n].event), config[n].tone  ) ;
            }   
        }                    
        free ( config ) ;
    }    
	
 	/* Read the mixed A2DP tone volume */
 	ret_len = PsRetrieve(PSKEY_A2DP_TONE_VOLUME, &data, sizeof(uint32));
 
 	/* If no key exists then set to a default value */
 	if(!ret_len)
	{
		data = 0x5fff1fff;
		PsStore(PSKEY_A2DP_TONE_VOLUME, &data, sizeof(uint32));
	}
}


/****************************************************************************
NAME 
    configManagerTimeouts

DESCRIPTION
    Read and configure the timeouts.
 
*/ 
static void configManagerTimeouts (uint16 pConfigID)
{	
    /* Get the timeout values */
	ConfigRetrieve(pConfigID, PSKEY_TIMEOUTS, &theHeadset.Timeouts, sizeof(Timeouts_t) ) ;
	
	    
    CONF_DEBUG(("Co : Pairing Timeout[%d] Mute Remind[%d] Auto Switch out[%d] Disable power off[%d] Supervision HFP[%d] Supervision A2DP[%d] Refresh Encryption timer[%d] Inquiry[%d]\n" 
											, theHeadset.Timeouts.PairModeTimeout_s
											, theHeadset.Timeouts.MuteRemindTime_s
    										, theHeadset.Timeouts.AutoSwitchOffTime_s
											, theHeadset.Timeouts.DisablePowerOffAfterPowerOnTime_s
											, theHeadset.Timeouts.LinkSupervisionTimeoutHfp_s
											, theHeadset.Timeouts.LinkSupervisionTimeoutA2dp_s
											, theHeadset.Timeouts.EncryptionRefreshTimeout_m
											, theHeadset.Timeouts.InquiryTimeout_s)) ;
}


/****************************************************************************
NAME 
    configManagerAmp

DESCRIPTION
    Read and configure the amp control.
 
*/ 
static void configManagerAmp (uint16 pConfigID)
{	
	Amp_t amp;
    /* Get the timeout values */
	ConfigRetrieve(pConfigID, PSKEY_AMP, &amp, sizeof(Amp_t) ) ;
	
	theHeadset.useAmp = amp.useAmp;
	theHeadset.ampAutoOff = amp.ampAutoOff;
	theHeadset.ampPio = amp.ampPio;
	theHeadset.ampOffDelay = amp.ampOffDelay;
	
	CONF_DEBUG(("Co : Amp config: Use Amp[%d] Auto off[%d] Pio[%d] Off delay[%d secs]\n" , theHeadset.useAmp,theHeadset.ampAutoOff,theHeadset.ampPio,theHeadset.ampOffDelay )) ;
	
	/* Configure the amp then turn it on */
	AmpSetOffDelay(theHeadset.ampOffDelay);
	AmpSetPio(theHeadset.ampPio);	
}


/****************************************************************************
NAME 
    configManagerFeatures

DESCRIPTION
    Read and configure the features block.
 
*/ 
static void configManagerFeatures (uint16 pConfigID)
{	
    /* Get the timeout values */
	ConfigRetrieve(pConfigID, PSKEY_FEATURES, &theHeadset.features, sizeof(Features_t) ) ;
	
	CONF_DEBUG(("Co : Features config: Auto AVRCP[%d] MITM[%d] WAE[%d] DebugKeys[%d] ConnectActionOnPowerOn[%d] LinkLossRetries[%d] ExitPairingAction[%d] PlayHfpTonesAtFixedVolume[%d]\n" ,
                                                theHeadset.features.autoSendAvrcp ,                                               
                                                theHeadset.features.forceMitmEnabled,
                                                theHeadset.features.writeAuthEnable,
                                                theHeadset.features.debugKeysEnabled,
												theHeadset.features.ConnectActionOnPowerOn,
												theHeadset.features.LinkLossRetries,
												theHeadset.features.exitPairingModeAction,
												theHeadset.features.PlayHfpTonesAtFixedVolume
												));
	
	CONF_DEBUG(("Co : Features config: MuteToneFixedVolume[%d] QueueEventTones[%d] MuteSpeakerAndMic[%d] UseLowPowerAudioCodecs[%d] QueueLEDEvents[%d] LedTimeMultiplier[%d] UseHFPprofile[%d] UseA2DPprofile[%d] UseAVRCPprofile[%d] mono[%d] UseAVRCPforVolume[%d] UseSCOforAudioTransfer[%d] OverideMute[%d] MuteLocalVolAction[%d] DisableRoleSwitching[%d]\n" ,
                                                theHeadset.features.MuteToneFixedVolume,
												theHeadset.features.QueueEventTones,
												theHeadset.features.MuteSpeakerAndMic,
												theHeadset.features.UseLowPowerAudioCodecs,
												theHeadset.features.QueueLEDEvents,
												theHeadset.features.LedTimeMultiplier,
												theHeadset.features.UseHFPprofile,
												theHeadset.features.UseA2DPprofile,
												theHeadset.features.UseAVRCPprofile,
												theHeadset.features.mono,
												theHeadset.features.UseAVRCPforVolume,
												theHeadset.features.UseSCOforAudioTransfer,
												theHeadset.features.OverideMute,
												theHeadset.features.MuteLocalVolAction,
												theHeadset.features.DisableRoleSwitching
												));
	
	CONF_DEBUG(("Co : Features config: audio_plugin[%d] AutoPowerOnAfterInitialisation[%d] LinkPolicyA2dpSigBeatsSLC[%d] PowerOnConnectToDevices[%d] ManualConnectToDevices[%d] AutoAnswerOnConnect[%d] LNRCancelsVoiceDialIfActive[%d] EndCallWithNoSCOtransfersAudio[%d] UseHWmicBias[%d] SecurePairing[%d] DiscoIfPDLLessThan[%d]\n",
												theHeadset.features.audio_plugin,
												theHeadset.features.AutoPowerOnAfterInitialisation,
												theHeadset.features.LinkPolicyA2dpSigBeatsSLC,
												theHeadset.features.PowerOnConnectToDevices,
												theHeadset.features.ManualConnectToDevices,
												theHeadset.features.AutoAnswerOnConnect,
												theHeadset.features.LNRCancelsVoiceDialIfActive,
												theHeadset.features.EndCallWithNoSCOtransfersAudio,
												theHeadset.features.UseHWmicBias,
												theHeadset.features.SecurePairing,
												theHeadset.features.DiscoIfPDLLessThan
												));
	
	CONF_DEBUG(("Co : Features config: PairIfPDLLessThan[%d]\n",
												theHeadset.features.PairIfPDLLessThan
												));
                                                                                
}

/****************************************************************************
NAME 
    configManagerSsr

DESCRIPTION
    Read and configure the Sniff Subrate parameters.
 
RETURNS
    void
*/ 
static void configManagerSsr(uint16 pConfigID)
{
    subrate_data ssr_params;

    /* get the parameters either from PS or constant space */
    ConfigRetrieve(pConfigID, PSKEY_SSR_PARAMS, &ssr_params, sizeof(subrate_data));
    /* store them in headset for later use */
    theHeadset.config->ssr_data.streaming_params.max_remote_latency = ssr_params.streaming_params.max_remote_latency;
    theHeadset.config->ssr_data.streaming_params.min_remote_timeout = ssr_params.streaming_params.min_remote_timeout;
    theHeadset.config->ssr_data.streaming_params.min_local_timeout = ssr_params.streaming_params.min_local_timeout;
    theHeadset.config->ssr_data.signalling_params.max_remote_latency = ssr_params.signalling_params.max_remote_latency;
    theHeadset.config->ssr_data.signalling_params.min_remote_timeout = ssr_params.signalling_params.min_remote_timeout;
    theHeadset.config->ssr_data.signalling_params.min_local_timeout = ssr_params.signalling_params.min_local_timeout;

    CONF_DEBUG(("Co : SSR config: Str - MRL[%d]MRT[%d]MLT[%d] Sig - MRL[%d]MRT[%d]MLT[%d]\n",
                                theHeadset.config->ssr_data.streaming_params.max_remote_latency,
                                theHeadset.config->ssr_data.streaming_params.min_remote_timeout,
                                theHeadset.config->ssr_data.streaming_params.min_local_timeout,
                                theHeadset.config->ssr_data.signalling_params.max_remote_latency,
                                theHeadset.config->ssr_data.signalling_params.min_remote_timeout,
                                theHeadset.config->ssr_data.signalling_params.min_local_timeout));
}


static void store_a2dp_caps(sep_config_type **to_caps, uint16 *from_caps)
{
	/* point to the memory allocation for this codec */
	uint16 *local_caps = (uint16*)*to_caps;
	/* copy the header information for the codec */
	memmove(local_caps, from_caps, 5);
   /* copy the capabilities for the codec, leaving a gap for the caps pointer */
	memmove(local_caps+6, &from_caps[5], from_caps[4]);
	/* update the application codec pointer */ 
	*to_caps = (sep_config_type *)local_caps;
	(*to_caps)->caps = (uint8 *)&local_caps[6]; 
}


/****************************************************************************
NAME 
    configManagerA2DPcodecCaps

DESCRIPTION
    Read and configure the A2DP codec caps.
 
RETURNS
    void
*/
static void configManagerA2DPcodecCaps(uint16 ps_id, uint16 max_size, sep_config_type **caps_a, sep_config_type **caps_b/*uint16 optional_bit_a, uint16 optional_bit_b*/)
{
	bool caps_set = FALSE;
	uint16 *CodecCaps = PanicUnlessMalloc ( sizeof(uint16) * max_size ) ;
	uint16 size_ps_key = PsRetrieve(ps_id, CodecCaps, max_size);
	
	if (size_ps_key)
	{
		uint16 capsAsize = (CodecCaps[0]>>8) & 0xff;
		uint16 capsBsize = (CodecCaps[0]) & 0xff;
		
		if (size_ps_key == ( (sizeof(uint16)*capsAsize) + 
                           (sizeof(uint16)*capsBsize) +
                            sizeof(uint16) ) )
		{
			if (capsAsize && (capsAsize > 6))
			{
				/* Store the PSKey defined capabilities for the first codec */
				store_a2dp_caps(caps_a, &CodecCaps[1]);
			}
			else
			{
				*caps_a = 0;
			}
			if (capsBsize && (capsBsize > 6))
			{
				/* Store the PSKey defined capabilities for the second codec */
				store_a2dp_caps(caps_b, &CodecCaps[6+CodecCaps[5]]);
			}
			else
			{
				*caps_b = 0;
			}
			caps_set = TRUE;
		}
	}
	
	if (!caps_set)
	{
		*caps_a = 0;
		*caps_b = 0;
	}
	
	free(CodecCaps);
}


/****************************************************************************
NAME 
    configManagerPowerTables

DESCRIPTION
    Read and configure the HFP and A2DP power tables.
 
RETURNS
    void
*/
static void configManagerPowerTables(void)
{
	uint16 size_table = (sizeof(lp_power_table) * MAX_POWER_TABLE_ENTRIES) + sizeof(uint16);
	power_table *PowerTable = (power_table *)PanicUnlessMalloc(size_table);
	uint16 size_ps_key = PsFullRetrieve(0x360, PowerTable, size_table);
	
	CONF_DEBUG(("Co: size_ps_key[%d] slc[%x] sco[%x] sig[%x] stream[%x]\n",size_ps_key,PowerTable->SLCentries,PowerTable->SCOentries,PowerTable->A2DPSigEntries,PowerTable->A2DPStreamEntries));
 	
    if ( size_ps_key && 
         (size_ps_key == ( (sizeof(lp_power_table)*PowerTable->SLCentries) + 
                           (sizeof(lp_power_table)*PowerTable->SCOentries) + 
						   (sizeof(lp_power_table)*PowerTable->A2DPSigEntries) +
						   (sizeof(lp_power_table)*PowerTable->A2DPStreamEntries) +
                            sizeof(uint16) ) )
       )
 	{
		memmove(theHeadset.user_power_table, PowerTable, size_ps_key);
		CONF_DEBUG(("Co: Read user power tables\n" ));
	}
	else
	{
		theHeadset.user_power_table = 0;
		CONF_DEBUG(("Co: Default power tables\n" ));
	}
	
	free(PowerTable);
}


/****************************************************************************
NAME 
    configManagerRetrieveLastDevices

DESCRIPTION
    Retrieve the last used devices information.
 
RETURNS
    void
*/
static void configManagerRetrieveLastDevices(void)
{
	bdaddr addr;
	uint8 lAttributes[ATTRIBUTE_SIZE];
	uint16 i;
	
	memset(theHeadset.LastDevices, 0, sizeof(last_devices_t));
	theHeadset.last_used_seid = 0;
	
	for (i=0; i<MAX_PAIRED_DEVICES; i++)
	{
		if (ConnectionSmGetIndexedAttributeNow(PSKEY_ATTRIBUTE_BASE, i, ATTRIBUTE_SIZE, lAttributes, &addr))
		{
			CONF_DEBUG(("Co: Retrieved attr index[%d] hfp[%d] a2dp[%d] hfp_vol[%d] a2dp_vol[%d] seid[%d] clock mis[%d]\n",
						i, lAttributes[attribute_hfp_hsp_profile], lAttributes[attribute_a2dp_profile],
						lAttributes[attribute_hfp_volume], lAttributes[attribute_a2dp_volume],
						lAttributes[attribute_seid],lAttributes[attribute_clock_mismatch]));
			if (lAttributes[attribute_hfp_hsp_profile] && BdaddrIsZero(&theHeadset.LastDevices->lastHfpConnected))
			{
				theHeadset.LastDevices->lastHfpConnected = addr;
				theHeadset.LastDevices->lastHfpHsp = lAttributes[attribute_hfp_hsp_profile];
			}
			if (lAttributes[attribute_a2dp_profile] && BdaddrIsZero(&theHeadset.LastDevices->lastA2dpConnected))
			{
				theHeadset.LastDevices->lastA2dpConnected = addr;
				if (lAttributes[attribute_seid])
					theHeadset.last_used_seid = lAttributes[attribute_seid];
			}
			if (!lAttributes[attribute_hfp_hsp_profile] && !lAttributes[attribute_a2dp_profile] && BdaddrIsZero(&theHeadset.LastDevices->lastPaired))
			{
				theHeadset.LastDevices->lastPaired = addr;
			}
		}
	}
	CONF_DEBUG(("Co: Retrieved last paired[%x][%lx][%x] last HFP conn[%x][%lx][%x] last a2dp conn[%x][%lx][%x] last seid[%x]\n",
				theHeadset.LastDevices->lastPaired.uap, theHeadset.LastDevices->lastPaired.lap, theHeadset.LastDevices->lastPaired.nap,
				theHeadset.LastDevices->lastHfpConnected.uap, theHeadset.LastDevices->lastHfpConnected.lap, theHeadset.LastDevices->lastHfpConnected.nap,
				theHeadset.LastDevices->lastA2dpConnected.uap, theHeadset.LastDevices->lastA2dpConnected.lap, theHeadset.LastDevices->lastA2dpConnected.nap,
				theHeadset.last_used_seid))
}

