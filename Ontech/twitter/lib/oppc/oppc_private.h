/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    OPP_private.h
    
DESCRIPTION
	Private header for the OPP Client library

*/

#ifndef	OPPC_PRIVATE_H_
#define	OPPC_PRIVATE_H_

#include <stdlib.h>
#include <panic.h>
#include <message.h>

#include "goep.h"

/* Macros for creating messages */
#define MAKE_OPPC_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);

#define OPPC_NO_PAYLOAD      (0x0)

#ifdef OPPC_LIBRARY_DEBUG
	#include <panic.h>
	#include <stdio.h>
	#define OPPC_DEBUG(x) {printf x; Panic();}
#else
	#define OPPC_DEBUG(x)
#endif

static const uint8 vcardType[] = {'t','e','x','t','/','x','-','v','c','a','r','d'};

/* Currently running FTPC Command */
typedef enum
{
    oppc_com_None,
	oppc_com_Connect,
	oppc_com_Disconnect,
	oppc_com_PushObject,
	oppc_com_PullVCard,
			
	oppc_command_end_of_list
} oppc_running_command;


/* Internal state structures */
struct __oppcState
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
    /* Currently running FTPC command */
    oppc_running_command currCom;
    
    /* Goep Handle */
    GOEP *handle;
	/* Server channel */
	uint8 rfcChan;
};

typedef struct __oppcState oppcState;





/* OPP Client Library private messages */
#define OPPC_INTERNAL_MESSAGE_BASE 0x00

enum
{
	/* Session Control */
	OPPC_INT_CONNECT = OPPC_INTERNAL_MESSAGE_BASE,
	OPPC_INT_AUTH_RESP,
	OPPC_INT_DISCONNECT,
	
	OPPC_INT_ABORT,  /* Might not need this one */
	
    OPPC_INT_PUSH_OBJECT,	
    OPPC_INT_PUSH_NEXT_PACKET,
    OPPC_INT_PUSH_OBJECT_SRC,	
    OPPC_INT_PUSH_NEXT_PACKET_SRC,
	
	OPPC_INT_PULL_VCARD,
	OPPC_INT_PULL_NEXT_VCARD,
    
    OPPC_INT_ENDOFLIST
};

/* Internal Message Structures */

typedef struct
{
    bdaddr	bdAddr;
    uint16	maxPacketSize;    
} OPPC_INT_CONNECT_T;

typedef struct
{
	const uint8 *digest;
	uint16 size_userid;
	const uint8 *userid;
	const uint8 *nonce;
} OPPC_INT_AUTH_RESP_T;

typedef struct
{
	uint16 descLen;
    const uint8 *desc;
} OPPC_INT_ABORT_T;

typedef struct
{
    uint16 nameLen;
    const uint8* name;
    uint16 typeLen;
    const uint8* type;
	uint16 length;
    const uint8 *packet;
    uint32 totalLen;
    bool onlyPacket;
} OPPC_INT_PUSH_OBJECT_T;

typedef struct
{
	uint16 length;
    const uint8 *packet;
    bool lastPacket;
} OPPC_INT_PUSH_NEXT_PACKET_T;

typedef struct
{
    uint16 nameLen;
    const uint8* name;
    uint16 typeLen;
    const uint8* type;
	uint16 length;
    Source src;
    uint32 totalLen;
    bool onlyPacket;
} OPPC_INT_PUSH_OBJECT_SRC_T;

typedef struct
{
	uint16 length;
    Source src;
    bool lastPacket;
} OPPC_INT_PUSH_NEXT_PACKET_SRC_T;


/****************************************************************************
NAME	
    oppcGetTheTask

DESCRIPTION
    This function returns the OPPC library task so that the OPPC library can use it.

RETURNS
    The FTPC library task.
*/
Task oppcGetTheTask(void);

/****************************************************************************
NAME	
    oppcHandler

DESCRIPTION
    Handler for messages received by the OPPC Task.

RETURNS
    Nothing.
*/
void oppcHandler(Task task, MessageId id, Message message);

void oppcMsgSendConnectCfm(oppcState *state, oppc_lib_status status, uint16 pktSize);
void oppcMsgSendDisConnectCfm(oppcState *state, oppc_lib_status status);
void oppcMsgPullBCCompleteInd(oppcState *state, oppc_lib_status status);
void oppcMsgPushCompleteInd(oppcState *state, oppc_lib_status status);


#endif /* OPPC_PRIVATE_H_*/


