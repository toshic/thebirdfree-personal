/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps_private.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library - private header.

*/

#ifndef	PBAPS_PRIVATE_H_
#define	PBAPS_PRIVATE_H_

#include <stdlib.h>
#include <panic.h>
#include <message.h>
#include <pbap_common.h>

#include <goep.h>

/* Macros for creating messages */
#define MAKE_PBAPS_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define PBAPS_SEND_IND(SESSION,TYPE) {TYPE##_T *message = PanicUnlessNew(TYPE##_T);message->pbaps=(PBAPS*)(SESSION);MessageSend((SESSION)->theAppTask, (TYPE), message);}

#define PBAPS_NO_PAYLOAD      (0x0)

#ifdef PBAPS_LIBRARY_DEBUG
	#include <panic.h>
	#include <stdio.h>
	#define PBAPS_DEBUG(x) {printf x; Panic();}
	#define PBAPS_ASSERT(c, x) { if (!(c)) { printf x; Panic();} }
#else
	#define PBAPS_DEBUG(x)
	#define PBAPS_ASSERT(c, x)
#endif

/* Currently running PBAP Server Command */
typedef enum
{
    pbaps_com_None,
    pbaps_com_Connect,
    pbaps_com_ObexPassword,
    pbaps_com_Disconnect,
	
	pbaps_com_Connecting,
	pbaps_com_SetPhonebook,
	
	pbaps_com_PullvCardList,
	pbaps_com_PullvCard,
	pbaps_com_PullPhonebook,
	
	pbaps_com_EOL
} pbaps_running_command;

/* Internal state structures */
struct __pbapsState
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
    pbaps_running_command currCom;
    
    /* Goep Handle */
    GOEP *handle;
	/* Server channel */
	unsigned int rfcChan:8;
	/* Repositories supported by the server */
	unsigned int repos:8;
	
	/* SDP Handle */
	uint32 sdpHandle;
	
	/* Store of App. Specific Parameters */
	uint16 pbook_size;
	uint8 new_missed;
};

typedef struct __pbapsState pbapsState;







enum
{
	/* Session Control */
	PBAPS_INT_CONNECTION_RESP,
	PBAPS_INT_AUTH_CHALLENGE,
			
	PBAPS_INT_SET_PB_ROOT_RESP,
	PBAPS_INT_SET_PB_REPOSITORY_RESP,
	PBAPS_INT_SET_PB_BOOK_RESP,
	PBAPS_INT_SET_PB_PARENT_RESP,
	
	PBAPS_GET_VCARD_LIST_FIRST,
	PBAPS_GET_VCARD_ENTRY_FIRST,
	PBAPS_GET_PHONEBOOK_FIRST,
	
	PBAPS_GET_SEND_FIRST_SRC,
	
	PBAPS_GET_SEND_NEXT,
	
	PBAPS_GET_SEND_NEXT_SRC,
	
    PBAPS_INT_ENDOFLIST
};

/* Internal Message Structures */
typedef struct
{
	bool accept;
	uint16 pktSize;
} PBAPS_INT_CONNECTION_RESP_T;

typedef struct
{
	const uint8 *nonce;
	uint8 options;
	uint16 size_realm;
	const uint8 *realm;
} PBAPS_INT_AUTH_CHALLENGE_T;

typedef struct
{
	pbaps_set_phonebook_result result;
} PBAPS_INT_SET_PB_ROOT_RESP_T;

typedef struct
{
	pbaps_set_phonebook_result result;
} PBAPS_INT_SET_PB_REPOSITORY_RESP_T;

typedef struct
{
	pbaps_set_phonebook_result result;
} PBAPS_INT_SET_PB_BOOK_RESP_T;

typedef struct
{
	pbaps_set_phonebook_result result;
} PBAPS_INT_SET_PB_PARENT_RESP_T;

typedef struct
{
	pbaps_get_result result;
	uint8 new_missed;
	uint16 pbook_size;
	uint32 totalLen;
	uint8 *packet;
	uint16 size_packet;
	bool onlyPacket;
} PBAPS_GET_VCARD_LIST_FIRST_T;

typedef struct
{
	pbaps_get_result result;
	uint32 totalLen;
	uint8 *packet;
	uint16 size_packet;
	bool onlyPacket;
} PBAPS_GET_VCARD_ENTRY_FIRST_T;

typedef struct
{
	pbaps_get_result result;
	uint8 new_missed;
	uint16 pbook_size;
	uint32 totalLen;
	uint8 *packet;
	uint16 size_packet;
	bool onlyPacket;
} PBAPS_GET_PHONEBOOK_FIRST_T;

typedef struct
{
	pbaps_running_command command;
	pbaps_get_result result;
	uint16 totalLen;
	Source src;
	uint16 size_packet;
	bool onlyPacket;
} PBAPS_GET_SEND_FIRST_SRC_T;

typedef struct
{
	pbaps_running_command command;
	Source src;
	uint16 size_packet;
	bool lastPacket;
} PBAPS_GET_SEND_NEXT_SRC_T;

typedef struct
{
	pbaps_running_command command;
	uint16 size_packet;
	const uint8 *packet;
	bool lastPacket;
} PBAPS_GET_SEND_NEXT_T;


/****************************************************************************
NAME	
    pbapsIntHandler

DESCRIPTION
    Handler for messages received by the PBABS Task.
*/
void pbapsIntHandler(Task task, MessageId id, Message message);


/****************************************************************************
NAME	
    pbabsGoepHandler

DESCRIPTION
    Handler for messages received by the PBABS Task from GOEP.
*/
void pbapsGoepHandler(pbapsState *state, MessageId id, Message message);

void pbapsMsgSendInitCfm(pbapsState *state, pbaps_lib_status status);
void pbapsMsgSendConnectCfm(pbapsState *state, pbaps_lib_status status, uint16 pktSize);
void pbapsMsgSendGetvCardListStartInd(pbapsState *state, pbaps_lib_status status,
										pbap_order_values order, uint8 *srchVal, uint16 size_srchVal,
										pbap_phone_book pbook,
										pbap_search_values srchAttr, uint16 maxList, uint16 listStart);
void pbapsMsgSendGetvCardListCompleteInd(pbapsState *state, pbaps_lib_status status);
void pbapsMsgSendGetvCardEntryStartInd(pbapsState *state, uint16 entry, pbap_format_values format, uint32 filter_lo, uint32 filter_hi);
void pbapsMsgSendGetvCardEntryCompleteInd(pbapsState *state, pbaps_lib_status status);
void pbapsMsgSendGetPhonebookStartInd(pbapsState *state, pbap_phone_repository repository, pbap_phone_book phonebook, 
									  	pbap_format_values format, uint32 filter_lo,uint32 filter_hi, 
										uint16 maxList, uint16 listStart);
void pbapsMsgSendGetPhonebookCompleteInd(pbapsState *state, pbaps_lib_status status);

void pbapsMsgSendGetCompleteInd(pbapsState *state, pbaps_running_command command, pbaps_lib_status status);

void pbapsExtractvCardListingParameters(pbapsState *state, GOEP_REMOTE_GET_START_HDRS_IND_T *msg);
void pbapsExtractvCardEntryParameters(pbapsState *state, GOEP_REMOTE_GET_START_HDRS_IND_T *msg);
bool pbapsExtractPhonebookParameters(pbapsState *state, GOEP_REMOTE_GET_START_HDRS_IND_T *msg);
uint16 pbapsAddApplicationHeaders(pbapsState *state, Sink sink);
uint16 pbapsBuildApplicationHeaders(pbapsState *state, uint8 *appHeader);

uint16 pbapsExtractvCardEntry(const uint8 *src);


#endif /* PBAPS_PRIVATE_H_ */

