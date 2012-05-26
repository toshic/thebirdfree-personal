/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_config_csr_1645.c
@brief    This file contains the default configuration parameters for a CSR Stereo headset.
*/


#include "headset_config.h"




/* PSKEY_USR_0 - Battery config */
static const config_battery_type battery_config =
{
 	sizeof(battery_config_type),
    {
        0x0000,  /*unused*/	
        0xA596,  /*Minimum threshold	*/         /*Shutdown threshold*/	
		0xB4B9,  /*level_1_threshold	*/         /*level_2_threshold*/	
        0x1e00,  /*Monitoring period (secs)*/		/*unused*/
		0x000a	 /*low_batt_tone_period*/
    }    
};


/* PSKEY_USR_1 - Button config */
static const config_button_type button_config =
{
  	sizeof(button_config_type),
    {
      	0x01f4, /* Button double press time */
      	0x03e8, /* Button long press time */
      	0x09c4, /* Button very long press time */
      	0x0320, /* Button repeat time */
      	0x1964, /* Button Very Very Long Duration*/
		0x040f,	/* Debounce (number of reads ; time between reads) */
		0x0000, 0x0000 /* Key Test PIOs */
  	}   
};


/* PSKEY_USR_2 - buttonpatterns */
#define NUM_BUTTON_PATTERNS 2

static const config_button_pattern_type button_pattern_config = 
{
    sizeof(button_pattern_config_type ) * NUM_BUTTON_PATTERNS ,
    
    { 
		/* unused */
        0x0   ,0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0,
		/* unused */
        0x0   ,0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0
    }    
    
};


/* PSKEY_USR_3 - HFP features */
static const config_hfp_features_params_type hfp_features_config =
{
  	7,
  	{
	0x001b,			/*HFP supported features - HFP_NREC_FUNCTION | HFP_VOICE_RECOGNITION | HFP_REMOTE_VOL_CONTROL | HFP_THREE_WAY_CALLING*/
	0x6bf8,			/*HFP_Version:2 supportedSyncPacketTypes:10 eSCO_Parameters_Enabled:1 reserved:3*/ 
	0x0000, 0x1f40,	/* bandwidth */	
	0x000c,			/* max_latency */
	0x0000,			/* voice_settings */
	0x0002			/* retx_effort */
	}
};


/* PSKEY_USR_4 - Auristreams parameters */
static const config_auristream_type auristream_config =
{
    sizeof(auristream_t),
  	{
		0x0000, 0x07d0,	/* AURISTREAM: bw_2bits 						*/
	 	0x0000, 0x0fa0,	/* AURISTREAM: bw_4bits 						*/
		0x0020,			/* AURISTREAM: max_latency						*/			
	 	0x0063, 		/* AURISTREAM: voice_settings					*/
	 	0x00ff,			/* AURISTREAM: retx_effort 						*/
	}
};


/* PSKEY_USR_5 - timeouts */
static const config_timeouts timeouts =
{
  	8,
  	{
  	0x00b4 , /*PairModeTimeout_s*/ 
    0x0005 , /*MuteRemindTime_s*/
    0x0258 , /*AutoSwitchOffTime_s*/
	0x0003 , /*DisablePowerOffAfterPowerOnTime_s*/ 
	0x1f80 , /*LinkSupervisionTimeoutHfp_s*/
	0x3f00 , /*LinkSupervisionTimeoutA2dp_s*/	
	0x000f , /*EncryptionRefreshTimeout_m*/	
	0x003C	 /*InquiryTimeout_s*/
	}
};


/* PSKEY_USR_6 - amp config */
static const config_amp amp_config =
{
  	1,
  	{
		0x4305  		/*useAmp:1 ampAutoOff:1 unused:1 ampPio:5 ampOffDelay:8*/
	}
};


/* PSKEY_USR_7 - Number of LED filters */
#define NO_LED_FILTERS  (0x0003)

static const config_uint16_type no_led_filters =
{
  	1,
  	{NO_LED_FILTERS}
};


/* PSKEY_USR_8 - LED filter configuration */
static const config_led_filters_type led_filters =
{
 	sizeof(led_filter_config_type) * NO_LED_FILTERS,
 	{
/*1*/        0x1b00, 0x800e, 0x8000,     /*new filter for charger connected */
/*2*/        0x1c00, 0x0010, 0x0000,     /*cancel for chg disconnected*/ 			 		 
/*3*/        0x0e00, 0x0010, 0x0000      /*cancel for DUT mode*/ 			 		 			 
 	}, 
};


/* PSKEY_USR_9 - Number of LED states */
#define NO_LED_STATES_A  (0x0010)

static const config_uint16_type no_led_states_a =
{
  	1,
  	{NO_LED_STATES_A}
};


/* PSKEY_USR_10 - LED state configuration */
static const config_led_states_type led_states_a =
{
  	sizeof(led_config_type) * NO_LED_STATES_A,
  	{
        0x0100, 0x0303, 0x0100, 0x002f, 0xe100, /*pairing mode*/
                
        0x0200, 0x0a32, 0x1400, 0x002f, 0xe100, /*hfp connectable,	a2dp connectable*/    	
    	0x0300, 0x4bff, 0x2800, 0x002f, 0xe100, /*hfp connected,	a2dp connectable*/    	
    	0x0400, 0x4bff, 0x2800, 0x002f, 0xe100, /*outgoing call,	a2dp connectable*/
    	0x0500, 0x1414, 0x0100, 0x002f, 0xe100, /*incoming call,	a2dp connectable*/    	
    	0x0600, 0x4bff, 0x2800, 0x002f, 0xe100, /*active call,		a2dp connectable*/
		0x0800, 0x4bff, 0x2800, 0x002f, 0xe100, /*TWCWaiting,     	a2dp connectable*/
		0x0900, 0x4bff, 0x2800, 0x002f, 0xe100, /*TWCOnHold,		a2dp connectable*/
		0x0a00, 0x4bff, 0x2800, 0x002f, 0xe100, /*TWCMulticall,     a2dp connectable*/
        
    	0x0201, 0x4bff, 0x2800, 0x002f, 0xe100, /*hfp connectable,	a2dp connected*/    	
    	0x0301, 0x4bff, 0x2800, 0x002f, 0xe100, /*hfp connected,	a2dp connected*/    	
    	0x0401, 0x4bff, 0x2800, 0x002f, 0xe100, /*outgoing call,	a2dp connected*/
    	0x0501, 0x1414, 0x0100, 0x002f, 0xe100, /*incoming call,	a2dp connected*/    	
    	0x0601, 0x4bff, 0x2800, 0x002f, 0xe100, /*active call,		a2dp connected*/
		0x0801, 0x4bff, 0x2800, 0x002f, 0xe100, /*TWCWaiting,     	a2dp connected*/
		0x0901, 0x4bff, 0x2800, 0x002f, 0xe100, /*TWCOnHold,		a2dp connected*/
  	}, 
};

/* PSKEY_USR_11 - Number of LED states */
#define NO_LED_STATES_B  (0x000a)

static const config_uint16_type no_led_states_b =
{
  	1,
  	{NO_LED_STATES_B}
};


/* PSKEY_USR_12 - LED state configuration */
static const config_led_states_type led_states_b =
{
  	sizeof(led_config_type) * NO_LED_STATES_B,
  	{ 
		0x0a01, 0x4bff, 0x2800, 0x002f, 0xe100, /*TWCMulticall,     a2dp connected*/
        
        0x0202, 0x4bff, 0x2800, 0x002f, 0xe100, /*hfp connectable,	a2dp streaming/paused*/    	
    	0x0302, 0x4bff, 0x2800, 0x002f, 0xe100, /*hfp connected,	a2dp streaming/paused*/    	
    	0x0402, 0x4bff, 0x2800, 0x002f, 0xe100, /*outgoing call,	a2dp streaming/paused*/
    	0x0502, 0x1414, 0x0100, 0x002f, 0xe100, /*incoming call,	a2dp streaming/paused*/    	
    	0x0602, 0x4bff, 0x2800, 0x002f, 0xe100, /*active call,		a2dp streaming/paused*/	
		0x0802, 0x4bff, 0x2800, 0x002f, 0xe100, /*TWCWaiting,     	a2dp streaming/paused*/
		0x0902, 0x4bff, 0x2800, 0x002f, 0xe100, /*TWCOnHold,		a2dp streaming/paused*/
		0x0a02, 0x4bff, 0x2800, 0x002f, 0xe100, /*TWCMulticall,     a2dp streaming/paused*/
		
		0x0700, 0xff00, 0x0100, 0x00ff, 0xe400  /*test mode*/   
		
  	}, 
};


/* PSKEY_USR_13 - Number of LED events */
#define NO_LED_EVENTS  (0x000a)

static const config_uint16_type no_led_events =
{
  	1,
  	{NO_LED_EVENTS}
};


/* PSKEY_USR_14 - LED event configuration */
static const config_led_events_type led_events =
{
 	sizeof(led_config_type) * NO_LED_EVENTS,
 	{
   		0x0100, 0x6464, 0x0000, 0x001f, 0xe200, /*power on*/
        0x0200, 0x6464, 0x0000, 0x001f, 0xe200, /*power off*/
        0x0600, 0x0a0a, 0x0000, 0x001f, 0xe200, /*answer*/
        0x1500, 0x0a0a, 0x0000, 0x001f, 0xe200, /*end of call*/
        0x0d00, 0x6432, 0x0000, 0x002f, 0xe200, /*reset paired devices*/
        0x1400, 0x0505, 0x0000, 0x002f, 0xe200, /*low battery*/
        0x1f00, 0x3232, 0x0000, 0x002f, 0xe200, /*link loss*/
        0x0a00, 0x0505, 0x0000, 0x001f, 0xe300, /*toggle mute*/
        0x2500, 0x0505, 0x0000, 0x001f, 0xe200, /*slc connected*/
		0x3a00, 0x0505, 0x0000, 0x001f, 0xe200  /*a2dp connected*/
 	}, 
};


#define  NO_EVENTS  (25)

/* PSKEY_USR_15 - System event configuration A */
static const config_events_type events_a =
{
    sizeof(event_config_type) * NO_EVENTS,
    {                                   /* event:8 (duration:8, button(s):32, states:(HFP:12 A2DP:4)) */
        0x0102, 0x0100, 0x0000, 0x0011, /* power On (long, MFB, logically off) */
        0x0203, 0x0100, 0x0000, 0x7fef, /* power Off (v long, MFB, powered on) */
        0x1b06, 0x0200, 0x0000, 0xffff, /* charger connected (rising edge, all states) */
        0x1c07, 0x0200, 0x0000, 0xffff, /* charger disconnected (falling edge, all states) */

        0x030b, 0x0100, 0x0000, 0x004f, /* enter pairing (vv long, MFB, HFP connectable) */
        0x0408, 0x0100, 0x0000, 0x00cf, /* initiate voice dial (short single, MFB, HFP connectable | connected) */
        0x0509, 0x0100, 0x0000, 0x008f, /* initiate LNR (long release MFB, HFP connected) */
        0x0608, 0x0100, 0x0000, 0x020f, /* answer call (short-single, MFB, incoming call) */

        0x0709, 0x0100, 0x0000, 0x020f, /* reject call (long release, MFB, incoming call) */
        0x0808, 0x0100, 0x0000, 0x050f, /* end call (short-single, MFB, outgoing | active call) */
        0x0909, 0x0100, 0x0000, 0x740f, /* transfer audio (long release, MFB, active call | any threeway) */
        0x0d0b, 0x0100, 0x0800, 0x0011, /* reset paired device list (vv long, MFB + vol up, logically off) */
        
        0x2904, 0x0000, 0x2000, 0x7fef, /* button locking (double, play/pause, powered on) */
        0x0b01, 0x0000, 0x0800, 0x7fef, /* vol up (short, vol up, powered on) */
        0x0b05, 0x0000, 0x0800, 0x7fef, /* vol up (repeat, vol up, powered on) */
        0x0c01, 0x0000, 0x1000, 0x7fef, /* vol down (short, vol down, powered on) */

        0x0c05, 0x0000, 0x1000, 0x7fef, /* vol down (repeat, vol down, powered on) */
        0x0a01, 0x0000, 0x1800, 0x750f, /* toggle mute (short, vol up + vol down, outgoing call | active call | any threeway) */
        0x4704, 0x0100, 0x0000, 0x340f, /* TWC AcceptWaitingHoldActive (double, MFB, TWCWaiting | TWCOnHold | Active call) */
        0x4902, 0x0100, 0x1000, 0x600f, /* TWC Connect2Disconnect (long, MFB + vol down, TWCOnHold | TWCMulticall) */

        0x4501, 0x0100, 0x1000, 0x300f, /* TWC ReleaseAllHeld (short, MFB + vol down, TWCWaiting | TWCOnHold) */
        0x4608, 0x0100, 0x0000, 0x700f, /* TWC AcceptWaitingReleaseActive (short single, MFB, TWCWaiting | TWCOnHold | TWCMulticall) */ 
        0x4801, 0x0100, 0x0800, 0x300f, /* TWC AddHeldTo3Way (short, MFB + vol up, TWCWaiting | TWCOnHold) */
        0x2702, 0x0100, 0x0000, 0x76cf, /* Long timer (long, MFB, HFP connectable | HFP connected | incoming call | active call | any threeway) */
        0x0501, 0x0100, 0x0800, 0x040f, /* initiate LNR (short, MFB + vol up, active call) */
    },
};


/* PSKEY_USR_16 - System event configuration B */
static const config_events_type events_b =
{

    sizeof(event_config_type) * NO_EVENTS,
    {
        0x3908, 0x0000, 0x2000, 0x0fc1, /*play (short press, a2dp not connected)*/        
        0x3008, 0x0000, 0x2000, 0x0fca, /*play (short press, a2dp connected/paused)*/
        0x3108, 0x0000, 0x2000, 0x0fc4, /*pause (short press, a2dp streaming)*/       
        0x3202, 0x0000, 0x2000, 0x0fce, /*stop (long press, a2dp connected/streaming/paused)*/    
        
        0x3701, 0x0000, 0x4000, 0x0fce, /*skip forward (short fwd, a2dp connected/streaming/paused)*/           
        0x3801, 0x0000, 0x8000, 0x0fce, /*skip backward (short back, a2dp connected/streaming/paused)*/        
        0x3302, 0x0000, 0x4000, 0x0fce, /*fast forward press (long fwd, a2dp connected/streaming/paused)*/
        0x3305, 0x0000, 0x4000, 0x0fce, /*fast forward repeat (fwd long held, a2dp connected/streaming/paused)*/        

        0x3409, 0x0000, 0x4000, 0x0fce, /*fast forward long release (fwd long release, a2dp connected/streaming/paused)*/
        0x340a, 0x0000, 0x4000, 0x0fce, /*fast forward v long release (fwd v long release, a2dp connected/streaming/paused)*/
        0x340c, 0x0000, 0x4000, 0x0fce, /*fast forward vv long release (fwd vv long release, a2dp connected/streaming/paused)*/
        0x3502, 0x0000, 0x8000, 0x0fce, /*fast rewind press (long back, a2dp connected/streaming/paused)*/

        0x3505, 0x0000, 0x8000, 0x0fce, /*fast rewind repeat (back long held, a2dp connected/streaming/paused)*/        
        0x3609, 0x0000, 0x8000, 0x0fce, /*fast rewind long release (back long release, a2dp connected/streaming/paused)*/
        0x360a, 0x0000, 0x8000, 0x0fce, /*fast rewind v long release (back v long release, a2dp connected/streaming/paused)*/
        0x360c, 0x0000, 0x8000, 0x0fce, /*fast rewind vv long release (back vv long release, a2dp connected/streaming/paused)*/

        0x160a, 0x0100, 0x0000, 0x004f,	/*establish SLC (v long release, MFB, HFP connectable)*/    
        0x3704, 0x0000, 0x4000, 0x0fce, /*skip forward (double fwd, a2dp connected/streaming/paused)*/           
        0x3804, 0x0000, 0x8000, 0x0fce, /*skip backward (double back, a2dp connected/streaming/paused)*/   
        0x3f0b, 0x0100, 0x1800, 0x0011,	/*enter DFU (vv long, MFB + vol up + vol down, logically off)*/
        
		0x5208, 0x0100, 0x0800, 0x00ec, /*switch audio mode (short, MFB + vol up, HFP connectable | HFP connected | connectable/discoverable, A2DP streaming | paused)*/
        0x4302, 0x0100, 0x1000, 0x00ff, /*toggle debug keys (long, MFB + vol down, Off | HFP connectable | HFP connected | connectable/discoverable)*/
        0x1609, 0x0100, 0x0000, 0x004f,	/*establish SLC (long release, MFB, HFP connectable)*/
		0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000
    },
};


/* PSKEY_USR_17 - Number of tone events */
#define NO_TONE_EVENTS  (0x001a)

static const config_uint16_type no_tone_events =
{
  	1,
  	{NO_TONE_EVENTS}
};


/* PSKEY_USR_18 - Tone event configuration */
static const config_tone_events_type tone_events =
{
  	sizeof(tone_config_type) * NO_TONE_EVENTS,
  	{
    	0x0101, /*power on*/
        0x0201, /*power off*/
        0x0302, /*enter pairing*/
        0x0d02, /*reset paired devices*/
        0x0409, /*voice dial*/
        0x050a, /*LNR*/
        0x090a, /*transfer audio*/
        0x0609, /*answer*/
        0x070a, /*reject*/
        0x0809, /*cancel end*/
        0x1405, /*low battery*/
        0x2104, /*mute on*/
        0x2203, /*mute off*/
        0x2507, /*slc connected*/
        0x2608, /*error*/
        0xff0c, /*ringtone*/
		0x3c06,	/*volume max*/	
		0x3d06,	/*volume min*/	
		0x3a07, /*a2dp connected*/
		0x230b,	/*mute reminder*/
		0x1609, /*connect hfp*/
		0x3909,	/*connect a2dp*/
		0x0f04, /*button lock on*/
        0x4003, /*button lock off*/
		0x2725,	/*long timer - event tone 1*/
		0x4304	/*toggle debug keys*/
  	}
};


/* PSKEY_USR_19 - volume gains */
static const config_volume_type vol_gains = 
{
    sizeof(vol_table_t),
    {
		0x88f0, 		/* default/max vols */
		0x000f,			/* tone vol */
		0x0010, 0x0000, /* index 0 */
	    0x0120, 0x0100, /* index 1 */
	    0x0231, 0x0200, /* index 2 */
	    0x0342, 0x0300, /* index 3 */
	    0x0453, 0x0400, /* index 4 */
	    0x0564, 0x0500, /* index 5 */
	    0x0675, 0x0600, /* index 6 */
	    0x0786, 0x0700, /* index 7 */
	    0x0897, 0x0800, /* index 8 */
	    0x09a8, 0x0900, /* index 9 */
	    0x0ab9, 0x0a00, /* index 10 */
	    0x0bca, 0x0b00, /* index 11 */
	    0x0cdb, 0x0c00, /* index 12 */
	    0x0dec, 0x0d00, /* index 13 */
	    0x0efd, 0x0e00, /* index 14 */
	    0x0ffe, 0x0f00  /* index 15 */
    }
};


/* PSKEY_USR_20 - features */
static const config_features features = 
{
    4,
    {
        0xc415,	/* autoSendAvrcp:1 unused:1 forceMitmEnabled:1 writeAuthEnable:1
				   debugKeysEnabled:1 ConnectActionOnPowerOn:2 LinkLossRetries:5 
					exitPairingModeAction:2 unused:1 PlayHfpTonesAtFixedVolume:1 */
		
		0x4BC2,	/* MuteToneFixedVolume:1 QueueEventTones:1 MuteSpeakerAndMic:1 UseLowPowerAudioCodecs:1 
					QueueLEDEvents:1 LedTimeMultiplier:2 UseHFPprofile:1 
					UseA2DPprofile:1 UseAVRCPprofile:1 mono:1 UseAVRCPforVolume:1 
					UseSCOforAudioTransfer:1 OverideMute:1 MuteLocalVolAction:1 DisableRoleSwitching:1 */
		
		0x1130,	/* audio_plugin:3 AutoPowerOnAfterInitialisation:1 
					LinkPolicyA2dpSigBeatsSLC:1 PowerOnConnectToDevices:1 ManualConnectToDevices:1 AutoAnswerOnConnect:1 
					LNRCancelsVoiceDialIfActive:1 EndCallWithNoSCOtransfersAudio:1 UseHWmicBias:1 SecurePairing:1
					DiscoIfPDLLessThan:4  */
		
		0x1000	/* PairIfPDLLessThan:4 unused:12 */
    }
};


/* PSKEY_USR_21 - Sniff Subrate parameters */
static const config_ssr_params_type ssr_config =
{
    6,
    {
        0x0000, 0x0000, 0x0000,     /* stream params SCO & A2DP Media */
        0x0000, 0x0000, 0x0000      /* signalling params SLC, A2DP Signalling & AVRCP */
    }
};

/* PSKEY_USR_24 - RSSI Pairing configuration */
static const config_rssi_type rssi =
{
    sizeof(rssi_pairing_t),
  	{
        0xFFBA,        /* Inquiry Tx Power				*/
        0xFFEC,        /* RSSI Threshold				*/
        0x0005,        /* RSSI difference threshold	    */	
        0x0040, 0x0200,/* HFP Class of device filter	*/	
		0x0000, 0x0400 /* A2DP Class of device filter	*/	
	}
};


/* CSR Pioneer Default Configuration */
const config_type csr_default_config_1645 = 
{
    &battery_config,        	/* Battery configuration */
  	&button_config,       		/* Button configuration */
  	&button_pattern_config,		/* Button pattern configuration*/
	&hfp_features_config,		/* HFP features*/
	&auristream_config,			/* Auristream parameters*/
    &timeouts, 		        	/* Timeouts*/
	&amp_config, 		        /* Amp*/
  	&no_led_filters,       		/* Number of LED filters */
  	&led_filters,         		/* LED filter configuration */
  	&no_led_states_a,      		/* Number of LED states */
  	&led_states_a,         		/* LED state configuration */
	&no_led_states_b,      		/* Number of LED states */
  	&led_states_b,         		/* LED state configuration */
  	&no_led_events,        		/* Number of LED events */
  	&led_events,         		/* LED event configuration */
  	&events_a,          		/* Number of system events */
  	&events_b,           		/* System event configuration */
  	&no_tone_events,       		/* Number of tone events */
  	&tone_events,         		/* Tone event configuration */
  	&vol_gains,        			/* Volume Gains */
    &features,         			/* Features */
    &ssr_config,				/* Sniff Subrate parameters */
	&rssi,       				/* RSSI configuration */
};


