/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    OPPS_private.h
    
DESCRIPTION
	Private header for the OPP Server library

*/

#ifndef	OPPS_PRIVATE_H_
#define	OPPS_PRIVATE_H_

#include <stdlib.h>
#include <panic.h>
#include <message.h>

#include "goep.h"

/* Macros for creating messages */
#define MAKE_OPPS_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);

/* Send Error Message */
#ifdef OPPS_LIBRARY_DEBUG
	#include <panic.h>
	#include <stdio.h>
	#define OPPS_DEBUG(x) {printf x; Panic();}
#else
	#define OPPS_DEBUG(x)
#endif

/* Currently running OPPS Command */
typedef enum
{
    opps_com_None,
	opps_com_Connecting,
	opps_com_PushObject,
	opps_com_PushVCard,
	opps_com_PullVCard,
			
	opps_command_end_of_list
} opps_running_command;

/* Internal state structures */
struct __oppsState
{
	TaskData            task;
	/* Task associated with this session */
	Task theAppTask;
	/* Sink associated with the session */
	Sink sink;
	/* Maximum packet length for this session */
	uint16 packetSize;
    /* Server Bluetooth address */
    const bdaddr *bdAddr;
    /* Currently running FTPC command */
    opps_running_command currCom;
	/* RFCOMM Channel for this server */
	uint8 rfcChan;
	/* SDP Handle */
	uint32 sdpHandle;
    
    /* Goep Handle */
    GOEP *handle;
};
typedef struct __oppsState oppsState;







/* OPP Client Library private messages */
#define OPPS_INTERNAL_MESSAGE_BASE 0x00

enum
{
	OPPS_INT_ACCEPT_CONNECTION = OPPS_INTERNAL_MESSAGE_BASE,
	OPPS_INT_AUTH_CHALLENGE,
	
	OPPS_INT_ABORT,
	
	OPPS_INT_GET_NEXT_PACKET,
    
    OPPS_INT_PUSH_VCARD,	
    OPPS_INT_PUSH_NEXT_VCARD,
    OPPS_INT_PUSH_VCARD_SRC,	
    OPPS_INT_PUSH_NEXT_VCARD_SRC,
	
    OPPS_INT_ENDOFLIST
};

/* Internal Message Structures */
typedef struct
{
	bool accept;
	uint16 pktSize;
} OPPS_INT_ACCEPT_CONNECTION_T;

typedef struct
{
	const uint8 *nonce;
	uint8 options;
	uint16 size_realm;
	const uint8 *realm;
} OPPS_INT_AUTH_CHALLENGE_T;

typedef struct
{
	bool moreData;
} OPPS_INT_GET_NEXT_PACKET_T;

typedef struct
{
    uint16 nameLen;
    const uint8* name;
	uint16 length;
    const uint8 *packet;
    uint32 totalLen;
    bool onlyPacket;
} OPPS_INT_PUSH_VCARD_T;

typedef struct
{
	uint16 length;
    const uint8 *packet;
    bool lastPacket;
} OPPS_INT_PUSH_NEXT_VCARD_T;

typedef struct
{
    uint16 nameLen;
    const uint8* name;
	uint16 length;
    Source src;
    uint32 totalLen;
    bool onlyPacket;
} OPPS_INT_PUSH_VCARD_SRC_T;

typedef struct
{
	uint16 length;
    Source src;
    bool lastPacket;
} OPPS_INT_PUSH_NEXT_VCARD_SRC_T;


/****************************************************************************
NAME	
    oppsGetTheTask

DESCRIPTION
    This function returns the OPPS library task so that the OPPS library can use it.

RETURNS
    The FTPC library task.
*/
Task oppsGetTheTask(void);

/****************************************************************************
NAME	
    oppsHandler

DESCRIPTION
    Handler for messages received by the OPPS Task.

RETURNS
    Nothing.
*/
void oppsHandler(Task task, MessageId id, Message message);


void oppsMsgSendInitCfm(oppsState *state, opps_lib_status status);
void oppsMsgSendConnectCfm(oppsState *state, opps_lib_status status, uint16 pktSize);
void oppsMsgSendPushObjectCompleteInd(oppsState *state, opps_lib_status status);
void oppsMsgSendPushBcCompleteInd(oppsState *state, opps_lib_status status);
void oppsMsgSendPullBcCompleteInd(oppsState *state, opps_lib_status status);
void oppsMsgSendPushObjStartInd(oppsState *state, Source src, uint16 nameOffset, uint16 nameLength,
				uint16 typeOffset, uint16 typeLength, uint32 totalLength, uint16 packetLen,
				uint16 packetOffset, bool moreData);
void oppsMsgSendPushBcStartInd(oppsState *state, Source src, uint16 nameOffset, uint16 nameLength,
										uint32 totalLength, uint16 packetLen, uint16 packetOffset,
										bool moreData);


#endif /* OPPS_PRIVATE_H_*/


