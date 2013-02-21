/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_private.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_PRIVATE_H_
#define AVRCP_PRIVATE_H_

#include "avrcp.h"

#include <connection.h>
#include <panic.h>
#include <source.h>
#include <stdlib.h>
#include <string.h>
#include <sink.h>
#include <print.h>

/* Macros for creating messages. */
#define MAKE_AVRCP_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_AVRCP_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);


/* L2CAP AVCTP PSM as defined in the assigned numbers section of the Bluetooth spec. */
#define AVCTP_PSM					0x17

/* Packet defines. */
#define AVCTP_HEADER_SIZE_SINGLE_PKT     3
#define AVCTP0_TRANSACTION_SHIFT	4
#define AVCTP0_PACKET_TYPE_MASK		0x0c
#define AVCTP0_PACKET_TYPE_SINGLE	0x00
#define AVCTP0_PACKET_TYPE_START	0x04
#define AVCTP0_PACKET_TYPE_CONTINUE	0x08
#define AVCTP0_PACKET_TYPE_END		0x0C
#define AVCTP0_CR_MASK				0x02
#define AVCTP0_CR_COMMAND			0x00
#define AVCTP0_CR_RESPONSE			0x02
#define AVCTP0_IPID					0x01
#define AVCTP1_PROFILE_AVRCP_HIGH	0x11
#define AVCTP2_PROFILE_AVRCP_CONTROLTARGET 	0x0c
#define AVCTP2_PROFILE_AVRCP_REMOTECONTROL	0x0e
#define AVCTP2_PROFILE_AVRCP_REMOTECONTROLLER	0x0f


#define AVRCP0_CTYPE_CONTROL			0x00
#define AVRCP0_CTYPE_STATUS				0x01
#define AVRCP0_CTYPE_NOTIFY				0x03
#define AVRCP1_SUBUNIT_TYPE_MASK		0xf8
#define AVRCP1_SUBUNIT_TYPE_SHIFT		3
#define AVRCP1_SUBUNIT_ID_MASK			0x07
#define AVRCP1_UNIT						0xff

#define AVRCP2_PASSTHROUGH				0x7C
#define AVRCP2_UNITINFO					0x30
#define AVRCP2_SUBUNITINFO				0x31
#define AVRCP2_VENDORDEPENDENT			0x00
#define	AVRCP3_PASSTHROUGH_OP_MASK		0x7f
#define AVRCP3_PASSTHROUGH_STATE_MASK	0x80
#define	AVRCP3_SUBUNITINFO_PAGE_SHIFT	4
#define	AVRCP3_SUBUNITINFO_EXTEND_MASK	7
#define AVRCP4_UNITINFO_UNIT_TYPE_SHIFT	3
#define AVRCP4_UNITINFO_UNIT_MASK		7

#define AVRCP_TOTAL_HEADER_SIZE			6

/* Value used to identify metadata PDU in Vendor Dependent data. */
#define AVRCP_BT_COMPANY_ID             ((uint32) 6488)

/* Number of bytes used to hold metadata header. */
#define METADATA_HEADER_SIZE						0x04

/* Metadata PDU values from the spec. */
#define AVRCP_GET_CAPS_PDU_ID						0x10
#define AVRCP_LIST_APP_ATTRIBUTES_PDU_ID			0x11
#define AVRCP_LIST_APP_VALUE_PDU_ID					0x12
#define AVRCP_GET_APP_VALUE_PDU_ID					0x13
#define AVRCP_SET_APP_VALUE_PDU_ID					0x14
#define AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID			0x15
#define AVRCP_GET_APP_VALUE_TEXT_PDU_ID				0x16
#define AVRCP_INFORM_CHARACTER_SET_PDU_ID			0x17
#define AVRCP_INFORM_BATTERY_STATUS_PDU_ID			0x18
#define AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID			0x20
#define AVRCP_GET_PLAY_STATUS_PDU_ID				0x30
#define AVRCP_REGISTER_NOTIFICATION_PDU_ID			0x31
#define AVRCP_REQUEST_CONTINUING_RESPONSE_PDU_ID	0x40
#define AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID		0x41


/* Event values from the spec. */
#define EVENT_PLAYBACK_STATUS_CHANGED				0x01
#define EVENT_TRACK_CHANGED							0x02
#define EVENT_TRACK_END								0x03
#define EVENT_TRACK_START							0x04
#define EVENT_PLAYBACK_POS_CHANGED					0x05
#define EVENT_BATTERY_STATUS_CHANGED				0x06
#define EVENT_SYSTEM_STATUS_CHANGED					0x07
#define EVENT_PLAYER_SETTING_CHANGED				0x08

#ifdef AVRCP_DEBUG_LIB

#define AVCTP_WATCHDOG_TIMEOUT					((uint32) 4000)
#define AVCTP_WATCHDOG_FRAGMENTED_TIMEOUT		((uint32) 2000)
#define AVCTP_SEND_RESPONSE_TIMEOUT				((uint32) 4000)

#else

#define AVCTP_WATCHDOG_TIMEOUT					((uint32) 4000/*TODO 500*/)
#define AVCTP_WATCHDOG_FRAGMENTED_TIMEOUT		((uint32) 2000)
#define AVCTP_SEND_RESPONSE_TIMEOUT				((uint32) 4000/* TODO 500*/)

#endif

  
/* Macro used to generate debug version of this library */
#ifdef AVRCP_DEBUG_LIB
#include <panic.h>
#include <stdio.h>
#define AVRCP_DEBUG(x) {printf x; Panic();}
#else
#define AVRCP_DEBUG(x)
#endif


/* Avrcp Profile Library private messages */
#define AVRCP_MSG_BASE     (0x0)

enum
{
    /* Initialisation */
    AVRCP_INTERNAL_TASK_INIT_REQ = AVRCP_MSG_BASE,
    AVRCP_INTERNAL_TASK_DELETE_REQ,
    AVRCP_INTERNAL_INIT_REQ,

	/* Connect */
	AVRCP_INTERNAL_CONNECT_REQ,
	AVRCP_INTERNAL_CONNECT_RES,
	AVRCP_INTERNAL_DISCONNECT_REQ,

	/* Signalling */
	AVRCP_INTERNAL_PASSTHROUGH_REQ,
	AVRCP_INTERNAL_PASSTHROUGH_RES,
	AVRCP_INTERNAL_UNITINFO_REQ,
	AVRCP_INTERNAL_UNITINFO_RES,
	AVRCP_INTERNAL_SUBUNITINFO_REQ,
	AVRCP_INTERNAL_SUBUNITINFO_RES,
	AVRCP_INTERNAL_VENDORDEPENDENT_REQ,
	AVRCP_INTERNAL_VENDORDEPENDENT_RES,
	AVRCP_INTERNAL_WATCHDOG_TIMEOUT,
	AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT,

    /* Metadata transfer */
    AVRCP_INTERNAL_GET_CAPS_RES,
	AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES,
	AVRCP_INTERNAL_LIST_APP_VALUE_RES,
	AVRCP_INTERNAL_GET_APP_VALUE_RES,
	AVRCP_INTERNAL_SET_APP_VALUE_RES,
	AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES,
	AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES,
	AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES,
	AVRCP_INTERNAL_GET_PLAY_STATUS_RES,
	AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES,
	AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES,
	AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES,
	AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES,
	AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES,
	AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES,
	AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES,
	AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES,
	AVRCP_INTERNAL_ABORT_CONTINUING_RES,
	AVRCP_INTERNAL_NEXT_GROUP_RES,
	AVRCP_INTERNAL_PREVIOUS_GROUP_RES,
	AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET,
	AVRCP_INTERNAL_INFORM_BATTERY_STATUS_RES,
	AVRCP_INTERNAL_INFORM_CHAR_SET_RES,
	AVRCP_INTERNAL_GET_FEATURES,
	AVRCP_INTERNAL_GET_EXTENSIONS,

	/* Common cfm message */
    AVRCP_INTERNAL_MESSAGE_MORE_DATA,
	AVRCP_COMMON_CFM_MESSAGE
};


typedef enum 
{
	avrcpInitialising = 0,
	avrcpReady,
	avrcpConnecting,
	avrcpConnected
} avrcpState;


typedef enum
{
	avrcp_none = 0,
	avrcp_passthrough,
	avrcp_unit_info,
	avrcp_subunit_info,
	avrcp_vendor,
    avrcp_get_caps,
    avrcp_list_app_attributes,
	avrcp_list_app_values,
	avrcp_get_app_values,
	avrcp_set_app_values,
	avrcp_get_app_attribute_text,
	avrcp_get_app_value_text,
	avrcp_get_element_attributes,
	avrcp_get_play_status,
	avrcp_playback_status,
	avrcp_track_changed,
	avrcp_track_reached_end,
	avrcp_track_reached_start,
	avrcp_playback_pos_changed,
	avrcp_batt_status_changed,
	avrcp_system_status_changed,
	avrcp_device_setting_changed,
	avrcp_request_continuation,
	avrcp_abort_continuation,
	avrcp_next_group,
	avrcp_previous_group,
	avrcp_battery_information,
	avrcp_character_set

} avrcpPending;


typedef enum
{
	avrcp_frag_none = 0,
	avrcp_frag_start,
	avrcp_frag_continue,
	avrcp_frag_end
} avrcpFragment;


typedef enum 
{
    avrcp_packet_type_single = 0,
    avrcp_packet_type_start = 1,
    avrcp_packet_type_continue = 2,
    avrcp_packet_type_end = 3
} avrcp_packet_type;


typedef enum 
{
    avrcp_sdp_search_none = 0,
    avrcp_sdp_search_profile_version,
	avrcp_sdp_search_app_profile_version,
    avrcp_sdp_search_features,
	avrcp_sdp_search_app_features
} avrcp_sdp_search;

typedef struct
{
    Task                        connectionTask;
    const profile_task_recipe   *recipe;
} AVRCP_INTERNAL_INIT_REQ_T;


typedef struct
{
	bdaddr				bd_addr;
} AVRCP_INTERNAL_CONNECT_REQ_T;


typedef struct
{
	uint16				connection_id;
	bool				accept;
} AVRCP_INTERNAL_CONNECT_RES_T;


typedef struct
{
	avc_subunit_type	subunit_type;
	avc_subunit_id		subunit_id;
	bool				state;
	avc_operation_id	opid;
	uint16				operation_data_length;
	Source			    operation_data;
} AVRCP_INTERNAL_PASSTHROUGH_REQ_T;


typedef struct
{
	avrcp_response_type	response;
} AVRCP_INTERNAL_PASSTHROUGH_RES_T;


typedef struct
{
	bool				accept;
	avc_subunit_type	unit_type;
	uint8				unit;
	uint32				company_id;
} AVRCP_INTERNAL_UNITINFO_RES_T;


typedef struct
{
	uint8 page;
} AVRCP_INTERNAL_SUBUNITINFO_REQ_T;


typedef struct
{
	bool	accept;
	uint8	page_data[PAGE_DATA_LENGTH];
} AVRCP_INTERNAL_SUBUNITINFO_RES_T;


typedef struct
{
	avc_subunit_type	subunit_type;
	avc_subunit_id		subunit_id;
	uint8				ctype;
	uint32				company_id;
	uint16				data_length;
	Source              data;	
} AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T;


typedef struct
{
	avrcp_response_type	response;
} AVRCP_INTERNAL_VENDORDEPENDENT_RES_T;
	

typedef struct
{
    AVRCP				*avrcp;
	avrcp_status_code	status;
	Sink				sink;	
} AVRCP_COMMON_CFM_MESSAGE_T;


typedef struct
{
    AVRCP				*avrcp;
	uint16              transaction;
	avrcp_status_code	status;
} AVRCP_COMMON_METADATA_CFM_MESSAGE_T;


typedef struct
{
    AVRCP				*avrcp;
    avrcp_status_code	status;
	uint16              transaction;
    uint16              no_packets;    
    uint16              ctp_packet_type;    
    uint16              metadata_packet_type;    
    uint16              number_of_data_items;	
	uint16				data_offset;   
    uint16				size_data;    
    Source				data;
} AVRCP_COMMON_FRAGMENTED_METADATA_CFM_T;


typedef struct
{
    AVRCP				*avrcp;
	uint16              transaction;
} AVRCP_COMMON_METADATA_IND_MESSAGE_T;


typedef struct
{
    AVRCP				*avrcp;
	uint16              transaction;
    uint16              no_packets;    
    uint16              ctp_packet_type;    
    uint16              metadata_packet_type;    
    uint16              number_of_data_items;	
	uint16				data_offset;   
    uint16				size_data;    
    Source				data;
} AVRCP_COMMON_FRAGMENTED_METADATA_IND_T;


typedef struct
{
	avrcpPending pending_command;
	uint16 data;
} AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT_T;


typedef struct
{
    avrcp_response_type response;
    avrcp_capability_id caps_id;
    uint16              size_caps_list;
    Source              caps_list;
} AVRCP_INTERNAL_GET_CAPS_RES_T;


typedef struct
{
	avrcp_response_type response;
	uint16	size_attributes_list; 
	Source attributes_list;
} AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES_T;


typedef struct
{
	avrcp_response_type response;
	uint16	size_values_list; 
	Source values_list;
} AVRCP_INTERNAL_LIST_APP_VALUE_RES_T;


typedef struct
{
	avrcp_response_type response;
	uint16	size_values_list; 
	Source values_list;
} AVRCP_INTERNAL_GET_APP_VALUE_RES_T;


typedef struct
{
	avrcp_response_type response;
} AVRCP_INTERNAL_SET_APP_VALUE_RES_T;


typedef struct
{
	avrcp_response_type response;
	uint16 number_of_attributes;
	uint16	size_attributes_list; 
	Source attributes_list;
} AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES_T;


typedef struct
{
	avrcp_response_type response;
	uint16 number_of_values;
	uint16	size_values_list; 
	Source values_list;
} AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES_T;


typedef struct
{
	avrcp_response_type response;
	uint16 number_of_attributes;
	uint16	size_attributes_list; 
	Source attributes_list;
} AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES_T;


typedef struct
{
	avrcp_response_type response;
	uint32 song_length;
	uint32 song_elapsed;
	avrcp_play_status play_status;
} AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T;


typedef struct
{
	avrcp_response_type response;
	avrcp_play_status play_status;
} AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T;


typedef struct
{
	avrcp_response_type response;
	uint32 track_index_high;
	uint32 track_index_low;
} AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T;


typedef struct
{
	avrcp_response_type response;
} AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES_T;


typedef struct
{
	avrcp_response_type response;
} AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES_T;


typedef struct
{
	avrcp_response_type response;
	uint32 playback_pos;
} AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T;


typedef struct
{
	avrcp_response_type response;
	avrcp_battery_status battery_status;
} AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T;


typedef struct
{
	avrcp_response_type response;
	avrcp_system_status system_status;
} AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T;


typedef struct
{
	avrcp_response_type response;
	uint16 size_attributes;
	Source attributes;
} AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T;


typedef struct
{
	avrcp_response_type response;
} AVRCP_INTERNAL_ABORT_CONTINUING_RES_T;


typedef struct
{
	avrcp_response_type response;
} AVRCP_INTERNAL_NEXT_GROUP_RES_T;


typedef struct
{
	avrcp_response_type response;
} AVRCP_INTERNAL_PREVIOUS_GROUP_RES_T;

typedef struct
{
	avrcp_response_type response;
} AVRCP_INTERNAL_INFORM_BATTERY_STATUS_RES_T;

typedef struct
{
	avrcp_response_type response;
} AVRCP_INTERNAL_INFORM_CHAR_SET_RES_T;

typedef struct
{
	Source data;
	uint16 param_length;
	uint16 pdu_id;
	uint16 curr_packet;
	uint16 response;
	uint8 transaction;
} AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET_T;


typedef struct 
{
    TaskData            cleanUpTask;
    uint8               *sent_data;
} AvrcpCleanUpTask;


struct __AVRCP
{
	TaskData				task;
	Task					clientTask;
	Sink 					sink;
	avrcpState				state:2;
	avrcpPending			pending:5;
	avrcp_supported_events	last_metadata_notification:4;
    avrcpPending			block_received_data:5;
	uint16					continuation_pdu;
	Source					continuation_data;
	uint16					srcUsed;
	unsigned				cmd_transaction_label:4;
	unsigned				rsp_transaction_label:4;
	unsigned				registered_events:8;
	uint8					notify_transaction_label[8];
	uint16 					l2cap_mtu;
	uint8					*identifier;
    uint16					lazy;
	avrcpFragment			last_ctp_fragment:2;
	avrcp_device_type		device_type:2;
	unsigned				local_extensions:2;
	unsigned				local_target_features:8;
	avrcpFragment			last_metadata_fragment:2;
	unsigned				local_controller_features:8;
	unsigned				unused:8;
	uint16					sdp_search_mode;
	uint16					last_metadata_pdu;
    AvrcpCleanUpTask		dataFreeTask;
};



#endif /* AVRCP_PRIVATE_H_ */
