/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    hfp_private.h
    
DESCRIPTION
	
*/

#ifndef HFP_PRIVATE_H_
#define HFP_PRIVATE_H_

#include "hfp.h"

#include <connection.h>
#include <message.h>
#include <app/message/system_message.h>
#include <stdlib.h>



/* Macros for creating messages */
#define MAKE_HFP_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_HFP_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);
#define COPY_HFP_MESSAGE(src, dst) *dst = *src;

/* Macro used to generate debug lib printfs */
#ifdef HFP_DEBUG_LIB
#include <panic.h>
#include <stdio.h>
#define HFP_DEBUG(x) {printf x;  Panic();}
#define checkHfpProfile(x) {if (!supportedProfileIsHfp(x)) Panic();}
#define checkHfpProfile15(x) {if (!supportedProfileIsHfp15(x)) Panic();}
#else
#define HFP_DEBUG(x)
#define checkHfpProfile(x)
#define checkHfpProfile15(x)
#endif

/* Constants used by the HFP profile lib. */
#define AT_RESPONSE_TIMEOUT		(5000)

/* Supported features bit masks - HFP AG */
#define AG_THREE_WAY_CALLING        (1)
#define AG_NREC_FUNCTION            (1<<1)
#define AG_VOICE_RECOGNITION        (1<<2)
#define AG_IN_BAND_RING             (1<<3)
#define AG_VOICE_TAG                (1<<4)
#define AG_REJECT_CALL              (1<<5)
#define AG_ENHANCED_CALL_STATUS     (1<<6)
#define AG_ENHANCED_CALL_CONTROL    (1<<7)
#define AG_EXTENDED_ERROR_CODES     (1<<8)


/* Hfp Profile Library private messages */
#define HFP_MSG_BASE     (0x0)

enum
{
    /* Initialisation */
    HFP_INTERNAL_INIT_REQ = HFP_MSG_BASE,
	HFP_INTERNAL_INIT_CFM,
	HFP_INTERNAL_SDP_REGISTER_CFM,

	/* SLC connection */
	HFP_INTERNAL_SLC_CONNECT_REQ,
	HFP_INTERNAL_SLC_CONNECT_RES,
	HFP_INTERNAL_RFCOMM_CONNECT_REQ,
	HFP_INTERNAL_SLC_DISCONNECT_REQ,
	HFP_INTERNAL_SLC_DISCONNECT_IND,
	HFP_INTERNAL_GET_SINK_REQ,

	/* AT cmd receive */
	HFP_INTERNAL_WAIT_AT_TIMEOUT_IND,
	HFP_INTERNAL_AT_INDICATOR_LIST_IND,
	HFP_INTERNAL_AT_INDICATOR_STATUS_IND,
	HFP_INTERNAL_AT_CALL_HOLD_SUPPORT_IND,

	/* AT cmd send */
	HFP_INTERNAL_AT_BRSF_REQ,
	HFP_INTERNAL_AT_CIND_TEST_REQ,
	HFP_INTERNAL_AT_CIND_READ_REQ,
	HFP_INTERNAL_AT_CMER_REQ,
	HFP_INTERNAL_AT_CKPD_REQ,
	HFP_INTERNAL_ATA_REQ,
	HFP_INTERNAL_AT_CHUP_REJECT_REQ,
	HFP_INTERNAL_AT_CHUP_TERMINATE_REQ,
	HFP_INTERNAL_AT_BLDN_REQ,
	HFP_INTERNAL_AT_ATD_NUMBER_REQ,
	HFP_INTERNAL_AT_ATD_MEMORY_REQ,
	HFP_INTERNAL_AT_BVRA_REQ,
	HFP_INTERNAL_AT_VGS_REQ,
	HFP_INTERNAL_AT_VGM_REQ,
	HFP_INTERNAL_AT_CLIP_REQ,
	HFP_INTERNAL_AT_BINP_REQ,
	HFP_INTERNAL_AT_NREC_REQ,
	HFP_INTERNAL_AT_VTS_REQ,
	HFP_INTERNAL_AT_CCWA_REQ,
	HFP_INTERNAL_AT_CHLD_0_REQ,
	HFP_INTERNAL_AT_CHLD_1_REQ,
	HFP_INTERNAL_AT_CHLD_1X_REQ,
	HFP_INTERNAL_AT_CHLD_2_REQ,
	HFP_INTERNAL_AT_CHLD_2X_REQ,
	HFP_INTERNAL_AT_CHLD_3_REQ,
	HFP_INTERNAL_AT_CHLD_4_REQ,
    HFP_INTERNAL_AT_CNUM_REQ,
    HFP_INTERNAL_AT_BTRH_STATUS_REQ,
    HFP_INTERNAL_AT_BTRH_HOLD_REQ,
    HFP_INTERNAL_AT_BTRH_ACCEPT_REQ,
    HFP_INTERNAL_AT_BTRH_REJECT_REQ,
    HFP_INTERNAL_AT_CLCC_REQ,
    HFP_INTERNAL_AT_COPS_REQ,
    HFP_INTERNAL_AT_CMEE_REQ,
    
	/* Audio connection */
    HFP_INTERNAL_AUDIO_TRANSFER_REQ,
    HFP_INTERNAL_AUDIO_CONNECT_REQ,
    HFP_INTERNAL_AUDIO_CONNECT_RES,
    HFP_INTERNAL_AUDIO_DISCONNECT_REQ,
    
    /* CSR Extensions */
    HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ,
    HFP_INTERNAL_CSR_SUPPORTED_FEATURES_ACK,
    HFP_INTERNAL_CSR_POWER_LEVEL_REQ,
    HFP_INTERNAL_CSR_POWER_SOURCE_REQ,
    HFP_INTERNAL_CSR_MOD_INDS_REQ,
    HFP_INTERNAL_CSR_MOD_INDS_DISABLE_REQ,
    HFP_INTERNAL_CSR_GET_SMS_REQ,
    HFP_INTERNAL_CSR_AG_BAT_REQ,
    HFP_INTERNAL_CSR_FEATURE_NEGOTIATION_RES,
        
	/* Defined here so its not visible outside this lib */
	HFP_COMMON_CFM_MESSAGE
};


typedef struct
{
	hfp_init_status	status;
	uint8			rfcomm_channel;
} HFP_INTERNAL_INIT_CFM_T;


typedef struct
{
	hfp_lib_status	status;
} HFP_INTERNAL_SDP_REGISTER_CFM_T;


typedef struct
{
	bdaddr		addr;
	uint8		*extra_indicators;
} HFP_INTERNAL_SLC_CONNECT_REQ_T;


typedef struct
{
	bdaddr		addr;
	bool		response;
	uint8		*extra_indicators;
} HFP_INTERNAL_SLC_CONNECT_RES_T;


typedef struct
{
	bdaddr	addr;
	uint8	rfc_channel;
} HFP_INTERNAL_RFCOMM_CONNECT_REQ_T;

typedef struct
{
    hfp_disconnect_status   status;
} HFP_INTERNAL_SLC_DISCONNECT_IND_T;


typedef struct
{
	uint16	service;
	uint16	call;
	uint16	call_setup;
    uint16  signal_strength;
    uint16  roaming_status;
    uint16  battery_charge;
    uint16  call_hold_status;
} hfp_indicators;

typedef struct
{
    hfp_indicators indexes;
} HFP_INTERNAL_AT_INDICATOR_LIST_IND_T;


typedef struct
{
    hfp_indicators values;
} HFP_INTERNAL_AT_INDICATOR_STATUS_IND_T;



typedef struct
{
	uint16 length;
	uint8  *number;
} HFP_INTERNAL_AT_ATD_NUMBER_REQ_T;


typedef struct
{
	uint16 length;
	uint8  *memory;
} HFP_INTERNAL_AT_ATD_MEMORY_REQ_T;


typedef struct
{
	uint16 enable;
} HFP_INTERNAL_AT_BVRA_REQ_T;


typedef struct
{
	uint16 volume_gain;
} HFP_INTERNAL_AT_VGS_REQ_T;


typedef struct
{
	uint16 mic_gain;
} HFP_INTERNAL_AT_VGM_REQ_T;


typedef struct
{
	bool enable;
} HFP_INTERNAL_AT_CLIP_REQ_T;


typedef struct
{
	uint8 dtmf;
} HFP_INTERNAL_AT_VTS_REQ_T;


typedef struct
{
	bool enable;
} HFP_INTERNAL_AT_CCWA_REQ_T;


typedef struct
{
    uint16  call_idx;
} HFP_INTERNAL_AT_CHLD_1X_REQ_T;


typedef struct
{
    uint16  call_idx;
} HFP_INTERNAL_AT_CHLD_2X_REQ_T;


typedef struct
{
    hfp_audio_transfer_direction    direction;
    sync_pkt_type                   packet_type;
	hfp_audio_params                audio_params;
} HFP_INTERNAL_AUDIO_TRANSFER_REQ_T;

typedef struct
{
    sync_pkt_type       packet_type;
	hfp_audio_params    audio_params;
} HFP_INTERNAL_AUDIO_CONNECT_REQ_T;
 

typedef struct
{
	bool                response;
    sync_pkt_type       packet_type;
    hfp_audio_params    audio_params;
	bdaddr				bd_addr;
} HFP_INTERNAL_AUDIO_CONNECT_RES_T;


typedef struct
{
    uint16  length;
    uint8   data[1];
} HFP_INTERNAL_USER_DATA_REQ_T;

typedef struct
{
	bool callerName;
	bool rawText;
	bool smsInd;
	bool battLevel;
	bool pwrSource;
	uint16 codecs ;
} HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ_T;

typedef struct
{
	bool callerName;
	bool rawText;
	bool smsInd;
	bool battLevel;
	bool pwrSource;
	uint16 codecs ;
} HFP_INTERNAL_CSR_SUPPORTED_FEATURES_ACK_T;

typedef struct
{
	uint16 pwr_level;
} HFP_INTERNAL_CSR_POWER_LEVEL_REQ_T;

typedef struct
{
	hfp_csr_power_status_report pwr_status;
} HFP_INTERNAL_CSR_POWER_SOURCE_REQ_T;

typedef struct
{
	uint16 size_indicators;
	hfp_csr_mod_indicator *indicators;
} HFP_INTERNAL_CSR_MOD_INDS_REQ_T;

typedef struct
{
	uint16 index;
} HFP_INTERNAL_CSR_GET_SMS_REQ_T;


typedef struct
{
    uint16 indicator ;
    uint16 value ;
} HFP_INTERNAL_CSR_FEATURE_NEGOTIATION_RES_T ;

/* 
	Many messages returned from the HFP lib to the app have 
	the form of this message. By defining it here its not visible 
	outside this lib. This way we can use a common function to 
	allocate this message and just set the type when sending the
	message to the app. 
*/
typedef struct
{
    HFP				*hfp;
	hfp_lib_status	status;
} HFP_COMMON_CFM_MESSAGE_T;


typedef enum 
{
	hfpInitialising,
	hfpReady,
	hfpSlcConnecting,
	hfpSlcConnected,
	hfpIncomingCallEstablish,
	hfpOutgoingCallEstablish,
	hfpOutgoingCallAlerting,
	hfpActiveCall
} hfpState;


typedef enum
{
	hfpNoCmdPending,
	hfpCmdPending,
	hfpCkpdCmdPending,
	hfpBrsfCmdPending,
	hfpCmerCmdPending,
	hfpAtaCmdPending,
	hfpChupCmdPending,
	hfpBldnCmdPending,
	hfpBvraCmdPending,
	hfpVgsCmdPending,
	hfpVgmCmdPending,
	hfpClipCmdPending,
	hfpAtdNumberCmdPending,
	hfpAtdMemoryCmdPending,
	hfpBinpCmdPending,
	hfpNrecCmdPending,
	hfpVtsCmdPending,
	hfpCcwaCmdPending,
	hfpChldZeroCmdPending,
	hfpChldOneCmdPending,
	hfpChldOneIdxCmdPending,
	hfpChldTwoCmdPending,
	hfpChldTwoIdxCmdPending,
	hfpChldThreeCmdPending,
	hfpChldFourCmdPending,
	hfpCnumCmdPending,
	hfpBtrhReqCmdPending,
	hfpBtrhZeroCmdPending,
	hfpBtrhOneCmdPending,
	hfpBtrhTwoCmdPending,
	hfpCopsFormatCmdPending,
	hfpCopsReqCmdPending,
	hfpCmeeCmdPending,
	hfpClccCmdPending,
	hfpBcsCmdPending,
	hfpBlhCmdPending,
	hfpCsrSfPending,
	hfpCsrBatPending,
	hfpCsrPwrPending,
	hfpCsrModIndPending,
	hfpCsrGetSmsPending
} hfp_at_cmd;


typedef struct
{
    hfp_indicators indexes;
	uint16		call;
	uint16		extra_inds_enabled;
	uint8		*extra_indicators;
} hfp_indicators_status;


typedef enum
{
    hfp_audio_disconnected,
    hfp_audio_connecting_esco,
    hfp_audio_connecting_sco,
    hfp_audio_accepting,
    hfp_audio_disconnecting,
    hfp_audio_connected
} hfp_audio_connect_state;


typedef enum
{
	hfp_sdp_search_none,
	hfp_sdp_search_rfcomm_channel,
	hfp_sdp_search_profile_version,
	hfp_sdp_search_supported_features
} hfp_sdp_search_mode;


struct __HFP
{
	TaskData				task;
	Task					clientTask;
	hfpState				state;
	hfp_profile				hfpSupportedProfile;
	hfp_profile				agSupportedProfile;
	uint16					hfpSupportedFeatures;
	uint16					agSupportedFeatures;
	Sink					sink;
    Sink                    audio_sink;
    hfp_audio_connect_state audio_connection_state;
    sync_pkt_type           audio_packet_type;
    sync_pkt_type           audio_packet_type_to_try;
    hfp_audio_params        audio_params;
	uint32					sdp_record_handle;
    uint16                  size_service_record;
	uint8					local_rfc_server_channel;
	uint8					*service_record;
	hfp_at_cmd				at_cmd_resp_pending;
	hfp_indicators_status	indicator_status;	
	uint16					vol_setting;
    uint16                  caller_id_name_sent;
    hfp_sdp_search_mode		sdp_search_mode;
	uint16					rfcomm_lock;

    int						cops_format_set:1;
    int use_csr2csr:1; /* CSR 2 CSR has been initialised */
    
};


#endif /* HFP_PRIVATE_H_ */
