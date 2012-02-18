/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_messages.c
    
DESCRIPTION
	Functions to send messages to the task using this GOEP instance.

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#include <panic.h>
#include <message.h>
#include <source.h>

#include "goep.h"
#include "goep_private.h"

void goepMsgSendConnectConfirm(goepState *goep, goep_lib_status result, uint16 maxPktLen)
{
	MAKE_GOEP_MESSAGE(GOEP_CONNECT_CFM);
	message->goep = goep;
	message->status = result;
    message->maxPacketLen = goep->pktLen;
    MessageSend(goep->theApp, GOEP_CONNECT_CFM, message);
}

void goepMsgSendDisconnectConfirm(goepState *goep, goep_lib_status result)
{
	MAKE_GOEP_MESSAGE(GOEP_DISCONNECT_IND);
	message->goep = goep;
	message->status = result;
    MessageSend(goep->theApp, GOEP_DISCONNECT_IND, message);
}

void goepMsgSendSetPathConfirm(goepState *goep, goep_lib_status result)
{
	MAKE_GOEP_MESSAGE(GOEP_SET_PATH_CFM);
	message->goep = goep;
	message->status = result;
    MessageSend(goep->theApp, GOEP_SET_PATH_CFM, message);
}

void goepMsgSendLocalPutCompleteInd(goepState *goep, goep_lib_status result)
{
	MAKE_GOEP_MESSAGE(GOEP_LOCAL_PUT_COMPLETE_IND);
	message->goep = goep;
	message->status = result;
    MessageSend(goep->theApp, GOEP_LOCAL_PUT_COMPLETE_IND, message);
}

void goepMsgSendLocalGetStartInd(goepState *goep,
								Source src,
								uint16 nameOffset, uint16 nameLength,
								uint16 typeOffset, uint16 typeLength,
								uint16 dataOffset, uint16 dataLength,
								uint32 totalLength, bool moreData)
{
	MAKE_GOEP_MESSAGE(GOEP_LOCAL_GET_START_IND);
	message->goep = goep;
	message->src = src;
	message->nameOffset = nameOffset;
	message->nameLength = nameLength;
	message->typeOffset = typeOffset;
	message->typeLength = typeLength;
	message->dataOffset = dataOffset;
	message->dataLength = dataLength;
	message->totalLength = totalLength;
	message->moreData = moreData;
	
    MessageSend(goep->theApp, GOEP_LOCAL_GET_START_IND, message);
}

void goepMsgSendLocalGetStartHdrInd(goepState *goep,
								Source src,
								uint16 nameOffset, uint16 nameLength,
								uint16 typeOffset, uint16 typeLength,
								uint16 dataOffset, uint16 dataLength,
								uint16 hdrOffset,  uint16 hdrLength,
								uint32 totalLength, bool moreData)
{
	MAKE_GOEP_MESSAGE(GOEP_LOCAL_GET_START_HDRS_IND);
	message->goep = goep;
	message->src = src;
	message->nameOffset   = nameOffset;
	message->nameLength   = nameLength;
	message->typeOffset   = typeOffset;
	message->typeLength   = typeLength;
	message->dataOffset   = dataOffset;
	message->dataLength   = dataLength;
	message->headerOffset = hdrOffset;
	message->headerLength = hdrLength;
	message->totalLength  = totalLength;
	message->moreData = moreData;
	
    MessageSend(goep->theApp, GOEP_LOCAL_GET_START_HDRS_IND, message);
}

void goepMsgSendLocalGetDataInd(goepState *goep,
								Source src,
								uint16 dataOffset, uint16 dataLength,
								bool moreData)
{
	MAKE_GOEP_MESSAGE(GOEP_LOCAL_GET_DATA_IND);
	message->goep = goep;
	message->src = src;
	message->dataOffset = dataOffset;
	message->dataLength = dataLength;
	message->moreData = moreData;
	
    MessageSend(goep->theApp, GOEP_LOCAL_GET_DATA_IND, message);
}

void goepMsgSendLocalGetCompleteInd(goepState *goep, goep_lib_status result)
{
	MAKE_GOEP_MESSAGE(GOEP_LOCAL_GET_COMPLETE_IND);
	message->goep = goep;
	message->status = result;
    MessageSend(goep->theApp, GOEP_LOCAL_GET_COMPLETE_IND, message);
}

void goepMsgSendRemotePutStartInd(goepState *goep,
								Source src, 
								uint16 nameOffset, uint16 nameLength,
								uint16 typeOffset, uint16 typeLength,
								uint16 dataOffset, uint16 dataLength,
								uint32 totalLength, bool moreData)
{
	MAKE_GOEP_MESSAGE(GOEP_REMOTE_PUT_START_IND);
	message->goep = goep;
	message->src = src;
	message->nameOffset = nameOffset;
	message->nameLength = nameLength;
	message->typeOffset = typeOffset;
	message->typeLength = typeLength;
	message->dataOffset = dataOffset;
	message->dataLength = dataLength;
	message->totalLength = totalLength;
	message->moreData = moreData;
	
    MessageSend(goep->theApp, GOEP_REMOTE_PUT_START_IND, message);
}

void goepMsgSendRemotePutDataInd(goepState *goep,
								Source src,
								uint16 dataOffset, uint16 dataLength,
								bool moreData)
{
	MAKE_GOEP_MESSAGE(GOEP_REMOTE_PUT_DATA_IND);
	message->goep = goep;
	message->src = src;
	message->dataOffset = dataOffset;
	message->dataLength = dataLength;
	message->moreData = moreData;
	
    MessageSend(goep->theApp, GOEP_REMOTE_PUT_DATA_IND, message);
}

void goepMsgSendRemotePutCompleteInd(goepState *goep, goep_lib_status result)
{
	MAKE_GOEP_MESSAGE(GOEP_REMOTE_PUT_COMPLETE_IND);
	message->goep = goep;
	message->status = result;
    MessageSend(goep->theApp, GOEP_REMOTE_PUT_COMPLETE_IND, message);
}

void goepMsgSendRemoteGetStartInd(goepState *goep,
								Source src,
								uint16 nameOffset, uint16 nameLength,
								uint16 typeOffset, uint16 typeLength)
{
	MAKE_GOEP_MESSAGE(GOEP_REMOTE_GET_START_IND);
	message->goep = goep;
	message->src=src;
	message->nameOffset = nameOffset;
	message->nameLength = nameLength;
	message->typeOffset = typeOffset;
	message->typeLength = typeLength;
	MessageSend(goep->theApp, GOEP_REMOTE_GET_START_IND, message);
}

void goepMsgSendRemoteGetStartHdrsInd(goepState *goep,
								Source src,
								uint16 nameOffset, uint16 nameLength,
								uint16 typeOffset, uint16 typeLength,
								uint16 hdrOffset,  uint16 hdrLength)
{
	MAKE_GOEP_MESSAGE(GOEP_REMOTE_GET_START_HDRS_IND);
	message->goep = goep;
	message->src=src;
	message->nameOffset = nameOffset;
	message->nameLength = nameLength;
	message->typeOffset = typeOffset;
	message->typeLength = typeLength;
	message->headerOffset = hdrOffset;
	message->headerLength = hdrLength;
	MessageSend(goep->theApp, GOEP_REMOTE_GET_START_HDRS_IND, message);
}

void goepMsgSendRemoteGetCompleteInd(goepState *goep, goep_lib_status result)
{
	MAKE_GOEP_MESSAGE(GOEP_REMOTE_GET_COMPLETE_IND);
	message->goep = goep;
	message->status = result;
    MessageSend(goep->theApp, GOEP_REMOTE_GET_COMPLETE_IND, message);
}

void goepMsgSendDeleteConfirm(goepState *goep, goep_lib_status result)
{
	MAKE_GOEP_MESSAGE(GOEP_DELETE_CFM);
	message->goep = goep;
	message->status = result;
    MessageSend(goep->theApp, GOEP_DELETE_CFM, message);
}

void goepMsgSendAbortConfirm(goepState *goep, goep_states cmd, goep_lib_status result)
{
	switch (cmd)
	{
	case goep_pushing:
    case goep_pushing_last:
		goepMsgSendLocalPutCompleteInd(goep, result);
		break;
    case goep_pulling_first:  /* Deliberate Fallthrough */
	case goep_pulling:
		goepMsgSendLocalGetCompleteInd(goep, result);
		break;
	case goep_remote_put:
		goepMsgSendRemotePutCompleteInd(goep, result);
		break;
	case goep_remote_get:
		goepMsgSendRemoteGetCompleteInd(goep, result);
		break;
	default:
		break;
	}
}

void goepMsgSendGetAppHeadersInd(goepState *goep, Sink sink, uint16 length)
{
	MAKE_GOEP_MESSAGE(GOEP_GET_APP_HEADERS_IND);
	message->goep = goep;
    message->sink = sink;
    message->length = length;
    MessageSend(goep->theApp, GOEP_GET_APP_HEADERS_IND, message);
}
