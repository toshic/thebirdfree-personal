/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Defines data structures used internally throughout the application.
*/

#ifndef AUDIOADAPTOR_PRIVATE_H
#define AUDIOADAPTOR_PRIVATE_H

#include <aghfp.h>
#include <a2dp.h> 
#include <audio.h>
#include <avrcp.h> 
#include <spp.h>
#include <battery.h>
#include <message.h>
#include <stdio.h>
#include <source.h>
#include <pbaps.h>
#include <syncs.h>
#include <pbap_common.h>
#include <file.h>
#include <stream.h>
#include <source.h>
#include <vm.h>

#include "audioAdaptor_buttons.h"
#include "audioAdaptor_states.h"
#include "uart.h"
#include "At_cmd.h"

#include "state.h"

/* ------------------------------------------------------------------------ */
/* Debug print defines */ 

#define ENABLE_DEBUG_MAIN
#define ENABLE_DEBUG_A2DPx
#define ENABLE_DEBUG_AGHFPx
#define ENABLE_DEBUG_AVRCPx
#define ENABLE_DEBUG_CLx
#define ENABLE_DEBUG_HIDx
#define ENABLE_DEBUG_KALIMBAx
#define ENABLE_DEBUG_USBx
#define ENABLE_DEBUG_DEV_INSTx
#define ENABLE_DEBUG_STATESx
#define ENABLE_DEBUG_CALLx
#define ENABLE_DEBUG_POWERx
#define ENABLE_DEBUG_CONFIGx
#define ENABLE_DEBUG_EVENTx
#define ENABLE_DEBUG_CONNx
#define ENABLE_DEBUG_SCANx
#define ENABLE_DEBUG_STREAMx
#define ENABLE_DEBUG_CODECx
#define ENABLE_DEBUG_PBAPx
#define ENABLE_DEBUG_VGENx

#define print_debug UartPrintf

#define SLOT()  {UartPrintf("Slot %d\n",VmGetAvailableAllocations());}

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_MAIN
    #define DEBUG(x)    {print_debug x;}
#else
    #define DEBUG(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_A2DP
    #define DEBUG_A2DP(x)    {print_debug x;}
#else
    #define DEBUG_A2DP(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_AGHFP
    #define DEBUG_AGHFP(x)    {print_debug x;}
#else
    #define DEBUG_AGHFP(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_AVRCP
    #define DEBUG_AVRCP(x)    {print_debug x;}
#else
    #define DEBUG_AVRCP(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_CL
    #define DEBUG_CL(x)    {print_debug x;}
#else
    #define DEBUG_CL(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_HID
    #define DEBUG_HID(x)    {print_debug x;}
#else
    #define DEBUG_HID(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_KALIMBA
    #define DEBUG_KALIMBA(x)    {print_debug x;}
#else
    #define DEBUG_KALIMBA(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_USB
    #define DEBUG_USB(x)    {print_debug x;}
#else
    #define DEBUG_USB(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_DEV_INST
    #define DEBUG_DEV(x)    {print_debug x;}
#else
    #define DEBUG_DEV(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_STATES
    #define DEBUG_STATES(x)    {print_debug x;}
#else
    #define DEBUG_STATES(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_CALL
    #define DEBUG_CALL(x)    {print_debug x;}
#else
    #define DEBUG_CALL(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_POWER
    #define DEBUG_POWER(x)    {print_debug x;}
#else
    #define DEBUG_POWER(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_CONFIG
    #define DEBUG_CONFIG(x)    {print_debug x;}
#else
    #define DEBUG_CONFIG(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_EVENT
    #define DEBUG_EVENT(x)    {print_debug x;}
#else
    #define DEBUG_EVENT(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_CONN
    #define DEBUG_CONN(x)    {print_debug x;}
#else
    #define DEBUG_CONN(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_SCAN
    #define DEBUG_SCAN(x)    {print_debug x;}
#else
    #define DEBUG_SCAN(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_STREAM
    #define DEBUG_STREAM(x)    {print_debug x;}
#else
    #define DEBUG_STREAM(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_CODEC
    #define DEBUG_CODEC(x)    {print_debug x;}
#else
    #define DEBUG_CODEC(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_PBAP
    #define DEBUG_PBAP(x)    {print_debug x;}
#else
    #define DEBUG_PBAP(x)
#endif

#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_VGEN
    #define DEBUG_VGEN(x)    {print_debug x;}
#else
    #define DEBUG_VGEN(x)
#endif

/* ------------------------------------------------------------------------ */
/* Set board specific defines - the board type should be defined in the project settings */

#if defined DEV_PC_1645_USB || defined DEV_PC_1645_ANALOGUE
    #define PIO_MFB           0x0001    /* PIO 0 */
    #define POWER_HOLD          0x8000

    #define DUT_PIO           0x0100    /* PIO 8 used to enter DUT mode */
#endif

#if (defined RED_PC_141) || (defined RED_PC_142)
    #define PIO_MFB           0x0004    /* PIO 2 */

    #define DUT_PIO           0x0200    /* PIO 9 used to enter DUT mode */
#endif

/* ------------------------------------------------------------------------ */
/* Define the class of device fields */

#define AV_MAJOR_DEVICE_CLASS    0x000400  
#define AV_MINOR_HIFI            0x000028
#define AV_COD_CAPTURE           0x080000

/* ------------------------------------------------------------------------ */
/* Defines for connection attempts */

#define MAX_APP_CONNECT_ATTEMPTS     3
#define MAX_PROFILE_CONNECT_ATTEMPTS 2

/* ------------------------------------------------------------------------ */
/* Message IDs for communicating with the DSP */

#define KALIMBA_SET_BITPOOL_MESSAGE              0x7070
#define KALIMBA_SET_POOR_LINK_BITPOOL_MESSAGE    0x7080
#define KALIMBA_ENCODER_SELECT                   0x7300
#define KALIMBA_AUDIO_USB_OUT_STATUS             0x7301
#define KALIMBA_AUDIO_USB_IN_STATUS              0x7302
#define KALIMBA_CODEC_TYPE_MESSAGE               0x7303

/* ------------------------------------------------------------------------ */
/* Size of inquiry results buffer */

#define INQUIRY_SCAN_BUFFER_SIZE 5

/* ------------------------------------------------------------------------ */
/* Enable SCO/eSCO packet support */
#if 1
#define AUDIO_PACKET_TYPES (sync_all_esco|sync_all_sco)
#else
#define AUDIO_PACKET_TYPES (sync_all_sco)
#endif
/* ------------------------------------------------------------------------ */
/* Defines for A2DP Stream End Points */

#define KALIMBA_RESOURCE_ID     1                  /*!< @brief Resource ID for Kalimba */


#define MAX_NUM_DEV_CONNECTIONS    1

/* Local stream end point codec IDs */
#define SBC_SEID                1                  /*!< @brief Local Stream End Point ID for SBC codec */    

#ifdef INCLUDE_MP3_ENCODER_PLUGIN
    #define MP3_SEID                2                  /*!< @brief Local Stream End Point ID for MP3 codec */
    #define FASTSTREAM_SEID         3                  /*!< @brief Local Stream End Point ID for FASTSTREAM codec */
#else
    #define FASTSTREAM_SEID         2                  /*!< @brief Local Stream End Point ID for FASTSTREAM codec */
#endif    

#define NUM_SEPS              (FASTSTREAM_SEID)  /*!< @brief The total number of SEPs */
#define NUM_OPTIONAL_CODECS   (NUM_SEPS-1)

#define a2dpSeidIsSbc(seid)           (seid == SBC_SEID)
#define a2dpSeidIsFaststream(seid)    (seid == FASTSTREAM_SEID)



/* The bits used to enable codec support for A2DP, as read from PSKEY_CODEC_ENABLED
   any other codecs can be added and supported for A2DP
*/
#define MP3_CODEC_BIT           0                  /*!< @brief Bit used to enable MP3 codec in PSKEY_CODEC_ENABLED */
#define FASTSTREAM_CODEC_BIT    1                  /*!< @brief Bit used to enable FASTSTREAM codec in PSKEY_CODEC_ENABLED */

/* ------------------------------------------------------------------------ */
/* Determines how often encryption keys are refreshed */

#define EPR_TIMEOUT                0x0F               /*!< @brief EPR Interval */

/* ------------------------------------------------------------------------ */
/* Enums and structs */

/*! @brief Audio Codec Type Bitmask */
typedef enum 
{
    audio_codec_none = 0,
    audio_codec_cvsd = 1,
    audio_codec_auristream_2_bit = 2,
    audio_codec_auristream_4_bit = 4
} audio_codec_type ;


/* Only a max of 16 unique commands can exist in this enum */
typedef enum
{
    HidCmdNop,
    HidCmdAnswer,
    HidCmdHangUp,
    HidCmdPlay,
    HidCmdPause,
    HidCmdPlayPause,
    HidCmdResume,
    HidCmdStop,
    HidCmdNext,
    HidCmdPrev,
    HidCmdReject,
    HidCmdCancel,
    HidCmdUnused1,
    HidCmdUnused2,
    HidCmdUnused3,
    HidCmdSequence
} mvdHidCommand;


typedef enum
{
    AppEventUiShortPress,
    AppEventUiLongPress,
    
    AppEventMicActive,
    AppEventMicInactive,
    AppEventSpeakerActive,
    AppEventSpeakerInactive,
    
    AppEventA2dpLinkLost,
    AppEventA2dpLinkResumed,
    AppEventA2dpLinkDisconnected,
    AppEventA2dpStreamingStarted,
    AppEventA2dpStreamingSuspended,
    
    AppEventHfpLinkLost,
    AppEventHfpLinkResumed,
    AppEventHfpLinkDisconnected,
    AppEventHfpAudioConnected,
    AppEventHfpAudioDisconnected,
    AppEventHfpCallOpened,
    AppEventHfpCallClosed,
    AppEventHfpAta,
    AppEventHfpChup,
    AppEventHfpAtd,
    AppEventHfpBldn,
    AppEventHfpBvra,
    
    AppEventAvrcpLinkLost,
    AppEventAvrcpLinkResumed,
    AppEventAvrcpLinkDisconnected,
    AppEventAvrcpPlay,
    AppEventAvrcpPause,
    AppEventAvrcpStop,
    AppEventAvrcpPrev,
    AppEventAvrcpNext,
    
    MaxAppEvents
} mvdAppEvent;


typedef enum
{
    HostNotifyCallAnswer,
    HostNotifyCallReject,
    HostNotifyCallCancel,
    HostNotifyCallTerminate
} mvdHostNotification;


typedef enum
{
    StateLedIdle,
    StateLedPhase,
    StateLedMark,
    StateLedSpace,
    StateLedDelay
} mvdLedState;


typedef struct
{
    uint16 phase_time;
    uint16 mark_time;
    uint16 space_time;
    uint16 delay_time;
    uint16 repeat_count;
    uint16 repeats;
    uint16 pio_lines;
    mvdLedState state;
} mvdLedConfig;


#define PS_PIN_CODE_LIST_MAX_SIZE  4
#define PS_BATTERY_CONFIG_SIZE     5 

#define PS_REMOTE_DEVICE_LIST_MAX_ENTRIES 5
#define PS_REMOTE_DEVICE_LIST_ELEMENT_SIZE 4
#define PS_REMOTE_DEVICE_LIST_MAX_SIZE (PS_REMOTE_DEVICE_LIST_MAX_ENTRIES * PS_REMOTE_DEVICE_LIST_ELEMENT_SIZE)
#define PS_HID_CONFIG_SIZE     8
#define PS_HID_MAPPING_SIZE    (MaxAppEvents)
#define PS_LED_CONFIG_ELEMENT_SIZE 2
#define PS_LED_CONFIG_KEY_SIZE (PS_LED_CONFIG_ELEMENT_SIZE * MaxAppStates)
#define PS_HID_SEQUENCE_MAX_SIZE 30

typedef struct
{
    uint16 play;
    uint16 pause;
    uint16 playpause;
    uint16 stop;
    uint16 prev;
    uint16 next;
    uint16 answer;
    uint16 hangup;
} mvdHidConfig;


typedef enum
{
    ProfileNone   = 0x00,
    ProfileAghsp  = 0x01,
    ProfileAghfp  = 0x02,
    ProfileA2dp   = 0x04,
    ProfileAvrcp  = 0x08,
    ProfileAll    = 0x0f
} mvdProfiles;


typedef struct
{    
    int16        path_loss;
    mvdProfiles  profiles:4;
    unsigned     profiles_complete:1;
} mvdEirData;


typedef struct
{
    uint16             read_idx;
    uint16             write_idx;
    bdaddr             buffer[INQUIRY_SCAN_BUFFER_SIZE];
    mvdEirData         eir_data[INQUIRY_SCAN_BUFFER_SIZE];
} mvdInquiryScanData;


typedef enum
{
    CommActionNone,                 /* Do nothing */
    CommActionCall,                 /* Create a call to either last/current remote device - a SLC will be created if necessary */
    CommActionEndCall,              /* Closes an existing call and returns to Connected state */
    CommActionStream,               /* Streams audio, via A2DP, to either last/current remote device */
    CommActionEndStream,            /* Stops streaming and returns to Connected state */
    CommActionConnect,              /* Creates an SLC connection with last remote device - a call will be created if audio exists */
    CommActionDisconnect,           /* Disconnect from all remote devices - any existing call will be ended */
    CommActionInquire,              /* Searches for a suitable remote device and connects to it */
    CommActionDiscover,             /* Discovery from any current device then issues a CommActionInquire */
    CommActionDiscoverWhileConnected/* Discovery from any current device then issues a CommActionInquire. Doesn't need to disconnect before doing discovery. */
} mvdCommAction;


typedef enum
{
    EncoderNone,
    EncoderAv,
    EncoderAnalog,
    EncoderSco,
    EncoderWbs
} mvdCodecMode;


typedef enum
{
    CodecSbc = 0,
    CodecMp3,
    CodecFaststream,
    CodecFaststream_bd
} mvdCodecType;


/* For Analogue mode, only two streaming state exit: 
   Pending and active, no active state     
*/
typedef enum
{
    StreamingInactive,      /* No audio is being streamed over USB */
    StreamingPending,       /* No audio is being streamed over USB but audio link to headset remains open */
    StreamingActive         /* Audio being streamed over USB */
    
} mvdStreamingState;


typedef enum
{
    SourceUsb = 0,
    SourceAnalog
} a2dpSouceType;


/*! @brief Source codec data. */
/* This struct must match with the codec_data_type in a2dp.h. We've just missed off the last uint16 which isn't needed here */
typedef struct
{
    unsigned source_type: 4;
    unsigned reserved:4;
    uint8 content_protection;
    uint32 voice_rate;
    unsigned bitpool:8;
    unsigned format:8;
    uint16 packet_size;
    Sink media_sink_b;
} source_codec_data_type;


typedef struct
{
    bool connect_sink;
    Sink media_sink;
} dualstream_mode_params;


/*! @brief A2DP Specific data */
typedef struct
{
    device_sep_list *sep_entries;       /*!< The Stream End Point (SEP) pointer */
    source_codec_data_type codecData;     /*!< The audio codec related data */
} a2dpData;


/*! @brief Definition of the battery monitoring configuration */
typedef struct
{
    unsigned unused:16;

    unsigned low_threshold:8 ;
    unsigned shutdown_threshold:8 ;  
    
    unsigned level_1_threshold:8 ;
    unsigned level_2_threshold:8 ;
    
    unsigned monitoring_period:8 ;
    unsigned unused2:8;
    
    /* the time period at which the low batt tone is played */
    unsigned low_batt_tone_period:16 ;

} battery_config_type;


/*! @brief Definition of the power configuration */
typedef struct
{
    battery_config_type battery;
} power_config_type;


/*! @brief Task to receive AIO readings */
typedef struct
{
    TaskData        task;
    BatteryState    state;
    uint16          current_reading;
} aioTask;


/*! @brief Enumeration of Charger states */
typedef enum
{
    disconnected,       /*!< Charger is disconnected */
    trickle_charge,     /*!< Charger is trickle charging */
    fast_charge,        /*!< Charger is fast charging */
    charge_error        /*!< Charger error */
} charging_state_type;


/*! @brief Definition of the power state */
typedef struct
{
    power_config_type   config;     /*!< Configuration */
    aioTask             vbat_task;  /*!< Vbat */
} power_type;


typedef struct devInstanceData
{
    TaskData        task;      /*!< Task */
    
    bdaddr          bd_addr;        /*!< Bluetooth Address of remote device */

    A2DP            *a2dp;          /*!< A2DP profile library instance */
    AVRCP           *avrcp;         /*!< AVRCP profile library instance */
    AGHFP           *aghfp;         /*!< AGHFP profile library instance */
    
    Sink            aghfp_sink;
    Sink            audio_sink;
    Sink            a2dp_sig_sink;
    Sink            a2dp_media_sink;
    
    uint8           a2dp_seid;    
    
    uint16          start_pin_idx;
    uint16          pin_idx;
    uint16          a2dp_closing;
    
    mvdA2dpState    a2dp_state:4;
    mvdAvrcpState   avrcp_state:3; 
    mvdAghfpState   aghfp_state:4;
    mvdProfiles     remote_profiles:4;        /* Which profiles remote device supports */
    unsigned        connect_timer_expired:1;
    
    mvdProfiles     responding_profiles:4; 
    unsigned        pin_requested:1;
    unsigned        pin_authorised:1;
    unsigned        pin_wrapped:1;
    unsigned        pin_list_exhausted:1;    
    unsigned        paired_list_read:1;    
    unsigned        available_profiles_connected:1;
    unsigned        a2dp_reopen:1; 
    unsigned        role:2; 
    unsigned        a2dp_reopen_tries:3;
    
    unsigned        a2dp_audio_quality:3;
    unsigned        a2dp_reopen_codec:3; 
    unsigned        unused:10;
    
    unsigned        sbc_min_bitpool:8; 
    unsigned        sbc_max_bitpool:8;
    
} devInstanceTaskData;  

/* PBAP struct */

/* Minimum buffer size to allow */
#define PBAPS_MIN_BUFFER_SIZE 60

/* Buffer size to try and allocate first */
#define PBAPS_BUFFER_START_SIZE 0x6c

/* PBAP Server State */
typedef struct
{
	/* PBAP Server Library instance */
	PBAPS *pbaps;
	/* Maximum packet size */
	uint16 pktSize;
} pbapStateData;

/* Folder state data */
typedef struct
{
	/* Current folder */
	pbap_phone_book current;
	/* Supported Phonebooks bit field */
	unsigned int supBooks:6;
	/* Send current entry.  Used when there is not enough space for the current entry in the data packet */
	unsigned int sentCurrent:1;
	/* Got all entries, but footer not sent yet */
	unsigned int sendFooter:1;
	/* Amount of the vCard Listing header still to send */
	uint16 listLeft;
} folderStateData;

/* Phonebook access state data */
typedef struct
{
	/* Is there an open phonebook? */
	unsigned int open:1;
	/* Examined and used the first entry */
	unsigned int usedFirst:1;
	/* Source of the open phonebook */
	Source bookSrc;
	/* Current Entry */
	uint16 entry;
} phonebookStateData;

/* Phonebook search Data */
typedef struct
{
	pbap_search_values srchAttr;
	uint8 *srchVal;
	uint16 maxList;
	uint16 count;
} phonebookSearchData;

/* Buffer structure */
typedef struct
{
	uint8 *buffer;
	uint16 sizeBuffer;
	uint16 freeSpace;
	uint16 nextPos;
	uint16 used;
} phonebookBuffer;

/* vCard Generation data */
typedef struct
{
	pbap_format_values format;
	uint16 maxList;
	unsigned int sentHeader:1;
	unsigned int sentFooter:1;
	unsigned int sentName:1;
	unsigned int sentPhone:1;
	unsigned int sentMobile:1;
	unsigned int sentBusiness:1;
} phonebookvCardGenData;


typedef struct
{
    TaskData        task;
    TaskData        audio_control;
    TaskData        audio_streaming;    
    
    Task            codecTask;                      /*!< Codec task */
    Task            a2dp_audio_plugin;              /*!< A2DP audio plugin */
    Task            aghfp_audio_plugin;             /*!< AGHFP SCO plugin */
    
    AGHFP           *aghsp;                /* Instance of AG using HSP  */
    AGHFP           *aghfp;                /* Instance of AG using HFP  */
                                                      
    a2dpData        a2dp_data;                      /*!< Local A2DP data */
    
    power_type      *power;                         /*!< Pointer to headset power configuration */
    
    devInstanceTaskData *dev_inst[MAX_NUM_DEV_CONNECTIONS];

	char			pin[8];
    bdaddr          search_bdaddr;
    bdaddr          connect_bdaddr;
    
    AUDIO_SINK_T    sink_type;
    
    PioState        pio_states;
    
    aghfp_call_type call_type;
    
    mvdHidConfig    hid_config;
    
    uint32          a2dp_sample_rate;               /*!< A2DP Sample Rate */

	uint16			vgs;
	uint16			vgm;
    
    uint16          speaker_volume;        /* The value we got in the most recent AGHFP_SYNC_SPEAKER_VOL_IND */
    uint16          pskey_sbc_codec_config;
    uint16          pskey_mp3_codec_config;
    uint16          pskey_faststream_codec_config;
    
    uint16          local_supported_features[4];
    
    mvdAppState     app_state:4;
    unsigned        bidirect_faststream:1;    
    unsigned        clearing_pdl:1;
    unsigned        a2dp_channel_mode:4;            /*!< A2DP Channel Mode */
    unsigned        a2dpCodecsEnabled:4;            /*!< Enabled Codecs for A2DP */
    
    signed          inquiry_tx:8;                    /* Inquiry Tx Power of local device */
    unsigned        remote_profiles_complete:1;        /* Remote Profile list complete */
    unsigned        connect_attempts: 3;            /* All profiles connection attempts  */
    unsigned        s_connect_attempts: 2;          /* Single profile connection attempts */
    unsigned        audioAdaptorPoweredOn:1;        /* Audio adaptor has been powered on */
    unsigned        PowerOffIsEnabled:1;            /* Enable poweroff the audio adaptor */
        
    charging_state_type charger_state:2;            /*!< The current charger state */
    unsigned        low_battery_warning:1;            /*!< Flag to indicate if to send the low battery warning */
    unsigned        initial_battery_reading:1;      /*!< Flag to indicate if the initail battery reading is being taken */
    unsigned        chargerconnected: 1;            /*!< Flag to indicate if charger is connected */
    mvdCodecMode    active_encoder:3;        
    unsigned        auto_connect_in_progress:1;  /* Dongle is attempting to connect to one of the devices in the recent device list */
    unsigned        disconnect_requested:1;
    unsigned        dfu_requested:1;
    unsigned        voip_call_active:1;
    mvdStreamingState   audio_streaming_state:2;
    unsigned         a2dp_media_stream_holdoff:1;
    unsigned         a2dp_media_stream_requested:1;
    
    unsigned         app_inquiry_timer_expired:1;
    unsigned         app_reconnect_timer_expired:1;
    unsigned         inquiring:1;
    
    mvdCommAction    comm_action:4;            /* Connect/disconnect action being performed */
    mvdCommAction    pending_comm_action:4;    /* queued comm action */
    mvdProfiles      initialised_profiles:4;    /* Which profiles (aghsp/aghfp/a2dp/avrcp) have been initialised */
    mvdProfiles      supported_profiles:4;        /* Which profiles (aghsp/aghfp/a2dp/avrcp) local device supports */
    
    mvdProfiles      remote_profiles:4;        /* Which profiles (hsp/hfp) remote device supports */
    mvdProfiles      connecting_profile:4;     /* Profile being connected by local device */
    unsigned         a2dp_active_seid:8;
	unsigned		 waiting_msg:1;
/* PBAP */

	pbap_states appState;

	pbapStateData pbapData;
	folderStateData folderData;
	phonebookStateData phonebookData;

	phonebookBuffer buffer;
	phonebookSearchData srchData;
	phonebookvCardGenData vCardData;

	uint16		pb_index;

/* SMS */
	char 			*message_body;
	char			*message_sender;
} mvdTaskData;


/* ------------------------------------------------------------------------ */
/* Application messages */

#define APP_MESSAGE_BASE 0x5000
enum
{
    APP_INIT = APP_MESSAGE_BASE,
    APP_INIT_CFM,    
    APP_POWERON_REQ,            /* power on the audio adaptor automatically after being initialised or power on after powering off */
    APP_POWEROFF_REQ,           /* power off the kalimba and enter the connectable state */
    APP_LIMBO_TIMEOUT,
    APP_RESET_PDL_REQ,          /* delete the PDL */ 
    APP_CHARGER_CONNECTED,
    APP_CHARGER_DISCONNECTED,
    APP_CHARGER_MONITOR,
    APP_CHECK_FOR_LOW_BATT,
    APP_LOW_BATTERY,
    APP_FAST_CHARGE,
    APP_TRICKLE_CHARGE,
    APP_CANCEL_LED_INDICATION,
    APP_OK_BATTERY,    
    APP_KICK_COMM_ACTION_REQ,    /* Kick comm action via a message rather than direct call */    
    APP_REFRESH_ENCRYPTION_REQ,    
    APP_DEVICE_CONNECT_REQ,
    APP_DEVICE_DISCOVER_REQ,
    APP_DEVICE_CONNECT_SECOND_REQ,    
    APP_ENTER_DFU_MODE,    
    APP_VOIP_CALL_INCOMING,    /* Incoming VoIP call is being set up */
    APP_VOIP_CALL_OUTGOING,    /* Outgoing VoIP call is being set up */
    APP_VOIP_CALL_CANCEL,      /* VoIP call setup has been cancelled */
    APP_VOIP_CALL_ACTIVE,      /* Active VoIP call - used to transfer an existing call or answer a call being set up */
    APP_VOIP_CALL_INACTIVE,    /* VoIP call is no longer active - will terminate an existing call or cancel one being set up */
    APP_AUDIO_STREAMING_ACTIVE,
    APP_AUDIO_STREAMING_INACTIVE,
    APP_AUDIO_STREAMING_TIMER,    
    APP_A2DP_MEDIA_STREAM_HOLDOFF,    
    APP_INQUIRY_TIMER,
    APP_CONNECT_TIMER,
    APP_RECONNECT_TIMER,
    APP_PROFILE_CONNECT_TIMER,    
    APP_LED_TIMER,    
    APP_HID_SEQUENCE,
    APP_INTERNAL_DESTROY_REQ,    
    APP_STREAM_CLOSE_COMPLETE,    
    APP_INTERNAL_CONNECT_MEDIA_CHANNEL_REQ,    
    APP_MEDIA_CHANNEL_REOPEN_REQ,    
    APP_CONNECT_A2DP_AUDIO,
    APP_PROCESS_INQUIRE_RESULT,
    APP_CONNECT_CFM,

	APP_MSG_TIMEOUT,
    
    APP_MESSAGE_TOP
};

typedef struct
{
    mvdCommAction comm_action;
} APP_KICK_COMM_ACTION_REQ_T;

typedef struct
{
    uint16 id;
    uint32 ticks;
} APP_LED_TIMER_T;

typedef struct
{
    bool status;
} APP_CALL_OPEN_CFM_T;

typedef struct
{
    bool status;
} APP_CALL_CLOSE_IND_T;

typedef struct
{
    bool status;
} APP_CALL_CLOSE_CFM_T;

typedef struct
{
    bool status;
} APP_STREAM_OPEN_CFM_T;

typedef struct
{
    bool status;
} APP_STREAM_CLOSE_CFM_T;

typedef struct
{
    bool disconnect_current;
} APP_DEVICE_CONNECT_REQ_T;    

typedef struct
{
    bool disconnect_current;
} APP_DEVICE_DISCOVER_REQ_T;

typedef struct
{
    Sink media_sink;
} APP_CONNECT_A2DP_AUDIO_T;

typedef struct
{
    bool success;
} APP_CONNECT_CFM_T;

/* ------------------------------------------------------------------------ */
/* Helper defines for messages */

#define MAKE_APP_MESSAGE(TYPE) TYPE##_T *message = malloc(sizeof(TYPE##_T));

#define A2DP_MESSAGE(id) (id>=A2DP_MESSAGE_BASE && id<A2DP_MESSAGE_TOP)
#define AVRCP_MESSAGE(id) (id>=AVRCP_MESSAGE_BASE && id<AVRCP_MESSAGE_TOP)
#define AGHFP_MESSAGE(id) (id>=AGHFP_MESSAGE_BASE && id<AGHFP_MESSAGE_TOP)
#define PBAP_MESSAGE(id) (id>=PBAPS_MESSAGE_BASE && id<PBAPS_MESSAGE_TOP)
#define SYNC_MESSAGE(id) (id>=SYNCS_MESSAGE_BASE && id<SYNCS_MESSAGE_TOP)
#define SPP_MESSAGE(id)	(id>=SPP_MESSAGE_BASE && id<SPP_MESSAGE_TOP)
#define APP_MESSAGE(id) (id>=APP_MESSAGE_BASE && id<APP_MESSAGE_TOP)
#define CL_MESSAGE(id) (id>=CL_MESSAGE_BASE && id<CL_MESSAGE_TOP)
#define CODEC_MESSAGE(id) (id >= CODEC_MESSAGE_BASE ) && (id <= CODEC_MESSAGE_TOP)
#define SYSTEM_MESSAGE(id) (id>=MESSAGE_FROM_HOST && id<= MESSAGE_CHARGER_CHANGED)

/* ------------------------------------------------------------------------ */
/* The task data structure - visible to all (so no need to pass it between functions) */

extern mvdTaskData *the_app ;


#endif
