/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file   headset_private.h
@brief  The private data structures used by the stereo headset application.

    The main application task and global data is declated here.
*/

#ifndef _HEADSET_PRIVATE_H_
#define _HEADSET_PRIVATE_H_


#include "headset_leddata.h"
#include "headset_states.h"

#include <app/message/system_message.h>
#include <a2dp.h>
#include <avrcp.h>
#include <battery.h>
#include <csrtypes.h>
#include <message.h>
#include <hfp.h>
#include <sink.h>

/* Class of device bitmask defines */
#define AUDIO_MAJOR_SERV_CLASS  0x200000    /*!< @brief Class of Device - Major Service - Audio */
#define AV_MAJOR_DEVICE_CLASS   0x000400    /*!< @brief Class of Device - Major Device - Audio/Video */
#define AV_MINOR_HEADSET        0x000004    /*!< @brief Class of Device - Minor Device - Wearable Headset Device */
#define AV_COD_RENDER           0x040000    /*!< @brief Class of Device - Major Device - Rendering */

#define A2DP_RESTART_DELAY      (uint32)500 /*!< @brief Restart A2DP Delay Time */

/* Local stream end point codec IDs */
#define SBC_SINK_SEID                1           /*!< @brief Local Stream End Point ID for SBC codec */
#define SBC_SOURCE_SEID              2           /*!< @brief Local Stream End Point ID for SBC codec */

#define NUM_SEPS                (SBC_SOURCE_SEID)  /*!< @brief The total number of SEPs with SBC_SOURCE_SEID included */

#define KALIMBA_RESOURCE_ID     1           /*!< @brief Resource ID for Kalimba */

#define LED_HEADSET_STATES		(3)
#define LED_HFP_STATES			(8)
#define LED_A2DP_STATES			(3)
#define LED_TOTAL_STATES		(LED_HEADSET_STATES + (LED_A2DP_STATES*LED_HFP_STATES))

#define MAX_PAIRED_DEVICES		(8)


#define HEADSET_MSG_BASE        (0x6000)       /*!< @brief Locally generated message base */

/*! @brief Enumeration of message IDs for application specific messages */
enum
{
    APP_RESUME_A2DP = HEADSET_MSG_BASE,
    APP_AVRCP_CONTROLS,
    APP_AVRCP_CONNECT_REQ,
    APP_SEND_PLAY,
    APP_CHARGER_MONITOR,
	APP_ENABLE_POWER_OFF,
	APP_LIMBO_TIMEOUT,
    APP_INQUIRY_CONTINUE,
    APP_CHECK_FOR_AUDIO_TRANSFER,
	APP_CONNECT_HFP_LINK_LOSS,
	APP_CONNECT_A2DP_LINK_LOSS,
	APP_EVENT_REFRESH_ENCRYPTION,
	APP_CANCEL_HSP_INCOMING_CALL,
	APP_CONNECT_A2DP,
	APP_TX_TEST_MODE,
    HEADSET_MSG_TOP
};


/*! @brief Enumeration of AVRCP controls */
typedef enum
{
    AVRCP_CTRL_PAUSE_PRESS,
    AVRCP_CTRL_PAUSE_RELEASE,
    AVRCP_CTRL_PLAY_PRESS,
    AVRCP_CTRL_PLAY_RELEASE,
    AVRCP_CTRL_FORWARD_PRESS,
    AVRCP_CTRL_FORWARD_RELEASE,
    AVRCP_CTRL_BACKWARD_PRESS,
    AVRCP_CTRL_BACKWARD_RELEASE,
    AVRCP_CTRL_STOP_PRESS,
    AVRCP_CTRL_STOP_RELEASE,
    AVRCP_CTRL_FF_PRESS,
    AVRCP_CTRL_FF_RELEASE,
    AVRCP_CTRL_REW_PRESS,
    AVRCP_CTRL_REW_RELEASE
} avrcp_controls;


/*! @brief Definition of application message used for sending AVRCP control commands */
typedef struct
{
    avrcp_controls control;
} APP_AVRCP_CONTROLS_T;

/*! @brief Definition of application message used for connecting AVRCP */
typedef struct
{
    bdaddr addr;
} APP_AVRCP_CONNECT_REQ_T;

typedef struct
{
	bool a2dp_ag_connect_signalling_only;
} APP_CONNECT_A2DP_T;


/*! @brief Enumeration of DSP processing task types */
typedef enum
{
    dsp_process_none = 0,
    dsp_process_sco  = 1,
    dsp_process_a2dp = 2
} dspProcessType;


/*! @brief Possible sample rate values for A2DP */
typedef enum
{
    a2dp_rate_48_000k,
    a2dp_rate_44_100k,
    a2dp_rate_32_000k,
    a2dp_rate_24_000k,
    a2dp_rate_22_050k,
    a2dp_rate_16_000k
} a2dp_rate_type;


/*! @brief AVRCP Specific data */
typedef struct
{
    bool pending;       /*!< AVRCP is pending a command response */
    uint16 send_play;   /*!< Should a play be sent */
} avrcpData;

/*! @brief Sink codec data. */
/* This struct must match with the codec_data_type in a2dp.h. We've just missed off the last uint16 which isn't needed here */
typedef struct
{
    uint8 content_protection;
    uint32 voice_rate;
    unsigned bitpool:8;
    unsigned format:8;
    uint16 packet_size;
	uint16 clock_mismatch;
} sink_codec_data_type;


/*! @brief A2DP Specific data */
typedef struct
{
    device_sep_list *sep_entries;       /*!< The Stream End Point (SEP) pointer */
    sink_codec_data_type codecData;     /*!< The audio codec related data */
} a2dpData;


/*! @brief The application timeouts */
typedef struct TimeoutsTag
{
    uint16 PairModeTimeout_s ;
    uint16 MuteRemindTime_s ;
    uint16 AutoSwitchOffTime_s ;
	uint16 DisablePowerOffAfterPowerOnTime_s;
	uint16 LinkSupervisionTimeoutHfp_s;
	uint16 LinkSupervisionTimeoutA2dp_s;
	uint16 EncryptionRefreshTimeout_m;
	uint16 InquiryTimeout_s ;
}Timeouts_t ;


typedef enum 
{
	audio_codec_cvsd 				= 0
} audio_codec_type ;

/*! @brief The application features block.

    Used to map application feature configuration bits when read from persistent storage.
    Corresponding bits are set in the hsTaskData structure for use by the application to determine if a feature is enabled.

    Please refer to the stereo headset user guide document for configuration details on
    the features listed here.
*/
typedef struct FeaturesTag
{
    unsigned autoSendAvrcp:1;       /*!< set to enable automatic sending of AVRCP commands by the application */
    unsigned unused1:1;          /*!< unused */
    unsigned forceMitmEnabled:1;    /*!< set to force ManInTheMiddle during pairing */
    unsigned writeAuthEnable:1;     /*!< set to enable writeAuthEnable */
    unsigned debugKeysEnabled:1;    /*!< set to enable toggling of debug keys for SSP */
	unsigned ConnectActionOnPowerOn:2;	/*!< set to specify the connect action on power on */
	unsigned LinkLossRetries:5;		/*!< connection retries on link loss */
	unsigned exitPairingModeAction:2;	/*!< exit pairing mode action */
	unsigned unused2:1;				/*!< unused */
    unsigned PlayHfpTonesAtFixedVolume:1; /*!< set to play tones for HFP at fixed vol */
	
	unsigned MuteToneFixedVolume:1; /*!< set to play mute reminder tone at fixed vol */
	unsigned QueueEventTones:1;		/*!< set to queue event tones */
	unsigned MuteSpeakerAndMic:1;	/*!< set to mute speaker as well as mic when mute activated */
	unsigned UseLowPowerAudioCodecs:1;	/*!< set to use low power codecs */
	unsigned QueueLEDEvents:1;		/*!< set to queue LED events */
	unsigned LedTimeMultiplier:2; 	/* multiply the times of the LED settings (x1 x2 x4 x8) */
	unsigned UseHFPprofile:1;		/*!< set to use HFP profile */
	unsigned UseA2DPprofile:1;		/*!< set to use A2DP profile */
	unsigned UseAVRCPprofile:1;		/*!< set to use AVRCP profile */
	unsigned mono:1;				/*!< set to only use one speaker */
	unsigned UseAVRCPforVolume:1;	/*!< set to use AVRCP as target role for receiving volume commands */
	unsigned UseSCOforAudioTransfer:1;	/*!< set to only use SCO when headset initiates audio connection */
	unsigned OverideMute:1 ;		/*!< whether or not to unmute if a vol msg is received */    
	unsigned MuteLocalVolAction:1;	/*!< whether or not to update the global vol whilst muted */   
	unsigned DisableRoleSwitching:1;/*!< disable the headset role switching when in scatternet */
	
	unsigned audio_plugin:3;		/*!< the audio plugin to use */
	unsigned AutoPowerOnAfterInitialisation:1 ; /*!< if the headset should power on after initialistaion. Only occurs if charger not connected */
	unsigned LinkPolicyA2dpSigBeatsSLC:1;	/*!< If the A2DP signalling link policy should beat the SLC link policy when both connected */
	unsigned PowerOnConnectToDevices:1;		/*!< Specify if to connect to last device on power on or to traverse the paired list */		
	unsigned ManualConnectToDevices:1;		/*!< Specify if to connect to last device on manual connect or to traverse the paired list */
	unsigned AutoAnswerOnConnect:1;			/*!< Specify if the headset should auto answer an incoming call on connection */
	unsigned LNRCancelsVoiceDialIfActive:1;	/*!< Specify if a Last Number Redial will cancel a Voice Dial if it is active */
	unsigned EndCallWithNoSCOtransfersAudio:1;	/*!< Specify if an EventCancelEnd will trigger a EventTransferToggle instead if the call audio is currently on the AG */
	unsigned UseHWmicBias:1;		/*!< Specifies to use the HW Mic Bias pin. This feature should be enabled instead of setting the Use Hardware Mic Bias Pin in PSKEY_CODEC_PIO, as this results in the mic bias being driven during A2DP which gives higher power consumption. */
	unsigned SecurePairing:1;		/*!< Specifies if pairing is allowed only when in dedicated Pairing Mode. */
	unsigned DiscoIfPDLLessThan:4;	/*!< enter pairing if number PDL entries is less than specified value */
	
	unsigned PairIfPDLLessThan: 4;	/* Use RSSI on inquiry responses to pair to nearest AG */
	unsigned unused3:12;			/*!< unused */

}Features_t ;

/*! @brief Sniff SubRate parameters. */
typedef struct
{
    uint16 max_remote_latency;      /*!< sniff subrate maximum remote latency */
    uint16 min_remote_timeout;      /*!< sniff subrate minimum remote timeout */
    uint16 min_local_timeout;       /*!< sniff subrate minimum local timeout */
} ssr_params;

/*! @brief Sniff Subrate data for all application connections. */
typedef struct
{
    ssr_params streaming_params;    /*!< Sniff subrate parameters for streaming connections - SCO & A2DP Media */
    ssr_params signalling_params;   /*!< Sniff subrate parameters for signalling connections - SLC, A2DP Signalling & AVRCP */

    /* potentially we might require separate params for each sink, but
       would require 9 more words */

} subrate_data;

/*the action to take on the auto reconnect*/
typedef enum AutoReconnectActionTag
{
    AR_None,
    AR_ConnectToAG,
	AR_ConnectToAGandA2DP
}ARAction_t ;

/*the action to take on the auto reconnect*/
typedef enum ConnectActionTag
{
    Connect_Last,
    Connect_List
}ConnectAction_t ;

/*the action to take on exit pairing*/
typedef enum PairingExitActionTag
{
    PairingExit_PowerOff,
	PairingExit_PowerOffPdlEmpty,
	PairingExit_Connectable
}PairingExitAction_t ;

/*! 
    @brief HFP Version
    
    Enum defining the values in the HFP_Version field of PSKEY_HFP_SUPPORTED_FEATURES
*/
typedef enum
{
	headset_hfp_version_1_0 = 0x00,
	headset_hfp_version_1_5 = 0x01
} HFP_Version;

/*! 
    @brief EQ Type
    
    Enum defining the EQ levels.
*/
typedef enum
{
	eq_mode_level1,
	eq_mode_level2,
	eq_mode_level3,
	eq_mode_passthrough
} eqModeType;

typedef struct 
{
	uint16 		HFP_Supported_Features;
	
    unsigned    HFP_Version:2;   
    unsigned    supportedSyncPacketTypes:10 ;
	unsigned    eSCO_Parameters_Enabled:1;
    unsigned    reserved:3;
    
    /*additional optional parameters for HFP */
    uint32      bandwidth;
    uint16      max_latency;
    uint16      voice_settings;
    uint16      retx_effort;  
} HFP_features_type ;

typedef struct
{	
	unsigned hfpGain:8;
	unsigned hfpIncVol:4;
	unsigned hfpDecVol:4;
	unsigned a2dpGain:8;
	unsigned reserved:8; 
} vol_levels_t;

typedef struct
{
	unsigned defaultHfpVolLevel:4;
	unsigned defaultA2dpVolLevel:4;
	unsigned maxA2dpVolLevel:4;
	unsigned reserved:4;
	unsigned reserved2:11;
	unsigned toneVol:5;
	vol_levels_t volumes[16];
} vol_table_t;

typedef struct
{
	unsigned 		reserved:8;
	unsigned        A2DPStreamEntries:2;
	unsigned        A2DPSigEntries:2;
    unsigned        SCOentries:2;
	unsigned		SLCentries:2;

    /* pointers to arrays of lp_power_tables */
	lp_power_table powertable[1];

} power_table;

typedef struct
{
	unsigned        capsAsize:8;
	unsigned        capsBsize:8;

	/* pointer to the capabilities */
    uint16			caps[1];

} a2dp_codec_caps_table;

typedef enum
{
	attribute_hfp_hsp_profile,
	attribute_a2dp_profile,
	attribute_hfp_volume,
	attribute_a2dp_volume,
	attribute_seid,
	attribute_clock_mismatch
} AttributesType;

#define ATTRIBUTE_SIZE	attribute_clock_mismatch+1


typedef enum
{
	attribute_hf_no_profile,
	attribute_hf_hfp_profile,
	attribute_hf_hsp_profile
} AttributesHfType;

typedef struct
{
	bdaddr lastPaired;
	bdaddr lastA2dpConnected;
	bdaddr lastHfpConnected;
	AttributesHfType lastHfpHsp;
} last_devices_t;

#define ATTRIBUTE_GET_HF_PROFILE(x) ((x==hfp_handsfree_profile)?attribute_hf_hfp_profile:attribute_hf_hsp_profile)

typedef struct
{
    bdaddr bd_addr;                     /* Address of device */
    int16  rssi;                        /* Highest received signal strength indication from device */
	bool hfp_inquiry;
}inquiry_data_t;

typedef struct
{
    uint16 tx_power;
	uint16 threshold;
	uint16 diff_threshold;
    uint32 hfp_cod_filter;
	uint32 a2dp_cod_filter;
}rssi_pairing_t;

typedef struct 
{	
	vol_table_t 		gVolLevels;
	subrate_data        ssr_data;
	rssi_pairing_t		rssi;
}Configuration_t;


/*! @brief Headset data

    Global data for the stereo headset application.
*/
typedef struct
{
    TaskData            task;                           /*!< Main task for the stereo headset application */
    Task                theCodecTask;                   /*!< Codec task */

    a2dpData            a2dp_data;                      /*!< Local A2DP data */
    avrcpData           avrcp_data;                     /*!< Local AVRCP data */

    HFP                 *hfp;                           /*!< Pointer to Hands Free Profile instance */
    HFP                 *hsp;                           /*!< Pointer to Headset Profile instance */
    HFP                 *hfp_hsp;                       /*!< Pointer to either the HFP or HSP instance (whichever has been instantiated) */
    A2DP                *a2dp;                          /*!< Pointer to A2DP Profile instance */
    AVRCP               *avrcp;                         /*!< Pointer to AVRCP Profile instance */

	power_table         *user_power_table;				/*!< The low power table entries */
	    
    Timeouts_t          Timeouts ;                      /*!< Application timeouts */   
	HFP_features_type   HFP_features;					/*!< HFP additional features (spec. version, (e)SCO packet types etc.) */
	Features_t			features;						/*!< The headset features */
	
	Configuration_t		*config;						/*!< Pointer to general configuration data */

    bdaddr*             confirmation_addr;              /*!< user confirmation data */
	
	inquiry_data_t		*inquiry_data;					/*!< Holds the results of headset inquiry */	
	
	uint16				ProfileLibrariesInitialising;	/*!< Functionality will be limited if libraries haven't been initialised */
	
	uint16				clock_mismatch_rate;			/*!< Holds the clock mismatch rate for the currently active A2DP device */
	
    hfp_profile         profile_connected:2;            /*!< Determines if the HFP or HSP profile is in use */
    unsigned			LinkLossAttemptA2dp:5;			/*!< Current connection attempt after A2DP link loss */
	unsigned			keyTestLedState:2;				/*!< The current key test LED state; 0 - RED on, 1 - RED off, 2 - BLUE 0n, 3 - BLUE off */
	unsigned			keyTestMode:1;					/*!< Flag set if Key test mode is entered */
    unsigned            initial_battery_reading:1;      /*!< Flag to indicate if the initail battery reading is being taken */
    unsigned            a2dp_channel_mode:4;            /*!< A2DP Channel Mode */
    unsigned            connect_a2dp_when_no_call:1;    /*!< Flag to indicate that A2DP should be connected once call activity has ended */

    unsigned            seid:8;	                        /*!< Active SEP ID */
    unsigned            gMuted:1 ;                      /*!< Flag to indicate if call audio is muted */
    unsigned            gHfpVolumeLevel:5 ;             /*!< The HFP audio volume level */

    unsigned            gAvVolumeLevel:5 ;              /*!< The A2DP audio volume level */
    a2dp_rate_type      a2dp_rate:3;                    /*!< A2DP rate */
    unsigned            voice_recognition_enabled:1;    /*!< Flag to indicate if voice recognition is enabled */

    unsigned            inquiry_scan_enabled:1;         /*!< Flag to indicate if inquiry scan is enabled */
    unsigned            page_scan_enabled:1 ;           /*!< Flag to indicate if page scan is enabled */
    unsigned            dsp_process:3;                  /*!< Flag to indicate if the dsp processing SCO, or A2DP, or neither */
    unsigned            slcConnectFromPowerOn:1 ;       /*!< Flag to indicate if headset is connecting HFP due to power on reconnect procedure */
    unsigned            slcConnecting:1 ;	        	/*!< Flag to indicate if headset is connecting HFP */
    unsigned            a2dpConnecting:1;               /*!< Flag to indicate if headset is connecting A2DP */
    unsigned            a2dpSourceSuspended:1;          /*!< Flag to indicate if headset successfully suspended A2DP source */
    unsigned            sendPlayOnConnection:1;         /*!< Flag to indicate if an avrcp_play should be sent once media is connected */
    unsigned            combined_link_loss:1;           /*!< Flag to indicate that if one profile has had link loss, it's probable the other profile will have link loss, if it's the same device */
    unsigned            InBandRingEnabled:1;            /*!< Flag to indicate if inband ringtones have been enabled from the AG */
    unsigned            PlayingState:1;                 /*!< Flag to indicate the suspected status of the playing music. ie. Playing (1) or Paused/Stopped (0). This is used to hold the state when not actually streaming as the source can be suspended but AVRCP commands can still be sent. */

    unsigned            RingTone:5;                     /*!< Flag to indicate the ringtone used by the headset for out-of-band ringing */
    unsigned            confirmation:1;                 /*!< Flag to indicate we have user confirmation data to send */
	unsigned			headsetPoweredOn:1;				/*!< flag to indicate headset is powered on */
    
    unsigned            debugKeysInUse:1;               /*!< Flag to indicate if the SSP debug keys are being used */
    unsigned            a2dpCodecsEnabled:4;            /*!< Codec enabled */
	unsigned			PowerOffIsEnabled:1;			/*!< Flag stating if power off is enabled */
	unsigned			LinkLossAttemptHfp:5;			/*!< Current connection attempt after HFP link loss */
	unsigned			combinedDevice:1;				/*!< Keeps track of if the headset is connected to a combined A2DP-HFP device */
	unsigned			switch_a2dp_source:1;			
	
	unsigned			last_used_seid:8;				/*!< The last used Stream End-Point ID */
	unsigned			hfp_list_index:4;				/*!< The current index in the HFP device list */
	unsigned			a2dp_list_index:4;				/*!< The current index in the A2DP device list */
	
	unsigned            incoming_call_power_up:1;		/*!< Flag to indicate if there is an incoimng call when the headset connects */
	unsigned            inquiry_tx:8;					/*!< Holds the result of a ConnectionReadInquiryTx response */
	unsigned			HSPIncomingCallInd:1;			/*!< flag to indicate ring in progress when using HSP */
	unsigned			HSPCallAnswered:1;				/*!< flag to indicate a call has been answered on the HS using HSP */
	unsigned			eqMode:2;						/*!< Indicates the currently selected EQ mode */
	unsigned			manualA2dpConnect:1;			/*!< flag to indicate if A2DP was manually initiated */
	unsigned			SecondIncomingCall:1; 			/*!< record second incoming call */
	unsigned			unused1:1;
	
	unsigned			unused2:4;

} hsTaskData;


/* The headset task data structure - visible to all (so no need to pass it between functions) */
extern hsTaskData theHeadset ;


#endif /* HEADSET_PRIVATE_H_ */
