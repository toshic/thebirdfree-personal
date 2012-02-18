/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_private.h
    
DESCRIPTION
	Private header file for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#ifndef	GOEP_PRIVATE_H
#define GOEP_PRIVATE_H

#include <stdlib.h>
#include <panic.h>
#include <message.h>

#include "goep.h"


/*!
	@brief The supported OBEX version (1.0)
*/
#define GOEP_OBEX_VER 0x10

/*! 
	@brief The maximum length of the target header. 
*/
#define MAX_TARGET_LEN 32

/* Macros for creating messages */
#define MAKE_GOEP_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_GOEP_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);

#ifdef GOEP_LIBRARY_DEBUG
	#include <stdio.h>
	#define GOEP_DEBUG(x) {printf x;}
#else
	#define GOEP_DEBUG(x)
#endif

#define GOEP_AUTH_REQ_NONCE 0x00
#define GOEP_AUTH_REQ_OPTIONS 0x01
#define GOEP_AUTH_REQ_REALM 0x02

#define GOEP_AUTH_RES_DIGEST 0x00
#define GOEP_AUTH_RES_USERID 0x01
#define GOEP_AUTH_RES_NONCE 0x02

typedef enum
{
	goep_unknown,
	goep_initialising,
	goep_initialised,
	goep_connecting,
	goep_connected,
	goep_pushing,
	goep_pushing_last,
	goep_deleting,
	goep_pulling_first,
	goep_pulling,
	goep_remote_put,
	goep_remote_get,
	goep_aborting,
	goep_setpath,
	goep_disconnecting,
	goep_rem_disconnecting,
	goep_connect_abort,
	goep_connect_refused,
	goep_connect_auth,
	goep_connect_cancel,
    
	goep_st_end_of_list
} goep_states;

/* GEOP Session data structure */
struct __goepState
{
	TaskData		task;  
	Task		theApp;
	Sink		sink;
	goep_states	state;
	goep_states	abort_state;
	uint16		pktLen;
	uint16		srcUsed;
	
	uint8			rfcChan;
	goep_serv_role  role;
    goep_serv_class servClass;
	
	bdaddr		bdAddr;
	uint16 		conLen;
	uint8* 	    conInfo; /* connection info. */
    uint32      conID;
	uint8*		authChallenge;
	
	uint16		firstOffset;
	
	bool useHeaders;
	bool useConID;
};
typedef struct __goepState goepState;



/****************************************************************************
NAME	
    goepHandler

DESCRIPTION
    Handler for messages received by the GOEP Task.

RETURNS
    Nothing.
*/
void goepHandler(Task task, MessageId id, Message message);

void goepMsgSendConnectConfirm(goepState *goep, goep_lib_status result, uint16 maxPktLen);
void goepMsgSendDisconnectConfirm(goepState *goep, goep_lib_status result);
void goepMsgSendSetPathConfirm(goepState *goep, goep_lib_status result);
void goepMsgSendLocalPutCompleteInd(goepState *goep, goep_lib_status result);
void goepMsgSendLocalGetStartInd(goepState *goep,
								Source src,
								uint16 nameOffset, uint16 nameLength,
								uint16 typeOffset, uint16 typeLength,
								uint16 dataOffset, uint16 dataLength,
								uint32 totalLength, bool moreData);
void goepMsgSendLocalGetDataInd(goepState *goep,
								Source src, 
								uint16 dataOffset, uint16 dataLength,
								bool moreData);
void goepMsgSendLocalGetCompleteInd(goepState *goep, goep_lib_status result);

void goepMsgSendRemotePutStartInd(goepState *goep,
								Source src, 
								uint16 nameOffset, uint16 nameLength,
								uint16 typeOffset, uint16 typeLength,
								uint16 dataOffset, uint16 dataLength,
								uint32 totalLength, bool moreData);
void goepMsgSendRemotePutDataInd(goepState *goep,
								Source src, 
								uint16 dataOffset, uint16 dataLength,
								bool moreData);
void goepMsgSendRemotePutCompleteInd(goepState *goep, goep_lib_status result);
void goepMsgSendRemoteGetStartInd(goepState *goep,
								Source src,
								uint16 nameOffset, uint16 nameLength,
								uint16 typeOffset, uint16 typeLength);
void goepMsgSendRemoteGetCompleteInd(goepState *goep, goep_lib_status result);

void goepMsgSendDeleteConfirm(goepState *goep, goep_lib_status result);
void goepMsgSendAbortConfirm(goepState *goep, goep_states cmd, goep_lib_status result);

void goepMsgSendGetAppHeadersInd(goepState *goep, Sink sink, uint16 length);
void goepMsgSendLocalGetStartHdrInd(goepState *goep,
								Source src,
								uint16 nameOffset, uint16 nameLength,
								uint16 typeOffset, uint16 typeLength,
								uint16 dataOffset, uint16 dataLength,
								uint16 hdrOffset,  uint16 hdrLength,
								uint32 totalLength, bool moreData);
void goepMsgSendRemoteGetStartHdrsInd(goepState *goep,
								Source src,
								uint16 nameOffset, uint16 nameLength,
								uint16 typeOffset, uint16 typeLength,
								uint16 hdrOffset,  uint16 hdrLength);

#endif /* GOEP_PRIVATE_H */
