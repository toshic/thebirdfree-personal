/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_private.h
    
DESCRIPTION
    This file contains data private to the a2dp library.
	
*/

#ifndef A2DP_PRIVATE_H_
#define A2DP_PRIVATE_H_

/*
#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif
*/

#include "a2dp.h"

#include <panic.h>


/* Macros for creating messages */
#define MAKE_A2DP_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_A2DP_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);


/*
#ifndef A2DP_DEBUG_LIB
#define A2DP_DEBUG_LIB
#endif
*/


/* Macro used to generate debug lib printfs. */
#ifdef A2DP_DEBUG_LIB
#include <panic.h>
#include <stdio.h>
#define A2DP_DEBUG(x)	{printf x; Panic();}
#else
#define A2DP_DEBUG(x)
#endif


/* L2CAP PSM */
#define AVDTP_PSM						(0x19)

/*
	The watchdog timeouts.
*/
/*
	The GAVDP specifies TGAVDP100 which is a
	short response timeout for certain signals
	in order to prevent the user seeing an
	obvious hang.
	The range proposed by the spec is 0.5 to
	3 seconds.

	In practice, when the AV device is in a
	scatternet with a SCO connection, baseband
	interactions can stall signals excessively.
*/
#ifndef A2DP_DEBUG_LIB
#define WATCHDOG_TGAVDP100				D_SEC(5)
#else
/* the timeout should be longer in debug mode
   as printf takes a long time. */
#define WATCHDOG_TGAVDP100				D_SEC(10)
#endif

/*
	For signals not specified to use TGAVDP100,
	we still need a timeout to prevent our device
	hanging.  This needs to be long as the spec
	does not specify a timeout and so the remote
	device could potentially prompt the user
	before responding to a signal.
*/
#define WATCHDOG_GENERAL				D_SEC(30)

/*
	Maximum number words needed to contain a
	bit flag for all possible SEID values
*/	
#define NUM_SEID_PACKED_WORDS			(4)


/* 
	Timeout for the signalling channel. 
	When this fires if the signalling channel is 
	still connected, drop it. 
*/
#define SIGNAL_TIMER					(1300)


/* A2dp Profile Library private messages */
#define A2DP_MSG_BASE     (0x0)


typedef struct sep_type
{
	const sep_config_type *sep_config;
	unsigned in_use:1;
	unsigned configured:1;
	unsigned unused:14;
} sep_type;


struct _device_sep_list
{
	uint16 size_seps;
	sep_type seps[1];
};


enum
{
    /* Initialisation */
    A2DP_INTERNAL_TASK_INIT_REQ = A2DP_MSG_BASE,
	A2DP_INTERNAL_CONNECT_SIGNALLING_REQ,
	A2DP_INTERNAL_CONNECT_SIGNALLING_RES,
	A2DP_INTERNAL_CONFIGURE_CODEC_RSP,
    A2DP_INTERNAL_OPEN_REQ,
	A2DP_INTERNAL_TASK_DELETE_REQ,
	A2DP_INTERNAL_WATCHDOG_IND,
	A2DP_INTERNAL_SIGNAL_PACKET_IND,
	A2DP_INTERNAL_SIGNAL_CONNECTION_TIMEOUT_IND,
	A2DP_INTERNAL_RECONFIGURE_REQ,
	A2DP_INTERNAL_START_REQ,
	A2DP_INTERNAL_SUSPEND_REQ,
	A2DP_INTERNAL_CLOSE_REQ,
	A2DP_INTERNAL_DISCONNECT_ALL_REQ,
	A2DP_INTERNAL_SEND_CODEC_PARAMS_REQ,
	A2DP_INTERNAL_GET_CAPS_TIMEOUT_IND
};


typedef struct
{
    Task						client;
    bdaddr						addr;
} A2DP_INTERNAL_OPEN_REQ_T;


typedef struct
{
    Task						client;
    bdaddr						addr;
} A2DP_INTERNAL_CONNECT_SIGNALLING_REQ_T;


typedef struct
{
    Task				client;
	bool				accept;
	uint16				connection_id;
} A2DP_INTERNAL_CONNECT_SIGNALLING_RES_T;


typedef struct
{				
	bool			accept;
	uint16			size_codec_service_caps;        	
	uint8			codec_service_caps[1];      		
} A2DP_INTERNAL_CONFIGURE_CODEC_RSP_T;


typedef struct
{
	uint16 size_sep_caps;
	uint8 *sep_caps;
} A2DP_INTERNAL_RECONFIGURE_REQ_T;


typedef struct
{
	Sink			sink;      		
} A2DP_INTERNAL_START_REQ_T;


typedef struct
{
	bool send_reconfigure_message;
} A2DP_INTERNAL_SEND_CODEC_PARAMS_REQ_T;


/*!
	@brief Error codes.
*/
typedef enum
{
    avdtp_bad_header_format             = (0x01),	/*!< The request packet header format error that is not specified above ERROR_CODE. */
    avdtp_bad_length                    = (0x11),	/*!< The request packet length is not match the assumed length. */
    avdtp_bad_acp_seid                  = (0x12),	/*!< The requested command indicates an invalid ACP SEID (not addressable). */
    avdtp_sep_in_use                    = (0x13),	/*!< The SEP is in use. */
    avdtp_sep_not_in_use                = (0x14),	/*!< The SEP is not in use. */
    avdtp_bad_serv_category             = (0x17),	/*!< The value of Service Category in the request packet is not defined in AVDTP. */
    avdtp_bad_payload_format            = (0x18),	/*!< The requested command has an incorrect payload format (Format errors not specified in this ERROR_CODE). */
    avdtp_not_supported_command         = (0x19),	/*!< The requested command is not supported by the device. */
    avdtp_invalid_capabilities          = (0x1a),	/*!< The reconfigure command is an attempt to reconfigure a transport service capabilities of the SEP. Reconfigure is only permitted for application service capabilities. */
    avdtp_bad_recovery_type             = (0x22),	/*!< The requested Recovery Type is not defined in AVDTP. */
    avdtp_bad_media_transport_format    = (0x23),	/*!< The format of Media Transport Capability is not correct. */
    avdtp_bad_recovery_format           = (0x25),	/*!< The format of Recovery Service Capability is not correct. */
    avdtp_bad_rohc_format               = (0x26),	/*!< The format of Header Compression Service. */
    avdtp_bad_cp_format                 = (0x27),	/*!< The format of Content Protection Service Capability is not correct. */
    avdtp_bad_multiplexing_format       = (0x28),	/*!< The format of Multiplexing Service Capability is not correct. */
    avdtp_unsupported_configuration     = (0x29),	/*!< Configuration not supported. */
    avdtp_bad_state                     = (0x31)	/*!< Indicates that the ACP state machine is in an invalid state in order to process the signal.*/
} avdtp_error_code;

/* AVDTP header message type */
enum
{
	avdtp_message_type_command = (0x0),
	avdtp_message_type_accept = (0x02),
	avdtp_message_type_reject = (0x03)
};

/* Signalling header packet type */
enum
{		
	avdtp_packet_type_single = (0x0),
	avdtp_packet_type_start = (0x01),
	avdtp_packet_type_continue = (0x02),
	avdtp_packet_type_end = (0x03)
};

/* AVDTP signals */
typedef enum
{	
	avdtp_null				= (0x00),
    avdtp_discover,
    avdtp_get_capabilities, 
    avdtp_set_configuration,
    avdtp_get_configuration,
    avdtp_reconfigure,
    avdtp_open,
    avdtp_start,
    avdtp_close,
    avdtp_suspend,
    avdtp_abort,
    avdtp_security_control
} avdtp_signal_id;

/* State for the signalling connection */
typedef enum
{
	avdtp_connection_idle,
	avdtp_connection_connecting,
    avdtp_connection_connecting_crossover,
	avdtp_connection_connected,
	avdtp_connection_disconnecting
} avdtp_connection;

/* State for the media connection */
typedef enum
{
	avdtp_state_idle,
	avdtp_state_discovering,
	avdtp_state_reading_caps,
	avdtp_state_processing_caps,
	avdtp_state_configuring,
	avdtp_state_configured,
	avdtp_state_local_opening,
	avdtp_state_remote_opening,
	avdtp_state_open,
	avdtp_state_streaming,
	avdtp_state_local_starting,
	avdtp_state_local_suspending,
	avdtp_state_local_closing,
	avdtp_state_remote_closing,
	avdtp_state_reconfig_reading_caps,
	avdtp_state_reconfiguring,
	avdtp_state_local_aborting,
	avdtp_state_remote_aborting
} avdtp_state;


/* The signalling connection information */
typedef struct signalling_channel
{
	Sink					sink;
	uint16					mtu;
	unsigned				transaction_label:8;
	unsigned				pending_transaction_label:8;	
	uint8                   *reassembled_packet;
    uint16                  reassembled_packet_length;	

	avdtp_connection		connection_state:3;
	avdtp_state				signalling_state:5;
	avdtp_state				preabort_state:5;	
	unsigned				connect_then_open_media:1;
	unsigned				waiting_response:2;
} signalling_channel;

/* The media connection information */
typedef struct media_channel
{
	Sink					sink;
	uint16					mtu;
	unsigned				media_connecting:1;
	unsigned				media_connected:1;
	unsigned				unused:14;
} media_channel;

/* The sep information */
typedef struct sep_info
{
	sep_type				*current_sep;

	device_sep_list			*sep_list;

	uint8					*configured_service_caps;
	uint16					configured_service_caps_size;	

	uint8					*reconfigure_caps;
	uint16					reconfigure_caps_size;

	uint8					*list_preferred_local_seids;
	unsigned				max_preferred_local_seids:8;
	unsigned				current_preferred_local_seid:8;

	uint8					*list_discovered_remote_seids;
	unsigned				max_discovered_remote_seids:8;
	unsigned				current_discovered_remote_seid:8;

	unsigned				remote_seid:8;
	unsigned				unused:8;
} sep_info;


struct __A2DP
{
	TaskData					task;
	Task						clientTask;

	signalling_channel			signal_conn;
	media_channel				media_conn;
	sep_info					sep;
};


#endif /* A2DP_PRIVATE_H_ */
