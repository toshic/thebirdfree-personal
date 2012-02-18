/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    connection_private.h
    
DESCRIPTION
    This file contains the type definitions and function prototypes for the 
    Connection Library
*/

#ifndef	CONNECTION_PRIVATE_H_
#define	CONNECTION_PRIVATE_H_

/* Header files */
#include <stdlib.h>


#include <app/message/system_message.h>
#include <message.h>
#include <panic.h>

#include "common.h"
#include "connection_manager.h"
#include "init.h"


/* Macros used to make messsage primitive creation simpler */
#define MAKE_PRIM_C(TYPE) TYPE##_T *prim = PanicUnlessNew(TYPE##_T); prim->common.op_code = TYPE; prim->common.length = sizeof(TYPE##_T);
#define MAKE_PRIM_T(TYPE) TYPE##_T *prim = PanicUnlessNew(TYPE##_T); prim->type = TYPE;

#define MAKE_CL_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_CL_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);
#define COPY_CL_MESSAGE(src, dst) *dst = *src;
#define COPY_CL_MESSAGE_WITH_LEN(TYPE, LEN, src, dst) memcpy(dst, src, sizeof(TYPE##_T) + LEN);

#define UNUSED(v) v=v;

/* Macro used to generate debug lib printfs. */
#ifdef CONNECTION_DEBUG_LIB
#include <panic.h>
#include <stdio.h>
#define CL_DEBUG(x)		{printf x;  Panic();}
#define checkStatus(x)	{ DM_HCI_STANDARD_COMMAND_COMPLETE_T *cfm = (DM_HCI_STANDARD_COMMAND_COMPLETE_T *) x; if (cfm->status != HCI_SUCCESS) CL_DEBUG(("Ignored result prim reports error: 0x%x\n", cfm->status)); }
#else
#define CL_DEBUG(x)
#define checkStatus(x)
#endif

/* Parameter checking limits for use by debug build */
#ifdef CONNECTION_DEBUG_LIB
#define RFCOMM_SERVER_CHANNEL_MIN       (1)
#define RFCOMM_SERVER_CHANNEL_MAX       (30)
#define MIN_AUTHENTICATION_TIMEOUT      (60)
#define MAX_AUTHENTICATION_TIMEOUT      (600)
#define MIN_WRITE_IAC_LAP				(1)
#define MAX_WRITE_IAC_LAP				(4)
#define MIN_TX_POWER					(-70)
#define MAX_TX_POWER					(20)
#endif

/* Passed to MessageSend functions if the message being sent does not have a payload defined. */
#define NO_PAYLOAD      (0x0)

/* Invalid L2CAP cid. */
#define L2CA_CID_INVALID	            (0x0000)

/* Max length of the name in change local name or read remote name requests. */
#define	 MAX_NAME_LENGTH		(31)

/* As we store the authentication requirements in HCI format we need to define 
   our own unknown bit to use internally */
#define AUTH_REQ_UNKNOWN 0x08

/* Connection Library private messages */
#define CM_MSG_BASE     (0x0)

enum
{
    /* Initialisation */
    CL_INTERNAL_INIT_TIMEOUT_IND = CM_MSG_BASE,
    CL_INTERNAL_INIT_REQ,
    CL_INTERNAL_INIT_CFM,

    /* Inquiry Entity */
    CL_INTERNAL_DM_INQUIRY_REQ,
	CL_INTERNAL_DM_INQUIRY_CANCEL_REQ,
	CL_INTERNAL_DM_READ_REMOTE_NAME_REQ,
	CL_INTERNAL_DM_READ_LOCAL_NAME_REQ,
	CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ,
	CL_INTERNAL_DM_READ_INQUIRY_TX_REQ,
	CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ,
	CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ,
    CL_INTERNAL_DM_WRITE_EIR_DATA_REQ,
    CL_INTERNAL_DM_READ_EIR_DATA_REQ,
	CL_INTERNAL_DM_EN_ACL_DETACH_REQ,

    /* Security Entity */
	CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ,
    CL_INTERNAL_SM_AUTHENTICATION_REQ,
	CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ,
    CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND,
    CL_INTERNAL_SM_SET_SC_MODE_REQ,
	CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ,
	CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ,
    CL_INTERNAL_SM_REGISTER_REQ,
    CL_INTERNAL_SM_REGISTER_OUTGOING_REQ,
    CL_INTERNAL_SM_UNREGISTER_REQ,
    CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ,
    CL_INTERNAL_SM_ENCRYPT_REQ,
	CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ,
    CL_INTERNAL_SM_AUTHORISE_RES,
    CL_INTERNAL_SM_PIN_REQUEST_RES,
	CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES,
	CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES,
	CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES,
	CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ,
    CL_INTERNAL_SM_DELETE_AUTH_DEVICE_REQ,
	CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ,
	CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ,
    CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ,

	/* Baseband Entity */
	CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ,
	CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ,
	CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ,
	CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ,
	CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ,
	CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ,
	CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ,
	CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ,
	CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ,
	CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ,
	CL_INTERNAL_DM_WRITE_IAC_LAP_REQ,
	CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ,

	/* Informational Entity*/
	CL_INTERNAL_DM_READ_BD_ADDR_REQ,
	CL_INTERNAL_DM_READ_LINK_QUALITY_REQ,
	CL_INTERNAL_DM_READ_RSSI_REQ,
	CL_INTERNAL_DM_READ_CLK_OFFSET_REQ,
	CL_INTERNAL_DM_SET_BT_VERSION_REQ,
	CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ,
	CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ,
	CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ,

	/* SDP Entity */
	CL_INTERNAL_SDP_REGISTER_RECORD_REQ,
	CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ,
	CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ,
	CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ,
	CL_INTERNAL_SDP_OPEN_SEARCH_REQ,
	CL_INTERNAL_SDP_CLOSE_SEARCH_REQ,
	CL_INTERNAL_SDP_SERVICE_SEARCH_REQ,
	CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ,
	CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ,
	CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ,

	/* L2CAP Connection Management Entity */
	CL_INTERNAL_L2CAP_REGISTER_REQ,
	CL_INTERNAL_L2CAP_UNREGISTER_REQ,
	CL_INTERNAL_L2CAP_CONNECT_REQ,
	CL_INTERNAL_L2CAP_CONNECT_RES,
	CL_INTERNAL_L2CAP_DISCONNECT_REQ,
	CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND,
    CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP,

    /* RFCOMM Connection Management Entity */
    CL_INTERNAL_RFCOMM_REGISTER_REQ,
    CL_INTERNAL_RFCOMM_CONNECT_REQ,
    CL_INTERNAL_RFCOMM_CONNECT_RES,
    CL_INTERNAL_RFCOMM_DISCONNECT_REQ,
    CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND,
    CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND,
    CL_INTERNAL_RFCOMM_CONTROL_REQ,

	/* Synchronous Connection Entity */
	CL_INTERNAL_SYNC_REGISTER_REQ,
	CL_INTERNAL_SYNC_UNREGISTER_REQ,
	CL_INTERNAL_SYNC_CONNECT_REQ,
	CL_INTERNAL_SYNC_CONNECT_RES,
	CL_INTERNAL_SYNC_DISCONNECT_REQ,
	CL_INTERNAL_SYNC_RENEGOTIATE_REQ,
    CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND,
    CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND,

    /* Link Policy Management Entity */
	CL_INTERNAL_DM_SET_ROLE_REQ,
	CL_INTERNAL_DM_GET_ROLE_REQ,
	CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ,
	CL_INTERNAL_DM_SET_LINK_POLICY_REQ,
	CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ,
    CL_INTERNAL_DM_SET_ROLE_SWITCH_PARAMS_REQ,

	/* DUT support */
	CL_INTERNAL_DM_DUT_REQ,
	
	/* Attribute read from PS */
	CL_INTERNAL_SM_GET_ATTRIBUTE_REQ,
	CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ
};


typedef struct
{
    connectionInitMask   mask;
} CL_INTERNAL_INIT_CFM_T;


typedef struct
{
    Task    theAppTask;
    uint32 inquiry_lap;
    uint8   max_responses;
    uint16  timeout;
    uint32  class_of_device;
} CL_INTERNAL_DM_INQUIRY_REQ_T;


typedef struct
{
	Task theAppTask;
} CL_INTERNAL_DM_INQUIRY_CANCEL_REQ_T;


typedef struct
{
	Task theAppTask;
	bdaddr bd_addr;
} CL_INTERNAL_DM_READ_REMOTE_NAME_REQ_T;


typedef struct
{
	Task theAppTask;
} CL_INTERNAL_DM_READ_LOCAL_NAME_REQ_T;


typedef struct
{
	Task theAppTask;
	int8 tx_power;
} CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ_T;


typedef struct
{
	Task theAppTask;
} CL_INTERNAL_DM_READ_INQUIRY_TX_REQ_T;


typedef struct
{
	Task theAppTask;
	inquiry_mode mode;
} CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ_T;

typedef struct
{
	uint8 fec_required;
	uint8 size_eir_data;
	uint8 *eir_data; 
} CL_INTERNAL_DM_WRITE_EIR_DATA_REQ_T;

typedef struct
{
	Task task;
} CL_INTERNAL_DM_READ_EIR_DATA_REQ_T;

typedef struct
{
	Task theAppTask;
} CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ_T;

typedef struct
{
	bdaddr bd_addr;
	uint8 reason;
} CL_INTERNAL_DM_EN_ACL_DETACH_REQ_T;

typedef struct
{
    Task task;
    bdaddr bd_addr;
    uint32  timeout;
} CL_INTERNAL_SM_AUTHENTICATION_REQ_T;

typedef struct
{
    Task task;
} CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ_T;

typedef struct
{
    Task task;
    bool force;
} CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ_T;


typedef struct
{
    Task theAppTask;
    bdaddr bd_addr;
} CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND_T;


typedef struct
{
    dm_security_level	secl_default;
} CL_INTERNAL_SM_SET_DEFAULT_SECURITY_REQ_T;


typedef struct
{
    Task                theAppTask;
    dm_security_mode  	mode;
    encryption_mode		mode3_enc;
} CL_INTERNAL_SM_SET_SC_MODE_REQ_T;


typedef struct
{
	dm_protocol_id 			protocol_id;
	uint32 					channel;
	uint32 					psm;
    dm_ssp_security_level	ssp_sec_level;
	bool 					outgoing_ok;
	bool 					authorised;
	bool 					disable_legacy;
} CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ_T;


typedef struct
{
	Task			theAppTask;
	cl_sm_wae		write_auth_enable;
	bool			debug_keys;
	bool			legacy_auto_pair_key_missing;
} CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ_T;


typedef struct
{
    dm_protocol_id		protocol_id;
    uint32	            channel;
    bool	            outgoing_ok;
    dm_security_level   security_level;
    uint16               psm;
} CL_INTERNAL_SM_REGISTER_REQ_T;


typedef struct
{
    dm_protocol_id	protocol_id;
    uint32	        channel;
} CL_INTERNAL_SM_UNREGISTER_REQ_T;


typedef struct
{
    bdaddr				bd_addr;
    dm_protocol_id		protocol_id;
    uint32	            remote_channel;
    dm_security_level   outgoing_security_level;
    uint16               psm;
} CL_INTERNAL_SM_REGISTER_OUTGOING_REQ_T;


typedef struct
{
    bdaddr           bd_addr;
    dm_protocol_id   protocol_id;
    uint32	         channel;
} CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ_T;


typedef struct
{   
    Task	theAppTask;
    Sink    sink;
    bool	encrypt;
} CL_INTERNAL_SM_ENCRYPT_REQ_T;


typedef struct
{   
    bdaddr			bd_addr;
} CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ_T;


typedef struct
{
    bdaddr           bd_addr;
    uint8		        pin_length;
    uint8	            pin[HCI_MAX_PIN_LENGTH];
} CL_INTERNAL_SM_PIN_REQUEST_RES_T;


typedef struct
{
    bdaddr           		bd_addr;
    cl_sm_io_capability		io_capability;
	bool					bonding;
	bool					mitm;
	uint8					oob_data_present;
	uint8*					oob_hash_c;
	uint8*					oob_rand_r;
} CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES_T;


typedef struct
{
	bdaddr				bd_addr;
	bool				confirm;
} CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES_T;

typedef struct
{
	bdaddr				bd_addr;
	bool				cancelled;
	uint32				numeric_value;
} CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES_T;

typedef struct
{
	bdaddr				bd_addr;
	cl_sm_keypress_type	type;
} CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ_T;

typedef struct
{
	bdaddr				bd_addr;
	bool				trusted;
} CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ_T;
 
typedef struct
{
    bdaddr           bd_addr;
    dm_protocol_id   protocol_id;
    uint32	         channel;
    bool             incoming;
    bool             authorised;
} CL_INTERNAL_SM_AUTHORISE_RES_T;


typedef struct
{
    Task            		theAppTask;
	bdaddr					bd_addr;
	cl_sm_link_key_type		link_key_type;
	uint8					link_key[SIZE_LINK_KEY];
	uint16					trusted;
        uint16                  bonded;
} CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ_T;


typedef struct
{
    Task            theAppTask;
	bdaddr			bd_addr;
} CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ_T;


typedef struct 
{
	Task theAppTask;
} CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ_T;


typedef struct
{
	uint32 class_of_device;
} CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ_T;


typedef struct
{
	uint16 ps_interval;
	uint16 ps_window;
} CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ_T;


typedef struct
{
	uint16 is_interval;
	uint16 is_window;
} CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ_T;


typedef struct
{
	hci_scan_enable mode;
} CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ_T;


typedef struct
{
	bdaddr				bd_addr;
	page_scan_mode		ps_mode;
	page_scan_rep_mode	ps_rep_mode;
} CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ_T;


typedef struct
{
	bdaddr	bd_addr;
	uint16	clock_offset;
} CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ_T;


typedef struct
{
	bdaddr	bd_addr;
} CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ_T;


typedef struct
{
	Sink	sink;
	uint16	flush_timeout;
} CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ_T;


typedef struct
{
	uint16	length_name;
	uint8	*name;
} CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ_T;

typedef struct
{
	Task theAppTask;
	uint16 num_iac;
	uint32 iac[1];
} CL_INTERNAL_DM_WRITE_IAC_LAP_REQ_T;

typedef struct
{
	Task theAppTask;
} CL_INTERNAL_DM_READ_BD_ADDR_REQ_T;


typedef struct
{
	Task theAppTask;
	Sink sink;
} CL_INTERNAL_DM_READ_LINK_QUALITY_REQ_T;


typedef struct
{
	Task theAppTask;
	Sink sink;
} CL_INTERNAL_DM_READ_RSSI_REQ_T;


typedef struct
{
	Task	theAppTask;
	Sink	sink;
} CL_INTERNAL_DM_READ_CLK_OFFSET_REQ_T;


typedef struct
{
	Task				theAppTask;
	uint8	version;
} CL_INTERNAL_DM_SET_BT_VERSION_REQ_T;


typedef struct
{
	Task	theAppTask;
	Sink	sink;
} CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ_T;


typedef struct
{
	Task	theAppTask;
	Sink	sink;
} CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ_T;


typedef struct
{
	Task	theAppTask;
} CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ_T;


typedef struct
{
	Sink	sink;
} CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ_T;


typedef struct
{
	Task	theAppTask;
	uint16	record_length;
	uint8	*record;
} CL_INTERNAL_SDP_REGISTER_RECORD_REQ_T;


typedef struct
{
	Task	theAppTask;
	uint32	service_handle;
} CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ_T;


typedef struct
{
	uint16 mtu;
} CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ_T;


typedef struct
{
	uint16 mtu;
} CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ_T;


typedef struct
{
	Task		theAppTask;
	bdaddr		bd_addr;
} CL_INTERNAL_SDP_OPEN_SEARCH_REQ_T;


typedef struct
{
	Task	theAppTask;
} CL_INTERNAL_SDP_CLOSE_SEARCH_REQ_T;


typedef struct
{
	Task	theAppTask;
	bdaddr	bd_addr;
	uint16	length;
	uint8	*search_pattern;
	uint16	max_responses;
} CL_INTERNAL_SDP_SERVICE_SEARCH_REQ_T;


typedef struct
{
	Task	theAppTask;
	bdaddr	bd_addr;
	uint32	service_handle;
	uint16	size_attribute_list;
    uint8	*attribute_list;
    uint16  max_num_attr;
} CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ_T;


typedef struct
{
	Task	theAppTask;
	bdaddr	bd_addr;
	uint16	max_num_attributes; 
	uint16	size_search_pattern; 
	uint8	*search_pattern;
	uint16	size_attribute_list; 
	uint8	*attribute_list;
} CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ_T;


typedef struct
{
	Task theAppTask;
} CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ_T;


typedef struct
{
	Task	                    clientTask;
	uint16	                    app_psm;
    Task                        connectionTask;
    const profile_task_recipe   *l2cap_recipe;
} CL_INTERNAL_L2CAP_REGISTER_REQ_T;


typedef struct
{
	Task	theAppTask;
	uint16	app_psm;
} CL_INTERNAL_L2CAP_UNREGISTER_REQ_T;


typedef struct
{
	Task				theAppTask;
	bdaddr			bd_addr;
	uint16				psm_local;
	uint16				psm_remote;
	l2cap_config_params config_params;
} CL_INTERNAL_L2CAP_CONNECT_REQ_T;


typedef struct
{
	Task				theAppTask;
	bool				response;
	uint16				psm_local;
	uint16				cid;
	l2cap_config_params config_params;
} CL_INTERNAL_L2CAP_CONNECT_RES_T;


typedef struct
{
	Task	theAppTask;
	Sink	sink;
} CL_INTERNAL_L2CAP_DISCONNECT_REQ_T;


typedef struct
{
	Task	theAppTask;
	uint16	psm;
	cid_t	cid;
} CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND_T;

typedef struct
{
    identifier_t id;
    cid_t cid;
} CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP_T;


typedef struct
{
    Task                        theAppTask;
    Task                        theConnectTask;
    const profile_task_recipe   *task_recipe;
} CL_INTERNAL_RFCOMM_REGISTER_REQ_T;


typedef struct
{
    Task                    theAppTask;
    bdaddr                  bd_addr;
    uint8                   remote_server_channel;
    uint8                   local_server_channel;
    rfcomm_config_params    config;
} CL_INTERNAL_RFCOMM_CONNECT_REQ_T;


typedef struct
{
    Task                    theAppTask;
    bool                    response;
    bdaddr                  bd_addr;    
    uint8                   server_channel;
    rfcomm_config_params    config;
} CL_INTERNAL_RFCOMM_CONNECT_RES_T;


typedef struct
{
    Task                    theAppTask;
    Sink                    sink;
} CL_INTERNAL_RFCOMM_DISCONNECT_REQ_T;


typedef struct
{
	Task	theAppTask;
} CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND_T;


typedef struct
{
    Task               theAppTask;
	bdaddr          bd_addr;
    uint8      server_channel;
} CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND_T;


typedef struct
{
    Task                theAppTask;
    Sink                sink;
    uint16              break_signal;
    uint16              modem_signal;
} CL_INTERNAL_RFCOMM_CONTROL_REQ_T;


typedef struct
{
	Task theAppTask;
} CL_INTERNAL_SYNC_REGISTER_REQ_T;

typedef struct
{
	Task theAppTask;
} CL_INTERNAL_SYNC_UNREGISTER_REQ_T;

typedef struct
{
    Task                     theAppTask;
    Sink                     sink;
    sync_config_params       config_params;
} CL_INTERNAL_SYNC_CONNECT_REQ_T;

typedef struct
{
    Task                theAppTask;
    bdaddr	            bd_addr;
    bool                response;
    sync_config_params  config_params;
} CL_INTERNAL_SYNC_CONNECT_RES_T;

typedef struct
{
    Sink                     audio_sink;
    hci_status               reason;
} CL_INTERNAL_SYNC_DISCONNECT_REQ_T;

typedef struct
{
    Task                     theAppTask;
    Sink                     audio_sink;
    sync_config_params       config_params;
} CL_INTERNAL_SYNC_RENEGOTIATE_REQ_T;


typedef struct
{
    Task                     theAppTask;
} CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND_T;


typedef struct
{
    Task                     theAppTask;
} CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND_T;


typedef struct
{
	Task		theAppTask;
	Sink 		sink;
	hci_role 	role;	
} CL_INTERNAL_DM_SET_ROLE_REQ_T;


typedef struct
{
	Task		theAppTask;
	Sink 		sink;
} CL_INTERNAL_DM_GET_ROLE_REQ_T;


typedef struct
{
	Sink 		sink;
	uint16		timeout;
} CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ_T;


typedef struct
{
	Sink 		sink;
	uint16 		size_power_table; 
	lp_power_table const *power_table;
} CL_INTERNAL_DM_SET_LINK_POLICY_REQ_T;


typedef struct
{
	Sink 		sink;
	uint16		max_remote_latency;
	uint16		min_remote_timeout;
	uint16		min_local_timeout;
} CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ_T;


typedef struct
{
	bdaddr	bd_addr;
	uint16	ps_base;
	uint16 	size_psdata;
} CL_INTERNAL_SM_GET_ATTRIBUTE_REQ_T;


typedef struct
{
	uint16	index;
	uint16	ps_base;
	uint16 	size_psdata;
} CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ_T;


typedef struct
{
    uint16 const *rs_table;
} CL_INTERNAL_DM_SET_ROLE_SWITCH_PARAMS_REQ_T;


/* State definition for the Connection Library */
typedef enum
{
    connectionUninitialised,
    connectionInitialising,
    connectionReady,
    connectionTestMode
} connectionStates;



/* Inquiry management state */
typedef struct
{
	Task inquiryLock;
	Task nameLock;
	Task iacLock;
} connectionInquiryState;


typedef struct
{
	Task 				stateInfoLock;
	Sink 				sink;
	
	/* Used to hold the bluetooth version of our device */
	cl_dm_bt_version 	version;
} connectionReadInfoState;


typedef struct
{
    /* Valid during the connectionInitialising state */
	uint16				noDevices;
	uint16				deviceCount;
	dm_security_mode	security_mode:8;
	encryption_mode     enc_mode:8;

    /* Message primitives locks */
    Task                setSecurityModeLock;
    Task                authReqLock;
	Task                encryptReqLock;
	Task				deviceReqLock;

    /* Used to hold address of device we are Authenticating against */
    bdaddr				authRemoteAddr;

	/* Used to tell us what type of pairing we are performing */
	unsigned			authentication_requirements:4;

	unsigned			reserved:4;
	
	/* Sink identifying the connecting being encrypted */
	Sink				sink;
} connectionSmState;


typedef struct
{
	Task		sdpLock;
	Task		sdpSearchLock;
	bdaddr  	sdpServerAddr;
} connectionSdpState;


typedef struct
{
	Task		registerLock;
    Task        lock;
} connectionRfcommState;


typedef struct
{
	Task		l2capRegisterLock;
} connectionL2capState;


typedef struct
{
	Sink roleLock;
} connectionLinkPolicyState;


/* Structure to hold the instance state for the Connection Library */
typedef struct
{
	TaskData						task;     
	Task							theAppTask; 
	connectionStates				state:8;      
	connectionInitMask				initMask:8;
	const msg_filter				*msgFilter;

    connectionInquiryState			inqState;
    connectionSmState				smState;
    connectionReadInfoState         infoState;
    connectionSdpState				sdpState;
    connectionRfcommState			rfcommState;
    connectionL2capState            l2capState;
	connectionLinkPolicyState		linkPolicyState;
} connectionState;


/****************************************************************************
NAME	
    connectionGetCmTask

DESCRIPTION
    This function returns the connection library task so that the connection
    library can post a message to itself.

RETURNS
    The connection library task.
*/
Task connectionGetCmTask(void);

/****************************************************************************
NAME	
    connectionGetAppTask

DESCRIPTION
    This function returns the application task.

RETURNS
    The application task.
*/
Task connectionGetAppTask(void);


/****************************************************************************
NAME	
    connectionGetMsgFilter

DESCRIPTION
    This function returns the connection library message filter.
*/
const msg_filter *connectionGetMsgFilter(void);


#endif	/* CONNECTION_PRIVATE_H_ */
