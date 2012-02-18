/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps_private.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library - private header.

*/

#ifndef	SYNCS_PRIVATE_H_
#define	SYNCS_PRIVATE_H_

#include <stdlib.h>
#include <panic.h>
#include <message.h>

#include <goep.h>

/* Macros for creating messages */
#define MAKE_SYNCS_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define SYNCS_SEND_IND(SESSION,TYPE) {TYPE##_T *message = PanicUnlessNew(TYPE##_T);message->syncs=(SYNCS*)(SESSION);MessageSend((SESSION)->theAppTask, (TYPE), message);}

#define SYNCS_NO_PAYLOAD      (0x0)

#ifdef SYNCS_LIBRARY_DEBUG
	#include <panic.h>
	#include <stdio.h>
	#define SYNCS_DEBUG(x) {printf x; Panic();}
	#define SYNCS_ASSERT(c, x) { if (!(c)) { printf x; Panic();} }
#else
	#define SYNCS_DEBUG(x)
	#define SYNCS_ASSERT(c, x)
#endif

/* Currently running PBAP Server Command */
typedef enum
{
    syncs_com_None,
    syncs_com_Connect,
    syncs_com_ObexPassword,
    syncs_com_Disconnect,
	
	syncs_com_Connecting,
	
	syncs_com_EOL
} syncs_running_command;

/* Internal state structures */
struct __syncsState
{
	TaskData            task;
	/* Task associated with this session */
	Task theAppTask;
	/* Sink associated with the session */
	Sink sink;
	/* Maximum packet length for this session */
	uint16 packetSize;
    /* Server Bluetooth address */
    bdaddr bdAddr;
    /* Currently running PBAPS command */
    syncs_running_command currCom;
    
    /* Goep Handle */
    GOEP *handle;
	/* Server channel */
	unsigned int rfcChan:8;
	/* Stores supported by the server */
	unsigned int stores:8;
	
	/* SDP Handle */
	uint32 sdpHandle;
	
	/* Store of App. Specific Parameters */
};

typedef struct __syncsState syncsState;







enum
{
	/* Session Control */
	SYNCS_INT_CONNECTION_RESP,
	SYNCS_INT_AUTH_CHALLENGE,
			
	SYNCS_GET_SEND_FIRST_SRC,
	
	SYNCS_GET_SEND_NEXT,
	
	SYNCS_GET_SEND_NEXT_SRC,
	
    SYNCS_INT_ENDOFLIST
};

/* Internal Message Structures */
typedef struct
{
	bool accept;
	uint16 pktSize;
} SYNCS_INT_CONNECTION_RESP_T;

typedef struct
{
	const uint8 *nonce;
	uint8 options;
	uint16 size_realm;
	const uint8 *realm;
} SYNCS_INT_AUTH_CHALLENGE_T;

typedef struct
{
	syncs_running_command command;
	syncs_get_result result;
	uint16 totalLen;
	Source src;
	uint16 size_packet;
	bool onlyPacket;
} SYNCS_GET_SEND_FIRST_SRC_T;

typedef struct
{
	syncs_running_command command;
	Source src;
	uint16 size_packet;
	bool lastPacket;
} SYNCS_GET_SEND_NEXT_SRC_T;

typedef struct
{
	syncs_running_command command;
	uint16 size_packet;
	const uint8 *packet;
	bool lastPacket;
} SYNCS_GET_SEND_NEXT_T;


/****************************************************************************
NAME	
    pbapsIntHandler

DESCRIPTION
    Handler for messages received by the PBABS Task.
*/
void syncsIntHandler(Task task, MessageId id, Message message);


/****************************************************************************
NAME	
    pbabsGoepHandler

DESCRIPTION
    Handler for messages received by the PBABS Task from GOEP.
*/
void syncsGoepHandler(syncsState *state, MessageId id, Message message);

void syncsMsgSendInitCfm(syncsState *state, syncs_lib_status status);
void syncsMsgSendConnectCfm(syncsState *state, syncs_lib_status status, uint16 pktSize);

#endif /* PBAPS_PRIVATE_H_ */

