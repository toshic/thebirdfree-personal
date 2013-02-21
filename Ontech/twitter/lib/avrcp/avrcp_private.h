/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    avrcp_private.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_PRIVATE_H_
#define AVRCP_PRIVATE_H_

#include "avrcp.h"

#include <connection.h>
#include <message.h>
#include <app/bluestack/types.h>
#include <app/message/system_message.h>
#include <stdlib.h>
#include <source.h>

/* Macros for creating messages */
#define MAKE_AVRCP_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_AVRCP_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);


/* L2CAP AVCTP PSM as defined in the assigned numbers section of the Bluetooth spec */
#define AVCTP_PSM					0x17


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
#define AVCTP2_PROFILE_AVRCP_LOW	0x0e
#define AVCTP_HEADER_SIZE_SINGLE_PKT_OCTETS     3

#define AVRCP0_CTYPE_CONTROL			0x00
#define AVRCP0_CTYPE_STATUS				0x01
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


#define AVCTP_WATCHDOG_TIMEOUT					((uint32) 2000)
#define AVCTP_WATCHDOG_FRAGMENTED_TIMEOUT		((uint32) 10000)

#include <stdio.h>
#define AVRCP_PRINT(x) {printf x;}

  
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

	/* Common cfm message */
	AVRCP_COMMON_CFM_MESSAGE
};


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


typedef enum 
{
	avrcpInitialising,
	avrcpReady,
	avrcpConnecting,
	avrcpConnected
} avrcpState;


typedef enum
{
	avrcp_none,
	avrcp_passthrough,
	avrcp_unit_info,
	avrcp_subunit_info,
	avrcp_vendor
} avrcpPending;


typedef enum
{
	avrcp_frag_none,
	avrcp_frag_start,
	avrcp_frag_continue,
	avrcp_frag_end
} avrcpFragment;


struct __AVRCP
{
	TaskData		    task;
	Task			    clientTask;
	avrcpState		    state;
	avrcpPending	    pending;
    uint8               block_received_data;
	uint8			    watchdog;
	Sink 			    sink;
	uint8			    transaction_label;
	avrcpFragment	    fragmented;
	uint16 			    l2cap_mtu;
    avrcp_device_type   device_type;
	uint8			    *identifier;
    uint16              lazy;
};


#endif /* AVRCP_PRIVATE_H_ */
